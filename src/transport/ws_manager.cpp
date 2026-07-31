//=============================================================================
// ws_manager.cpp - High-level WebSocket orchestration implementation
//=============================================================================
// Part of Hyperliquid Plugin for Zorro
//
// LAYER: Transport
// NOTE: Account data parsing delegated to ws_parsers.cpp
//=============================================================================

#include "ws_manager.h"
#include "ws_parsers.h"
#include "json_helpers.h"
#include "../foundation/hl_globals.h"
#include "../foundation/hl_protocol.h"
#include <IXNetSystem.h>
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <algorithm>

namespace hl {
namespace ws {

// --- Construction / Destruction ---

WebSocketManager::WebSocketManager(PriceCache& cache)
    : cache_(cache), connectionThread_(NULL),
      shutdownEvent_(NULL), running_(false), testnet_(false),
      zorroWindow_(NULL), diagLevel_(0), logCallback_(nullptr),
      orderUpdateCallback_(nullptr), fillNotifyCallback_(nullptr),
      subscribedUserFills_(false), subscribedClearinghouse_(false),
      subscribedOpenOrders_(false), pendingUserFillsSub_(false),
      pendingClearinghouseSub_(false), pendingOpenOrdersSub_(false),
      initialSubsQueued_(false),
      nextRequestId_(1000),
      consecutiveReconnects_(0), circuitOpen_(false), circuitOpenedAt_(0) {
    ix::initNetSystem();  // WSAStartup (ref-counted, safe to call multiple times) [OPM-127]
    InitializeCriticalSection(&l2SubCs_);
    InitializeCriticalSection(&accountSubCs_);
    InitializeCriticalSection(&postCs_);
    InitializeCriticalSection(&responseCs_);
    InitializeCriticalSection(&indexMapCs_);
    // [OPM-550] Phase 1.6 — zero per-channel timing stats
    for (int i = 0; i < CK_COUNT; ++i) {
        channelStats_[i].totalMs = 0;
        channelStats_[i].maxMs = 0;
        channelStats_[i].count = 0;
    }
}

WebSocketManager::~WebSocketManager() {
    stop();
    DeleteCriticalSection(&l2SubCs_);
    DeleteCriticalSection(&accountSubCs_);
    DeleteCriticalSection(&postCs_);
    DeleteCriticalSection(&responseCs_);
    DeleteCriticalSection(&indexMapCs_);
    ix::uninitNetSystem();  // WSACleanup (ref-counted) [OPM-127]
}

// --- Logging ---

void WebSocketManager::setDiagLevel(int level) {
    diagLevel_ = level;
    connection_.setLogCallback(logCallback_, level);
}

void WebSocketManager::setLogCallback(LogCallback cb) {
    logCallback_ = cb;
    connection_.setLogCallback(cb, diagLevel_);
}

void WebSocketManager::log(int minLevel, const char* msg) {
    if (logCallback_ && diagLevel_ >= minLevel) logCallback_(msg);
}

void WebSocketManager::logf(int minLevel, const char* fmt, ...) {
    if (logCallback_ && diagLevel_ >= minLevel) {
        char buf[512];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);
        logCallback_(buf);
    }
}

// --- Lifecycle ---

void WebSocketManager::start(const std::string& hostname, bool testnet) {
    if (running_) return;

    hostname_ = hostname;
    testnet_ = testnet;
    running_ = true;

    shutdownEvent_ = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!shutdownEvent_) {
        log(1, "WS: Failed to create shutdown event");
        running_ = false;
        return;
    }

    connection_.setMessageHandler([this](const char* data, size_t len) {
        handleMessage(data, len);
    });

    // Enable IXWebSocket auto-reconnect — handles backoff internally [OPM-128]
    connection_.enableAutoReconnect(1000, 30000);

    connectionThread_ = CreateThread(NULL, 0, ConnectionThreadProc, this, 0, NULL);
    if (!connectionThread_) {
        log(1, "WS: Failed to create connection thread");
        CloseHandle(shutdownEvent_); shutdownEvent_ = NULL;
        running_ = false;
        return;
    }

    log(1, "WS: Manager started");
}

void WebSocketManager::stop() {
    if (!running_) return;

    log(1, "WS: Manager stopping...");
    running_ = false;

    // Suppress logging BEFORE blocking operations to prevent deadlock [OPM-133].
    // Both connectionThread_ and IXWebSocket thread call logCallback_ →
    // BrokerMessage → SendMessage to GUI. If the main (GUI) thread is blocked
    // in ws_.stop() or WaitForSingleObject, SendMessage deadlocks.
    logCallback_ = nullptr;

    if (shutdownEvent_) SetEvent(shutdownEvent_);

    // Disconnect to unblock any pending operations [OPM-16]
    connection_.disconnect();

    if (connectionThread_) {
        DWORD result = WaitForSingleObject(connectionThread_, 3000);
        CloseHandle(connectionThread_);
        connectionThread_ = NULL;
    }
    if (shutdownEvent_) {
        CloseHandle(shutdownEvent_);
        shutdownEvent_ = NULL;
    }
}

bool WebSocketManager::isHealthy() const {
    if (!connection_.isConnected()) return false;
    time_t lastMsg = connection_.lastMessageTime();
    return (time(NULL) - lastMsg) < 60;
}

