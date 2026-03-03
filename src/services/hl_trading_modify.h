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

} // namespace trading
} // namespace hl
