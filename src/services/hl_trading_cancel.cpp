//=============================================================================
// hl_trading_cancel.cpp - Order cancellation and the dead man's switch
//=============================================================================
// LAYER: Services | DEPENDENCIES: hl_trading_service.h, hl_crypto.h, hl_http.h
//
// This module provides:
// - Order cancellation (cancelOrder, cancelOrderByTradeId, cancelAllOrders)
// - Dead man's switch (scheduleCancel, clearScheduleCancel) [OPM-83]
//
// Order lookup and exchange status reconciliation live in
// hl_trading_lookup.cpp.
//=============================================================================

#include "hl_trading_service.h"
#include "hl_trading_openorders.h"
#include "hl_meta.h"
#include "../foundation/hl_globals.h"
#include "../foundation/hl_utils.h"
#include "../foundation/hl_crypto.h"
#include "../foundation/hl_eip712.h"
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
// ORDER CANCELLATION
// =============================================================================

bool cancelOrder(const char* coin, const char* oid) {
    if (!coin || !oid) return false;

    // [OPM-797] Guard synthetic tradeMap IDs (PENDING_/RESUMED_/IMPORTED_).
    if (!utils::isExchangeOrderId(oid)) {
        g_logger.logf(1, "cancelOrder: refusing to cancel non-exchange order id "
                         "'%s' on %s — nothing was placed under it", oid, coin);
        return false;
    }

    if (g_config.diagLevel >= 1) {
        char msg[128];
        sprintf_s(msg, "Cancel request: %s oid=%s", coin, oid);
        logMsg(1, "cancelOrder", msg);
    }

    // Find asset index
    int assetIndex = meta::findAssetIndex(coin);
    if (assetIndex < 0) {
        logMsg(1, "cancelOrder", "Asset not found in meta");
        return false;
    }

    // Build EIP-712 cancel action
    eip712::CancelAction cancelAction;
    cancelAction.asset = meta::getApiAssetId(assetIndex);
    cancelAction.orderId = (uint64_t)_atoi64(oid);

    // Hash with EIP-712 for signing
    uint64_t nonce = generateNonce();
    bool isMainnet = !g_config.isTestnet;
    std::string vault(g_config.vaultAddress);  // [OPM-202]
    eip712::ByteArray msgHash = eip712::hashCancelForSigning(
        cancelAction, isMainnet, nonce, vault);

    if (msgHash.empty() || msgHash.size() != 32) {
        logMsg(1, "cancelOrder", "Failed to generate EIP-712 message hash for cancel");
        return false;
    }

    // Sign the hash
    crypto::Signature sig;
    if (!crypto::signHash(msgHash.data(), g_config.privateKey, sig)) {
        logMsg(1, "cancelOrder", "Failed to sign cancel order");
        return false;
    }

    // Build signed cancel JSON
    // [OPM-202] Format vaultAddress for JSON payload
    char vaultJson[128];
    if (vault.empty()) {
        strcpy_s(vaultJson, "null");
    } else {
        sprintf_s(vaultJson, "\"%s\"", vault.c_str());
    }

    char cancelJson[1024];
    long long oidNum = _atoi64(oid);
    sprintf_s(cancelJson, sizeof(cancelJson),
        "{"
            "\"action\":{"
                "\"type\":\"cancel\","
                "\"cancels\":[{"
                    "\"a\":%d,"
                    "\"o\":%lld"
                "}]"
            "},"
            "\"nonce\":%llu,"
            "\"signature\":%s,"
            "\"vaultAddress\":%s,"
            "\"expiresAfter\":null"
        "}",
        cancelAction.asset,  // API asset ID, not registry index [OPM-191]
        oidNum,
        nonce,
        sig.toJson().c_str(),
        vaultJson
    );

    if (g_config.diagLevel >= 2) {
        char msg[512];
        sprintf_s(msg, "Cancel JSON (first 200 chars): %.200s", cancelJson);
        logMsg(2, "cancelOrder", msg);
    }

    // Submit to exchange
    http::Response resp = http::exchangePost(cancelJson);
    if (!resp.success()) {
        logMsg(1, "cancelOrder", "HTTP request to exchange failed");
        return false;
    }

    // Check response status for errors
    if (!resp.body.empty()) {
        yyjson_doc* cancelDoc = yyjson_read(resp.body.c_str(), resp.body.size(), 0);
        if (cancelDoc) {
            yyjson_val* cancelRoot = yyjson_doc_get_root(cancelDoc);
            const char* st = json::getStringPtr(cancelRoot, "status");
            if (st && strncmp(st, "err", 3) == 0) {
                logMsg(1, "cancelOrder", "Exchange rejected cancel request");
                if (g_config.diagLevel >= 2) {
                    char msg[512];
                    sprintf_s(msg, "Cancel response: %.400s", resp.body.c_str());
                    logMsg(2, "cancelOrder", msg);
                }
                yyjson_doc_free(cancelDoc);
                return false;
            }
            yyjson_doc_free(cancelDoc);
        }
    }

    if (g_config.diagLevel >= 1) {
        char msg[256];
        sprintf_s(msg, "Cancel submitted successfully: %s oid=%s", coin, oid);
        logMsg(1, "cancelOrder", msg);
    }

    return true;
}

