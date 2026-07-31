//=============================================================================
// hl_trading_response.h - Exchange order-response parsing (internal)
//=============================================================================
// Part of Hyperliquid Plugin for Zorro
//
// LAYER: Services (internal — not part of the plugin's public surface)
// DEPENDENCIES: hl_types.h
// THREAD SAFETY: parseOrderStatusResponse is stateless and safe from any
//                thread; the last-order-error accessors in hl_trading_service.h
//                are main-thread only.
//
// Both `order` and `batchModify` return the same envelope:
//   {"status":"ok","response":{"type":"order","data":{"statuses":[ ... ]}}}
// where each status is exactly one of {"filled":{...}}, {"resting":{...}},
// {"error":"..."} — or the bare string "success" for a modify. Parsing it in
// one place keeps placeOrderWithId and modifyOrder from drifting apart, which
// is how statuses[].error came to be read by neither. [OPM-795]
//=============================================================================

#pragma once

#include "../foundation/hl_types.h"
#include <string>

namespace hl {
namespace trading {

/// Decoded form of response.data.statuses[0].
struct OrderResponseStatus {
    bool valid = false;         // JSON parsed and an envelope was recognized
    bool filled = false;        // statuses[0].filled present
    bool resting = false;       // statuses[0].resting present
    bool success = false;       // statuses[0] == "success" (batchModify)
    char oid[64] = {0};         // Exchange order ID from filled/resting
    double filledSize = 0.0;
    double avgPrice = 0.0;
    std::string error;          // statuses[0].error, or the top-level err text
    bool topLevelErr = false;   // true when status:"err" (whole action failed)
};

/// Parse an exchange order/batchModify response body.
/// @param body   Raw response JSON
/// @param len    Length of body
/// @param out    Decoded status (out.valid is false when the JSON is unparseable)
/// @return true when the JSON parsed, regardless of whether it reported an error
bool parseOrderStatusResponse(const char* body, size_t len, OrderResponseStatus& out);

// =============================================================================
// LAST ORDER ERROR [OPM-795]
// =============================================================================
// Hyperliquid reports per-order rejects inside statuses[i].error under a
// top-level status:"ok", so they never reach the err branch and used to
// collapse into "No order ID in exchange response". Scripts could not tell
// "post-only would cross -> reprice one tick back" from "margin/signing
// failure -> stop trading this asset"; both were BrokerBuy2 == 0.
//
// Classes are deliberately coarse and stable — the exact text stays available
// via getLastOrderErrorText() for anything finer.

enum OrderErrorClass {
    ORDER_ERR_NONE          = 0,
    ORDER_ERR_POST_ONLY     = 1,  // "Post only order would have immediately matched"
    ORDER_ERR_MARGIN        = 2,  // "Insufficient margin to place order"
    ORDER_ERR_OTHER         = 3   // tick size, min notional, reduce-only, signing, ...
};

/// Classify an exchange reject string into an OrderErrorClass.
/// Matches the error strings documented in Hyperliquid's API reference.
int classifyOrderError(const char* errorText);

/// Record the most recent order reject (also called internally on every reject).
void setLastOrderError(const char* errorText);

/// Clear the recorded reject (called on every successful order placement).
void clearLastOrderError();

/// Class of the most recent order reject, or ORDER_ERR_NONE.
int getLastOrderErrorClass();

/// Full text of the most recent order reject ("" when none).
const char* getLastOrderErrorText();

} // namespace trading
} // namespace hl
