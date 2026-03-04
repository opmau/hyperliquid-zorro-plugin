//=============================================================================
// hl_trading_twap.h - TWAP order placement and cancellation [OPM-81]
//=============================================================================
// Part of Hyperliquid Plugin for Zorro
//
// LAYER: Services
// DEPENDENCIES: hl_types.h
// THREAD SAFETY: All public functions are thread-safe
//=============================================================================

#pragma once

#include "../foundation/hl_types.h"
#include <cstdint>

namespace hl {
namespace trading {

/// Place a TWAP order on the exchange.
/// The exchange splits it into suborders executed every ~30 seconds.
/// @param request  TWAP parameters (coin, side, size, duration, etc.)
/// @return TwapResult with success flag and twapId for cancellation
TwapResult placeTwapOrder(const TwapRequest& request);

/// Cancel an active TWAP order.
/// @param coin    Asset name (e.g., "BTC")
/// @param twapId  TWAP ID returned from placeTwapOrder
/// @return true if cancel was submitted successfully
bool cancelTwapOrder(const char* coin, uint64_t twapId);

} // namespace trading
} // namespace hl
