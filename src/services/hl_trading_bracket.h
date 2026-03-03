//=============================================================================
// hl_trading_bracket.h - Bracket order placement (entry + TP + SL) [OPM-79]
//=============================================================================
// Part of Hyperliquid Plugin for Zorro
//
// LAYER: Services
// DEPENDENCIES: hl_types.h
// THREAD SAFETY: All public functions are thread-safe
//=============================================================================

#pragma once

#include "../foundation/hl_types.h"

namespace hl {
namespace trading {

/// Place a bracket order: entry + optional TP + optional SL in a single call.
/// Uses Hyperliquid's "normalTpsl" grouping so that when one TP/SL fills,
/// the sibling is automatically cancelled (siblingFilledCanceled).
///
/// @param request  Bracket parameters (coin, side, size, TP/SL prices)
/// @return BracketResult with trade IDs for entry, TP, and SL
BracketResult placeBracketOrder(const BracketRequest& request);

} // namespace trading
} // namespace hl
