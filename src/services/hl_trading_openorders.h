//=============================================================================
// hl_trading_openorders.h - Exchange-sourced resting order queries
//=============================================================================
// Part of Hyperliquid Plugin for Zorro
//
// LAYER: Services
// DEPENDENCIES: hl_types.h
// THREAD SAFETY: Stateless. Issues a blocking HTTP request — call from the
//                Zorro thread, never from the WS thread.
//=============================================================================

#pragma once

#include "../foundation/hl_types.h"
#include <string>
#include <vector>

namespace hl {
namespace trading {

// =============================================================================
// OPEN ORDERS (exchange-sourced) [OPM-797, OPM-798]
// =============================================================================

/// One resting order as reported by the exchange.
struct OpenOrderInfo {
    std::string oid;        // Exchange order ID (numeric string)
    std::string coin;       // API coin name ("BTC", "xyz:TSLA")
    bool isBuy = false;
    double limitPx = 0.0;
    double sz = 0.0;        // Remaining size
    double origSz = 0.0;
};

/// Fetch resting orders straight from the exchange via POST /info
/// {"type":"openOrders","user":"..."}.
///
/// Deliberately NOT sourced from the tradeMap: after a Zorro restart the map is
/// empty while the exchange still holds orders from the previous session.
/// @param perpDex Perp dex name, or nullptr/"" for the first (main) dex
std::vector<OpenOrderInfo> fetchOpenOrders(const char* perpDex = nullptr);

} // namespace trading
} // namespace hl