int WebSocketManager::getSecondsSinceLastMessage() const {
    return static_cast<int>(time(NULL) - connection_.lastMessageTime());
}

// --- Threads ---

DWORD WINAPI WebSocketManager::ConnectionThreadProc(LPVOID param) {
    static_cast<WebSocketManager*>(param)->connectionLoop();
    return 0;
}

// connectionLoop() — simplified with IXWebSocket auto-reconnect [OPM-128]
//
// IXWebSocket handles protocol-level ping/pong and automatic reconnection
// with exponential backoff. This loop only needs to:
//   1. Initiate the first connection
//   2. Drain messages via poll()
//   3. Re-subscribe channels after auto-reconnect
//   4. Send pending work (subscriptions, posts)
//   5. Send periodic HL application pings (30s)
void WebSocketManager::connectionLoop() {
    const char* host = testnet_ ?
        "api.hyperliquid-testnet.xyz" : "api.hyperliquid.xyz";

    // Initial connection — auto-reconnect handles subsequent retries
    if (connection_.connect(host, true, "/ws", 30000)) {
        subscribeInitialChannels();
    } else {
        log(1, "WS: Initial connection failed (auto-reconnect will retry)");
    }

    DWORD lastHlPingTick = GetTickCount();
    const DWORD HL_PING_INTERVAL_MS = 30000;  // 30s — protocol pings keep connection alive
    const int MAX_CONSECUTIVE_RECONNECTS = 15;
    const DWORD CIRCUIT_COOLDOWN_MS = 300000;  // 5 minutes

    // [OPM-550] Diagnostic instrumentation — runtime-gated on diagLevel_ >= 3.
    // Off by default (zero overhead). Operator can re-enable mid-run via
    // SET_DIAGNOSTICS=3 to bring full Phase 1/1.5/1.6 telemetry back online.
    DWORD lastDiagTick = GetTickCount();
    DWORD prevTickTime = lastDiagTick;
    DWORD loopTickCount = 0;
    DWORD longestTickIntervalMs = 0;
    uint64_t lastIxMsgSnapshot = 0;
    uint64_t lastPollDispatchSnapshot = 0;
    const DWORD DIAG_INTERVAL_MS = 60000;  // 1 minute
    DWORD maxPhaseMs = 0;
    const char* maxPhaseName = "none";
    bool prevDiagOn = false;

    while (running_) {
        if (WaitForSingleObject(shutdownEvent_, 0) == WAIT_OBJECT_0) break;

        // [OPM-550] Re-check gate per iteration so SET_DIAGNOSTICS takes effect live.
        const bool diagOn = (diagLevel_ >= 3);
        if (diagOn && !prevDiagOn) {
            // Transition off→on: reset snapshots so the first window starts clean.
            prevTickTime = GetTickCount();
            lastDiagTick = prevTickTime;
            loopTickCount = 0;
            longestTickIntervalMs = 0;
            maxPhaseMs = 0;
            maxPhaseName = "none";
            lastIxMsgSnapshot = connection_.getIxMessageCount();
            lastPollDispatchSnapshot = connection_.getPollDispatchCount();
            cache_.resetWsHotPathStats();
        }
        prevDiagOn = diagOn;

        DWORD phaseStart = diagOn ? GetTickCount() : 0;
        DWORD phaseEnd;
        DWORD phaseMs;
        #define HL550_PHASE(name) do {                                    \
            if (diagOn) {                                                 \
                phaseEnd = GetTickCount();                                \
                phaseMs = phaseEnd - phaseStart;                          \
                if (phaseMs > maxPhaseMs) {                               \
                    maxPhaseMs = phaseMs;                                 \
                    maxPhaseName = name;                                  \
                }                                                         \
                phaseStart = phaseEnd;                                    \
            }                                                             \
        } while (0)

        // Check if IXWebSocket auto-reconnected [OPM-128]
        if (connection_.wasReconnected()) {
            consecutiveReconnects_++;

            if (consecutiveReconnects_ > MAX_CONSECUTIVE_RECONNECTS) {
                logf(1, "WS: Circuit breaker OPEN — %d consecutive reconnects, "
                     "pausing for %ds",
                     consecutiveReconnects_, CIRCUIT_COOLDOWN_MS / 1000);
                connection_.stopAutoReconnect();
                circuitOpen_ = true;
                circuitOpenedAt_ = GetTickCount();
            } else {
                log(1, "WS: Re-subscribing after auto-reconnect");
                requeueSubscriptionsAfterReconnect();
                subscribeInitialChannels();
            }
        }
        HL550_PHASE("reconnect");

        // Send pending work (only if connected)
        if (connection_.isConnected()) {
            sendPendingPosts();
            HL550_PHASE("sendPosts");

            if (initialSubsQueued_) {
                sendPendingL2Subscriptions();
                HL550_PHASE("sendL2Subs");

                sendPendingAccountSubscriptions();
                HL550_PHASE("sendAcctSubs");
            }

            // HL application ping at reduced frequency [OPM-128]
            // Protocol-level pings (IXWebSocket) keep the connection alive.
            // HL app pings keep the subscription channels active.
            DWORD now = GetTickCount();
            if (now - lastHlPingTick >= HL_PING_INTERVAL_MS) {
                connection_.send("{\"method\":\"ping\"}");
                lastHlPingTick = now;
            }
            HL550_PHASE("hlPing");
        }

        // Circuit breaker cooldown — probe reconnect
        if (circuitOpen_) {
            DWORD elapsed = GetTickCount() - circuitOpenedAt_;
            if (elapsed >= CIRCUIT_COOLDOWN_MS) {
                log(1, "WS: Circuit breaker probe — attempting reconnect");
                circuitOpen_ = false;
                consecutiveReconnects_ = 0;
                connection_.enableAutoReconnect(1000, 30000);
                connection_.connect(host, true, "/ws", 30000);
                // wasReconnected() will fire next iteration → normal re-subscribe
            }
        }

        // Reset reconnect counter when data flows (connection is healthy)
        if (connection_.isConnected() && consecutiveReconnects_ > 0) {
            time_t lastMsg = connection_.lastMessageTime();
            if (lastMsg > 0 && (time(NULL) - lastMsg) < 10) {
                logf(2, "WS: Reconnect counter reset (was %d)",
                     consecutiveReconnects_);
                consecutiveReconnects_ = 0;
            }
        }
        HL550_PHASE("circuitMisc");

        // Drain messages from IXWebSocket queue
        connection_.poll(100);
        HL550_PHASE("poll");

        Sleep(10);
        HL550_PHASE("sleep");

        #undef HL550_PHASE

        if (!diagOn) continue;  // [OPM-550] Skip diag bookkeeping when gated off.

        // [OPM-550] H1 instrumentation: track loop cadence + emit per-minute summary
        DWORD nowTick = GetTickCount();
        DWORD interval = nowTick - prevTickTime;
        if (interval > longestTickIntervalMs) longestTickIntervalMs = interval;
        prevTickTime = nowTick;
        loopTickCount++;

        if (nowTick - lastDiagTick >= DIAG_INTERVAL_MS) {
            uint64_t ixMsgNow = connection_.getIxMessageCount();
            uint64_t pollNow = connection_.getPollDispatchCount();
            uint64_t ixDelta = ixMsgNow - lastIxMsgSnapshot;
            uint64_t pollDelta = pollNow - lastPollDispatchSnapshot;
            DWORD csMaxWait = cache_.getLongestSetBidAskWaitMs();
            uint64_t csLongWaits = cache_.getSetBidAskLongWaitCount();

            logf(1,
                 "WS DIAG ticks=%lu longestTickGap=%lums "
                 "ixMsg=+%llu pollDisp=+%llu queueLag=%lld "
                 "cacheCsMaxWait=%lums longCsWaits=%llu "
                 "maxPhase=%s/%lums",
                 (unsigned long)loopTickCount,
                 (unsigned long)longestTickIntervalMs,
                 (unsigned long long)ixDelta,
                 (unsigned long long)pollDelta,
                 (long long)(ixMsgNow - pollNow),
                 (unsigned long)csMaxWait,
                 (unsigned long long)csLongWaits,
                 maxPhaseName,
                 (unsigned long)maxPhaseMs);

            // [OPM-550] Phase 1.6 — per-channel handler timing breakdown
            for (int i = 0; i < CK_COUNT; ++i) {
                if (channelStats_[i].count > 0) {
                    logf(1,
                         "WS DIAG   ch=%-12s count=%llu totalMs=%llu "
                         "maxMs=%lu avgMs=%.2f",
                         channelKindName(i),
                         (unsigned long long)channelStats_[i].count,
                         (unsigned long long)channelStats_[i].totalMs,
                         (unsigned long)channelStats_[i].maxMs,
                         channelStats_[i].count > 0
                             ? (double)channelStats_[i].totalMs / (double)channelStats_[i].count
                             : 0.0);
                }
                channelStats_[i].totalMs = 0;
                channelStats_[i].maxMs = 0;
                channelStats_[i].count = 0;
            }

            lastDiagTick = nowTick;
            lastIxMsgSnapshot = ixMsgNow;
            lastPollDispatchSnapshot = pollNow;
            loopTickCount = 0;
            longestTickIntervalMs = 0;
            maxPhaseMs = 0;
            maxPhaseName = "none";
            cache_.resetWsHotPathStats();
        }
    }

    log(1, "WS: Connection loop exited");
}

