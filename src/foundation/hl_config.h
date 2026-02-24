//=============================================================================
// hl_config.h - Compile-time and runtime configuration constants
//=============================================================================
// LAYER: Foundation | DEPENDENCIES: None | THREAD SAFETY: N/A (constants only)
//=============================================================================

#pragma once

namespace hl {
namespace config {

// =============================================================================
// API ENDPOINTS
// =============================================================================

// Mainnet endpoints
constexpr const char* MAINNET_REST = "https://api.hyperliquid.xyz";
constexpr const char* MAINNET_WS   = "wss://api.hyperliquid.xyz/ws";

// Testnet endpoints
constexpr const char* TESTNET_REST = "https://api.hyperliquid-testnet.xyz";
constexpr const char* TESTNET_WS   = "wss://api.hyperliquid-testnet.xyz/ws";

// =============================================================================
// HTTP TIMEOUTS (milliseconds)
// =============================================================================

constexpr int HTTP_TIMEOUT_MS = 10000;  // 10 seconds for HTTP requests

// =============================================================================
// WEBSOCKET SETTINGS (milliseconds)
// =============================================================================

constexpr int WS_CONNECT_TIMEOUT_MS    = 30000;  // 30s connection timeout
constexpr int WS_RECEIVE_TIMEOUT_MS    = 1000;   // 1s receive timeout (polling interval)
constexpr int WS_PING_INTERVAL_MS      = 20000;  // 20s keepalive ping interval
constexpr int WS_ORDER_RESPONSE_TIMEOUT_MS = 5000;  // 5s order response timeout
constexpr int WS_HEALTH_THRESHOLD_SEC  = 60;     // Consider unhealthy after 60s silence

// =============================================================================
// CACHE SETTINGS
// =============================================================================

// Price cache
constexpr int PRICE_STALE_MS           = 5000;   // Price considered stale after 5s
constexpr int PRICE_MAX_AGE_HTTP_MS    = 1500;   // Max age before HTTP fallback
constexpr int WS_FIRST_DATA_WAIT_MS    = 300;    // Max wait for WS data on first query [OPM-142]

// Position/account cache
constexpr int POSITION_CACHE_MS        = 2000;   // 2s cache for clearinghouseState
constexpr int ORDERS_CACHE_MS          = 1000;   // 1s cache for openOrders

// Metadata cache
constexpr int META_CACHE_SECONDS       = 300;    // 5 minutes for asset metadata

// HTTP seeding cooldown (prevents excessive HTTP calls when WS slow)
constexpr int HTTP_SEED_COOLDOWN_MS    = 1000;   // 1s between HTTP seeds per symbol

// =============================================================================
// ORDER DEFAULTS
// =============================================================================

// Slippage buffer for market orders (IOC with no limit price).
// Applied as a multiplier: buy at ask*(1+slippage), sell at bid*(1-slippage).
// Ensures IOC market orders fill even if price moves slightly.
constexpr double MARKET_ORDER_SLIPPAGE  = 0.05;   // 5%

// =============================================================================
// LIMITS
// =============================================================================

constexpr int MAX_ASSETS               = 1024;   // Maximum supported assets
constexpr int MAX_PENDING_ORDERS       = 100;    // Maximum concurrent pending orders
constexpr int MAX_RECENT_FILLS         = 100;    // Recent fills to keep in cache

// =============================================================================
// PLUGIN INFO
// =============================================================================

constexpr int PLUGIN_TYPE              = 2;      // Zorro plugin type (2 = broker)

#ifdef DEV_BUILD
constexpr const char* PLUGIN_NAME      = "Hyperliquid-DEV";
constexpr const char* PLUGIN_VERSION   = "1.1.0-DEV";
#else
constexpr const char* PLUGIN_NAME      = "Hyperliquid";
constexpr const char* PLUGIN_VERSION   = "1.1.0-FullNative";
#endif

} // namespace config
} // namespace hl
