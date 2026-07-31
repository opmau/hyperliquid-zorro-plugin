//=============================================================================
// hl_globals.cpp - Global state implementation
//=============================================================================
// LAYER: Foundation | DEPENDENCIES: hl_globals.h
//=============================================================================

#include "hl_globals.h"
#include <cstdarg>
#include <cstdio>
#include <cstring>

namespace hl {

// =============================================================================
// GLOBAL INSTANCES
// =============================================================================

RuntimeConfig g_config;
AssetRegistry g_assets;
TradingState  g_trading;
Logger        g_logger;

// Lazy-initialized (set by BrokerLogin)
void* g_wsManager = nullptr;
void* g_priceCache = nullptr;

// Fatal error — stops all broker functions [OPM-170]
std::atomic<bool> g_fatalError{false};
char g_fatalErrorMsg[256] = {0};

// =============================================================================
// ASSET REGISTRY IMPLEMENTATION
// =============================================================================

void AssetRegistry::init() {
    if (!csInit) {
        InitializeCriticalSection(&cs);
        csInit = true;
    }
    clear();
}

void AssetRegistry::cleanup() {
    if (csInit) {
        DeleteCriticalSection(&cs);
        csInit = false;
    }
}

void AssetRegistry::clear() {
    if (csInit) EnterCriticalSection(&cs);
    count = 0;
    memset(assets, 0, sizeof(assets));
    if (csInit) LeaveCriticalSection(&cs);
}

int AssetRegistry::findByName(const char* name) const {
    if (!name || !*name) return -1;
    if (csInit) EnterCriticalSection(&cs);
    for (int i = 0; i < count; ++i) {
        if (_stricmp(assets[i].name, name) == 0) {
            if (csInit) LeaveCriticalSection(&cs);
            return i;
        }
    }
    if (csInit) LeaveCriticalSection(&cs);
    return -1;
}

int AssetRegistry::findByCoin(const char* coin) const {
    if (!coin || !*coin) return -1;
    if (csInit) EnterCriticalSection(&cs);
    for (int i = 0; i < count; ++i) {
        if (_stricmp(assets[i].coin, coin) == 0) {
            if (csInit) LeaveCriticalSection(&cs);
            return i;
        }
    }
    if (csInit) LeaveCriticalSection(&cs);
    return -1;
}

const AssetInfo* AssetRegistry::getByIndex(int idx) const {
    if (idx < 0) return nullptr;
    if (csInit) EnterCriticalSection(&cs);
    const AssetInfo* result = (idx < count) ? &assets[idx] : nullptr;
    if (csInit) LeaveCriticalSection(&cs);
    return result;
}

bool AssetRegistry::add(const AssetInfo& info) {
    if (csInit) EnterCriticalSection(&cs);
    if (count >= config::MAX_ASSETS) {
        if (csInit) LeaveCriticalSection(&cs);
        return false;
    }
    assets[count++] = info;
    if (csInit) LeaveCriticalSection(&cs);
    return true;
}

// =============================================================================
// TRADING STATE IMPLEMENTATION
// =============================================================================

void TradingState::init() {
    if (!tradeCsInit) {
        InitializeCriticalSection(&tradeCs);
        tradeCsInit = true;
    }
    nextTradeId = 2;  // Zorro treats BrokerBuy2 return 0 or 1 as failure
    lastNonce = 0;
    lastOrderId[0] = 0;
    httpRequestId = 0;
    lotSize = 1.0;
    tradeMap.clear();
}

void TradingState::cleanup() {
    if (tradeCsInit) {
        EnterCriticalSection(&tradeCs);
        tradeMap.clear();
        LeaveCriticalSection(&tradeCs);
        DeleteCriticalSection(&tradeCs);
        tradeCsInit = false;
    }
}

int TradingState::generateTradeId() {
    return nextTradeId.fetch_add(1);
}

uint64_t TradingState::generateNonce() {
    // Monotonic nonce prevents collision when orders placed in same millisecond.
    // Uses GetSystemTimeAsFileTime for ms precision (vs time() which was 1-second).
    // [OPM-160]
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    uint64_t time64 = (static_cast<uint64_t>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
    time64 -= 116444736000000000ULL;  // Windows epoch -> Unix epoch
    uint64_t now = time64 / 10000;    // 100ns intervals -> milliseconds

    uint64_t expected = lastNonce.load();
    uint64_t desired;
    do {
        desired = (now > expected) ? now : expected + 1;
    } while (!lastNonce.compare_exchange_weak(expected, desired));
    return desired;
}

void TradingState::setOrder(int tradeId, const OrderState& state) {
    if (!tradeCsInit) return;
    EnterCriticalSection(&tradeCs);
    tradeMap[tradeId] = state;
    LeaveCriticalSection(&tradeCs);
}

bool TradingState::getOrder(int tradeId, OrderState& out) const {
    if (!tradeCsInit) return false;
    EnterCriticalSection(&tradeCs);
    auto it = tradeMap.find(tradeId);
    bool found = (it != tradeMap.end());
    if (found) out = it->second;
    LeaveCriticalSection(&tradeCs);
    return found;
}

bool TradingState::updateOrder(int tradeId, const OrderState& state) {
    if (!tradeCsInit) return false;
    EnterCriticalSection(&tradeCs);
    auto it = tradeMap.find(tradeId);
    bool found = (it != tradeMap.end());
    if (found) it->second = state;
    LeaveCriticalSection(&tradeCs);
    return found;
}

void TradingState::removeOrder(int tradeId) {
    if (!tradeCsInit) return;
    EnterCriticalSection(&tradeCs);
    tradeMap.erase(tradeId);
    LeaveCriticalSection(&tradeCs);
}

// =============================================================================
// LOGGER IMPLEMENTATION [OPM-550]
// Async producer/consumer queue. See header for design rationale.
// =============================================================================

Logger::Logger() {
    InitializeCriticalSection(&cs_);
}

Logger::~Logger() {
    // Discard queue without invoking callback — at static destruction time
    // the BrokerMessage function pointer may already be invalid.
    EnterCriticalSection(&cs_);
    queue_.clear();
    LeaveCriticalSection(&cs_);
    DeleteCriticalSection(&cs_);
}

void Logger::log(int minLevel, const char* msg) {
    if (!msg) return;
    if (level.load() < minLevel) return;

    EnterCriticalSection(&cs_);
    if (queue_.size() >= MAX_QUEUE_DEPTH) {
        // Drop oldest to bound memory; producer must never block.
        queue_.pop_front();
        dropped_.fetch_add(1, std::memory_order_relaxed);
    }
    queue_.emplace_back(msg);
    LeaveCriticalSection(&cs_);
}

void Logger::logf(int minLevel, const char* fmt, ...) {
    if (!fmt) return;
    if (level.load() < minLevel) return;

    char buf[2048];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    buf[sizeof(buf) - 1] = 0;

    EnterCriticalSection(&cs_);
    if (queue_.size() >= MAX_QUEUE_DEPTH) {
        queue_.pop_front();
        dropped_.fetch_add(1, std::memory_order_relaxed);
    }
    queue_.emplace_back(buf);
    LeaveCriticalSection(&cs_);
}

size_t Logger::drain(size_t maxMessages) {
    if (!callback) {
        // No callback yet — just discard to prevent unbounded growth.
        EnterCriticalSection(&cs_);
        size_t n = queue_.size();
        if (n > maxMessages) n = maxMessages;
        for (size_t i = 0; i < n; ++i) queue_.pop_front();
        LeaveCriticalSection(&cs_);
        return n;
    }

    size_t drained = 0;
    while (drained < maxMessages) {
        std::string msg;
        EnterCriticalSection(&cs_);
        if (queue_.empty()) {
            LeaveCriticalSection(&cs_);
            break;
        }
        msg = std::move(queue_.front());
        queue_.pop_front();
        LeaveCriticalSection(&cs_);

        // Synchronous BrokerMessage call is safe here — main thread.
        callback(msg.c_str());
        drained++;
    }

    // Surface dropped-count when it changes — operator visibility.
    // lastReportedDrops_ is accessed only here (main thread / drain only).
    uint64_t now = dropped_.load(std::memory_order_relaxed);
    if (now > lastReportedDrops_) {
        char warn[160];
        snprintf(warn, sizeof(warn),
                 "WARN Logger dropped %llu messages (queue overflow)",
                 (unsigned long long)(now - lastReportedDrops_));
        if (callback) callback(warn);
        lastReportedDrops_ = now;
    }
    return drained;
}

void Logger::enqueue(const char* msg) {
    // Bypass level check — caller (typically ws_manager / ws_connection) has
    // already filtered. Just enqueue with overflow protection.
    if (!msg) return;
    EnterCriticalSection(&cs_);
    if (queue_.size() >= MAX_QUEUE_DEPTH) {
        queue_.pop_front();
        dropped_.fetch_add(1, std::memory_order_relaxed);
    }
    queue_.emplace_back(msg);
    LeaveCriticalSection(&cs_);
}

void Logger::flush() {
    if (!callback) {
        EnterCriticalSection(&cs_);
        queue_.clear();
        LeaveCriticalSection(&cs_);
        return;
    }
    while (true) {
        std::string msg;
        EnterCriticalSection(&cs_);
        if (queue_.empty()) {
            LeaveCriticalSection(&cs_);
            break;
        }
        msg = std::move(queue_.front());
        queue_.pop_front();
        LeaveCriticalSection(&cs_);
        callback(msg.c_str());
    }
}

size_t Logger::queueDepth() const {
    EnterCriticalSection(&cs_);
    size_t n = queue_.size();
    LeaveCriticalSection(&cs_);
    return n;
}

uint64_t Logger::messagesDropped() const {
    return dropped_.load(std::memory_order_relaxed);
}

// =============================================================================
// GLOBAL INITIALIZATION / CLEANUP
// =============================================================================

void initGlobals() {
    // Reset config to defaults
    g_config = RuntimeConfig();
    strcpy_s(g_config.baseUrl, config::TESTNET_REST);

    // Initialize thread-safe structures
    g_assets.init();
    g_trading.init();

    // Logger starts with callback=nullptr, level=0
    g_logger.callback = nullptr;
    g_logger.level = 0;

    // WebSocket pointers start as nullptr
    g_wsManager = nullptr;
    g_priceCache = nullptr;

    // Clear fatal error
    g_fatalError = false;
    g_fatalErrorMsg[0] = 0;
}

void cleanupGlobals() {
    // Note: WebSocket cleanup should be done by caller before this
    g_assets.cleanup();
    g_trading.cleanup();
}

} // namespace hl