// --- Initial Channel Setup ---

void WebSocketManager::subscribeInitialChannels() {
    // Subscribe to orderUpdates if user address is set
    if (!userAddress_.empty()) {
        char sub[512];
        sprintf_s(sub, "{\"method\":\"subscribe\",\"subscription\":"
                 "{\"type\":\"orderUpdates\",\"user\":\"%s\"}}",
                 userAddress_.c_str());
        connection_.send(sub);
    }
}

// --- Subscriptions ---

void WebSocketManager::subscribeL2Book(const std::string& coin) {
    // Reject coins that were banned for causing disconnects [OPM-170]
    if (bannedL2Coins_.count(coin)) {
        logf(1, "WS: Rejecting l2Book subscription for banned coin '%s'", coin.c_str());
        return;
    }

    EnterCriticalSection(&l2SubCs_);
    // Check if already subscribed or pending
    for (const auto& c : l2Subscriptions_) if (c == coin) { LeaveCriticalSection(&l2SubCs_); return; }
    for (const auto& c : pendingL2Subs_) if (c == coin) { LeaveCriticalSection(&l2SubCs_); return; }

    // Try immediate send if connected [OPM-142]
    if (connection_.isConnected()) {
        l2Subscriptions_.push_back(coin);
        LeaveCriticalSection(&l2SubCs_);

        char sub[256];
        sprintf_s(sub, "{\"method\":\"subscribe\",\"subscription\":"
                 "{\"type\":\"l2Book\",\"coin\":\"%s\"}}", coin.c_str());

        if (diagLevel_ >= 2)
            logf(2, "WS: Subscribe l2Book (immediate): %s", coin.c_str());

        if (!connection_.send(sub)) {
            // Send failed — move back to pending for retry on next iteration
            EnterCriticalSection(&l2SubCs_);
            auto it = std::find(l2Subscriptions_.begin(), l2Subscriptions_.end(), coin);
            if (it != l2Subscriptions_.end()) {
                l2Subscriptions_.erase(it);
            }
            pendingL2Subs_.push_back(coin);
            LeaveCriticalSection(&l2SubCs_);
            logf(1, "WS: Failed to send l2Book subscription for %s, queuing for retry (%lds since WS open)", coin.c_str(), connection_.connectedAt() > 0 ? (long)(time(NULL) - connection_.connectedAt()) : -1L);
        }
    } else {
        // Not connected — queue for later [OPM-142]
        pendingL2Subs_.push_back(coin);
        LeaveCriticalSection(&l2SubCs_);
        if (diagLevel_ >= 2)
            logf(2, "WS: Subscribe l2Book (queued, not connected): %s", coin.c_str());
    }
}

