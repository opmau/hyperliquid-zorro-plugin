//=============================================================================
// hl_trading_errors.cpp - Exchange order-reject tracking and classification
//=============================================================================
// Part of Hyperliquid Plugin for Zorro
//
// LAYER: Services
// DEPENDENCIES: hl_trading_service.h
// THREAD SAFETY: Main thread only — orders are placed from the Zorro thread,
//                and the recorded error is read back by the same thread via
//                brokerCommand(HL_GET_LAST_ORDER_ERROR).
//
// [OPM-795] Hyperliquid reports per-order rejects inside
// response.data.statuses[i].error under a TOP-LEVEL status:"ok". The err
// branch never sees them, so every reject used to collapse into the generic
// "No order ID in exchange response" and surfaced to scripts only as
// BrokerBuy2 == 0. A maker strategy has to tell "post-only would cross ->
// reprice one tick back" apart from "margin/signing failure -> stop trading
// this asset"; this module is what makes that distinction reachable.
//
// Error strings are those documented in Hyperliquid's API error responses.
//=============================================================================

#include "hl_trading_service.h"
#include "hl_trading_response.h"
#include "../transport/json_helpers.h"
#include <cstring>
#include <cstdio>

namespace hl {
namespace trading {

// =============================================================================
// RESPONSE PARSING
// =============================================================================

/// Read an "oid" field that HL may encode as int, real, or string.
static void readOid(yyjson_val* obj, char* out, size_t outSize) {
    if (!obj || !out || outSize == 0) return;
    yyjson_val* oidVal = yyjson_obj_get(obj, "oid");
    if (!oidVal) return;
    if (yyjson_is_str(oidVal))
        strncpy_s(out, outSize, yyjson_get_str(oidVal), _TRUNCATE);
    else if (yyjson_is_int(oidVal))
        sprintf_s(out, outSize, "%lld", (long long)yyjson_get_sint(oidVal));
    else if (yyjson_is_real(oidVal))
        sprintf_s(out, outSize, "%.0f", yyjson_get_real(oidVal));
}

bool parseOrderStatusResponse(const char* body, size_t len, OrderResponseStatus& out) {
    if (!body || len == 0) return false;

    yyjson_doc* doc = yyjson_read(body, len, 0);
    if (!doc) return false;
    yyjson_val* root = yyjson_doc_get_root(doc);

    // Top-level failure: the whole action was rejected (signing, nonce, ...).
    const char* statusVal = json::getStringPtr(root, "status");
    if (statusVal && strncmp(statusVal, "err", 3) == 0) {
        yyjson_val* respVal = yyjson_obj_get(root, "response");
        out.error = (respVal && yyjson_is_str(respVal))
            ? yyjson_get_str(respVal) : "Exchange rejected action (no detail)";
        out.topLevelErr = true;
        out.valid = true;
        yyjson_doc_free(doc);
        return true;
    }

    yyjson_val* response = json::getObject(root, "response");
    yyjson_val* rdata = response ? json::getObject(response, "data") : nullptr;
    yyjson_val* statuses = rdata ? json::getArray(rdata, "statuses") : nullptr;
    yyjson_val* status0 = statuses ? yyjson_arr_get(statuses, 0) : nullptr;

    if (!status0) {
        yyjson_doc_free(doc);
        return false;
    }
    out.valid = true;

    // batchModify acknowledges with the bare string "success".
    if (yyjson_is_str(status0)) {
        const char* s = yyjson_get_str(status0);
        if (s && strcmp(s, "success") == 0) out.success = true;
        else if (s && *s) out.error = s;
        yyjson_doc_free(doc);
        return true;
    }

    // [OPM-795] The error field HL returns under a top-level status:"ok".
    // Read it FIRST — a status carrying an error carries nothing else.
    yyjson_val* errVal = yyjson_obj_get(status0, "error");
    if (errVal && yyjson_is_str(errVal)) {
        out.error = yyjson_get_str(errVal);
        yyjson_doc_free(doc);
        return true;
    }

    yyjson_val* filledObj = json::getObject(status0, "filled");
    yyjson_val* restingObj = json::getObject(status0, "resting");

    if (filledObj) {
        out.filled = true;
        out.filledSize = json::getDouble(filledObj, "totalSz");
        out.avgPrice = json::getDouble(filledObj, "avgPx");
        readOid(filledObj, out.oid, sizeof(out.oid));
    } else if (restingObj) {
        out.resting = true;
        readOid(restingObj, out.oid, sizeof(out.oid));
    }

    yyjson_doc_free(doc);
    return true;
}

// =============================================================================
// LAST ORDER ERROR
// =============================================================================

static char s_lastOrderError[256] = {0};
static int  s_lastOrderErrorClass = ORDER_ERR_NONE;

int classifyOrderError(const char* errorText) {
    if (!errorText || !*errorText) return ORDER_ERR_NONE;

    // Substring match, not equality: HL appends context to some strings, e.g.
    // "Post only order would have immediately matched, bbo was {bbo}".
    if (strstr(errorText, "Post only order would have immediately matched"))
        return ORDER_ERR_POST_ONLY;
    if (strstr(errorText, "Insufficient margin"))
        return ORDER_ERR_MARGIN;

    return ORDER_ERR_OTHER;
}

void setLastOrderError(const char* errorText) {
    if (!errorText || !*errorText) {
        clearLastOrderError();
        return;
    }
    strncpy_s(s_lastOrderError, errorText, _TRUNCATE);
    s_lastOrderErrorClass = classifyOrderError(errorText);
}

void clearLastOrderError() {
    s_lastOrderError[0] = '\0';
    s_lastOrderErrorClass = ORDER_ERR_NONE;
}

int getLastOrderErrorClass() {
    return s_lastOrderErrorClass;
}

const char* getLastOrderErrorText() {
    return s_lastOrderError;
}

} // namespace trading
} // namespace hl
