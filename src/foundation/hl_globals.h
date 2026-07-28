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
#include <deque>
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
    char walletAddress[96] = {0};   // 0x... address (user or sub-account)
    char privateKey[96] = {0};      // Private key for signing
    char vaultAddress[96] = {0};    // Sub-account/vault address for order routing [OPM-202]

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

    // [OPM-791] Sticky order-type override. Zorro auto-calls SET_ORDERTYPE at
    // every order entry and can only derive 0/1/2/3 from TradeMode, so it
    // always downgraded a script's brokerCommand(50012,"Alo") back to "Ioc"
    // before the order was built — no ALO order ever reached the exchange.
    // When true, SET_ORDERTYPE leaves orderType alone (it still consumes the
    // +8 STOP flag). Set by 50012("Alo"), cleared by 50012("Ioc"/"Gtc") and
    // at login/logout.
    bool orderTypeSticky = false;

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
// LOGGER (decouples from Zorro's BrokerMessage) [OPM-550]
// =============================================================================
// Threading model:
//   - Producers (any thread, especially WS connection thread): call log/logf.
//     These enqueue to a bounded internal queue; never block on Zorro.
//   - Consumer (main/Zorro thread): calls drain() to flush queued messages
//     through the user callback (= BrokerMessage). The main thread is the
//     only safe place for SendMessage(GUI), which BrokerMessage uses.
//
// Why: previously logf() called BrokerMessage directly from the WS thread,
// which is a synchronous SendMessage(GUI). When the GUI thread was busy, the
// WS thread blocked for hundreds of ms, starving message dispatch and causing
// price-cache staleness storms (OPM-550).
//
// Bounded queue: if producers outrun the consumer the oldest entries are
// dropped and a "messages dropped" counter is incremented. This is reported
// at flush time so the operator knows logging is being throttled.
// =============================================================================

using LogCallback = int(*)(const char*);

class Logger {
public:
    LogCallback callback = nullptr;          // set once at plugin load
    std::atomic<int> level{0};

    Logger();
    ~Logger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    // Producer API — safe to call from any thread.
    void log(int minLevel, const char* msg);
    void logf(int minLevel, const char* fmt, ...);
    void error(const char* msg) { log(1, msg); }
    void info(const char* msg)  { log(2, msg); }
    void debug(const char* msg) { log(3, msg); }

    // Enqueue a pre-filtered message (bypasses the internal level check).
    // Used as the logCallback target for ws_manager / ws_connection, which
    // do their own level filtering before calling out.
    void enqueue(const char* msg);

    // Consumer API — call from MAIN THREAD ONLY (Zorro / BrokerCommand path).
    // Drains up to maxMessages from the queue and forwards them to callback.
    // Returns the number of messages drained.
    size_t drain(size_t maxMessages = 256);

    // Synchronous flush — drain everything that's queued. Use on shutdown.
    void flush();

    // Diagnostics
    size_t queueDepth() const;
    uint64_t messagesDropped() const;

private:
    mutable CRITICAL_SECTION cs_;
    std::deque<std::string> queue_;
    static const size_t MAX_QUEUE_DEPTH = 10000;  // ~10 min at 17 msg/s
    std::atomic<uint64_t> dropped_{0};
    uint64_t lastReportedDrops_ = 0;  // accessed from main thread only (drain)
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

// Fatal error flag — set when WS detects an unrecoverable issue (e.g., toxic subscription).
// When true, all broker functions return failure so Zorro stops trading. [OPM-170]
extern std::atomic<bool> g_fatalError;
extern char g_fatalErrorMsg[256];

// =============================================================================
// INITIALIZATION / CLEANUP
// =============================================================================

void initGlobals();     // Call once at DLL load or BrokerOpen
void cleanupGlobals();  // Call at DLL unload

} // namespace hl
