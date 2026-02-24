//=============================================================================
// ws_parsers.cpp - WebSocket message parsers for account data
//=============================================================================
// Part of Hyperliquid Plugin for Zorro
//
// LAYER: Transport
// DEPENDENCIES: ws_price_cache.h, ws_types.h, yyjson.h, json_helpers.h
// THREAD SAFETY: Thread-safe via PriceCache methods
//
// Uses yyjson for structure-aware JSON parsing. Handles both:
//   WS path:   {"channel":"...","data":{...,"clearinghouseState":{...}}}
//   HTTP path:  {"assetPositions":[...],"marginSummary":{...},...}
//=============================================================================

#include "ws_parsers.h"
#include "json_helpers.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>

namespace hl {
namespace ws {

//=============================================================================
// HELPERS
//=============================================================================

static void logMsg(int diagLevel, LogCallback logCb, int minLevel,
                   const char* fmt, ...) {
    if (!logCb || diagLevel < minLevel) return;
    char buf[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    logCb(buf);
}

//=============================================================================
// parseL2Book
//=============================================================================

L2BookUpdate parseL2Book(const char* jsonStr, int diagLevel, LogCallback logCb) {
    L2BookUpdate result;

    yyjson_doc* doc = yyjson_read(jsonStr, strlen(jsonStr), 0);
    if (!doc) {
        logMsg(diagLevel, logCb, 2, "WS parseL2Book: JSON parse error");
        return result;
    }
    yyjson_val* root = yyjson_doc_get_root(doc);

    // Navigate: root.data (WS path) or root directly (HTTP path)
    yyjson_val* data = json::getObject(root, "data");
    yyjson_val* bookObj = data ? data : root;

    // Extract coin name
    if (!json::getString(bookObj, "coin", result.coin, sizeof(result.coin))) {
        logMsg(diagLevel, logCb, 2, "WS parseL2Book: no coin field");
        yyjson_doc_free(doc);
        return result;
    }

    // Extract bid/ask from levels: [[{px,sz,...},...],[{px,sz,...},...]]
    yyjson_val* levels = json::getArray(bookObj, "levels");
    if (!levels) {
        logMsg(diagLevel, logCb, 2, "WS parseL2Book: no levels array");
        yyjson_doc_free(doc);
        return result;
    }

    // levels[0] = bids array, levels[1] = asks array
    yyjson_val* bids = yyjson_arr_get(levels, 0);
    yyjson_val* asks = yyjson_arr_get(levels, 1);

    if (bids && yyjson_is_arr(bids)) {
        yyjson_val* topBid = yyjson_arr_get(bids, 0);
        if (topBid) result.bid = json::getDouble(topBid, "px");
    }
    if (asks && yyjson_is_arr(asks)) {
        yyjson_val* topAsk = yyjson_arr_get(asks, 0);
        if (topAsk) result.ask = json::getDouble(topAsk, "px");
    }

    result.valid = (result.bid > 0 && result.ask > 0);

    yyjson_doc_free(doc);
    return result;
}

//=============================================================================
// parsePostResponse
//=============================================================================

OrderResponse parsePostResponse(const char* jsonStr, int diagLevel, LogCallback logCb) {
    OrderResponse result;

    yyjson_doc* doc = yyjson_read(jsonStr, strlen(jsonStr), 0);
    if (!doc) {
        logMsg(diagLevel, logCb, 2, "WS parsePostResponse: JSON parse error");
        return result;
    }
    yyjson_val* root = yyjson_doc_get_root(doc);

    // Navigate: root.data for WS path, or root directly
    yyjson_val* data = json::getObject(root, "data");
    yyjson_val* respObj = data ? data : root;

    // Extract request ID
    result.requestId = (int)json::getInt64(respObj, "id");
    if (result.requestId == 0) {
        yyjson_doc_free(doc);
        return result;
    }

    // Check for error at top level
    yyjson_val* errorVal = yyjson_obj_get(respObj, "error");
    if (errorVal) {
        result.success = false;
        const char* errStr = json::valToString(errorVal);
        if (errStr) result.error = errStr;
        yyjson_doc_free(doc);
        return result;
    }

    // Success — check response.data.statuses for filled/resting
    result.success = true;
    yyjson_val* response = json::getObject(respObj, "response");
    yyjson_val* rdata = response ? json::getObject(response, "data") : nullptr;
    yyjson_val* statuses = rdata ? json::getArray(rdata, "statuses") : nullptr;
    yyjson_val* status0 = statuses ? yyjson_arr_get(statuses, 0) : nullptr;

    if (status0) {
        if (json::getObject(status0, "filled"))
            result.status = "filled";
        else if (json::getObject(status0, "resting"))
            result.status = "open";
    } else {
        // Fallback: simple keyword detection for simpler response formats
        if (yyjson_obj_get(respObj, "filled") || yyjson_obj_get(respObj, "response"))
            result.status = "filled";
    }

    yyjson_doc_free(doc);
    return result;
}

//=============================================================================
// parseClearinghouseState
//=============================================================================

void parseClearinghouseState(PriceCache& cache, const char* jsonStr,
                             int diagLevel, LogCallback logCb) {
    yyjson_doc* doc = yyjson_read(jsonStr, strlen(jsonStr), 0);
    if (!doc) {
        logMsg(diagLevel, logCb, 1, "WS clearinghouseState: JSON parse error");
        return;
    }
    yyjson_val* root = yyjson_doc_get_root(doc);

    // Navigate to clearinghouseState object.
    // WS path:   root.data.clearinghouseState
    // HTTP path:  root IS the clearinghouseState directly
    yyjson_val* state = json::getObject(json::getObject(root, "data"), "clearinghouseState");
    if (!state) state = root;

    // --- Parse asset positions ---
    cache.clearPositions();

    yyjson_val* positions = json::getArray(state, "assetPositions");
    size_t idx, max;
    yyjson_val* posItem;
    yyjson_arr_foreach(positions, idx, max, posItem) {
        yyjson_val* posObj = json::getObject(posItem, "position");
        if (!posObj) continue;

        PositionData pos;
        char buf[64];

        if (json::getString(posObj, "coin", buf, sizeof(buf)))
            pos.coin = buf;

        pos.size          = json::getDouble(posObj, "szi");
        pos.entryPx       = json::getDouble(posObj, "entryPx");
        pos.unrealizedPnl = json::getDouble(posObj, "unrealizedPnl");
        pos.marginUsed    = json::getDouble(posObj, "marginUsed");

        // leverage: try "leverage" first, fall back to "maxLeverage"
        yyjson_val* levItem = yyjson_obj_get(posObj, "leverage");
        if (!levItem) levItem = yyjson_obj_get(posObj, "maxLeverage");
        if (levItem) {
            if (yyjson_is_obj(levItem)) {
                // leverage can be {"type":"cross","value":50} — extract value
                pos.leverage = json::getDouble(levItem, "value");
            } else {
                pos.leverage = json::valToDouble(levItem);
            }
        }

        // liquidationPx (can be null)
        yyjson_val* liqItem = yyjson_obj_get(posObj, "liquidationPx");
        if (liqItem && !json::isNull(liqItem)) {
            pos.liquidationPx = json::valToDouble(liqItem);
        }

        if (!pos.coin.empty()) {
            cache.setPosition(pos);
            logMsg(diagLevel, logCb, 2,
                   "WS clearinghouseState: %s size=%.6f entry=%.2f pnl=%.2f",
                   pos.coin.c_str(), pos.size, pos.entryPx, pos.unrealizedPnl);
        }
    }

    // --- Parse account totals from marginSummary / crossMarginSummary ---
    // marginSummary = aggregate (cross + isolated positions)
    // crossMarginSummary = cross-margin only (for unified accounts, includes spot)
    // Use the HIGHER accountValue to capture unified accounts correctly (Issue #23).
    double accValue = 0, marginUsed = 0, withdrawable = 0, ntlPos = 0;
    bool found = false;

    yyjson_val* margin = json::getObject(state, "marginSummary");
    if (margin) {
        accValue   = json::getDouble(margin, "accountValue");
        marginUsed = json::getDouble(margin, "totalMarginUsed");
        ntlPos     = json::getDouble(margin, "totalNtlPos");
        found = true;
    }

    yyjson_val* crossMargin = json::getObject(state, "crossMarginSummary");
    if (crossMargin) {
        double crossAccValue = json::getDouble(crossMargin, "accountValue");
        if (crossAccValue > accValue) {
            accValue   = crossAccValue;
            marginUsed = json::getDouble(crossMargin, "totalMarginUsed");
            ntlPos     = json::getDouble(crossMargin, "totalNtlPos");
            logMsg(diagLevel, logCb, 2,
                   "WS: Using crossMarginSummary (%.2f > marginSummary)", crossAccValue);
        }
        found = true;
    }

    // withdrawable is at the clearinghouseState level, not inside summary objects
    withdrawable = json::getDouble(state, "withdrawable");

    if (found) {
        cache.setAccountData(accValue, marginUsed, withdrawable, ntlPos);
        logMsg(diagLevel, logCb, 2,
               "WS clearinghouseState: acct=%.2f margin=%.2f withdraw=%.2f",
               accValue, marginUsed, withdrawable);
    }

    yyjson_doc_free(doc);
}

//=============================================================================
// parseOpenOrders
//=============================================================================

void parseOpenOrders(PriceCache& cache, const char* jsonStr,
                     int diagLevel, LogCallback logCb) {
    yyjson_doc* doc = yyjson_read(jsonStr, strlen(jsonStr), 0);
    if (!doc) return;
    yyjson_val* root = yyjson_doc_get_root(doc);

    cache.clearOpenOrders();

    // WS path: root.data.orders (or root.data is the array directly)
    // The format is: {"channel":"openOrders","data":{"dex":"","user":"...","orders":[...]}}
    // Or sometimes: {"channel":"openOrders","data":[...]}
    yyjson_val* data = yyjson_obj_get(root, "data");
    yyjson_val* orders = nullptr;
    if (data && yyjson_is_obj(data))
        orders = json::getArray(data, "orders");
    if (!orders && data && yyjson_is_arr(data))
        orders = data;

    size_t idx, max;
    yyjson_val* orderItem;
    yyjson_arr_foreach(orders, idx, max, orderItem) {
        OpenOrderData order;
        char buf[64];

        if (json::getString(orderItem, "oid", buf, sizeof(buf)))
            order.oid = buf;
        if (json::getString(orderItem, "coin", buf, sizeof(buf)))
            order.coin = buf;

        // side
        yyjson_val* sideItem = yyjson_obj_get(orderItem, "side");
        if (sideItem && yyjson_is_str(sideItem)) {
            const char* s = yyjson_get_str(sideItem);
            order.isBuy = (s[0] == 'B' || s[0] == 'b');
        }

        order.limitPx = json::getDouble(orderItem, "limitPx");
        order.sz      = json::getDouble(orderItem, "sz");
        order.origSz  = json::getDouble(orderItem, "origSz");

        if (!order.oid.empty() && !order.coin.empty()) {
            cache.setOpenOrder(order);
            logMsg(diagLevel, logCb, 2,
                   "WS openOrders: %s %s %s %.6f @ %.2f",
                   order.coin.c_str(), order.isBuy ? "BUY" : "SELL",
                   order.oid.c_str(), order.sz, order.limitPx);
        }
    }

    yyjson_doc_free(doc);
}

//=============================================================================
// parseUserFills
//=============================================================================

void parseUserFills(PriceCache& cache, const char* jsonStr,
                    int diagLevel, LogCallback logCb) {
    yyjson_doc* doc = yyjson_read(jsonStr, strlen(jsonStr), 0);
    if (!doc) return;
    yyjson_val* root = yyjson_doc_get_root(doc);

    // WS path: root.data is the fills array (or data.fills)
    yyjson_val* data = yyjson_obj_get(root, "data");
    yyjson_val* fills = nullptr;
    if (data && yyjson_is_arr(data))
        fills = data;
    else if (data && yyjson_is_obj(data))
        fills = json::getArray(data, "fills");

    size_t idx, max;
    yyjson_val* fillItem;
    yyjson_arr_foreach(fills, idx, max, fillItem) {
        FillData fill;
        char buf[64];

        if (json::getString(fillItem, "oid", buf, sizeof(buf)))
            fill.oid = buf;
        if (json::getString(fillItem, "coin", buf, sizeof(buf)))
            fill.coin = buf;
        if (json::getString(fillItem, "tid", buf, sizeof(buf)))
            fill.tid = buf;

        // side
        yyjson_val* sideItem = yyjson_obj_get(fillItem, "side");
        if (sideItem && yyjson_is_str(sideItem)) {
            const char* s = yyjson_get_str(sideItem);
            fill.isBuy = (s[0] == 'B' || s[0] == 'b');
        }

        fill.px   = json::getDouble(fillItem, "px");
        fill.sz   = json::getDouble(fillItem, "sz");
        fill.fee  = json::getDouble(fillItem, "fee");
        fill.time = json::getInt64(fillItem, "time");

        if (!fill.oid.empty() && fill.sz > 0) {
            cache.addFill(fill);
            logMsg(diagLevel, logCb, 2,
                   "WS userFill: %s %s %.6f @ %.2f (oid=%s)",
                   fill.coin.c_str(), fill.isBuy ? "BUY" : "SELL",
                   fill.sz, fill.px, fill.oid.c_str());
        }
    }

    yyjson_doc_free(doc);
}

//=============================================================================
// parseOrderUpdates
//=============================================================================

void parseOrderUpdates(const char* jsonStr, OrderUpdateCallback callback,
                       int diagLevel, LogCallback logCb) {
    if (!callback) return;

    yyjson_doc* doc = yyjson_read(jsonStr, strlen(jsonStr), 0);
    if (!doc) {
        logMsg(diagLevel, logCb, 2, "WS parseOrderUpdates: JSON parse error");
        return;
    }
    yyjson_val* root = yyjson_doc_get_root(doc);

    // Format: {"channel":"orderUpdates","data":[{"order":{...},"status":"..."},...]}
    yyjson_val* data = yyjson_obj_get(root, "data");
    if (!data || !yyjson_is_arr(data)) {
        yyjson_doc_free(doc);
        return;
    }

    size_t idx, max;
    yyjson_val* item;
    yyjson_arr_foreach(data, idx, max, item) {
        yyjson_val* order = json::getObject(item, "order");
        if (!order) continue;

        // Extract status
        char status[64] = {0};
        json::getString(item, "status", status, sizeof(status));
        if (!status[0]) continue;

        // Extract oid (can be number or string)
        char oid[64] = {0};
        yyjson_val* oidVal = yyjson_obj_get(order, "oid");
        if (oidVal) {
            if (yyjson_is_str(oidVal))
                strncpy_s(oid, yyjson_get_str(oidVal), _TRUNCATE);
            else if (yyjson_is_int(oidVal))
                sprintf_s(oid, "%lld", (long long)yyjson_get_sint(oidVal));
        }
        if (!oid[0]) continue;

        // Extract cloid (optional — present when set during order placement)
        char cloid[64] = {0};
        json::getString(order, "cloid", cloid, sizeof(cloid));

        // Extract sizes: origSz (original), sz (remaining)
        double origSz = json::getDouble(order, "origSz");
        double sz = json::getDouble(order, "sz");
        double filledSz = origSz - sz;
        if (filledSz < 0) filledSz = 0;

        // limitPx as approximate fill price (exact price comes from userFills)
        double avgPx = json::getDouble(order, "limitPx");

        logMsg(diagLevel, logCb, 1,
               "WS orderUpdate: oid=%s cloid=%s status=%s filled=%.6f px=%.2f",
               oid, cloid[0] ? cloid : "(none)", status, filledSz, avgPx);

        callback(oid, cloid[0] ? cloid : nullptr, status, filledSz, avgPx);
    }

    yyjson_doc_free(doc);
}

} // namespace ws
} // namespace hl
