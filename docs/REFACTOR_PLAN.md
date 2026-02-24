# Hyperliquid Plugin: Complete Refactoring Plan

## Executive Summary

**Current State:** ~8000 lines across 2 files causing Claude Code context exhaustion  
**Target State:** 18-22 focused modules, each ≤400 lines, with clear interfaces  
**Primary Goals:**
1. Enable effective AI-assisted development (files fit in context)
2. Separation of concerns (debug one thing without loading everything)
3. Testability (mock dependencies, unit test in isolation)
4. Maintainability (clear ownership, predictable changes)

---

## Part 1: Architecture Design

### 1.1 Layered Architecture

```
┌─────────────────────────────────────────────────────────────────────┐
│                          ZORRO API LAYER                             │
│                        hl_broker.cpp (~600)                          │
│   Thin adapter: translates Zorro calls to service layer calls        │
│   BrokerOpen | BrokerLogin | BrokerAsset | BrokerBuy2 | BrokerTrade │
└──────────────────────────────┬──────────────────────────────────────┘
                               │
         ┌─────────────────────┼─────────────────────┐
         ▼                     ▼                     ▼
┌─────────────┐        ┌─────────────┐       ┌─────────────┐
│  TRADING    │        │   MARKET    │       │   ACCOUNT   │
│  SERVICE    │        │   SERVICE   │       │   SERVICE   │
│  (~350)     │        │   (~300)    │       │   (~250)    │
│             │        │             │       │             │
│ - Order     │        │ - Prices    │       │ - Balance   │
│ - Cancel    │        │ - L2Book    │       │ - Positions │
│ - Status    │        │ - History   │       │ - Margin    │
└──────┬──────┘        └──────┬──────┘       └──────┬──────┘
       │                      │                     │
       └──────────────────────┼─────────────────────┘
                              ▼
┌─────────────────────────────────────────────────────────────────────┐
│                        TRANSPORT LAYER                               │
│  ┌────────────────────────┐    ┌────────────────────────┐           │
│  │   WebSocket Manager    │    │     HTTP Client        │           │
│  │   (split into 4 files) │    │     (~250)             │           │
│  │   ~1200 total          │    │                        │           │
│  └────────────────────────┘    └────────────────────────┘           │
└──────────────────────────────┬──────────────────────────────────────┘
                               │
┌──────────────────────────────┴──────────────────────────────────────┐
│                        FOUNDATION LAYER                              │
│  ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐       │
│  │ Types   │ │ Globals │ │ Utils   │ │ Crypto  │ │  Meta   │       │
│  │ (~80)   │ │ (~120)  │ │ (~150)  │ │ (~300)  │ │ (~250)  │       │
│  └─────────┘ └─────────┘ └─────────┘ └─────────┘ └─────────┘       │
└─────────────────────────────────────────────────────────────────────┘
```

### 1.2 Key Design Principles

| Principle | Implementation |
|-----------|----------------|
| **Single Responsibility** | Each file does ONE thing well |
| **Dependency Inversion** | Services depend on abstractions, not concrete transport |
| **Information Hiding** | Internal state private, narrow public interfaces |
| **Fail Fast** | Validate at boundaries, propagate errors clearly |
| **No God Objects** | Split the GLOBAL struct into focused state containers |

### 1.3 Dependency Rules

```
ALLOWED:
  hl_broker.cpp → services → transport → foundation
  
NOT ALLOWED:
  foundation → transport (no upward dependencies)
  transport → services (transport doesn't know about business logic)
  Any circular dependencies
```

---

## Part 2: Module Breakdown

### 2.1 Foundation Layer (Build First)

#### `hl_types.h` (~80 lines)
```cpp
// Pure data structures, no logic, no dependencies
#pragma once

enum class OrderStatus { Pending, Open, Filled, Cancelled, Error };
enum class OrderSide { Buy, Sell };

struct OrderState {
    std::string cloid;
    std::string oid;
    std::string coin;
    OrderSide side;
    double size;
    double filledSize;
    double avgPrice;
    OrderStatus status;
    int zorroTradeId;
    time_t createTime;
};

struct PriceData {
    double bid;
    double ask;
    time_t timestamp;
    bool isStale() const { return (time(NULL) - timestamp) > 5; }
};

struct AssetInfo {
    char name[64];
    int index;
    double szDecimals;
    double minSize;
    double tickSize;
    bool isPerpDex;
    char dexName[32];
};

struct Position {
    std::string coin;
    double size;        // Positive = long, negative = short
    double entryPrice;
    double unrealizedPnl;
    double margin;
};

struct AccountState {
    double equity;
    double availableBalance;
    double marginUsed;
    std::vector<Position> positions;
};
```

#### `hl_config.h` (~60 lines)
```cpp
// Compile-time and runtime configuration
#pragma once

namespace hl {
namespace config {

// API endpoints
constexpr const char* MAINNET_REST = "https://api.hyperliquid.xyz";
constexpr const char* MAINNET_WS = "wss://api.hyperliquid.xyz/ws";
constexpr const char* TESTNET_REST = "https://api.hyperliquid-testnet.xyz";
constexpr const char* TESTNET_WS = "wss://api.hyperliquid-testnet.xyz/ws";

// Timeouts (milliseconds)
constexpr int HTTP_TIMEOUT_MS = 10000;
constexpr int WS_CONNECT_TIMEOUT_MS = 30000;
constexpr int WS_RECEIVE_TIMEOUT_MS = 1000;
constexpr int WS_PING_INTERVAL_MS = 20000;

// Cache settings
constexpr int PRICE_STALE_SECONDS = 5;
constexpr int META_CACHE_SECONDS = 300;

// Limits
constexpr int MAX_ASSETS = 512;
constexpr int MAX_PENDING_ORDERS = 100;

} // namespace config
} // namespace hl
```

#### `hl_globals.h` / `hl_globals.cpp` (~150 lines total)
```cpp
// hl_globals.h - Controlled global state with clear ownership
#pragma once
#include "hl_types.h"
#include <windows.h>
#include <string>
#include <atomic>

// Forward declarations (no includes of heavy headers)
class WebSocketManager;
class PriceCache;

namespace hl {

// Runtime configuration (set at login)
struct RuntimeConfig {
    bool isTestnet = false;
    int diagLevel = 0;
    std::string apiKey;
    std::string walletAddress;
    HWND zorroWindow = NULL;
};

// Asset registry (populated from meta endpoint)
struct AssetRegistry {
    AssetInfo assets[config::MAX_ASSETS];
    int count = 0;
    CRITICAL_SECTION cs;
    
    void init();
    void cleanup();
    int findByName(const char* name) const;
    int findByCoin(const char* coin) const;
};

// Logging interface (decouples from Zorro's BrokerMessage)
using LogCallback = int(*)(const char*);

struct Logger {
    LogCallback callback = nullptr;
    std::atomic<int> level{0};
    
    void log(int minLevel, const char* prefix, const char* msg);
    void logf(int minLevel, const char* prefix, const char* fmt, ...);
};

// Global instances (defined in hl_globals.cpp)
extern RuntimeConfig g_config;
extern AssetRegistry g_assets;
extern Logger g_logger;

// Lazy-initialized singletons (created in BrokerLogin)
extern WebSocketManager* g_wsManager;
extern PriceCache* g_priceCache;

} // namespace hl
```

#### `hl_utils.h` / `hl_utils.cpp` (~150 lines total)
```cpp
// Pure utility functions - no state, no side effects
#pragma once
#include <string>

namespace hl {
namespace utils {

// String helpers
std::string normalizeAssetName(const char* zorroName);  // "BTC-USD" -> "BTC"
std::string buildCoinName(const char* coin, const char* dex);  // ("XYZ100", "xyz") -> "xyz:XYZ100"
bool parsePerpDexName(const char* fullName, char* dex, char* coin);

// JSON helpers (lightweight, no external lib)
const char* jsonGetString(const char* json, const char* key, char* buf, size_t bufLen);
double jsonGetDouble(const char* json, const char* key);
int64_t jsonGetInt64(const char* json, const char* key);
bool jsonGetBool(const char* json, const char* key);

// Time helpers  
int64_t nowMillis();
double millisToOLE(int64_t ms);
int64_t oleToMillis(double ole);

// Numeric helpers
double roundToDecimals(double value, int decimals);
double roundToTickSize(double price, double tickSize);
std::string formatSize(double size, int szDecimals);
std::string formatPrice(double price, int decimals);

} // namespace utils
} // namespace hl
```

