//=============================================================================
// ws_parsers.h - WebSocket message parsers
//=============================================================================
// Part of Hyperliquid Plugin for Zorro
//
// LAYER: Transport
// DEPENDENCIES: ws_price_cache.h, ws_types.h
// THREAD SAFETY: Thread-safe via PriceCache (callers synchronize externally)
//
// Parses WebSocket subscription messages using yyjson:
// - l2Book: top-of-book bid/ask prices
// - clearinghouseState: positions + account margin summary
// - openOrders: resting orders snapshot
// - userFills: trade fill events
// - post response: order confirmation/rejection
//=============================================================================

#pragma once

#include "ws_price_cache.h"
#include "ws_types.h"

namespace hl {
namespace ws {

/// Parsed L2Book price update (returned by parseL2Book)
struct L2BookUpdate {
    char coin[64];
    double bid;
    double ask;
    bool valid;
    L2BookUpdate() : bid(0), ask(0), valid(false) { coin[0] = 0; }
};

/// Parse l2Book channel message, extracting coin + top-of-book bid/ask
/// Format: {"channel":"l2Book","data":{"coin":"BTC","levels":[[{"px":"50000",...}],[{"px":"50001",...}]]}}
L2BookUpdate parseL2Book(const char* json, int diagLevel, LogCallback logCb);

/// Parse post/order response from WebSocket
/// Extracts requestId, success/error, and filled/resting status
/// Format: {"channel":"post","data":{"id":123,"response":{...}}}
OrderResponse parsePostResponse(const char* json, int diagLevel, LogCallback logCb);

/// Parse clearinghouseState subscription message
/// Populates positions and account data in cache
/// Format: {"channel":"clearinghouseState","data":{"assetPositions":[...],"marginSummary":{...}}}
void parseClearinghouseState(PriceCache& cache, const char* json,
                             int diagLevel, LogCallback logCb);

/// Parse openOrders subscription message
/// Replaces all open orders in cache (full snapshot)
/// Format: {"channel":"openOrders","data":[{"coin":"BTC","oid":"123",...},...]}
void parseOpenOrders(PriceCache& cache, const char* json,
                     int diagLevel, LogCallback logCb);

/// Parse userFills subscription message
/// Appends fill events to cache
/// Format: {"channel":"userFills","data":[{"coin":"BTC","oid":"123","px":"91000",...},...]}
void parseUserFills(PriceCache& cache, const char* json,
                    int diagLevel, LogCallback logCb);

/// Parse orderUpdates subscription message
/// Calls callback for each order status change (filled, canceled, etc.)
/// Format: {"channel":"orderUpdates","data":[{"order":{...},"status":"filled",...},...]}
void parseOrderUpdates(const char* json, OrderUpdateCallback callback,
                       int diagLevel, LogCallback logCb);

} // namespace ws
} // namespace hl
