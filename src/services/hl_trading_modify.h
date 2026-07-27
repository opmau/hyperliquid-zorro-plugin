//=============================================================================
// hl_trading_modify.h - Atomic order modification (batchModify) [OPM-80]
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

/// Atomically modify an existing order's price/size.
/// Uses Hyperliquid's batchModify action (preserves queue priority).
/// @param request  Modify parameters (target oid/cloid + new order params)
/// @return ModifyResult with success flag and error message
ModifyResult modifyOrder(const ModifyRequest& request);

// =============================================================================
// SCRIPT-REACHABLE REPRICE [OPM-793]
// =============================================================================
// ModifyRequest holds std::string members, so it cannot be constructed from
// Lite-C — 50042 has never been callable from a strategy. This shim resolves
// coin/side/oid from the tracked order and takes only scalars, so the ALO
// engine's reprice loop can modify in one call instead of
// DO_CANCEL -> settle-poll -> re-place (two round trips and a fill-race window
// per reprice).

/// Result codes for modifyByTradeId, returned verbatim by brokerCommand.
enum ModifyByTradeIdResult {
    MODIFY_OK              =  1,
    MODIFY_FAILED          =  0,   // exchange rejected, or HTTP failure
    MODIFY_UNKNOWN_TRADE   = -1,   // tradeId not in the tradeMap
    MODIFY_NOT_RESTING     = -2    // already filled/cancelled — nothing to move
};

/// Reprice (and optionally resize) the resting order behind a Zorro trade ID.
/// @param tradeId   Zorro trade ID returned by BrokerBuy2/BrokerSell2
/// @param newPrice  New limit price (must be > 0)
/// @param newSize   New size in contracts, or <= 0 to keep the current size
/// @return a ModifyByTradeIdResult
int modifyByTradeId(int tradeId, double newPrice, double newSize);

} // namespace trading
} // namespace hl