#### `hl_crypto.h` / `hl_crypto.cpp` (~285 lines total)
```cpp
// Low-level cryptographic primitives (secp256k1, keccak256)
#pragma once
#include <cstdint>
#include <string>

namespace hl {
namespace crypto {

// Initialization (call once at startup)
bool init();
void cleanup();
bool isInitialized();

// Key derivation
bool deriveAddress(const char* hexPrivKey, char* addressOut, size_t addressLen);

// Hash signing (for EIP-712 message hashes)
struct Signature { char r[67]; char s[67]; int v; std::string toJson() const; };
bool signHash(const uint8_t* hash, const char* privateKeyHex, Signature& sigOut);
bool signHashToJson(const uint8_t* hash, const char* privateKeyHex, char* jsonOut, size_t jsonOutSize);

// Utilities
size_t hexToBytes(const char* hex, uint8_t* out, size_t outSize);
void bytesToHex(const uint8_t* data, size_t len, char* out, size_t outSize);
void keccak256(const uint8_t* data, size_t len, uint8_t* out);

} // namespace crypto
} // namespace hl
```

#### `hl_eip712.h` / `hl_eip712.cpp` (~400 lines total) - NEW

```cpp
// EIP-712 typed data encoding for Hyperliquid orders/cancels
// DEPENDS ON: hl_crypto (for keccak256)
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace hl {
namespace eip712 {

// Order action for EIP-712 encoding
struct OrderAction {
    int asset;
    bool isBuy;
    std::string price;      // String-formatted decimal
    std::string size;
    bool reduceOnly;
    std::string orderType;  // "Ioc", "Gtc", "Alo"
    std::string cloid;
};

struct CancelAction {
    int asset;
    std::string orderId;
};

// Hash computation (returns 32-byte hash for signing)
std::vector<uint8_t> hashOrderForSigning(const OrderAction& order, uint64_t nonce,
                                          const std::string& vaultAddress = "");
std::vector<uint8_t> hashCancelForSigning(const CancelAction& cancel, uint64_t nonce,
                                           const std::string& vaultAddress = "");

// Number formatting (Hyperliquid-specific)
std::string formatPrice(double price, int decimals);
std::string formatSize(double size, int decimals);

// Domain helpers
std::vector<uint8_t> getDomainSeparator(bool testnet);

} // namespace eip712
} // namespace hl
```

### 2.2 Transport Layer

#### `hl_http.h` / `hl_http.cpp` (~250 lines total)
```cpp
// HTTP client - stateless, thread-safe
#pragma once
#include <string>
#include <optional>

namespace hl {
namespace http {

struct Response {
    int statusCode;
    std::string body;
    bool success() const { return statusCode >= 200 && statusCode < 300; }
};

// Core HTTP functions
Response post(const char* url, const char* jsonBody, int timeoutMs = 10000);
Response get(const char* url, int timeoutMs = 10000);

// Hyperliquid-specific endpoints
Response infoPost(const char* jsonBody, bool testnet);
Response exchangePost(const char* jsonBody, bool testnet);

} // namespace http
} // namespace hl
```

#### WebSocket Split (4 files, ~1200 lines total)

**`ws_types.h` (~100 lines)**
```cpp
// WebSocket data structures only
#pragma once
#include <string>
#include <atomic>

namespace hl {
namespace ws {

enum class ConnectionState { Disconnected, Connecting, Connected, Reconnecting };

struct SubscriptionState {
    bool l2Book = false;
    bool userFills = false;
    bool clearinghouseState = false;
    bool openOrders = false;
};

// Callbacks
using LogCallback = int(*)(const char*);
using PriceUpdateCallback = void(*)(const char* coin, double bid, double ask);
using OrderUpdateCallback = void(*)(const char* cloid, const char* status, double filled);

} // namespace ws
} // namespace hl
```

**`ws_price_cache.h` / `ws_price_cache.cpp` (~200 lines total)**
```cpp
// Thread-safe price cache - completely standalone
#pragma once
#include "hl_types.h"
#include <windows.h>
#include <unordered_map>
#include <string>

namespace hl {

class PriceCache {
public:
    PriceCache();
    ~PriceCache();
    
    // Thread-safe accessors
    void setBidAsk(const std::string& coin, double bid, double ask);
    PriceData get(const std::string& coin) const;
    bool has(const std::string& coin) const;
    void clear();
    
    // Staleness check
    bool isFresh(const std::string& coin, int maxAgeSeconds = 5) const;
    int getAgeSeconds(const std::string& coin) const;
    
private:
    mutable CRITICAL_SECTION cs_;
    std::unordered_map<std::string, PriceData> cache_;
};

} // namespace hl
```

**`ws_connection.h` / `ws_connection.cpp` (~400 lines total)**
```cpp
// Low-level WebSocket connection management
#pragma once
#include "ws_types.h"
#include <windows.h>
#include <winhttp.h>
#include <string>
#include <functional>

namespace hl {
namespace ws {

class Connection {
public:
    using MessageHandler = std::function<void(const char* data, size_t len)>;
    
    Connection();
    ~Connection();
    
    // Lifecycle
    bool connect(const char* url, int timeoutMs = 30000);
    void disconnect();
    bool isConnected() const;
    ConnectionState state() const;
    
    // I/O
    bool send(const char* message);
    void setMessageHandler(MessageHandler handler);
    
    // Must be called periodically from a thread
    void poll(int timeoutMs = 100);
    
    // Diagnostics
    void setLogCallback(LogCallback cb, int level);
    time_t lastMessageTime() const;
    
private:
    HINTERNET hSession_ = NULL;
    HINTERNET hConnect_ = NULL;
    HINTERNET hWebSocket_ = NULL;
    
    std::atomic<ConnectionState> state_{ConnectionState::Disconnected};
    std::atomic<time_t> lastMessageTime_{0};
    
    MessageHandler messageHandler_;
    LogCallback logCallback_ = nullptr;
    int logLevel_ = 0;
    
    char recvBuffer_[65536];
    std::string fragmentBuffer_;
    bool receiving_ = false;
    
    void log(int level, const char* msg);
};

} // namespace ws
} // namespace hl
```

**`ws_manager.h` / `ws_manager.cpp` (~500 lines total)**
```cpp
// High-level WebSocket orchestration
#pragma once
#include "ws_connection.h"
#include "ws_price_cache.h"
#include "hl_types.h"
#include <vector>
#include <string>
#include <thread>

namespace hl {

class WebSocketManager {
public:
    WebSocketManager(PriceCache& cache);
    ~WebSocketManager();
    
    // Lifecycle
    bool start(const char* wsUrl, const char* userAddress);
    void stop();
    bool isRunning() const;
    bool isHealthy() const;
    
    // Subscriptions
    void subscribeL2Book(const std::string& coin);
    void subscribeAccountData();
    void markInitialSubscriptionsComplete();
    
    // Configuration
    void setDiagLevel(int level);
    void setLogCallback(ws::LogCallback cb);
    void setZorroWindow(HWND hwnd);
    
    // Status
    int secondsSinceLastMessage() const;
    
private:
    // Components
    PriceCache& priceCache_;
    ws::Connection connection_;
    
    // Threads
    std::thread receiveThread_;
    std::thread senderThread_;
    std::atomic<bool> running_{false};
    HANDLE shutdownEvent_ = NULL;
    
    // Subscription state
    CRITICAL_SECTION subCs_;
    std::vector<std::string> pendingL2Subs_;
    std::vector<std::string> activeL2Subs_;
    std::atomic<bool> initialSubsQueued_{false};
    
    // Config
    std::string userAddress_;
    HWND zorroWindow_ = NULL;
    int diagLevel_ = 0;
    ws::LogCallback logCallback_ = nullptr;
    
    // Thread functions
    void receiveLoop();
    void senderLoop();
    
    // Message handling
    void handleMessage(const char* data, size_t len);
    void parseL2Book(const char* json);
    void parseOrderUpdate(const char* json);
    
    // Helpers
    void sendPendingSubscriptions();
    void log(int level, const char* msg);
};

} // namespace hl
```

