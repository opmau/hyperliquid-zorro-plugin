//=============================================================================
// hl_globals.h - Controlled global state with clear ownership
//=============================================================================
// LAYER: Foundation | DEPENDENCIES: hl_types.h, hl_config.h
// THREAD SAFETY: Individual structs document their own thread safety
//=============================================================================

#pragma once

#include "hl_types.h"
#include "hl_config.h"
#include <windows.h>
#include <string>
#include <map>
#include <set>
#include <atomic>
#include <cstdint>

namespace hl {

// =============================================================================
// RUNTIME CONFIGURATION (set at login, rarely changes)
// Thread safety: Set once at login, read-only thereafter
// =============================================================================

struct RuntimeConfig {
    // Network
    bool isTestnet = true;          // true = testnet (safe default)
    char baseUrl[256] = {0};        // REST API base URL

    // Credentials (do NOT log privateKey)
    char walletAddress[96] = {0};   // 0x... address
    char privateKey[96] = {0};      // Private key for signing

    // Diagnostics
    int diagLevel = 0;              // 0=off, 1=errors, 2=info, 3=verbose

    // Features
    bool enableWebSocket = true;    // Use WS for prices
    bool useWsOrders = true;        // Use WS for order placement
    bool enableHttpSeed = true;     // HTTP fallback when WS stale
    int httpSeedCooldownMs = 1000;  // Min time between HTTP seeds

    // Trading
    char orderType[16] = "Ioc";     // Default: Immediate-or-cancel
    int maxRetries = 3;             // Order retry attempts
    bool dryRun = false;            // Build orders but don't send
    int accountMode = 0;            // 0=API wallet, 1=vault/subaccount
    bool stopOrderPending = false;  // True when SET_ORDERTYPE +8 was set [OPM-77]

    // Zorro integration
    HWND zorroWindow = NULL;        // For WM_APP+1 notifications
    int cacheTimeoutMs = 2000;      // General cache timeout
};

// =============================================================================
// ASSET REGISTRY (populated from /info meta endpoint)
// Thread safety: Protected by critical section
// =============================================================================

struct AssetRegistry {
    AssetInfo assets[config::MAX_ASSETS];
    int count = 0;

    mutable CRITICAL_SECTION cs;
    bool csInit = false;

    void init();
    void cleanup();
    void clear();

    // Thread-safe accessors
    int findByName(const char* name) const;     // "BTC-USD" -> index
    int findByCoin(const char* coin) const;     // "BTC" -> index
    const AssetInfo* getByIndex(int idx) const;
    bool add(const AssetInfo& info);
};

// =============================================================================
// TRADING STATE (runtime trading data)
// Thread safety: Use critical section for TradeMap access
// =============================================================================

struct TradingState {
    std::atomic<int> nextTradeId{1};     // Sequential trade ID counter
    std::atomic<uint64_t> lastNonce{0};  // Monotonic nonce for orders
    char lastOrderId[128] = {0};         // Last placed order ID
    int httpRequestId = 0;               // HTTP request counter
    double lotSize = 1.0;                // Contracts per lot

    // Current asset context (set by SET_SYMBOL or BrokerAsset, used by GET_POSITION etc.)
    char currentSymbol[64] = {0};

    // Price lookup symbol (set ONLY by SET_SYMBOL, used by GET_PRICE)
    // Isolates GET_PRICE from BrokerAsset subscription loops that overwrite currentSymbol.
    char priceSymbol[64] = {0};

    // Trade tracking: Zorro trade ID -> OrderState
    std::map<int, OrderState> tradeMap;
    mutable CRITICAL_SECTION tradeCs;
    bool tradeCsInit = false;

    void init();
    void cleanup();

    // Thread-safe trade map access
    int generateTradeId();
    uint64_t generateNonce();
    void setOrder(int tradeId, const OrderState& state);
    bool getOrder(int tradeId, OrderState& out) const;
    bool updateOrder(int tradeId, const OrderState& state);
    void removeOrder(int tradeId);
};

// =============================================================================
// LOGGER (decouples from Zorro's BrokerMessage)
// Thread safety: Atomic level, callback assumed thread-safe
// =============================================================================

using LogCallback = int(*)(const char*);

struct Logger {
    LogCallback callback = nullptr;
    std::atomic<int> level{0};

    void log(int minLevel, const char* msg) const;
    void logf(int minLevel, const char* fmt, ...) const;
    void error(const char* msg) const { log(1, msg); }
    void info(const char* msg) const { log(2, msg); }
    void debug(const char* msg) const { log(3, msg); }
};

// =============================================================================
// GLOBAL INSTANCES (defined in hl_globals.cpp)
// =============================================================================

extern RuntimeConfig g_config;
extern AssetRegistry g_assets;
extern TradingState  g_trading;
extern Logger        g_logger;

// Lazy-initialized singletons (created in BrokerLogin, destroyed in BrokerLogin logout)
// Typed as void* to avoid Foundation→Transport dependency; cast to hl::ws::* in consumers
extern void* g_wsManager;
extern void* g_priceCache;

// =============================================================================
// INITIALIZATION / CLEANUP
// =============================================================================

void initGlobals();     // Call once at DLL load or BrokerOpen
void cleanupGlobals();  // Call at DLL unload

} // namespace hl
