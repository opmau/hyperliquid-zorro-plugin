//=============================================================================
// hl_trading_margin.h - Per-asset leverage and isolated margin [OPM-1584]
//=============================================================================
// Part of Hyperliquid Plugin for Zorro
//
// LAYER: Services
// DEPENDENCIES: hl_types.h
// THREAD SAFETY: Main thread only (signs and posts like the order path)
//
// Leverage on Hyperliquid is an account setting per asset, not an order
// field, and an isolated position's margin is fixed at entry until it is
// topped up explicitly. A strategy that opens isolated positions therefore
// needs both actions, or the position runs at whatever leverage the account
// last had for that asset and is liquidated as soon as its entry margin is
// used up.
//=============================================================================

#pragma once

#include "../foundation/hl_types.h"
#include <cmath>
#include <cstdint>
#include <cstring>

namespace hl {
namespace trading {

/// Margin mode of an asset, as returned by HL_GET_MARGIN_MODE.
enum MarginMode {
    MARGIN_MODE_UNKNOWN          = 0,  // Asset not found, or an unrecognized flag
    MARGIN_MODE_CROSS_ALLOWED    = 1,  // Cross or isolated
    MARGIN_MODE_ISOLATED_ONLY    = 2,  // "noCross": isolated, margin removable
    MARGIN_MODE_STRICT_ISOLATED  = 3   // "strictIsolated": margin cannot be removed
};

/// Classify an asset from its meta flags. Header-inline so it is testable
/// without the services layer.
/// The legacy onlyIsolated flag without a marginMode cannot tell noCross from
/// strictIsolated, so it is read as the stricter of the two.
inline int marginModeOf(const AssetInfo& asset) {
    const char* m = asset.marginMode;
    if (strcmp(m, "strictIsolated") == 0) return MARGIN_MODE_STRICT_ISOLATED;
    if (strcmp(m, "noCross") == 0) return MARGIN_MODE_ISOLATED_ONLY;
    if (strcmp(m, "normal") == 0) return MARGIN_MODE_CROSS_ALLOWED;
    if (*m) return MARGIN_MODE_UNKNOWN;
    return asset.onlyIsolated ? MARGIN_MODE_STRICT_ISOLATED
                              : MARGIN_MODE_CROSS_ALLOWED;
}

/// USD amount to the exchange's ntli integer (USD x 1e6, sign kept).
inline int64_t usdToNtli(double usd) {
    return (int64_t)llround(usd * 1e6);
}

/// Margin mode of a coin ("BTC", "xyz:TSLA", "TSLA-USDC_xyz").
/// @return a MarginMode value; MARGIN_MODE_UNKNOWN if the coin is not in meta
int getMarginMode(const char* coin);

/// Set the account's leverage and margin type for one asset (updateLeverage).
/// Refused locally, before signing: leverage outside 1..maxLeverage, a spot
/// asset, or cross margin on an asset that only allows isolated.
/// @return true if the exchange accepted it; on failure the reason is
///         readable via HL_GET_LAST_ORDER_ERROR (50023)
bool updateLeverage(const char* coin, int leverage, bool isCross);

/// Add (usd > 0) or remove (usd < 0) margin on the open isolated position in
/// one asset (updateIsolatedMargin). Reads the position first and refuses:
/// no position, a cross position, an amount under $0.000001, or a removal on
/// a strictIsolated asset.
/// @return true if the exchange accepted it; on failure the reason is
///         readable via HL_GET_LAST_ORDER_ERROR (50023)
bool adjustIsolatedMargin(const char* coin, double usd);

} // namespace trading
} // namespace hl