bool WebSocketManager::hasL2BookData(const std::string& coin) {
    return cache_.getBid(coin) > 0 && cache_.getAsk(coin) > 0;
}

void WebSocketManager::subscribeUserFills() {
    if (userAddress_.empty()) return;
    EnterCriticalSection(&accountSubCs_);
    if (!subscribedUserFills_ && !pendingUserFillsSub_) pendingUserFillsSub_ = true;
    LeaveCriticalSection(&accountSubCs_);
}

void WebSocketManager::subscribeClearinghouseState() {
    if (userAddress_.empty()) return;
    EnterCriticalSection(&accountSubCs_);
    if (!subscribedClearinghouse_ && !pendingClearinghouseSub_) pendingClearinghouseSub_ = true;
    LeaveCriticalSection(&accountSubCs_);
}

void WebSocketManager::subscribeClearinghouseStateDex(const std::string& dex) {
    if (userAddress_.empty() || dex.empty()) return;
    EnterCriticalSection(&accountSubCs_);
    // Already subscribed or pending?
    if (subscribedClearinghouseDexes_.count(dex)) {
        LeaveCriticalSection(&accountSubCs_);
        return;
    }
    for (const auto& d : pendingClearinghouseDexSubs_) {
        if (d == dex) { LeaveCriticalSection(&accountSubCs_); return; }
    }
    pendingClearinghouseDexSubs_.push_back(dex);
    LeaveCriticalSection(&accountSubCs_);

    if (diagLevel_ >= 1)
        logf(1, "WS: Queued clearinghouseState subscription for dex=%s", dex.c_str());
}

void WebSocketManager::subscribeOpenOrders() {
    if (userAddress_.empty()) return;
    EnterCriticalSection(&accountSubCs_);
    if (!subscribedOpenOrders_ && !pendingOpenOrdersSub_) pendingOpenOrdersSub_ = true;
    LeaveCriticalSection(&accountSubCs_);
}

void WebSocketManager::subscribeAllAccountData() {
    subscribeUserFills();
    subscribeClearinghouseState();
    subscribeOpenOrders();
}

void WebSocketManager::sendPendingL2Subscriptions() {
    EnterCriticalSection(&l2SubCs_);
    auto toSend = std::move(pendingL2Subs_);
    pendingL2Subs_.clear();
    LeaveCriticalSection(&l2SubCs_);

    if (toSend.empty()) return;
    if (diagLevel_ >= 2)
        logf(2, "WS: Sending %d l2Book subscriptions", (int)toSend.size());

    size_t sent = 0;
    for (size_t i = 0; i < toSend.size(); ++i) {
        char sub[256];
        sprintf_s(sub, "{\"method\":\"subscribe\",\"subscription\":"
                 "{\"type\":\"l2Book\",\"coin\":\"%s\"}}", toSend[i].c_str());
        if (diagLevel_ >= 2) logf(2, "WS: Subscribe l2Book: %s", toSend[i].c_str());
        if (!connection_.send(sub)) {
            logf(1, "WS: Failed to send l2Book subscription for %s (drainer, %lds since WS open)", toSend[i].c_str(), connection_.connectedAt() > 0 ? (long)(time(NULL) - connection_.connectedAt()) : -1L);
            // Re-queue unsent coins for retry on next iteration [OPM-99]
            EnterCriticalSection(&l2SubCs_);
            for (size_t j = i; j < toSend.size(); ++j)
                pendingL2Subs_.push_back(toSend[j]);
            LeaveCriticalSection(&l2SubCs_);
            break;
        }
        ++sent;
    }

    // Only mark successfully-sent coins as active [OPM-99]
    if (sent > 0) {
        EnterCriticalSection(&l2SubCs_);
        for (size_t i = 0; i < sent; ++i)
            l2Subscriptions_.push_back(toSend[i]);
        LeaveCriticalSection(&l2SubCs_);
    }
}

