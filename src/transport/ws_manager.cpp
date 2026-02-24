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

    while (running_) {
        if (WaitForSingleObject(shutdownEvent_, 0) == WAIT_OBJECT_0) break;

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

        // Send pending work (only if connected)
        if (connection_.isConnected()) {
            sendPendingPosts();
            if (initialSubsQueued_) {
                sendPendingL2Subscriptions();
                sendPendingAccountSubscriptions();
            }

            // HL application ping at reduced frequency [OPM-128]
            // Protocol-level pings (IXWebSocket) keep the connection alive.
            // HL app pings keep the subscription channels active.
            DWORD now = GetTickCount();
            if (now - lastHlPingTick >= HL_PING_INTERVAL_MS) {
                connection_.send("{\"method\":\"ping\"}");
                lastHlPingTick = now;
            }
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

        // Drain messages from IXWebSocket queue
        connection_.poll(100);
        Sleep(10);
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
            logf(1, "WS: Failed to send l2Book subscription for %s, queuing for retry", coin.c_str());
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
            logf(1, "WS: Failed to send l2Book subscription for %s", toSend[i].c_str());
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
}

void WebSocketManager::requeueSubscriptionsAfterReconnect() {
    EnterCriticalSection(&l2SubCs_);
    for (const auto& coin : l2Subscriptions_) pendingL2Subs_.push_back(coin);
    l2Subscriptions_.clear();
    LeaveCriticalSection(&l2SubCs_);

    EnterCriticalSection(&accountSubCs_);
    if (subscribedUserFills_) { pendingUserFillsSub_ = true; subscribedUserFills_ = false; }
    if (subscribedClearinghouse_) { pendingClearinghouseSub_ = true; subscribedClearinghouse_ = false; }
    if (subscribedOpenOrders_) { pendingOpenOrdersSub_ = true; subscribedOpenOrders_ = false; }
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

void WebSocketManager::handleMessage(const char* data, size_t len) {
    // Parse JSON once to extract channel for routing
    yyjson_doc* doc = yyjson_read(data, len, 0);
    if (!doc) {
        if (diagLevel_ >= 2) logf(2, "WS: JSON parse failed (%zu bytes)", len);
        return;
    }
    yyjson_val* root = yyjson_doc_get_root(doc);
    const char* channel = json::getStringPtr(root, "channel");

    if (channel) {
        if (strcmp(channel, "l2Book") == 0) parseL2Book(data);
        else if (strcmp(channel, "clearinghouseState") == 0) parseClearinghouseState(data);
        else if (strcmp(channel, "openOrders") == 0) parseOpenOrders(data);
        else if (strcmp(channel, "userFills") == 0) parseUserFills(data);
        else if (strcmp(channel, "orderUpdates") == 0) parseOrderUpdates(data);
        else if (strcmp(channel, "post") == 0) parsePostResponse(data);
        else if (strcmp(channel, "pong") == 0) { /* expected, ignore */ }
        else if (strcmp(channel, "subscriptionResponse") == 0) {
            if (diagLevel_ >= 2) logf(2, "WS: Subscription ACK (%zu bytes)", len);
        }
        else if (strcmp(channel, "error") == 0) {
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
        if (yyjson_obj_get(root, "response")) parsePostResponse(data);
        else if (diagLevel_ >= 2) logf(2, "WS: No channel in message (%zu bytes): %.120s", len, data);
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
    hl::ws::parseClearinghouseState(cache_, json, diagLevel_, logCallback_);
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