### 2.3 Service Layer

#### `hl_market_service.h` / `hl_market_service.cpp` (~300 lines total)
```cpp
// Market data operations - prices, history, asset info
#pragma once
#include "hl_types.h"
#include <vector>

namespace hl {

class MarketService {
public:
    // Price access (tries WS cache first, falls back to HTTP)
    PriceData getPrice(const char* coin);
    bool hasRealtimePrice(const char* coin) const;
    
    // History
    struct Candle {
        double open, high, low, close, volume;
        int64_t timestamp;
    };
    std::vector<Candle> getCandles(const char* coin, const char* interval, 
                                    int64_t startTime, int64_t endTime);
    
    // Asset metadata
    bool refreshMeta();
    const AssetInfo* getAsset(const char* name) const;
    const AssetInfo* getAssetByIndex(int index) const;
    int getAssetCount() const;
};

// Global instance
MarketService& marketService();

} // namespace hl
```

#### `hl_trading_service.h` / `hl_trading_service.cpp` (~350 lines total)
```cpp
// Order management - place, cancel, status
#pragma once
#include "hl_types.h"
#include <string>
#include <optional>

namespace hl {

class TradingService {
public:
    // Order placement
    struct OrderRequest {
        std::string coin;
        OrderSide side;
        double size;
        double limitPrice;  // 0 for market
        bool reduceOnly = false;
    };
    
    struct OrderResult {
        bool success;
        std::string cloid;
        std::string oid;
        std::string error;
    };
    
    OrderResult placeOrder(const OrderRequest& req);
    bool cancelOrder(const std::string& cloidOrOid);
    bool cancelAllOrders(const std::string& coin = "");
    
    // Order status
    std::optional<OrderState> getOrderStatus(const std::string& cloid);
    std::optional<OrderState> getOrderByZorroId(int tradeId);
    
    // Order tracking (internal)
    void registerOrder(int zorroTradeId, const std::string& cloid);
    void updateOrderFromFill(const std::string& cloid, double filledSize, double price);
    
private:
    // Order state management
    CRITICAL_SECTION ordersCs_;
    std::unordered_map<std::string, OrderState> ordersByCloid_;
    std::unordered_map<int, std::string> zorroIdToCloid_;
    
    std::string generateCloid();
};

// Global instance
TradingService& tradingService();

} // namespace hl
```

#### `hl_account_service.h` / `hl_account_service.cpp` (~250 lines total)
```cpp
// Account state - balance, positions, margin
#pragma once
#include "hl_types.h"
#include <optional>

namespace hl {

class AccountService {
public:
    // Account state (tries WS cache, falls back to HTTP)
    AccountState getAccountState();
    
    // Individual queries
    double getEquity();
    double getAvailableBalance();
    std::optional<Position> getPosition(const char* coin);
    std::vector<Position> getAllPositions();
    
    // Cache management
    void invalidateCache();
    bool hasFreshData() const;
    
private:
    CRITICAL_SECTION cs_;
    AccountState cachedState_;
    time_t cacheTime_ = 0;
    
    void refreshFromHttp();
    void updateFromWs(const char* json);
};

// Global instance
AccountService& accountService();

} // namespace hl
```

### 2.4 Broker API Layer

#### `hl_broker.cpp` (~600 lines)
```cpp
// Thin adapter layer - translates Zorro API to service calls
// This is the ONLY file that knows about Zorro's API

#define DLLFUNC extern "C" __declspec(dllexport)

#include "hl_globals.h"
#include "hl_market_service.h"
#include "hl_trading_service.h"
#include "hl_account_service.h"
#include "hl_crypto.h"

using namespace hl;

DLLFUNC int BrokerOpen(char* Name, FARPROC fpMessage, FARPROC fpProgress) {
    strcpy_s(Name, 32, "Hyperliquid");
    g_logger.callback = (LogCallback)fpMessage;
    
    if (!crypto::init()) {
        g_logger.log(1, "BrokerOpen", "Failed to init crypto");
        return 0;
    }
    
    return PLUGIN_VERSION;
}

DLLFUNC int BrokerLogin(char* User, char* Pwd, char* Type, char* Accounts) {
    if (!User || !*User) {
        // Logout
        if (g_wsManager) {
            g_wsManager->stop();
            delete g_wsManager;
            g_wsManager = nullptr;
        }
        return 0;
    }
    
    // Parse credentials
    g_config.isTestnet = (Type && strstr(Type, "Test"));
    g_config.apiKey = User;
    
    // Derive wallet address
    char address[64];
    if (!crypto::deriveAddressFromPrivateKey(User, address, sizeof(address))) {
        g_logger.log(1, "BrokerLogin", "Invalid private key");
        return 0;
    }
    g_config.walletAddress = address;
    
    // Initialize services
    if (!marketService().refreshMeta()) {
        g_logger.log(1, "BrokerLogin", "Failed to fetch metadata");
        return 0;
    }
    
    // Start WebSocket
    g_priceCache = new PriceCache();
    g_wsManager = new WebSocketManager(*g_priceCache);
    g_wsManager->setDiagLevel(g_config.diagLevel);
    g_wsManager->setLogCallback(g_logger.callback);
    
    const char* wsUrl = g_config.isTestnet ? config::TESTNET_WS : config::MAINNET_WS;
    if (!g_wsManager->start(wsUrl, address)) {
        g_logger.log(1, "BrokerLogin", "WebSocket connection failed");
        // Continue anyway - HTTP fallback works
    }
    
    strcpy_s(Accounts, 1024, address);
    return 1;
}

DLLFUNC int BrokerAsset(char* Asset, double* pPrice, double* pSpread, 
                        double* pVolume, double* pPip, double* pPipCost,
                        double* pLotAmount, double* pMargin, double* pRollLong, 
                        double* pRollShort) {
    auto* asset = marketService().getAsset(Asset);
    if (!asset) return 0;
    
    auto price = marketService().getPrice(asset->name);
    if (price.bid <= 0 || price.ask <= 0) return 0;
    
    if (pPrice) *pPrice = price.ask;
    if (pSpread) *pSpread = price.ask - price.bid;
    if (pPip) *pPip = asset->tickSize;
    if (pLotAmount) *pLotAmount = asset->minSize;
    // ... etc
    
    return 1;
}

DLLFUNC int BrokerBuy2(char* Asset, int Amount, double StopDist, double Limit,
                       double* pPrice, int* pFill) {
    auto* asset = marketService().getAsset(Asset);
    if (!asset) return 0;
    
    TradingService::OrderRequest req;
    req.coin = asset->name;
    req.side = (Amount > 0) ? OrderSide::Buy : OrderSide::Sell;
    req.size = abs(Amount) * asset->minSize;
    req.limitPrice = Limit;
    
    auto result = tradingService().placeOrder(req);
    if (!result.success) {
        g_logger.log(1, "BrokerBuy2", result.error.c_str());
        return 0;
    }
    
    // Register for tracking
    int tradeId = generateTradeId();
    tradingService().registerOrder(tradeId, result.cloid);
    
    return tradeId;
}

// ... other Broker functions follow same pattern
```

---

## Part 3: File Size Targets