bool cancelOrderByTradeId(int tradeId) {
    OrderState state;
    if (!getOrder(tradeId, state)) return false;

    // [OPM-792] When BrokerSell2 left a close order working against this
    // position, that is the order the script means to cancel — the entry order
    // has already filled, and cancelling its oid is a no-op at best.
    if (state.closeTradeId > 0) {
        OrderState closeState;
        if (getOrder(state.closeTradeId, closeState)
            && closeState.status != OrderStatus::Filled
            && closeState.status != OrderStatus::Cancelled) {

            bool ok = cancelOrder(closeState.coin, closeState.orderId);
            g_logger.logf(1, "cancelOrderByTradeId: trade %d -> cancelling its "
                             "resting CLOSE order %d (oid=%s): %s",
                          tradeId, state.closeTradeId, closeState.orderId,
                          ok ? "OK" : "FAILED");
            if (ok) {
                updateOrder(state.closeTradeId, closeState.filledSize,
                            closeState.avgPrice, OrderStatus::Cancelled);
                // Position is intact again — unlink so a later DO_CANCEL falls
                // through to the entry order.
                OrderState parent;
                if (getOrder(tradeId, parent)) {
                    parent.closeTradeId = 0;
                    storeOrder(tradeId, parent);
                }
            }
            return ok;
        }
        // Close order already resolved — drop the stale link and continue.
        OrderState parent;
        if (getOrder(tradeId, parent)) {
            parent.closeTradeId = 0;
            storeOrder(tradeId, parent);
        }
    }

    bool success = cancelOrder(state.coin, state.orderId);
    if (success) {
        OrderState current;
        if (getOrder(tradeId, current) && current.status != OrderStatus::Filled) {
            updateOrder(tradeId, current.filledSize, current.avgPrice, OrderStatus::Cancelled);
            logMsg(1, "cancelOrderByTradeId", "TradeMap updated to Cancelled");
        }
    }
    return success;
}

// =============================================================================
// CANCEL ALL [OPM-797]
// =============================================================================
// Sourced from the exchange's openOrders info endpoint rather than the local
// tradeMap: after a Zorro restart the tradeMap is empty, and an order left
// resting by the crashed session was previously only cancellable through the
// Hyperliquid web UI.
//
// Each cancel is submitted individually. packCancelAction() encodes exactly one
// {asset, oid} pair, so batching would mean a new msgpack/EIP-712 shape and a
// new signing path; cancel-all is a rare, latency-insensitive safety operation,
// so N round trips is the right trade for not touching the signing code.

int cancelAllOrders(const char* coin) {
    if (g_config.diagLevel >= 1) {
        char msg[128];
        sprintf_s(msg, "Cancel all orders: coin=%s", coin ? coin : "(all)");
        logMsg(1, "cancelAllOrders", msg);
    }

    std::vector<OpenOrderInfo> orders = fetchOpenOrders();
    if (orders.empty()) {
        logMsg(1, "cancelAllOrders", "No open orders on exchange");
        return 0;
    }

    // Each cancel is a separate signed HTTP round trip, and Hyperliquid meters
    // the exchange endpoint by weight. An account with hundreds of resting
    // orders would otherwise fire an unbounded burst and start collecting
    // rate-limit rejections partway through, with no signal to the caller.
    // Bound it, and report explicitly when the bound bites — a partial
    // cancel-all that looks complete is worse than one that says so.
    const size_t MAX_CANCELS_PER_CALL = 64;

    int cancelled = 0, failed = 0, skipped = 0;
    size_t attempted = 0, deferred = 0;
    for (const auto& o : orders) {
        if (coin && *coin && !utils::coinMatches(o.coin.c_str(), coin)) {
            skipped++;
            continue;
        }
        if (attempted >= MAX_CANCELS_PER_CALL) {
            deferred++;
            continue;
        }
        attempted++;
        if (cancelOrder(o.coin.c_str(), o.oid.c_str())) {
            cancelled++;
            // Keep the tradeMap consistent when we happen to track this order.
            int tid = findTradeIdByOid(o.oid.c_str());
            if (tid > 0) {
                OrderState st;
                if (getOrder(tid, st) && st.status != OrderStatus::Filled) {
                    updateOrder(tid, st.filledSize, st.avgPrice, OrderStatus::Cancelled);
                }
            }
        } else {
            failed++;
        }
    }

    g_logger.logf(1, "cancelAllOrders(%s): %d cancelled, %d failed, %d other symbols",
                  coin && *coin ? coin : "ALL", cancelled, failed, skipped);
    if (deferred > 0) {
        g_logger.logf(1, "cancelAllOrders(%s): NOT ALL CANCELLED — %d order(s) "
                         "left resting (per-call cap %d). Call again to continue.",
                      coin && *coin ? coin : "ALL", (int)deferred,
                      (int)MAX_CANCELS_PER_CALL);
    }
    return cancelled;
}