void WebSocketManager::sendPendingAccountSubscriptions() {
    EnterCriticalSection(&accountSubCs_);
    bool sendFills = pendingUserFillsSub_;
    bool sendClearing = pendingClearinghouseSub_;
    bool sendOrders = pendingOpenOrdersSub_;
    pendingUserFillsSub_ = pendingClearinghouseSub_ = pendingOpenOrdersSub_ = false;
    // [OPM-218] Snapshot perpDex pending subs under same lock
    std::vector<std::string> dexSubs;
    dexSubs.swap(pendingClearinghouseDexSubs_);
    LeaveCriticalSection(&accountSubCs_);

    if ((sendFills || sendClearing || sendOrders) && diagLevel_ >= 2) {
        logf(2, "WS: Sending account subs (fills=%d clearing=%d orders=%d)",
             sendFills, sendClearing, sendOrders);
    }

    if (sendFills) {
        char sub[512];
        sprintf_s(sub, "{\"method\":\"subscribe\",\"subscription\":"
                 "{\"type\":\"userFills\",\"user\":\"%s\"}}", userAddress_.c_str());
        if (connection_.send(sub)) subscribedUserFills_ = true;
    }
    if (sendClearing) {
        char sub[512];
        sprintf_s(sub, "{\"method\":\"subscribe\",\"subscription\":"
                 "{\"type\":\"clearinghouseState\",\"user\":\"%s\"}}", userAddress_.c_str());
        if (connection_.send(sub)) subscribedClearinghouse_ = true;
    }
    if (sendOrders) {
        char sub[512];
        sprintf_s(sub, "{\"method\":\"subscribe\",\"subscription\":"
                 "{\"type\":\"openOrders\",\"user\":\"%s\"}}", userAddress_.c_str());
        if (connection_.send(sub)) subscribedOpenOrders_ = true;
    }

    // [OPM-218] Send perpDex clearinghouseState subscriptions
    for (const auto& dex : dexSubs) {
        char sub[512];
        sprintf_s(sub, "{\"method\":\"subscribe\",\"subscription\":"
                 "{\"type\":\"clearinghouseState\",\"user\":\"%s\",\"dex\":\"%s\"}}",
                 userAddress_.c_str(), dex.c_str());
        if (connection_.send(sub)) {
            EnterCriticalSection(&accountSubCs_);
            subscribedClearinghouseDexes_.insert(dex);
            LeaveCriticalSection(&accountSubCs_);
            logf(1, "WS: Subscribed clearinghouseState dex=%s", dex.c_str());
        } else {
            // Re-queue for retry
            EnterCriticalSection(&accountSubCs_);
            pendingClearinghouseDexSubs_.push_back(dex);
            LeaveCriticalSection(&accountSubCs_);
            logf(1, "WS: Failed to send clearinghouseState sub for dex=%s, re-queued", dex.c_str());
        }
    }
}

void WebSocketManager::requeueSubscriptionsAfterReconnect() {
    const int MAX_REQUEUE_WITHOUT_DATA = 3;  // Drop after 3 reconnects with no data [OPM-170]

    EnterCriticalSection(&l2SubCs_);
    for (const auto& coin : l2Subscriptions_) {
        if (cache_.getBid(coin) > 0) {
            // Coin has received data before — safe to requeue
            l2RequeueFailCount_.erase(coin);
            pendingL2Subs_.push_back(coin);
        } else {
            // Never received data — might be causing the disconnect
            int& count = l2RequeueFailCount_[coin];
            count++;
            if (count <= MAX_REQUEUE_WITHOUT_DATA) {
                pendingL2Subs_.push_back(coin);
            }
            // else: silently dropped — logged below outside CS
        }
    }
    // Collect dropped coins for logging outside critical section
    std::vector<std::string> dropped;
    for (auto it = l2RequeueFailCount_.begin(); it != l2RequeueFailCount_.end(); ) {
        if (it->second > MAX_REQUEUE_WITHOUT_DATA) {
            dropped.push_back(it->first);
            it = l2RequeueFailCount_.erase(it);
        } else {
            ++it;
        }
    }
    l2Subscriptions_.clear();
    LeaveCriticalSection(&l2SubCs_);

    for (const auto& coin : dropped) {
        logf(1, "WS: Symbol '%s' NOT FOUND on exchange — "
             "caused %d consecutive disconnects, subscription removed. "
             "Check asset name or perpDex format.",
             coin.c_str(), MAX_REQUEUE_WITHOUT_DATA + 1);
        bannedL2Coins_.insert(coin);
    }

    // Fatal: toxic subscription crashed the connection repeatedly.
    // Set global fatal error so ALL broker functions return failure → Zorro stops.
    if (!dropped.empty()) {
        log(1, "WS: FATAL — invalid symbol caused repeated disconnects. "
             "Fix the asset list and restart.");
        sprintf_s(hl::g_fatalErrorMsg, "Symbol '%s' not found on exchange",
                  dropped[0].c_str());
        hl::g_fatalError = true;
        connection_.stopAutoReconnect();
        return;  // Don't requeue anything — connection is done
    }

    EnterCriticalSection(&accountSubCs_);
    if (subscribedUserFills_) { pendingUserFillsSub_ = true; subscribedUserFills_ = false; }
    if (subscribedClearinghouse_) { pendingClearinghouseSub_ = true; subscribedClearinghouse_ = false; }
    if (subscribedOpenOrders_) { pendingOpenOrdersSub_ = true; subscribedOpenOrders_ = false; }
    // [OPM-218] Requeue perpDex clearinghouseState subscriptions
    for (const auto& dex : subscribedClearinghouseDexes_)
        pendingClearinghouseDexSubs_.push_back(dex);
    subscribedClearinghouseDexes_.clear();
    LeaveCriticalSection(&accountSubCs_);
}