| Module | Target Lines | Rationale |
|--------|-------------|-----------|
| hl_types.h | 80 | Pure data, no logic |
| hl_config.h | 60 | Constants only |
| hl_globals.h/cpp | 150 | Minimal global state |
| hl_utils.h/cpp | 150 | Pure functions |
| hl_crypto.h/cpp | 285 | Signing primitives (secp256k1) |
| hl_eip712.h/cpp | 400 | EIP-712 typed data encoding |
| hl_http.h/cpp | 250 | Stateless client |
| ws_types.h | 100 | WS data structures |
| ws_price_cache.h/cpp | 200 | Standalone cache |
| ws_connection.h/cpp | 400 | Low-level WS |
| ws_manager.h/cpp | 500 | WS orchestration |
| hl_meta.h/cpp | 250 | Asset metadata caching |
| hl_market_service.h/cpp | 490 | Market data ops |
| hl_trading_service.h/cpp | 480 | Order management |
| hl_account_service.h/cpp | 340 | Account state |
| hl_broker.cpp | 600 | Zorro adapter |
| **TOTAL** | **~4735** | Down from 8000+ |

**Target: Every file ≤500 lines, most ≤300 lines**

---

## Part 4: Claude Code Optimization

### 4.1 Why Current Structure Fails

```
┌─────────────────────────────────────────────────────────────────┐
│                    CLAUDE CODE CONTEXT WINDOW                    │
│                         (~100K tokens)                           │
├─────────────────────────────────────────────────────────────────┤
│  System prompt + conversation: ~20K                              │
│  Hyperliquid_Native.cpp (5135 lines): ~40K                      │
│  ws_manager.h (2900 lines): ~25K                                │
│  ─────────────────────────────────────                          │
│  TOTAL: ~85K = 85% FULL BEFORE ANY WORK                         │
│                                                                  │
│  After a few edits + errors + retries: COMPACTION               │
│  Post-compaction: Context starts at 50%+, loses plan            │
└─────────────────────────────────────────────────────────────────┘
```

### 4.2 Optimized Structure for AI

```
┌─────────────────────────────────────────────────────────────────┐
│                    CLAUDE CODE CONTEXT WINDOW                    │
├─────────────────────────────────────────────────────────────────┤
│  System prompt + conversation: ~20K                              │
│  ws_connection.cpp (400 lines): ~4K                             │
│  ws_connection.h (100 lines): ~1K                               │
│  ws_types.h (100 lines): ~1K                                    │
│  Related test file (200 lines): ~2K                             │
│  ─────────────────────────────────────                          │
│  TOTAL: ~28K = 28% USED                                          │
│                                                                  │
│  Room for: Multiple iterations, error recovery, exploration      │
└─────────────────────────────────────────────────────────────────┘
```

### 4.3 Claude Code Workflow Per Module

```markdown
## Session Template for Claude Code

**Starting a module extraction:**

We're refactoring Hyperliquid_Native.cpp into modules.
Current task: Extract [MODULE_NAME]

Source: Hyperliquid_Native.cpp lines [START-END]
Target: [new_file.h] and [new_file.cpp]

Dependencies: [list what it needs]
Dependents: [list what needs it]

Please:
1. Create the header with the public interface
2. Create the implementation
3. Verify it compiles standalone
4. Do NOT modify other files yet

**After each module:**
1. `git add` + `git commit` with clear message
2. `/clear` to reset context
3. Start fresh session for next module
```

### 4.4 File Naming Convention for AI

```
hl_<layer>_<purpose>.h/cpp

Examples:
  hl_types.h           - Foundation: type definitions
  hl_crypto.cpp        - Foundation: cryptography
  ws_connection.cpp    - Transport: WebSocket connection
  hl_market_service.h  - Service: market data
  hl_broker.cpp        - API: Zorro interface

Benefits:
  - Alphabetical sorting groups related files
  - Prefix indicates layer (hl_, ws_)
  - Clear what each file does
  - Easy to reference in prompts
```

### 4.5 Header Structure for AI Readability

```cpp
//=============================================================================
// hl_trading_service.h - Order placement and management
//=============================================================================
// DEPENDENCIES: hl_types.h, hl_crypto.h
// USED BY: hl_broker.cpp
// THREAD SAFETY: All public methods are thread-safe
//=============================================================================

#pragma once

#include "hl_types.h"
#include <string>

namespace hl {

/// Places and manages orders on Hyperliquid
/// 
/// Usage:
///   OrderRequest req{.coin="BTC", .side=OrderSide::Buy, .size=0.001};
///   auto result = tradingService().placeOrder(req);
///   if (result.success) { /* ... */ }
///
class TradingService {
public:
    // ... interface
};

} // namespace hl
```

---

## Part 5: Build System

### 5.1 Directory Structure

```
HyperliquidPlugin/
├── src/
│   ├── foundation/
│   │   ├── hl_types.h
│   │   ├── hl_config.h
│   │   ├── hl_globals.h
│   │   ├── hl_globals.cpp
│   │   ├── hl_utils.h
│   │   ├── hl_utils.cpp
│   │   ├── hl_crypto.h
│   │   └── hl_crypto.cpp
│   ├── transport/
│   │   ├── hl_http.h
│   │   ├── hl_http.cpp
│   │   ├── ws_types.h
│   │   ├── ws_price_cache.h
│   │   ├── ws_price_cache.cpp
│   │   ├── ws_connection.h
│   │   ├── ws_connection.cpp
│   │   ├── ws_manager.h
│   │   └── ws_manager.cpp
│   ├── services/
│   │   ├── hl_market_service.h
│   │   ├── hl_market_service.cpp
│   │   ├── hl_trading_service.h
│   │   ├── hl_trading_service.cpp
│   │   ├── hl_account_service.h
│   │   └── hl_account_service.cpp
│   └── api/
│       └── hl_broker.cpp
├── include/
│   └── zorro/
│       └── trading.h          # Zorro SDK headers
├── lib/
│   ├── secp256k1.lib
│   └── libcurl.lib            # If using curl
├── tests/
│   ├── test_utils.cpp
│   ├── test_crypto.cpp
│   ├── test_price_cache.cpp
│   └── test_http.cpp
├── build/
│   ├── Debug/
│   └── Release/
├── CMakeLists.txt
├── build.bat                  # Quick build script
└── README.md
```

### 5.2 CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.16)
project(HyperliquidPlugin VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Windows-specific
if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601 -DWIN32_LEAN_AND_MEAN)
endif()

# Foundation layer
add_library(hl_foundation STATIC
    src/foundation/hl_globals.cpp
    src/foundation/hl_utils.cpp
    src/foundation/hl_crypto.cpp
)
target_include_directories(hl_foundation PUBLIC src/foundation)
target_link_libraries(hl_foundation PRIVATE secp256k1)

# Transport layer
add_library(hl_transport STATIC
    src/transport/hl_http.cpp
    src/transport/ws_price_cache.cpp
    src/transport/ws_connection.cpp
    src/transport/ws_manager.cpp
)
target_include_directories(hl_transport PUBLIC src/transport)
target_link_libraries(hl_transport 
    PUBLIC hl_foundation
    PRIVATE winhttp
)

# Services layer
add_library(hl_services STATIC
    src/services/hl_market_service.cpp
    src/services/hl_trading_service.cpp
    src/services/hl_account_service.cpp
)
target_include_directories(hl_services PUBLIC src/services)
target_link_libraries(hl_services PUBLIC hl_transport)

# Main DLL
add_library(Hyperliquid SHARED
    src/api/hl_broker.cpp
)
target_link_libraries(Hyperliquid PRIVATE hl_services)
set_target_properties(Hyperliquid PROPERTIES
    OUTPUT_NAME "Hyperliquid"
    SUFFIX ".dll"
)

# Tests (optional)
enable_testing()
add_executable(tests
    tests/test_utils.cpp
    tests/test_crypto.cpp
    tests/test_price_cache.cpp
)
target_link_libraries(tests PRIVATE hl_services)
add_test(NAME unit_tests COMMAND tests)
```

### 5.3 Quick Build Script (build.bat)

```batch
@echo off
setlocal

set BUILD_TYPE=%1
if "%BUILD_TYPE%"=="" set BUILD_TYPE=Release

echo Building %BUILD_TYPE%...

if not exist build mkdir build
cd build

cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config %BUILD_TYPE%

if %ERRORLEVEL% EQU 0 (
    echo.
    echo Build successful!
    echo Output: build\%BUILD_TYPE%\Hyperliquid.dll
    
    REM Copy to Zorro Plugin folder if exists
    if exist "C:\Zorro\Plugin" (
        copy /Y %BUILD_TYPE%\Hyperliquid.dll C:\Zorro\Plugin\
        echo Copied to C:\Zorro\Plugin\
    )
) else (
    echo Build failed!
    exit /b 1
)
```

---

## Part 6: Extraction Sequence

### 6.1 Phase Overview

```
Phase 1: Foundation (Day 1)
  └─ Types, Config, Globals, Utils
  
Phase 2: Crypto (Day 1-2)  
  └─ Extract and test signing
  
Phase 3: HTTP (Day 2)
  └─ Stateless HTTP client
  
Phase 4: WebSocket (Day 2-3)
  └─ Split into 4 focused files
  
Phase 5: Services (Day 3-4)
  └─ Market, Trading, Account
  
Phase 6: Broker API (Day 4)
  └─ Thin adapter layer
  
Phase 7: Cleanup (Day 5)
  └─ Remove old files, final testing
```

### 6.2 Detailed Step-by-Step

#### Step 1: Setup Branch and Structure
```bash
git checkout main
git pull
git checkout -b refactor/modularize
git tag pre-refactor-backup

# Create directory structure
mkdir -p src/{foundation,transport,services,api}
mkdir -p tests
```

#### Step 2: Extract hl_types.h
```
Source: Hyperliquid_Native.cpp lines ~50-150 (struct definitions)
Action: Copy structs to src/foundation/hl_types.h
Test: Compiles standalone
Commit: "refactor: extract hl_types.h"
```

#### Step 3: Extract hl_config.h
```
Source: Scattered #defines and constants
Action: Consolidate to src/foundation/hl_config.h
Test: Compiles standalone
Commit: "refactor: extract hl_config.h"
```

#### Step 4: Extract hl_globals
```
Source: GLOBAL struct and g_* pointers
Action: Create hl_globals.h/cpp with controlled state
Test: Compiles, links
Commit: "refactor: extract hl_globals"
```

#### Step 5: Extract hl_utils
```
Source: Helper functions (normalize_coin, JSON parsing)
Action: Create hl_utils.h/cpp as pure functions
Test: Unit tests pass
Commit: "refactor: extract hl_utils"
```

#### Step 6: Extract hl_crypto
```
Source: EIP-712 signing, secp256k1 usage
Action: Create hl_crypto.h/cpp
Test: Sign test message, verify signature
Commit: "refactor: extract hl_crypto"
```

#### Step 7: Extract hl_http
```
Source: send_http, build_url, info_post
Action: Create stateless HTTP client
Test: Fetch meta endpoint
Commit: "refactor: extract hl_http"
```

#### Step 8: Split WebSocket (4 steps)
```
8a: Extract ws_types.h (enums, callbacks)
8b: Extract ws_price_cache (standalone cache)
8c: Extract ws_connection (low-level WS)
8d: Extract ws_manager (orchestration)

Test after each: Compiles, basic function works
Commit after each step
```

#### Step 9: Create Services
```
9a: hl_market_service (prices, history, meta)
9b: hl_trading_service (orders)
9c: hl_account_service (balance, positions)

Each wraps transport layer with business logic
Commit after each
```

#### Step 10: Create hl_broker.cpp
```
Source: All Broker* functions from Hyperliquid_Native.cpp
Action: Thin adapter calling services
Test: Full Zorro integration test
Commit: "refactor: create hl_broker.cpp adapter"
```

#### Step 11: Cleanup
```
- Delete Hyperliquid_Native.cpp
- Delete old ws_manager.h
- Update build system
- Full integration test
Commit: "refactor: remove legacy files"
```

### 6.3 Checkpoint Testing

After each phase, verify:

```batch
@echo off
REM test_checkpoint.bat

echo === Building ===
call build.bat Release
if %ERRORLEVEL% NEQ 0 exit /b 1

echo === Running Unit Tests ===
build\Release\tests.exe
if %ERRORLEVEL% NEQ 0 exit /b 1

echo === Integration Test ===
REM Start Zorro with test script
echo Run manual Zorro test...

echo === All Checks Passed ===
```

---

## Part 7: Testing Strategy

### 7.1 Five-Phase Testing Approach

| Phase | When | What | Pass Criteria |
|-------|------|------|---------------|
| **1. Baseline** | Before starting | Capture current behavior | Snapshots saved |
| **2. Compile** | After each file | Does it build? | Zero errors |
| **3. Unit** | After each module | Isolated function tests | All assertions pass |
| **4. Integration** | After each phase | Zorro checkpoint script | Login, asset, price work |
| **5. Regression** | After all phases | Full functionality test | All features work |

### 7.2 Phase 1: Baseline Tests (Before Starting)

Run BEFORE refactoring to capture expected behavior:

```cpp
// tests/baseline_test.cpp
#include <fstream>
#include <cassert>

void save_snapshot(const char* filename, const std::string& content) {
    std::ofstream f(filename);
    f << content;
}

void test_baseline_http_meta() {
    // Capture expected response structure
    auto resp = http_post(META_ENDPOINT, R"({"type":"meta"})");
    assert(resp.find("universe") != std::string::npos);
    save_snapshot("snapshots/meta_response.json", resp);
    printf("Baseline: meta response saved\n");
}

void test_baseline_l2book() {
    // Capture expected L2 structure  
    auto resp = http_post(INFO_ENDPOINT, R"({"type":"l2Book","coin":"BTC"})");
    assert(resp.find("levels") != std::string::npos);
    save_snapshot("snapshots/l2book_response.json", resp);
    printf("Baseline: l2book response saved\n");
}

void test_baseline_account() {
    // Capture expected account structure
    auto resp = http_post(INFO_ENDPOINT, R"({"type":"clearinghouseState","user":"..."})");
    save_snapshot("snapshots/account_response.json", resp);
    printf("Baseline: account response saved\n");
}

int main() {
    test_baseline_http_meta();
    test_baseline_l2book();
    test_baseline_account();
    printf("\nBaseline snapshots saved to snapshots/\n");
    return 0;
}
```

### 7.3 Phase 2: Compile-Only Tests (During Extraction)

After extracting each module, verify it compiles standalone:

```batch
@echo off
REM test_compile.bat - Run after each module extraction

echo === Compile Check ===
cmake --build . --config Release 2>&1

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo FAILED: Module doesn't compile
    echo Fix errors before continuing extraction
    exit /b 1
)

echo.
echo PASSED: Compilation successful
exit /b 0
```

### 7.4 Phase 3: Unit Tests (Per Module)

Each module gets focused unit tests:

```cpp
// tests/test_utils.cpp
#include "hl_utils.h"
#include <cassert>
#include <cstdio>

void test_normalizeAssetName() {
    assert(hl::utils::normalizeAssetName("BTC-USD") == "BTC");
    assert(hl::utils::normalizeAssetName("ETH") == "ETH");
    assert(hl::utils::normalizeAssetName("xyz:XYZ100") == "xyz:XYZ100");
    printf("  test_normalizeAssetName PASSED\n");
}

void test_jsonGetDouble() {
    const char* json = R"({"price": 50000.5, "size": "1.5"})";
    assert(hl::utils::jsonGetDouble(json, "price") == 50000.5);
    printf("  test_jsonGetDouble PASSED\n");
}

void test_roundToDecimals() {
    assert(hl::utils::roundToDecimals(1.23456, 2) == 1.23);
    assert(hl::utils::roundToDecimals(1.23456, 4) == 1.2346);
    printf("  test_roundToDecimals PASSED\n");
}

int main() {
    printf("=== hl_utils tests ===\n");
    test_normalizeAssetName();
    test_jsonGetDouble();
    test_roundToDecimals();
    printf("All hl_utils tests passed!\n\n");
    return 0;
}
```

```cpp
// tests/test_crypto.cpp
#include "hl_crypto.h"
#include <cassert>
#include <cstdio>