// =============================================================================
// DEAD MAN'S SWITCH (scheduleCancel) [OPM-83]
// =============================================================================

/// Internal helper: submit scheduleCancel action to the exchange.
/// @param timeMs  Absolute UTC time in ms (0 = clear scheduled cancel)
static bool submitScheduleCancel(uint64_t timeMs) {
    // Hash with EIP-712 for signing
    uint64_t nonce = generateNonce();
    bool isMainnet = !g_config.isTestnet;
    std::string vault(g_config.vaultAddress);  // [OPM-202]
    eip712::ByteArray msgHash = eip712::hashScheduleCancelForSigning(
        timeMs, isMainnet, nonce, vault);

    if (msgHash.empty() || msgHash.size() != 32) {
        logMsg(1, "scheduleCancel", "Failed to generate EIP-712 message hash");
        return false;
    }

    // Sign the hash
    crypto::Signature sig;
    if (!crypto::signHash(msgHash.data(), g_config.privateKey, sig)) {
        logMsg(1, "scheduleCancel", "Failed to sign scheduleCancel action");
        return false;
    }

    // [OPM-202] Format vaultAddress for JSON payload
    char vaultJson[128];
    if (vault.empty()) {
        strcpy_s(vaultJson, "null");
    } else {
        sprintf_s(vaultJson, "\"%s\"", vault.c_str());
    }

    // Build signed JSON payload
    char json[512];
    if (timeMs > 0) {
        sprintf_s(json, sizeof(json),
            "{"
                "\"action\":{\"type\":\"scheduleCancel\",\"time\":%llu},"
                "\"nonce\":%llu,"
                "\"signature\":%s,"
                "\"vaultAddress\":%s,"
                "\"expiresAfter\":null"
            "}",
            (unsigned long long)timeMs,
            (unsigned long long)nonce,
            sig.toJson().c_str(),
            vaultJson
        );
    } else {
        sprintf_s(json, sizeof(json),
            "{"
                "\"action\":{\"type\":\"scheduleCancel\"},"
                "\"nonce\":%llu,"
                "\"signature\":%s,"
                "\"vaultAddress\":%s,"
                "\"expiresAfter\":null"
            "}",
            (unsigned long long)nonce,
            sig.toJson().c_str(),
            vaultJson
        );
    }

    if (g_config.diagLevel >= 2) {
        char msg[256];
        sprintf_s(msg, "scheduleCancel JSON (first 200): %.200s", json);
        logMsg(2, "scheduleCancel", msg);
    }

    // Submit to exchange
    http::Response resp = http::exchangePost(json);
    if (!resp.success()) {
        logMsg(1, "scheduleCancel", "HTTP request failed");
        return false;
    }

    // Check response for errors
    if (!resp.body.empty()) {
        yyjson_doc* doc = yyjson_read(resp.body.c_str(), resp.body.size(), 0);
        if (doc) {
            yyjson_val* root = yyjson_doc_get_root(doc);
            const char* st = json::getStringPtr(root, "status");
            if (st && strncmp(st, "err", 3) == 0) {
                char errMsg[512];
                yyjson_val* respVal = yyjson_obj_get(root, "response");
                if (respVal && yyjson_is_str(respVal)) {
                    sprintf_s(errMsg, "Exchange error: %s", yyjson_get_str(respVal));
                } else {
                    sprintf_s(errMsg, "Exchange rejected scheduleCancel");
                }
                logMsg(1, "scheduleCancel", errMsg);
                yyjson_doc_free(doc);
                return false;
            }
            yyjson_doc_free(doc);
        }
    }

    if (g_config.diagLevel >= 1) {
        char msg[128];
        if (timeMs > 0) {
            sprintf_s(msg, "scheduleCancel set: time=%llu ms", (unsigned long long)timeMs);
        } else {
            sprintf_s(msg, "scheduleCancel cleared");
        }
        logMsg(1, "scheduleCancel", msg);
    }

    return true;
}

bool scheduleCancel(uint64_t timeMs) {
    if (timeMs == 0) {
        logMsg(1, "scheduleCancel", "timeMs is 0 — use clearScheduleCancel() instead");
        return false;
    }
    return submitScheduleCancel(timeMs);
}

bool clearScheduleCancel() {
    return submitScheduleCancel(0);
}

} // namespace trading
} // namespace hl