// --- Order Posts ---

void WebSocketManager::sendPendingPosts() {
    EnterCriticalSection(&postCs_);
    if (pendingPosts_.empty()) { LeaveCriticalSection(&postCs_); return; }
    auto post = pendingPosts_.front();
    pendingPosts_.pop();
    LeaveCriticalSection(&postCs_);

    connection_.send(post.json.c_str());
}

OrderResponse WebSocketManager::sendOrderSync(const std::string& orderJson, DWORD timeoutMs) {
    OrderResponse resp;
    resp.requestId = 0;
    resp.success = false;

    if (!connection_.isConnected()) {
        resp.error = "Not connected";
        return resp;
    }

    int reqId = nextRequestId_++;
    HANDLE waitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!waitEvent) {
        resp.error = "Failed to create wait event";
        return resp;
    }

    EnterCriticalSection(&responseCs_);
    responseEvents_[reqId] = waitEvent;
    LeaveCriticalSection(&responseCs_);

    char postJson[8192];
    sprintf_s(postJson, "{\"method\":\"post\",\"id\":%d,\"request\":%s}", reqId, orderJson.c_str());

    EnterCriticalSection(&postCs_);
    pendingPosts_.push({reqId, postJson});
    LeaveCriticalSection(&postCs_);

    DWORD waitResult = WaitForSingleObject(waitEvent, timeoutMs);

    EnterCriticalSection(&responseCs_);
    auto it = completedResponses_.find(reqId);
    if (it != completedResponses_.end()) {
        resp = it->second;
        completedResponses_.erase(it);
    }
    responseEvents_.erase(reqId);
    LeaveCriticalSection(&responseCs_);

    CloseHandle(waitEvent);

    if (waitResult == WAIT_TIMEOUT && !resp.success) {
        resp.requestId = reqId;
        resp.error = "Timeout";
    }

    return resp;
}

// --- Message Handling ---

const char* WebSocketManager::channelKindName(int k) {
    namespace ch = hl::protocol::ws_channel;
    switch (k) {
        case CK_L2BOOK:        return ch::L2_BOOK;
        case CK_CLEARING:      return "clearing";  // diagnostic alias for CLEARINGHOUSE_STATE
        case CK_OPEN_ORDERS:   return ch::OPEN_ORDERS;
        case CK_USER_FILLS:    return ch::USER_FILLS;
        case CK_ORDER_UPDATES: return ch::ORDER_UPDATES;
        case CK_POST:          return ch::POST;
        default:               return "other";
    }
}

void WebSocketManager::handleMessage(const char* data, size_t len) {
    // Parse JSON once to extract channel for routing
    yyjson_doc* doc = yyjson_read(data, len, 0);
    if (!doc) {
        if (diagLevel_ >= 2) logf(2, "WS: JSON parse failed (%zu bytes)", len);
        return;
    }
    yyjson_val* root = yyjson_doc_get_root(doc);
    const char* channel = json::getStringPtr(root, "channel");

    // [OPM-550] Phase 1.6 — time the parser dispatch per channel kind.
    // Runtime-gated on diagLevel_ >= 3. Same-thread access (connection thread only).
    const bool diagOn = (diagLevel_ >= 3);
    DWORD parseStart = diagOn ? GetTickCount() : 0;
    int kind = CK_OTHER;

    if (channel) {
        namespace ch = hl::protocol::ws_channel;
        if (strcmp(channel, ch::L2_BOOK) == 0)              { kind = CK_L2BOOK;        parseL2Book(data); }
        else if (strcmp(channel, ch::CLEARINGHOUSE_STATE) == 0) { kind = CK_CLEARING;     parseClearinghouseState(data); }
        else if (strcmp(channel, ch::OPEN_ORDERS) == 0)     { kind = CK_OPEN_ORDERS;   parseOpenOrders(data); }
        else if (strcmp(channel, ch::USER_FILLS) == 0)      { kind = CK_USER_FILLS;    parseUserFills(data); }
        else if (strcmp(channel, ch::ORDER_UPDATES) == 0)   { kind = CK_ORDER_UPDATES; parseOrderUpdates(data); }
        else if (strcmp(channel, ch::POST) == 0)            { kind = CK_POST;          parsePostResponse(data); }
        else if (strcmp(channel, ch::PONG) == 0) { /* expected, ignore */ }
        else if (strcmp(channel, ch::SUBSCRIPTION_RESPONSE) == 0) {
            if (diagLevel_ >= 2) logf(2, "WS: Subscription ACK (%zu bytes)", len);
        }
        else if (strcmp(channel, ch::ERR) == 0) {
            // Log subscription errors (previously silently discarded) [OPM-74]
            const char* errData = json::getStringPtr(root, "data");
            logf(1, "WS ERROR from server: %s", errData ? errData : "(no details)");

            // Reset account sub flags for retry on next iteration
            EnterCriticalSection(&accountSubCs_);
            if (subscribedClearinghouse_) {
                pendingClearinghouseSub_ = true;
                subscribedClearinghouse_ = false;
            }
            if (subscribedUserFills_) {
                pendingUserFillsSub_ = true;
                subscribedUserFills_ = false;
            }
            if (subscribedOpenOrders_) {
                pendingOpenOrdersSub_ = true;
                subscribedOpenOrders_ = false;
            }
            LeaveCriticalSection(&accountSubCs_);
        }
        else if (diagLevel_ >= 2) {
            logf(2, "WS: Unhandled channel '%s' (%zu bytes)", channel, len);
        }
    } else {
        // No channel field — check for order response without channel wrapper
        if (yyjson_obj_get(root, "response")) { kind = CK_POST; parsePostResponse(data); }
        else if (diagLevel_ >= 2) logf(2, "WS: No channel in message (%zu bytes): %.120s", len, data);
    }

    // [OPM-550] Phase 1.6 — record dispatch time for this channel kind (gated)
    if (diagOn) {
        DWORD parseMs = GetTickCount() - parseStart;
        channelStats_[kind].count++;
        channelStats_[kind].totalMs += parseMs;
        if (parseMs > channelStats_[kind].maxMs) channelStats_[kind].maxMs = parseMs;
    }

    yyjson_doc_free(doc);
}

