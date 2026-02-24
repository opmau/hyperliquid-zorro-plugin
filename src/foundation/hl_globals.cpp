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
// LOGGER IMPLEMENTATION
// =============================================================================

void Logger::log(int minLevel, const char* msg) const {
    if (!callback || !msg) return;
    if (level.load() >= minLevel) {
        callback(msg);
    }
}

void Logger::logf(int minLevel, const char* fmt, ...) const {
    if (!callback || !fmt) return;
    if (level.load() < minLevel) return;

    char buf[2048];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    buf[sizeof(buf) - 1] = 0;

    callback(buf);
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
}

void cleanupGlobals() {
    // Note: WebSocket cleanup should be done by caller before this
    g_assets.cleanup();
    g_trading.cleanup();
}

} // namespace hl
