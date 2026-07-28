//=============================================================================
// hl_trading_openorders.cpp - Exchange-sourced resting order queries
//=============================================================================
// Part of Hyperliquid Plugin for Zorro
//
// LAYER: Services
// DEPENDENCIES: hl_trading_service.h, hl_globals.h, hl_http.h
// THREAD SAFETY: Stateless; safe to call from the Zorro thread. Issues a
//                blocking HTTP request, so do not call from the WS thread.
//
// [OPM-797][OPM-798] The WS openOrders cache is the fast path, but it cannot
// be the only one: the subscription is populated by the WS thread and is empty
// for the first moments after login, and after a Zorro restart the tradeMap
// holds nothing at all while the exchange still holds orders placed by the
// previous session. Those orders were previously uncancellable through the
// plugin — the operator had to use the Hyperliquid web UI. This module is the
// authoritative fallback.
//=============================================================================

#include "hl_trading_openorders.h"
#include "hl_trading_service.h"
#include "../foundation/hl_globals.h"
#include "../transport/hl_http.h"
#include "../transport/json_helpers.h"
#include <cstdio>
#include <cstring>

namespace hl {
namespace trading {

std::vector<OpenOrderInfo> fetchOpenOrders(const char* perpDex) {
    std::vector<OpenOrderInfo> out;

    if (!g_config.walletAddress[0]) {
        g_logger.log(1, "fetchOpenOrders: no wallet address — not logged in");
        return out;
    }

    char body[256];
    if (perpDex && *perpDex) {
        sprintf_s(body, "{\"type\":\"openOrders\",\"user\":\"%s\",\"dex\":\"%s\"}",
                  g_config.walletAddress, perpDex);
    } else {
        sprintf_s(body, "{\"type\":\"openOrders\",\"user\":\"%s\"}",
                  g_config.walletAddress);
    }

    http::Response resp = http::infoPost(body, false);
    if (!resp.success() || resp.body.empty()) {
        g_logger.logf(1, "fetchOpenOrders: HTTP query failed (dex=%s)",
                      perpDex && *perpDex ? perpDex : "main");
        return out;
    }

    yyjson_doc* doc = yyjson_read(resp.body.c_str(), resp.body.size(), 0);
    if (!doc) {
        g_logger.log(1, "fetchOpenOrders: response JSON parse failed");
        return out;
    }

    // The info endpoint returns a bare array of orders.
    yyjson_val* root = yyjson_doc_get_root(doc);
    yyjson_val* orders = root;
    if (root && yyjson_is_obj(root)) orders = json::getArray(root, "orders");

    size_t idx, max;
    yyjson_val* item;
    yyjson_arr_foreach(orders, idx, max, item) {
        OpenOrderInfo o;
        char buf[64];

        // HL encodes oid as a JSON number here and as a string over WS.
        long long oidNum = json::getInt64(item, "oid");
        if (oidNum > 0) {
            sprintf_s(buf, "%lld", oidNum);
            o.oid = buf;
        }
        if (json::getString(item, "coin", buf, sizeof(buf))) o.coin = buf;

        yyjson_val* sideItem = yyjson_obj_get(item, "side");
        if (sideItem && yyjson_is_str(sideItem)) {
            const char* s = yyjson_get_str(sideItem);
            o.isBuy = (s && (s[0] == 'B' || s[0] == 'b'));
        }
        o.limitPx = json::getDouble(item, "limitPx");
        o.sz      = json::getDouble(item, "sz");
        o.origSz  = json::getDouble(item, "origSz");

        if (!o.oid.empty() && !o.coin.empty()) out.push_back(o);
    }
    yyjson_doc_free(doc);

    if (g_config.diagLevel >= 2) {
        g_logger.logf(2, "fetchOpenOrders(dex=%s): %d resting orders",
                      perpDex && *perpDex ? perpDex : "main", (int)out.size());
    }
    return out;
}

} // namespace trading
} // namespace hl