void WebSocketManager::parseL2Book(const char* json) {
    auto result = hl::ws::parseL2Book(json, diagLevel_, logCallback_);
    if (result.valid) {
        // Log first data arrival per asset at level 1 (confirms WS flowing) [OPM-99]
        bool isFirst = (cache_.getBid(result.coin) <= 0);
        cache_.setBidAsk(result.coin, result.bid, result.ask);
        if (isFirst && diagLevel_ >= 1)
            logf(1, "WS: l2Book LIVE %s bid=%.4f ask=%.4f", result.coin, result.bid, result.ask);
        else if (diagLevel_ >= 2)
            logf(2, "WS: l2Book %s bid=%.4f ask=%.4f", result.coin, result.bid, result.ask);
        if (zorroWindow_) PostMessage(zorroWindow_, WM_APP + 1, 0, 0);
    } else if (result.coin[0]) {
        if (diagLevel_ >= 2) logf(2, "WS: l2Book %s INVALID bid=%.2f ask=%.2f", result.coin, result.bid, result.ask);
    }
}

void WebSocketManager::parseClearinghouseState(const char* json) {
    // [OPM-218] If perpDex subscriptions exist, determine dex for this message
    EnterCriticalSection(&accountSubCs_);
    bool hasPerpDexSubs = !subscribedClearinghouseDexes_.empty();
    LeaveCriticalSection(&accountSubCs_);

    if (!hasPerpDexSubs) {
        hl::ws::parseClearinghouseState(cache_, json, diagLevel_, logCallback_);
        return;
    }

    // [OPM-226] Try to extract "dex" field from WS data object first.
    // The WS response for perpDex subscriptions may include:
    //   {"channel":"clearinghouseState","data":{"dex":"xyz",...}}
    std::string dex;
    {
        yyjson_doc* doc = yyjson_read(json, strlen(json), 0);
        if (doc) {
            yyjson_val* root = yyjson_doc_get_root(doc);
            yyjson_val* data = json::getObject(root, "data");
            if (data) {
                char dexBuf[64] = {0};
                json::getString(data, "dex", dexBuf, sizeof(dexBuf));
                if (dexBuf[0] && dexBuf[0] != '\0') {
                    // Verify this dex is one we subscribed to
                    EnterCriticalSection(&accountSubCs_);
                    if (subscribedClearinghouseDexes_.count(dexBuf)) {
                        dex = dexBuf;
                    }
                    LeaveCriticalSection(&accountSubCs_);
                }
            }
            yyjson_doc_free(doc);
        }
    }

    // Fall back to heuristic inference if no "dex" field found
    if (dex.empty()) {
        dex = inferDexFromPositions(json);
    }

    if (diagLevel_ >= 3 && !dex.empty()) {
        logf(3, "WS clearinghouseState: dex=%s", dex.c_str());
    }

    hl::ws::parseClearinghouseState(cache_, json, diagLevel_, logCallback_, dex.c_str());
}