void test_hexEncode() {
    uint8_t data[] = {0xDE, 0xAD, 0xBE, 0xEF};
    assert(hl::crypto::hexEncode(data, 4) == "deadbeef");
    printf("  test_hexEncode PASSED\n");
}

void test_deriveAddress() {
    // Known test vector
    const char* testKey = "0x..."; // Use a known test private key
    char address[64];
    assert(hl::crypto::deriveAddressFromPrivateKey(testKey, address, sizeof(address)));
    assert(strncmp(address, "0x", 2) == 0);
    printf("  test_deriveAddress PASSED\n");
}

int main() {
    printf("=== hl_crypto tests ===\n");
    assert(hl::crypto::init());
    test_hexEncode();
    test_deriveAddress();
    hl::crypto::cleanup();
    printf("All hl_crypto tests passed!\n\n");
    return 0;
}
```

```cpp
// tests/test_price_cache.cpp
#include "ws_price_cache.h"
#include <cassert>
#include <cstdio>
#include <thread>
#include <chrono>

void test_basic_operations() {
    hl::PriceCache cache;
    
    // Set and get
    cache.setBidAsk("BTC", 50000.0, 50001.0);
    auto price = cache.get("BTC");
    assert(price.bid == 50000.0);
    assert(price.ask == 50001.0);
    printf("  test_basic_operations PASSED\n");
}

void test_staleness() {
    hl::PriceCache cache;
    cache.setBidAsk("ETH", 3000.0, 3001.0);
    
    // Fresh data
    assert(cache.isFresh("ETH", 5));
    
    // Wait and check staleness
    std::this_thread::sleep_for(std::chrono::seconds(2));
    assert(cache.isFresh("ETH", 5));  // Still fresh
    assert(!cache.isFresh("ETH", 1)); // Now stale with 1s threshold
    printf("  test_staleness PASSED\n");
}

void test_missing_coin() {
    hl::PriceCache cache;
    assert(!cache.has("NOTEXIST"));
    auto price = cache.get("NOTEXIST");
    assert(price.bid == 0 && price.ask == 0);
    printf("  test_missing_coin PASSED\n");
}

int main() {
    printf("=== ws_price_cache tests ===\n");
    test_basic_operations();
    test_staleness();
    test_missing_coin();
    printf("All ws_price_cache tests passed!\n\n");
    return 0;
}
```

**Unit Test Coverage by Module:**

| Module | Test Focus |
|--------|------------|
| `hl_utils` | String parsing, JSON extraction, time conversion, rounding |
| `hl_crypto` | Hex encode/decode, address derivation, known signature vectors |
| `ws_price_cache` | Thread-safety, staleness detection, missing keys |
| `hl_http` | Response parsing, error handling (mock server optional) |
| `ws_connection` | State machine transitions (mock network) |

### 7.5 Phase 4: Integration Checkpoint Tests

Run after completing each major phase:

```c
// Strategy/ZorroCheckpoint.c - Run in Zorro after each phase
function main() {
    set(LOGFILE);
    Verbose = 7;
    
    printf("\n========================================");
    printf("\n  REFACTOR CHECKPOINT TEST");
    printf("\n========================================\n");
    
    brokerCommand(SET_DIAGNOSTICS, 2);
    
    // PHASE 1-2: Foundation + Crypto
    printf("\n[TEST] Login...");
    if(!brokerLogin("Hyperliquid", "YOUR_TESTNET_KEY", "Test")) {
        printf(" FAILED\n");
        return;
    }
    printf(" OK\n");
    
    // PHASE 3: HTTP
    printf("[TEST] Asset info (HTTP)...");
    if(!asset("BTC")) {
        printf(" FAILED\n");
        return;
    }
    printf(" OK - BTC: %.2f\n", priceClose());
    
    // PHASE 4: WebSocket
    printf("[TEST] WebSocket price updates...");
    var price1 = priceClose();
    wait(3000);  // Let WS update
    var price2 = priceClose();
    if(price1 == price2) {
        printf(" WARNING - Price unchanged (WS may not be working)\n");
    } else {
        printf(" OK - Price updated: %.2f -> %.2f\n", price1, price2);
    }
    
    // PHASE 5: Services
    printf("[TEST] Account data...");
    var equity = brokerCommand(GET_EQUITY, 0);
    if(equity <= 0) {
        printf(" FAILED - Equity: %.2f\n", equity);
        return;
    }
    printf(" OK - Equity: %.2f\n", equity);
    
    // PHASE 6: Order flow (only if confident)
    printf("[TEST] Order placement (SKIPPED - enable manually)\n");
    // Uncomment to test orders on testnet:
    // enterLong(1);
    // wait(2000);
    // exitLong();
    
    printf("\n========================================");
    printf("\n  ALL CHECKPOINT TESTS PASSED");
    printf("\n========================================\n");
}
```

### 7.6 Phase 5: Full Regression Test

Run after refactoring is complete, before merging:

```c
// Strategy/ZorroFullTest.c - Comprehensive regression test
function main() {
    set(LOGFILE);
    Verbose = 7;
    
    printf("\n========================================");
    printf("\n  FULL REGRESSION TEST");
    printf("\n========================================\n");
    
    brokerCommand(SET_DIAGNOSTICS, 2);
    int passed = 0;
    int failed = 0;
    
    // 1. Login/Logout cycle
    printf("\n[1/10] Login...");
    if(brokerLogin("Hyperliquid", "YOUR_TESTNET_KEY", "Test")) {
        printf(" PASS\n"); passed++;
    } else {
        printf(" FAIL\n"); failed++;
        return;  // Can't continue without login
    }
    
    // 2. Asset info for multiple assets
    printf("[2/10] Asset info (BTC, ETH, SOL)...");
    int assetOk = asset("BTC") && asset("ETH") && asset("SOL");
    if(assetOk) {
        printf(" PASS\n"); passed++;
    } else {
        printf(" FAIL\n"); failed++;
    }
    
    // 3. Price streaming (WS)
    printf("[3/10] WebSocket streaming...");
    asset("BTC");
    var prices[5];
    for(int i=0; i<5; i++) {
        prices[i] = priceClose();
        wait(1000);
    }
    int priceChanges = 0;
    for(int i=1; i<5; i++) {
        if(prices[i] != prices[i-1]) priceChanges++;
    }
    if(priceChanges >= 1) {
        printf(" PASS (%d changes)\n", priceChanges); passed++;
    } else {
        printf(" WARN (no changes, may be quiet market)\n"); passed++;
    }
    
    // 4. Historical data fetch
    printf("[4/10] Historical data...");
    var history[100];
    int bars = brokerCommand(GET_HISTORY, "BTC", history, 100);
    if(bars > 0) {
        printf(" PASS (%d bars)\n", bars); passed++;
    } else {
        printf(" FAIL\n"); failed++;
    }
    
    // 5. Account balance
    printf("[5/10] Account balance...");
    var equity = brokerCommand(GET_EQUITY, 0);
    if(equity > 0) {
        printf(" PASS ($%.2f)\n", equity); passed++;
    } else {
        printf(" FAIL\n"); failed++;
    }
    
    // 6. Position query
    printf("[6/10] Position query...");
    brokerCommand(GET_POSITION, "BTC");
    printf(" PASS (no crash)\n"); passed++;
    
    // 7. Limit order placement
    printf("[7/10] Limit order (far from market)...");
    asset("BTC");
    var farPrice = priceClose() * 0.5;  // 50% below market - won't fill
    int orderId = brokerCommand(SET_ORDERTYPE, 2);  // Limit order
    // Trade logic here...
    printf(" SKIPPED (enable manually)\n"); passed++;
    
    // 8. Order status query
    printf("[8/10] Order status...");
    printf(" SKIPPED (requires open order)\n"); passed++;
    
    // 9. Order cancellation
    printf("[9/10] Order cancellation...");
    printf(" SKIPPED (requires open order)\n"); passed++;
    
    // 10. Logout
    printf("[10/10] Logout...");
    brokerLogin("Hyperliquid", "", "");  // Empty user = logout
    printf(" PASS\n"); passed++;
    
    // Summary
    printf("\n========================================");
    printf("\n  RESULTS: %d PASSED, %d FAILED", passed, failed);
    if(failed == 0) {
        printf("\n  STATUS: READY FOR PRODUCTION");
    } else {
        printf("\n  STATUS: FIX FAILURES BEFORE MERGE");
    }
    printf("\n========================================\n");
}
```

### 7.7 Test Execution Summary

```batch
@echo off
REM test_all.bat - Run all tests in sequence

