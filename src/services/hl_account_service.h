//=============================================================================
// hl_account_service.h - Account state service (balance, positions, margin)
//=============================================================================
// Part of Hyperliquid Plugin for Zorro
//
// LAYER: Services
// DEPENDENCIES: hl_types.h, hl_globals.h, ws_price_cache.h, hl_http.h
// THREAD SAFETY: All public functions are thread-safe
//
// This module provides:
// - Account balance/equity access (WebSocket first, HTTP fallback)
// - Position queries by coin
// - Margin information
// - Cache staleness checking
//=============================================================================

#pragma once

#include "../foundation/hl_types.h"
#include <string>
#include <vector>
#include <cstdint>

namespace hl {
namespace account {

// =============================================================================
// ACCOUNT STATE
// =============================================================================

/// Account balance data (from clearinghouseState + spotClearinghouseState)
struct Balance {
    double accountValue = 0.0;    // Total tradeable equity. Source depends on
                                  // the collateral model — see getBalance().
    double withdrawable = 0.0;    // Raw `withdrawable` from the API. Reads 0.0
                                  // on unified accounts regardless of free
                                  // equity — prefer freeCollateral. [OPM-824]
    double marginUsed = 0.0;      // Total margin currently used
    double totalNotional = 0.0;   // Total notional position value
    double freeCollateral = 0.0;  // Equity available after maintenance margin.
                                  // Unified: HL's own figure. Otherwise
                                  // accountValue - marginUsed.
    bool unifiedCollateral = false;  // Spot USDC is the collateral pool
    uint32_t timestamp = 0;       // GetTickCount when updated
    bool dataReceived = false;    // WS delivered data (true even if balance is 0)

    /// Calculate unrealized PnL (equity - cash)
    double unrealizedPnl() const { return accountValue - withdrawable; }

    /// Check if data is stale
    /// clearinghouseState is event-driven — fires on balance/position changes only.
    /// 60s matches the WS health check timeout (isHealthy).
    bool isStale(uint32_t maxAgeMs = 60000) const;
};

/// Get current account balance (WebSocket first, HTTP fallback)
/// @param maxAgeMs Maximum acceptable age for cached data
/// @return Balance data (all 0 if not available)
Balance getBalance(uint32_t maxAgeMs = 60000);

/// Check if we have fresh account data
/// @param maxAgeMs Maximum acceptable age
/// @return true if WebSocket has fresh data
bool hasRealtimeBalance(uint32_t maxAgeMs = 60000);

/// Force refresh account data via HTTP (perps clearinghouseState)
/// @return true if successful
bool refreshBalance();

// =============================================================================
// POSITION DATA
// =============================================================================

/// Position information for a single asset
struct PositionInfo {
    std::string coin;
    double size = 0.0;            // Positive = long, negative = short
    double entryPrice = 0.0;
    double unrealizedPnl = 0.0;
    double leverage = 0.0;
    double liquidationPrice = 0.0;
    double marginUsed = 0.0;
    uint32_t timestamp = 0;

    /// Check if this is an open position
    bool isOpen() const { return size != 0.0; }

    /// Check if position is long
    bool isLong() const { return size > 0.0; }

    /// Check if position is short
    bool isShort() const { return size < 0.0; }
};

/// Get position for a specific coin
/// @param coin Coin name (e.g., "BTC" or "xyz:XYZ100")
/// @return Position info (size=0 if no position)
PositionInfo getPosition(const char* coin);

/// Get all open positions
/// @return Vector of all positions with non-zero size
std::vector<PositionInfo> getAllPositions();

/// Check if we have a position in a coin
/// @param coin Coin name
/// @return true if position size != 0
bool hasPosition(const char* coin);

/// Get position size in contracts
/// @param coin Coin name
/// @return Size (positive=long, negative=short, 0=no position)
double getPositionSize(const char* coin);

/// Apply a fill to the position cache so GET_POSITION sees it immediately.
/// Bridges the gap between BrokerBuy2 HTTP fill confirmation and the
/// asynchronous WS clearinghouseState update. [OPM-85]
///
/// @param coin     Asset name (e.g., "BTC")
/// @param fillSize Absolute size filled (always positive)
/// @param fillPx   Average fill price
/// @param isBuy    true=long, false=short
void applyFill(const char* coin, double fillSize, double fillPx, bool isBuy);

// =============================================================================
// ZORRO HELPERS (for BrokerAccount compatibility)
// =============================================================================

/// Get Zorro-style account values
/// @param outBalance Cash balance (withdrawable)
/// @param outTradeVal Unrealized PnL (equity - cash)
/// @param outMarginVal Margin used
/// @return true if data is available
///
/// Zorro expects: Balance + TradeVal = Total Equity
bool getZorroAccountValues(double* outBalance, double* outTradeVal, double* outMarginVal);

/// Convert position from WS cache format
/// Handles perpDex index-to-coin conversion (e.g., "@110001" -> "xyz:XYZ100")
PositionInfo convertWsPosition(const std::string& coin, double size, double entryPx,
                               double unrealizedPnl, double leverage);

// =============================================================================
// CACHE MANAGEMENT
// =============================================================================

/// Clear all cached account/position data
void clearCache();

/// Get age of account data cache in milliseconds
/// @return Age in ms, or MAXDWORD if no data
uint32_t getAccountDataAge();

/// Get age of position data cache in milliseconds
/// @return Age in ms, or MAXDWORD if no data
uint32_t getPositionsAge();

// =============================================================================
// INITIALIZATION
// =============================================================================

/// Subscribe to account data updates (clearinghouseState)
/// Call after WebSocket is connected
void subscribeAccountData();

/// Mark a perpDex as actively used by the running strategy. [OPM-439]
///
/// Call this when Zorro subscribes a perpDex asset via BrokerAsset so the
/// plugin only opens clearinghouseState WS subscriptions and HTTP position
/// fetches for perpDexes the strategy actually trades. Main-dex-only
/// strategies (the common case) will activate zero perpDexes and avoid
/// ~54k unused WS messages per session plus 7 unnecessary HTTP calls at
/// INITRUN.
///
/// Idempotent. Safe to call repeatedly for the same perpDex.
/// @param perpDex PerpDex name (e.g., "xyz"). NULL or empty is a no-op.
void markPerpDexActive(const char* perpDex);

// Account abstraction mode and spotClearinghouseState now live in
// hl_account_spot.h. [OPM-824]

// =============================================================================
// ADDRESS VALIDATION
// =============================================================================

/// User role as reported by Hyperliquid's userRole endpoint
enum class UserRole { Unknown, User, Agent, Vault, Subaccount, Missing };

/// Check the role of an address via HTTP (userRole endpoint).
/// Returns UserRole::Agent if the address is an API/agent wallet.
/// Common pitfall: querying clearinghouseState with an agent address
/// returns empty — must use the master account address instead.
UserRole checkUserRole();

} // namespace account
} // namespace hl