std::string WebSocketManager::inferDexFromPositions(const char* json) {
    // Extract first coin from assetPositions, look up in g_assets to find its dex.
    // Returns "" for main-dex, or perpDex name if coin belongs to a perpDex.
    yyjson_doc* doc = yyjson_read(json, strlen(json), 0);
    if (!doc) return "";
    yyjson_val* root = yyjson_doc_get_root(doc);

    yyjson_val* state = json::getObject(json::getObject(root, "data"), "clearinghouseState");
    if (!state) state = root;

    yyjson_val* positions = json::getArray(state, "assetPositions");
    if (!positions || yyjson_arr_size(positions) == 0) {
        yyjson_doc_free(doc);
        // [OPM-226] Empty snapshot — can't determine dex. Return "" (main-dex).
        // This is safe for main-dex but WRONG for perpDex with no positions —
        // clearPositionsByDex("") would wipe main-dex data. Callers should
        // prefer extracting "dex" from the WS data object first.
        if (diagLevel_ >= 1)
            logf(1, "WS inferDex: empty assetPositions, defaulting to main-dex");
        return "";
    }

    yyjson_val* first = yyjson_arr_get_first(positions);
    yyjson_val* posObj = json::getObject(first, "position");
    char coinBuf[64] = {0};
    if (posObj) json::getString(posObj, "coin", coinBuf, sizeof(coinBuf));
    yyjson_doc_free(doc);

    if (coinBuf[0] == 0) return "";

    // [OPM-226] Handle @index format coins (e.g., "@110001") by resolving
    // through the WS index mapping → then look up the resolved coin in g_assets
    if (coinBuf[0] == '@') {
        int idx = atoi(coinBuf + 1);
        if (idx > 0) {
            std::string resolved = getCoinByIndex(idx);
            if (!resolved.empty()) {
                if (diagLevel_ >= 1)
                    logf(1, "WS inferDex: resolved %s → %s", coinBuf, resolved.c_str());
                strncpy_s(coinBuf, resolved.c_str(), _TRUNCATE);
            } else if (diagLevel_ >= 1) {
                logf(1, "WS inferDex: no index mapping for %s", coinBuf);
            }
        }
    }

    // Look up coin in asset registry
    for (int i = 0; i < hl::g_assets.count; ++i) {
        const hl::AssetInfo* a = hl::g_assets.getByIndex(i);
        if (a && strcmp(a->coin, coinBuf) == 0 && a->isPerpDex && a->perpDex[0])
            return a->perpDex;
    }

    if (diagLevel_ >= 2)
        logf(2, "WS inferDex: coin=%s not found in perpDex assets, treating as main-dex", coinBuf);
    return "";  // Main-dex or unknown coin
}

void WebSocketManager::parseOpenOrders(const char* json) {
    hl::ws::parseOpenOrders(cache_, json, diagLevel_, logCallback_);
}

void WebSocketManager::parseUserFills(const char* json) {
    hl::ws::parseUserFills(cache_, json, diagLevel_, logCallback_);

    // Propagate fills to TradeMap via callback [OPM-87]
    if (!fillNotifyCallback_) return;

    // Compute per-OID cumulative totals from all cached fills.
    // Fires callback for each OID — the callback decides whether to update
    // TradeMap (only if cumulative total >= current filledSize).
    auto fills = cache_.getRecentFills(100);
    std::map<std::string, std::pair<double, double>> totals;  // oid -> {totalSz, totalValue}
    for (const auto& f : fills) {
        auto& t = totals[f.oid];
        t.first += f.sz;
        t.second += f.sz * f.px;
    }
    for (const auto& entry : totals) {
        double avgPx = entry.second.first > 0
            ? entry.second.second / entry.second.first : 0;
        fillNotifyCallback_(entry.first.c_str(), entry.second.first, avgPx);
    }
}

void WebSocketManager::parseOrderUpdates(const char* json) {
    if (orderUpdateCallback_) {
        hl::ws::parseOrderUpdates(json, orderUpdateCallback_, diagLevel_, logCallback_);
    }
}

void WebSocketManager::parsePostResponse(const char* json) {
    OrderResponse resp = hl::ws::parsePostResponse(json, diagLevel_, logCallback_);
    if (resp.requestId == 0) return;

    EnterCriticalSection(&responseCs_);
    completedResponses_[resp.requestId] = resp;
    auto evIt = responseEvents_.find(resp.requestId);
    if (evIt != responseEvents_.end() && evIt->second) SetEvent(evIt->second);
    LeaveCriticalSection(&responseCs_);
}

bool WebSocketManager::isCoinBanned(const std::string& coin) const {
    // No lock needed — bannedL2Coins_ is only written from connectionLoop thread
    // and read from main thread. Worst case: brief race returns false (safe).
    return bannedL2Coins_.count(coin) > 0;
}

// --- Debug ---

void WebSocketManager::forceDisconnectForTest() {
    // Force a disconnect without disabling auto-reconnect.
    // This simulates a server-side drop (code 1006 scenario).
    // IXWebSocket will auto-reconnect, triggering the wasReconnected() path.
    log(1, "WS: DEBUG — forcing disconnect (auto-reconnect stays active)");
    connection_.forceCloseForTest();
}

// --- Index Mappings ---

void WebSocketManager::setIndexMapping(int index, const std::string& coin) {
    EnterCriticalSection(&indexMapCs_);
    indexToCoin_[index] = coin;
    LeaveCriticalSection(&indexMapCs_);
}

void WebSocketManager::clearIndexMappings() {
    EnterCriticalSection(&indexMapCs_);
    indexToCoin_.clear();
    LeaveCriticalSection(&indexMapCs_);
}

std::string WebSocketManager::getCoinByIndex(int index) const {
    EnterCriticalSection(&indexMapCs_);
    auto it = indexToCoin_.find(index);
    std::string result = (it != indexToCoin_.end()) ? it->second : std::string();
    LeaveCriticalSection(&indexMapCs_);
    return result;
}

} // namespace ws
} // namespace hl
