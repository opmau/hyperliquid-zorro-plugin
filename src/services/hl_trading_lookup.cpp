//=============================================================================
// hl_trading_lookup.cpp - Order lookup and exchange status reconciliation
//=============================================================================
// LAYER: Services | DEPENDENCIES: hl_trading_service.h, hl_http.h
//
// This module provides the READ side of order tracking:
// - Order lookup by CLOID/OID (getOrderByCloid, getOrderByOid)
// - Trade ID lookup (findTradeIdByCloid, findTradeIdByOid, updateOrderByCloid)
// - Order status query (queryOrderByCloid — three-state reconciliation [OPM-89])
//
// Cancellation and the dead man's switch live in hl_trading_cancel.cpp.
//=============================================================================

#include "hl_trading_service.h"
#include "hl_meta.h"
#include "../foundation/hl_globals.h"
#include "../foundation/hl_utils.h"
#include "../transport/hl_http.h"
#include "../transport/json_helpers.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>

namespace hl {
namespace trading {

// --- Logging helper (matches hl_trading_service.cpp) ---

static void logMsg(int level, const char* prefix, const char* msg) {
    if (g_config.diagLevel >= level) {
        char buf[512];
        sprintf_s(buf, "%s: %s", prefix, msg);
        g_logger.log(level, buf);
    }
}


// =============================================================================
// ORDER LOOKUP
// =============================================================================

bool getOrderByCloid(const char* cloid, OrderState& outState) {
    if (!cloid) return false;

    if (!g_trading.tradeCsInit) return false;

    EnterCriticalSection(&g_trading.tradeCs);
    for (const auto& pair : g_trading.tradeMap) {
        if (strcmp(pair.second.cloid, cloid) == 0) {
            outState = pair.second;
            LeaveCriticalSection(&g_trading.tradeCs);
            return true;
        }
    }
    LeaveCriticalSection(&g_trading.tradeCs);
    return false;
}

bool getOrderByOid(const char* oid, OrderState& outState) {
    if (!oid) return false;

    if (!g_trading.tradeCsInit) return false;

    EnterCriticalSection(&g_trading.tradeCs);
    for (const auto& pair : g_trading.tradeMap) {
        if (strcmp(pair.second.orderId, oid) == 0) {
            outState = pair.second;
            LeaveCriticalSection(&g_trading.tradeCs);
            return true;
        }
    }
    LeaveCriticalSection(&g_trading.tradeCs);
    return false;
}

int findTradeIdByCloid(const char* cloid) {
    if (!cloid) return 0;

    // First try extracting from CLOID format
    int embedded = extractTradeIdFromCloid(cloid);
    if (embedded > 0) {
        OrderState state;
        if (g_trading.getOrder(embedded, state)) {
            return embedded;
        }
    }

    // Search through trade map
    if (!g_trading.tradeCsInit) return 0;

    EnterCriticalSection(&g_trading.tradeCs);
    for (const auto& pair : g_trading.tradeMap) {
        if (strcmp(pair.second.cloid, cloid) == 0) {
            int result = pair.first;
            LeaveCriticalSection(&g_trading.tradeCs);
            return result;
        }
    }
    LeaveCriticalSection(&g_trading.tradeCs);
    return 0;
}

int findTradeIdByOid(const char* oid) {
    if (!oid || !g_trading.tradeCsInit) return 0;

    EnterCriticalSection(&g_trading.tradeCs);
    for (const auto& pair : g_trading.tradeMap) {
        if (strcmp(pair.second.orderId, oid) == 0) {
            int result = pair.first;
            LeaveCriticalSection(&g_trading.tradeCs);
            return result;
        }
    }
    LeaveCriticalSection(&g_trading.tradeCs);
    return 0;
}

bool updateOrderByCloid(const char* cloid, double filledSize, double avgPrice, OrderStatus status) {
    int tradeId = findTradeIdByCloid(cloid);
    if (tradeId == 0) return false;
    return updateOrder(tradeId, filledSize, avgPrice, status);
}

// =============================================================================
// ORDER STATUS QUERY (three-state reconciliation) [OPM-89]
// =============================================================================
// Ported from legacy/Hyperliquid_Native.cpp:748-901.
// Verified against docs/hyperliquid-api/05-info-endpoint.md:285-439.

CloidQueryResult queryOrderByCloid(const char* cloid) {
    CloidQueryResult result;

    if (!cloid || !*cloid || !g_config.walletAddress[0]) {
        strncpy_s(result.status, "invalid_params", _TRUNCATE);
        return result;
    }

    // POST /info with {"type":"orderStatus","user":"<addr>","oid":"<cloid>"}
    char body[512];
    sprintf_s(body, "{\"type\":\"orderStatus\",\"user\":\"%s\",\"oid\":\"%s\"}",
              g_config.walletAddress, cloid);

    logMsg(2, "queryOrderByCloid", cloid);

    http::Response resp = http::infoPost(body);
    if (!resp.success() || resp.body.empty()) {
        strncpy_s(result.status, "query_failed", _TRUNCATE);
        result.outcome = QueryOutcome::Failed;
        logMsg(1, "queryOrderByCloid", "HTTP query failed -> FAILED");
        return result;
    }

    yyjson_doc* doc = yyjson_read(resp.body.c_str(), resp.body.size(), 0);
    if (!doc) {
        strncpy_s(result.status, "parse_error", _TRUNCATE);
        result.outcome = QueryOutcome::Failed;
        logMsg(1, "queryOrderByCloid", "JSON parse failed -> FAILED");
        return result;
    }
    yyjson_val* root = yyjson_doc_get_root(doc);

    // Check for unknownOid — order DEFINITELY didn't land
    const char* statusStr = json::getStringPtr(root, "status");
    if (statusStr && strcmp(statusStr, "unknownOid") == 0) {
        result.outcome = QueryOutcome::NotFound;
        strncpy_s(result.status, "unknownOid", _TRUNCATE);
        logMsg(1, "queryOrderByCloid", "unknownOid -> NOT_FOUND");
        yyjson_doc_free(doc);
        return result;
    }

    // Navigate to order.status for the real order status
    yyjson_val* orderObj = json::getObject(root, "order");
    const char* orderStatus = nullptr;
    if (orderObj) {
        orderStatus = json::getStringPtr(orderObj, "status");
    }
    if (!orderStatus) orderStatus = statusStr;

    if (!orderStatus || !*orderStatus) {
        strncpy_s(result.status, "no_status", _TRUNCATE);
        result.outcome = QueryOutcome::Failed;
        logMsg(1, "queryOrderByCloid", "No status in response -> FAILED");
        yyjson_doc_free(doc);
        return result;
    }

    strncpy_s(result.status, orderStatus, _TRUNCATE);

    if (strcmp(orderStatus, "filled") == 0 ||
        strcmp(orderStatus, "open") == 0 ||
        strcmp(orderStatus, "canceled") == 0 ||
        strcmp(orderStatus, "triggered") == 0) {

        result.outcome = QueryOutcome::Found;

        // [OPM-679] The orderStatus response nests the order fields one level
        // deeper than the status: root.order = {"order":{...,"oid":...},
        // "status":"open","statusTimestamp":...}. Read oid/sz from the inner
        // "order" object; status was correctly read from the outer object
        // above. Falling back to orderObj keeps this robust if HL ever flattens.
        yyjson_val* orderNode = json::getObject(orderObj, "order");
        if (!orderNode) orderNode = orderObj;

        if (orderNode) {
            yyjson_val* oidVal = yyjson_obj_get(orderNode, "oid");
            if (oidVal) {
                if (yyjson_is_int(oidVal))
                    sprintf_s(result.oid, "%lld", (long long)yyjson_get_sint(oidVal));
                else if (yyjson_is_real(oidVal))
                    sprintf_s(result.oid, "%.0f", yyjson_get_real(oidVal));
                else if (yyjson_is_str(oidVal))
                    strncpy_s(result.oid, yyjson_get_str(oidVal), _TRUNCATE);
            }
        }

        if (strcmp(orderStatus, "filled") == 0) {
            result.filledSize = json::getDouble(orderNode, "sz");
            result.avgPrice = json::getDouble(orderNode, "avgPx");
        }

        char msg[256];
        sprintf_s(msg, "FOUND: oid=%s status=%s filled=%.6f avgPx=%.2f",
                  result.oid, result.status, result.filledSize, result.avgPrice);
        logMsg(1, "queryOrderByCloid", msg);

    } else if (strcmp(orderStatus, "rejected") == 0 ||
               strcmp(orderStatus, "marginCanceled") == 0) {
        result.outcome = QueryOutcome::NotFound;
        logMsg(1, "queryOrderByCloid", "rejected/marginCanceled -> NOT_FOUND");

    } else {
        result.outcome = QueryOutcome::Failed;
        char msg[128];
        sprintf_s(msg, "Unexpected status: %s -> FAILED", orderStatus);
        logMsg(1, "queryOrderByCloid", msg);
    }

    yyjson_doc_free(doc);
    return result;
}

} // namespace trading
} // namespace hl