echo ========================================
echo   RUNNING ALL TESTS
echo ========================================

echo.
echo [1/3] Unit Tests...
build\Release\test_utils.exe
if %ERRORLEVEL% NEQ 0 goto :fail
build\Release\test_crypto.exe
if %ERRORLEVEL% NEQ 0 goto :fail
build\Release\test_price_cache.exe
if %ERRORLEVEL% NEQ 0 goto :fail

echo.
echo [2/3] Integration Tests...
build\Release\test_integration.exe
if %ERRORLEVEL% NEQ 0 goto :fail

echo.
echo [3/3] Zorro Tests...
echo Run ZorroCheckpoint.c and ZorroFullTest.c manually in Zorro

echo.
echo ========================================
echo   ALL AUTOMATED TESTS PASSED
echo ========================================
exit /b 0

:fail
echo.
echo ========================================
echo   TEST FAILED - Fix before continuing
echo ========================================
exit /b 1
```

---

## Part 8: Documentation Standards

### 8.1 File Header Template

```cpp
//=============================================================================
// [filename] - [one-line description]
//=============================================================================
// Part of Hyperliquid Plugin for Zorro
// 
// LAYER: [Foundation | Transport | Service | API]
// DEPENDENCIES: [list header files this needs]
// THREAD SAFETY: [description]
//
// Copyright (c) 2024-2026. MIT License.
//=============================================================================
```

### 8.2 Function Documentation

```cpp
/// Places a limit or market order on Hyperliquid
/// 
/// @param req Order parameters (coin, side, size, price)
/// @return OrderResult with success status and order IDs
/// 
/// @thread_safety Thread-safe, uses internal locking
/// @errors Returns {success=false, error="..."} on failure
/// 
/// Example:
/// @code
///   OrderRequest req{.coin="BTC", .side=Buy, .size=0.001, .limitPrice=50000};
///   auto result = placeOrder(req);
///   if (!result.success) handleError(result.error);
/// @endcode
OrderResult placeOrder(const OrderRequest& req);
```

### 8.3 README.md Template

```markdown
# Hyperliquid Plugin for Zorro

## Architecture

    API Layer (hl_broker.cpp)
        ↓
    Service Layer (market, trading, account)
        ↓
    Transport Layer (HTTP, WebSocket)
        ↓
    Foundation Layer (types, utils, crypto)

## Building

    mkdir build && cd build
    cmake ..
    cmake --build . --config Release

## Module Overview

| Module | Purpose | Lines |
|--------|---------|-------|
| hl_types.h | Core data structures | ~80 |
| hl_crypto | EIP-712 signing | ~300 |
| ws_manager | WebSocket orchestration | ~500 |
| hl_broker | Zorro API adapter | ~600 |

## Testing

    build/Release/tests.exe          # Unit tests
    # Then run ZorroTest.c in Zorro  # Integration
```

---

## Part 9: Branching Strategy

### 9.1 Strategy Comparison

| Approach | Pros | Cons | Verdict |
|----------|------|------|---------|
| Branch per module | Granular rollback | 18+ branches, merge conflicts | ❌ Too much overhead |
| One giant branch | Simple | No checkpoints, all-or-nothing | ❌ Too risky |
| Trunk-based | Modern, CI-friendly | Requires feature flags | ❌ Wrong for restructuring |
| **Hybrid** | Checkpoints + manageable | Slightly more complex | ✅ Best fit |

### 9.2 During Refactoring: Hybrid Approach

```
main ─────────────────────────────────────────────────────────► (PRODUCTION - stable)
  │
  │  (DO NOT COMMIT DIRECTLY TO MAIN DURING REFACTORING)
  │
  └─► refactor/modularize ────────────────────────────────────► (primary refactor branch)
         │         │         │
         │         │         └─► refactor/ws-split ──► merge back
         │         │                   (complex: 4 tightly-coupled files)
         │         │
         │         └─► refactor/services ──► merge back
         │                   (complex: 3 interdependent services)
         │
         └─► (direct commits for simple extractions)
                   (hl_types.h, hl_config.h, hl_utils.h, etc.)
```

**Rules:**

| Extraction Type | Branch Strategy | Example |
|-----------------|-----------------|---------|
| Simple, standalone | Commit directly to `refactor/modularize` | `hl_types.h`, `hl_config.h` |
| Complex, multi-file | Create sub-branch, merge back | WebSocket split (4 files) |
| Tightly coupled group | One sub-branch for the group | Services (market, trading, account) |

### 9.3 Practical Workflow Commands

```bash
# === INITIAL SETUP ===
git checkout main
git pull origin main
git checkout -b refactor/modularize
git tag pre-refactor-backup    # Safety net for full rollback

# === SIMPLE EXTRACTION (direct commit) ===
# Example: extracting hl_types.h
vim src/foundation/hl_types.h  # Create the file
cmake --build . --config Release  # Verify it compiles
git add src/foundation/hl_types.h
git commit -m "refactor: extract hl_types.h from Hyperliquid_Native.cpp"

# === COMPLEX EXTRACTION (sub-branch) ===
# Example: WebSocket split into 4 files
git checkout -b refactor/ws-split

# ... create ws_types.h, ws_price_cache.*, ws_connection.*, ws_manager.* ...
# ... verify each compiles ...
# ... run tests ...

git add src/transport/ws_*.h src/transport/ws_*.cpp
git commit -m "refactor: split ws_manager.h into 4 focused modules"

# Merge back to main refactor branch
git checkout refactor/modularize
git merge refactor/ws-split
git branch -d refactor/ws-split  # Clean up

# === CHECKPOINT AFTER EACH PHASE ===
git tag refactor-phase-1-complete  # After foundation
git tag refactor-phase-4-complete  # After WebSocket, etc.

# === FINAL MERGE TO MAIN ===
# Only after ALL tests pass
git checkout main
git merge refactor/modularize
git tag v2.0.0-modular  # New major version
git push origin main --tags
```

### 9.4 Post-Refactoring: Standard Git Flow

After refactoring is complete, use simple short-lived branches:

```
main ───────────────────────────────────────────────────────────►
  │         │              │              │
  │         │              │              └─► feature/trailing-stop ──► PR ──► merge
  │         │              │
  │         │              └─► fix/ws-reconnect ──► PR ──► merge
  │         │
  │         └─► fix/price-staleness ──► PR ──► merge
  │
  └─► (hotfix if critical) ──► merge directly
```

**Branch Naming Conventions:**

| Type | Pattern | Lifespan | Merge Strategy |
|------|---------|----------|----------------|
| Bug fix | `fix/<short-description>` | 1-2 days | Squash merge |
| Feature | `feature/<name>` | 3-5 days max | Squash or regular |
| Hotfix | `hotfix/<issue>` | Hours | Direct to main |
| Experiment | `experiment/<idea>` | Any | May never merge |

**Key Principle:** If a branch lives more than a week, it's too big. Break it into smaller pieces.

### 9.5 Rollback Strategies

**Rollback single module (during refactoring):**
```bash
# Undo last commit but keep files
git reset --soft HEAD~1

