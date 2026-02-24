//=============================================================================
// hl_market_service.h - Market data service (prices, history, asset info)
//=============================================================================
// Part of Hyperliquid Plugin for Zorro
//
// LAYER: Services
// DEPENDENCIES: hl_types.h, hl_meta.h, ws_price_cache.h, hl_http.h
// THREAD SAFETY: All public functions are thread-safe
//
// This module provides:
// - Real-time price access (WebSocket first, HTTP fallback)
// - Historical candle data fetching
// - Asset metadata access (via hl_meta)
// - HTTP seed cooldown management
//=============================================================================

#pragma once

#include "../foundation/hl_types.h"
#include <string>
#include <vector>
#include <cstdint>

namespace hl {
namespace market {

// =============================================================================
// CANDLE DATA
// =============================================================================

/// OHLCV candle data structure
struct Candle {
    double open = 0.0;
    double high = 0.0;
    double low = 0.0;
    double close = 0.0;
    double volume = 0.0;
    int64_t timestamp = 0;  // Milliseconds since epoch (candle END time)
};

// =============================================================================
// PRICE ACCESS
// =============================================================================

/// Get current price for a coin (WebSocket first, HTTP fallback)
/// @param coin Coin name (e.g., "BTC" or "xyz:XYZ100")
/// @param maxAgeMs Maximum acceptable age for cached data (default 1500ms)
/// @return PriceData with bid/ask/mid (all 0 if not available)
///
/// Price retrieval strategy:
/// 1. Check WebSocket cache - return immediately if fresh
/// 2. If stale/missing and HTTP seed allowed, fetch via l2Book endpoint
/// 3. Update WS cache with HTTP data for consistency
/// 4. Return 0 prices if no data available (caller should retry)
PriceData getPrice(const char* coin, uint32_t maxAgeMs = 1500);

/// Get current price for a perpDex coin
/// @param perpDex PerpDex name (e.g., "xyz")
/// @param coin Coin name without prefix (e.g., "XYZ100")
/// @param maxAgeMs Maximum acceptable age for cached data
/// @return PriceData with bid/ask/mid (all 0 if not available)
PriceData getPerpDexPrice(const char* perpDex, const char* coin, uint32_t maxAgeMs = 1500);

/// Check if real-time (WebSocket) price is available and fresh
/// @param coin Coin name (e.g., "BTC" or "xyz:XYZ100")
/// @param maxAgeMs Maximum acceptable age (default 5000ms)
/// @return true if WS has fresh data
bool hasRealtimePrice(const char* coin, uint32_t maxAgeMs = 5000);

/// Request WebSocket subscription for a coin's price data
/// @param coin Coin name (e.g., "BTC" or "xyz:XYZ100")
/// This is idempotent - calling multiple times is safe
void subscribePrice(const char* coin);

// =============================================================================
// HISTORICAL DATA
// =============================================================================

/// Supported candle intervals
enum class CandleInterval {
    M1,   // 1 minute
    M3,   // 3 minutes
    M5,   // 5 minutes
    M15,  // 15 minutes
    M30,  // 30 minutes
    H1,   // 1 hour
    H2,   // 2 hours
    H4,   // 4 hours
    D1    // 1 day
};

/// Convert minutes to CandleInterval enum
/// @param minutes Bar size in minutes
/// @return Corresponding interval, or M1 if unsupported
CandleInterval minutesToInterval(int minutes);

/// Get interval string for API (e.g., "1m", "15m", "1h")
/// @param interval Interval enum value
/// @return String for API request
const char* intervalToString(CandleInterval interval);

/// Get interval duration in minutes
/// @param interval Interval enum value
/// @return Duration in minutes
int intervalToMinutes(CandleInterval interval);

/// Fetch historical candles for a coin
/// @param coin Coin name (e.g., "BTC" or "xyz:XYZ100")
/// @param interval Candle interval
/// @param startTimeMs Start time (milliseconds since epoch)
/// @param endTimeMs End time (milliseconds since epoch)
/// @param maxCandles Maximum candles to fetch (Hyperliquid limit: 5000)
/// @return Vector of candles (oldest first), empty on error
///
/// Note: Hyperliquid returns candles with timestamp = bar OPEN time
/// This function converts to bar END time for Zorro compatibility
std::vector<Candle> getCandles(const char* coin, CandleInterval interval,
                               int64_t startTimeMs, int64_t endTimeMs,
                               int maxCandles = 5000);

/// Fetch historical candles using minutes instead of enum
/// @param coin Coin name
/// @param barMinutes Bar size in minutes (1, 3, 5, 15, 30, 60, 120, 240, 1440)
/// @param startTimeMs Start time (milliseconds since epoch)
/// @param endTimeMs End time (milliseconds since epoch)
/// @param maxCandles Maximum candles to fetch
/// @return Vector of candles (oldest first), empty on error
std::vector<Candle> getCandlesByMinutes(const char* coin, int barMinutes,
                                        int64_t startTimeMs, int64_t endTimeMs,
                                        int maxCandles = 5000);

// =============================================================================
// ASSET METADATA (delegates to hl_meta)
// =============================================================================

/// Get asset info by name
/// @param name Asset name (e.g., "BTC-USD", "BTC", or "xyz:XYZ100")
/// @return Pointer to AssetInfo, or nullptr if not found
const AssetInfo* getAsset(const char* name);

/// Get asset info by index
/// @param index Asset index (0-based)
/// @return Pointer to AssetInfo, or nullptr if invalid
const AssetInfo* getAssetByIndex(int index);

/// Find asset index by coin name
/// @param coin Coin name (e.g., "BTC" or "XYZ100")
/// @return Asset index, or -1 if not found
int findAssetIndex(const char* coin);

/// Get total number of cached assets
int getAssetCount();

/// Refresh all asset metadata from API
/// @return Number of assets cached, or 0 on failure
int refreshMeta();

/// Check if metadata has been loaded
bool isMetaLoaded();

// =============================================================================
// HTTP SEED COOLDOWN
// =============================================================================

/// Check if HTTP seed is allowed for a coin (cooldown expired)
/// @param coin Coin name
/// @return true if HTTP seed is allowed
bool canSeedHttp(const char* coin);

/// Record that HTTP seed was performed for a coin
/// @param coin Coin name
void recordSeed(const char* coin);

/// Clear all seed cooldowns
void clearSeedCooldowns();

/// Cleanup market service (call at logout)
/// Deletes s_seedCs critical section and clears cooldown state
void cleanup();

} // namespace market
} // namespace hl