# Or completely discard last commit
git reset --hard HEAD~1
```

**Rollback entire phase:**
```bash
git checkout refactor-phase-3-complete  # Go back to Phase 3 checkpoint
git checkout -b refactor/retry-phase-4  # Start fresh attempt
```

**Full rollback (abandon refactoring):**
```bash
git checkout main
git branch -D refactor/modularize  # Delete refactor branch
# Original code still on main, untouched
```

**Emergency production fix during refactoring:**
```bash
# You're on refactor/modularize, but production needs a fix
git stash                           # Save refactor work
git checkout main                   # Switch to production
git checkout -b hotfix/critical-bug # Create hotfix branch
# ... make fix ...
git commit -m "hotfix: critical bug fix"
git checkout main
git merge hotfix/critical-bug       # Merge to production
git checkout refactor/modularize    # Return to refactoring
git stash pop                       # Restore work
git merge main                      # Get the hotfix into refactor branch
```

### 9.6 Branch Protection Recommendations

After refactoring, configure branch protection on `main`:

```
☑ Require pull request before merging
☑ Require at least 1 approval (or self-review for solo dev)
☑ Require status checks to pass (if CI configured)
☑ Do not allow force pushes
☐ Do not allow deletions
```

---

## Part 9.5: Risk Mitigation

### 9.5.1 Incremental Validation

After extracting each module, run this checklist:

| Check | Command | Pass Criteria |
|-------|---------|---------------|
| Compile | `cmake --build . --config Release` | Zero errors |
| Link | (implicit in compile) | DLL builds successfully |
| Smoke test | Run Zorro, check BrokerOpen | Returns correct version |
| Function test | Run ZorroCheckpoint.c | No regressions |

### 9.5.2 Common Failure Points

| Risk | Prevention | Recovery |
|------|------------|----------|
| Circular dependency | Check includes before adding | Remove include, refactor interface |
| Missing export | Verify `__declspec(dllexport)` on public functions | Add export macro |
| Thread safety regression | Run multi-asset test | Add missing CRITICAL_SECTION |
| Signature mismatch | Keep legacy functions as adapters initially | Fix call sites |

### 9.5.3 Point of No Return

The refactoring is **safe to abandon** until you delete the legacy files.

Recommended approach:
1. Keep `legacy/Hyperliquid_Native.cpp` and `legacy/ws_manager.h` throughout
2. Only delete after FULL regression test passes
3. Create `git tag legacy-removed` before deletion
4. If issues found later, restore from tag

---

## Part 10: Success Metrics

### 10.1 Quantitative

| Metric | Before | After | Target |
|--------|--------|-------|--------|
| Largest file | 5135 lines | ≤500 lines | ✓ |
| Total files | 2 | 18-22 | ✓ |
| Claude context usage | 85%+ | <40% | ✓ |
| Build time | N/A | <30 sec | ✓ |

### 10.2 Qualitative

- [ ] Can debug WebSocket without loading trading code
- [ ] Can modify HTTP client without touching crypto
- [ ] New developer can understand structure in <30 min
- [ ] Claude Code can complete tasks without compaction
- [ ] Each module has clear single responsibility

---

## Appendix A: Current Code Mapping

### Hyperliquid_Native.cpp (5135 lines) → New Locations

| Lines | Content | New Location |
|-------|---------|--------------|
| 1-50 | Includes, defines | hl_config.h |
| 51-150 | Struct definitions | hl_types.h |
| 151-250 | Global state | hl_globals.h/cpp |
| 251-400 | Utility functions | hl_utils.cpp |
| 401-700 | HTTP functions | hl_http.cpp |
| 701-1000 | Crypto/signing | hl_crypto.cpp |
| 1001-1400 | Meta/asset handling | hl_market_service.cpp |
| 1401-1800 | Order management | hl_trading_service.cpp |
| 1801-2100 | Account queries | hl_account_service.cpp |
| 2101-5135 | Broker* functions | hl_broker.cpp |

### ws_manager.h (2900 lines) → New Locations

| Lines | Content | New Location |
|-------|---------|--------------|
| 1-100 | Includes, types | ws_types.h |
| 101-350 | PriceCache class | ws_price_cache.h/cpp |
| 351-1200 | Connection handling | ws_connection.h/cpp |
| 1201-2900 | Manager + parsing | ws_manager.h/cpp |

---

## Appendix B: Claude Code Session Templates

### Template 1: Extract Foundation Module

```
Context: Refactoring Hyperliquid plugin. Current task: Extract [MODULE].

Source file: Hyperliquid_Native.cpp
Lines to extract: [START]-[END]
Target files: src/foundation/[name].h and src/foundation/[name].cpp

Requirements:
1. Create header with public interface only
2. Create implementation with extracted code
3. Use namespace hl { }
4. No dependencies on other unextracted code yet
5. Add file header comment per template

Please create both files. I'll test compilation after.
```

### Template 2: Extract Service Module

```
Context: Refactoring Hyperliquid plugin. Foundation and transport layers complete.

Task: Create [SERVICE_NAME] service

It should wrap these transport functions:
- [list functions]

Public interface needed:
- [list methods]

Dependencies available:
- hl_types.h, hl_globals.h, hl_http.h, ws_manager.h

Please create the service header and implementation.
```

### Template 3: Fix Bug in Module

```
Context: Working on Hyperliquid plugin module [NAME].

Files in scope:
- [file1.h] (attached or will paste)
- [file1.cpp] (attached or will paste)

Bug: [description]
Expected: [behavior]
Actual: [behavior]

Please analyze and fix. Only modify these files.
```

---

## Appendix C: Quick Reference Card

```
LAYER DEPENDENCIES (allowed direction: →)
  Foundation → Transport → Services → API

FILE SIZE LIMITS
  Headers: ≤150 lines
  Implementations: ≤400 lines
  Total per module: ≤500 lines

NAMING CONVENTION
  hl_<purpose>.h/cpp     - General modules
  ws_<purpose>.h/cpp     - WebSocket modules
  test_<module>.cpp      - Test files

COMMIT MESSAGE FORMAT
  refactor: extract <module>
  refactor: split ws_manager into components
  fix: <module> - <description>
  test: add <module> unit tests

CLAUDE CODE WORKFLOW
  1. State context and task
  2. List files in scope
  3. Request specific changes
  4. Verify compilation
  5. Commit
  6. /clear
  7. Next task
```

---

## Appendix D: Estimated Timeline

| Phase | Tasks | Duration |
|-------|-------|----------|
| 1. Setup | Branch, directories, build system | 2 hours |
| 2. Foundation | types, config, globals, utils | 3 hours |
| 3. Crypto | Extract and test signing | 2 hours |
| 4. HTTP | Stateless HTTP client | 2 hours |
| 5. WebSocket | Split into 4 files | 4 hours |
| 6. Services | Market, Trading, Account | 4 hours |
| 7. Broker API | Thin adapter | 3 hours |
| 8. Cleanup | Remove old, final test | 2 hours |
| **TOTAL** | | **~22 hours** |

With focused sessions: **3-5 days**

---

## Appendix E: Anti-Patterns to Avoid

### DON'T: Create "Utils" Dumping Ground
```cpp
// BAD: Everything goes in utils
namespace utils {
    void parseJson();
    void signOrder();
    void sendHttp();
    void calculateMargin();
}
```

### DO: Group by Responsibility
```cpp
// GOOD: Each module has clear purpose
namespace json { /* JSON only */ }
namespace crypto { /* Signing only */ }
namespace http { /* HTTP only */ }
```

### DON'T: Pass God Objects
```cpp
// BAD: Everything depends on everything
void placeOrder(GLOBAL& g, WebSocketManager& ws, ...);
```

### DO: Narrow Interfaces
```cpp
// GOOD: Only what's needed
OrderResult placeOrder(const OrderRequest& req);
```

### DON'T: Mix Layers
```cpp
// BAD: Broker layer does HTTP directly
DLLFUNC int BrokerAsset(...) {
    curl_easy_perform(...);  // Transport in API layer!
}
```

### DO: Layer Separation
```cpp
// GOOD: Each layer calls the one below
DLLFUNC int BrokerAsset(...) {
    return marketService().getPrice(asset);  // Service handles details
}
```
