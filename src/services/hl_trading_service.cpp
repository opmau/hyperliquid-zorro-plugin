//=============================================================================
// hl_trading_service.cpp - Order placement and tracking
//=============================================================================
// LAYER: Services | DEPENDENCIES: hl_globals.h, hl_crypto.h, hl_http.h
//
// This module provides:
// - Order placement (placeOrder, placeOrderWithId)
// - Trade ID management (generate, extract, nonce)
// - Order state storage and updates
// - Configuration (order type, dry run)
// - Fill notification callbacks
//
// Cancel/query operations: hl_trading_cancel.cpp
//=============================================================================

#include "hl_trading_service.h"
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
#include <ctime>

namespace hl {
namespace trading {

// =============================================================================
// INTERNAL STATE
// =============================================================================

static FillCallback s_fillCallback = nullptr;
static char s_orderType[16] = "Ioc";
static bool s_dryRun = false;
static bool s_initialized = false;

// =============================================================================
// LOGGING HELPER
// =============================================================================

static void logMsg(int level, const char* prefix, const char* msg) {
    if (g_config.diagLevel >= level) {
        char buf[512];
        sprintf_s(buf, "%s: %s", prefix, msg);
        g_logger.log(level, buf);
    }
}

// =============================================================================
// INITIALIZATION
// =============================================================================

void init() {
    if (!s_initialized) {
        g_trading.init();
        s_initialized = true;
    }
}

void cleanup() {
    if (s_initialized) {
        g_trading.cleanup();
        s_initialized = false;
        s_fillCallback = nullptr;
    }
}

// =============================================================================
// TRADE ID MANAGEMENT
// =============================================================================

int generateTradeId() {
    return g_trading.generateTradeId();
}

void generateCloid(int tradeId, char* outCloid, size_t outSize) {
    if (!outCloid || outSize == 0) return;

    // Format: 0x + 32 hex chars (128-bit hex string per Hyperliquid API spec)
    // Encoding: tradeID (4 bytes) + timestamp (8 bytes) + counter (4 bytes)
    static int s_cloidCounter = 0;
    uint64_t now = (uint64_t)time(nullptr);
    int counter = ++s_cloidCounter;

    sprintf_s(outCloid, outSize, "0x%08x%016llx%08x",
              (unsigned int)tradeId,
              (unsigned long long)now,
              (unsigned int)counter);
}

int extractTradeIdFromCloid(const char* cloid) {
    if (!cloid || strlen(cloid) < 10) return 0;

    if (cloid[0] != '0' || (cloid[1] != 'x' && cloid[1] != 'X')) return 0;

    char hexBuf[9] = {0};
    strncpy_s(hexBuf, cloid + 2, 8);
    unsigned int tradeId = 0;
    if (sscanf_s(hexBuf, "%x", &tradeId) == 1) {
        return (int)tradeId;
    }
    return 0;
}

uint64_t generateNonce() {
    return g_trading.generateNonce();
}

// =============================================================================
// ORDER TRACKING
// =============================================================================

void storeOrder(int tradeId, const OrderState& state) {
    g_trading.setOrder(tradeId, state);

    if (g_config.diagLevel >= 2) {
        char msg[256];
        sprintf_s(msg, "Stored trade %d: %s %s %.6f @ %.2f (CLOID: %s)",
                 tradeId,
                 state.side == OrderSide::Buy ? "BUY" : "SELL",
                 state.coin,
                 state.requestedSize,
                 state.avgPrice,
                 state.cloid);
        logMsg(2, "storeOrder", msg);
    }
}

bool getOrder(int tradeId, OrderState& outState) {
    return g_trading.getOrder(tradeId, outState);
}

bool updateOrder(int tradeId, double filledSize, double avgPrice, OrderStatus status) {
    OrderState state;
    if (!g_trading.getOrder(tradeId, state)) return false;

    state.filledSize = filledSize;
    state.avgPrice = avgPrice;
    state.status = status;
    state.lastUpdate = (double)time(nullptr) / 86400.0 + 25569.0; // OLE DATE

    return g_trading.updateOrder(tradeId, state);
}

void removeOrder(int tradeId) {
    g_trading.removeOrder(tradeId);
}

// =============================================================================
// CONFIGURATION
// =============================================================================

void setOrderType(const char* orderType) {
    if (orderType) {
        strncpy_s(s_orderType, orderType, _TRUNCATE);
    }
}

const char* getOrderType() {
    return s_orderType;
}

void setDryRun(bool enabled) {
    s_dryRun = enabled;
}

bool isDryRun() {
    return s_dryRun;
}

// =============================================================================
// CALLBACKS
// =============================================================================

void setFillCallback(FillCallback callback) {
    s_fillCallback = callback;
}

void notifyFill(const char* cloid, double filledSize, double avgPrice, const char* status) {
    OrderStatus orderStatus = OrderStatus::Open;
    if (status) {
        if (strcmp(status, "filled") == 0) orderStatus = OrderStatus::Filled;
        else if (strcmp(status, "canceled") == 0 || strcmp(status, "cancelled") == 0) orderStatus = OrderStatus::Cancelled;
        else if (strcmp(status, "partial") == 0) orderStatus = OrderStatus::PartialFill;
        else if (strcmp(status, "error") == 0) orderStatus = OrderStatus::Error;
    }

    int tradeId = findTradeIdByCloid(cloid);
    if (tradeId > 0) {
        updateOrder(tradeId, filledSize, avgPrice, orderStatus);

        if (s_fillCallback) {
            s_fillCallback(tradeId, filledSize, avgPrice, orderStatus);
        }
    }

    if (g_config.diagLevel >= 1) {
        char msg[256];
        sprintf_s(msg, "Fill: cloid=%s filled=%.6f avg=%.2f status=%s tradeId=%d",
                 cloid ? cloid : "(null)", filledSize, avgPrice, status ? status : "(null)", tradeId);
        logMsg(1, "notifyFill", msg);
    }
}

// =============================================================================
// ORDER PLACEMENT
// =============================================================================

OrderResult placeOrder(const OrderRequest& request) {
    int tradeId = generateTradeId();
    return placeOrderWithId(request, tradeId);
}

OrderResult placeOrderWithId(const OrderRequest& request, int tradeId) {
    OrderResult result;
    result.success = false;

    // Validate request
    if (request.coin.empty()) {
        result.error = "Coin is required";
        return result;
    }
    if (request.size <= 0) {
        result.error = "Size must be positive";
        return result;
    }

    // Find asset index
    int assetIndex = request.assetIndex;
    if (assetIndex == 0) {
        assetIndex = meta::findAssetIndex(request.coin.c_str());
        if (assetIndex < 0) {
            result.error = "Asset not found in meta";
            return result;
        }
    }

    // Generate CLOID
    char cloid[64];
    if (request.cloid.empty()) {
        generateCloid(tradeId, cloid, sizeof(cloid));
    } else {
        strncpy_s(cloid, request.cloid.c_str(), _TRUNCATE);
    }
    result.cloid = cloid;

    if (g_config.diagLevel >= 1) {
        char msg[256];
        sprintf_s(msg, "Placing order: %s %s %.6f @ %.2f (tradeId=%d, cloid=%s)",
                 request.side == OrderSide::Buy ? "BUY" : "SELL",
                 request.coin.c_str(), request.size, request.limitPrice,
                 tradeId, cloid);
        logMsg(1, "placeOrder", msg);
    }

    // Dry run - just store and return
    if (s_dryRun) {
        OrderState state;
        strncpy_s(state.cloid, cloid, _TRUNCATE);
        strncpy_s(state.orderId, "DRY_RUN", _TRUNCATE);
        strncpy_s(state.coin, request.coin.c_str(), _TRUNCATE);
        state.side = request.side;
        state.requestedSize = request.size;
        state.filledSize = 0;
        state.avgPrice = request.limitPrice;
        state.status = OrderStatus::Open;
        state.zorroTradeId = tradeId;
        state.lastUpdate = (double)time(nullptr) / 86400.0 + 25569.0;

        storeOrder(tradeId, state);
        result.success = true;
        result.oid = "DRY_RUN";
        logMsg(1, "placeOrder", "DRY RUN - order not sent to exchange");
        return result;
    }

    // STEP 1: Build EIP-712 order action
    const AssetInfo* assetInfo = g_assets.getByIndex(assetIndex);

    eip712::OrderAction orderAction;
    orderAction.asset = assetIndex;
    orderAction.isBuy = (request.side == OrderSide::Buy);

    // Round price per Hyperliquid rules: 5 sig figs + max decimal places [OPM-76]
    if (assetInfo) {
        orderAction.price = utils::formatPriceForExchange(request.limitPrice, assetInfo->szDecimals);
        orderAction.size = utils::formatSize(request.size, assetInfo->szDecimals);
    } else {
        orderAction.price = eip712::formatNumber(request.limitPrice);
        orderAction.size = eip712::formatNumber(request.size);
    }

    orderAction.reduceOnly = request.reduceOnly;
    orderAction.cloid = cloid;

    if (request.isTriggerOrder()) {
        // Trigger order fields [OPM-77]
        orderAction.isTrigger = true;
        orderAction.triggerIsMarket = request.triggerIsMarket;
        if (assetInfo) {
            orderAction.triggerPx = utils::formatPriceForExchange(request.triggerPx, assetInfo->szDecimals);
        } else {
            orderAction.triggerPx = eip712::formatNumber(request.triggerPx);
        }
        orderAction.tpsl = (request.triggerType == TriggerType::SL) ? "sl" : "tp";
    } else {
        switch (request.orderType) {
            case OrderType::Gtc: orderAction.orderType = "Gtc"; break;
            case OrderType::Alo: orderAction.orderType = "Alo"; break;
            default: orderAction.orderType = s_orderType; break;
        }
    }

    // STEP 2: Hash with EIP-712 for signing
    uint64_t nonce = generateNonce();
    bool isMainnet = !g_config.isTestnet;
    eip712::ByteArray msgHash = eip712::hashOrderForSigning(
        orderAction, isMainnet, nonce, "");

    if (msgHash.empty() || msgHash.size() != 32) {
        result.error = "Failed to generate EIP-712 message hash";
        logMsg(1, "placeOrder", "Failed to generate EIP-712 message hash");
        return result;
    }

    // STEP 3: Sign the hash with private key
    crypto::Signature sig;
    if (!crypto::signHash(msgHash.data(), g_config.privateKey, sig)) {
        result.error = "Failed to sign order";
        logMsg(1, "placeOrder", "Failed to sign order");
        return result;
    }

    // STEP 4: Build signed order JSON for submission
    // Build the "t" (type) field — different for trigger vs limit [OPM-77]
    char tField[256];
    if (request.isTriggerOrder()) {
        sprintf_s(tField, sizeof(tField),
            "\"t\":{\"trigger\":{\"isMarket\":%s,\"triggerPx\":\"%s\",\"tpsl\":\"%s\"}}",
            orderAction.triggerIsMarket ? "true" : "false",
            orderAction.triggerPx.c_str(),
            orderAction.tpsl.c_str()
        );
    } else {
        sprintf_s(tField, sizeof(tField),
            "\"t\":{\"limit\":{\"tif\":\"%s\"}}",
            orderAction.orderType.c_str()
        );
    }

    char orderJson[2048];
    sprintf_s(orderJson, sizeof(orderJson),
        "{"
            "\"action\":{"
                "\"type\":\"order\","
                "\"orders\":[{"
                    "\"a\":%d,"
                    "\"b\":%s,"
                    "\"p\":\"%s\","
                    "\"s\":\"%s\","
                    "\"r\":%s,"
                    "%s,"
                    "\"c\":\"%s\""
                "}],"
                "\"grouping\":\"na\""
            "},"
            "\"nonce\":%llu,"
            "\"signature\":%s,"
            "\"vaultAddress\":null,"
            "\"expiresAfter\":null"
        "}",
        assetIndex,
        orderAction.isBuy ? "true" : "false",
        orderAction.price.c_str(),
        orderAction.size.c_str(),
        orderAction.reduceOnly ? "true" : "false",
        tField,
        cloid,
        nonce,
        sig.toJson().c_str()
    );

    {
        char msg[512];
        sprintf_s(msg, "Order JSON (first 200 chars): %.200s", orderJson);
        logMsg(1, "placeOrder", msg);
    }

    // STEP 5: Submit to exchange via HTTP
    http::Response resp = http::exchangePost(orderJson);
    if (!resp.success()) {
        logMsg(1, "placeOrder", "HTTP request failed — querying exchange for order status [OPM-89]");

        Sleep(1000);

        CloidQueryResult qr = queryOrderByCloid(cloid);

        if (qr.outcome == QueryOutcome::Found) {
            logMsg(1, "placeOrder", "Order VERIFIED on exchange after HTTP failure");
            OrderState state;
            strncpy_s(state.cloid, cloid, _TRUNCATE);
            strncpy_s(state.orderId, qr.oid[0] ? qr.oid : "UNKNOWN", _TRUNCATE);
            strncpy_s(state.coin, request.coin.c_str(), _TRUNCATE);
            state.side = request.side;
            state.requestedSize = request.size;
            state.filledSize = qr.filledSize;
            state.avgPrice = qr.avgPrice > 0 ? qr.avgPrice : request.limitPrice;
            state.zorroTradeId = tradeId;
            state.lastUpdate = (double)time(nullptr) / 86400.0 + 25569.0;
            if (strcmp(qr.status, "filled") == 0)
                state.status = determineFilledStatus(qr.filledSize, request.size);
            else if (strcmp(qr.status, "canceled") == 0) state.status = OrderStatus::Cancelled;
            else state.status = OrderStatus::Open;
            storeOrder(tradeId, state);

            result.success = true;
            result.oid = state.orderId;
            result.avgPrice = state.avgPrice;
            result.filledSize = state.filledSize;
            return result;

        } else if (qr.outcome == QueryOutcome::NotFound) {
            logMsg(1, "placeOrder", "Order confirmed NOT on exchange — safe to retry");
            result.error = "Order not on exchange (confirmed)";
            return result;

        } else {
            logMsg(1, "placeOrder", "Order status UNKNOWN — creating PENDING mapping");
            OrderState state;
            strncpy_s(state.cloid, cloid, _TRUNCATE);
            char pendingOid[64];
            sprintf_s(pendingOid, "PENDING_%s", cloid);
            strncpy_s(state.orderId, pendingOid, _TRUNCATE);
            strncpy_s(state.coin, request.coin.c_str(), _TRUNCATE);
            state.side = request.side;
            state.requestedSize = request.size;
            state.filledSize = 0;
            state.avgPrice = request.limitPrice;
            state.status = OrderStatus::Pending;
            state.zorroTradeId = tradeId;
            state.lastUpdate = (double)time(nullptr) / 86400.0 + 25569.0;
            storeOrder(tradeId, state);

            result.success = true;
            result.oid = pendingOid;
            result.avgPrice = request.limitPrice;
            return result;
        }
    }

    if (resp.body.empty()) {
        result.error = "Exchange response is empty";
        logMsg(1, "placeOrder", "Exchange response is empty");
        return result;
    }

    // STEP 6: Parse response and extract order ID + fill data
    const char* body = resp.body.c_str();

    {
        char msg[512];
        sprintf_s(msg, "Exchange response (first 400): %.400s", body);
        logMsg(1, "placeOrder", msg);
    }

    yyjson_doc* doc = yyjson_read(body, resp.body.size(), 0);
    if (!doc) {
        result.error = "Failed to parse exchange response JSON";
        logMsg(1, "placeOrder", result.error.c_str());
        return result;
    }
    yyjson_val* root = yyjson_doc_get_root(doc);

    const char* statusVal = json::getStringPtr(root, "status");
    if (statusVal && strncmp(statusVal, "err", 3) == 0) {
        yyjson_val* respVal = yyjson_obj_get(root, "response");
        char errMsg[512];
        if (respVal && yyjson_is_str(respVal)) {
            sprintf_s(errMsg, "Exchange error: %s", yyjson_get_str(respVal));
        } else {
            sprintf_s(errMsg, "Exchange rejected order (no detail)");
        }
        result.error = errMsg;
        logMsg(1, "placeOrder", errMsg);
        yyjson_doc_free(doc);
        return result;
    }

    yyjson_val* response = json::getObject(root, "response");
    yyjson_val* rdata = response ? json::getObject(response, "data") : nullptr;
    yyjson_val* statuses = rdata ? json::getArray(rdata, "statuses") : nullptr;
    yyjson_val* status0 = statuses ? yyjson_arr_get(statuses, 0) : nullptr;

    bool isFilled = false, isResting = false;
    char oid[64] = {0};
    double filledSize = 0.0;
    double fillPrice = request.limitPrice;

    if (status0) {
        yyjson_val* filledObj = json::getObject(status0, "filled");
        yyjson_val* restingObj = json::getObject(status0, "resting");

        if (filledObj) {
            isFilled = true;
            filledSize = json::getDouble(filledObj, "totalSz");
            fillPrice = json::getDouble(filledObj, "avgPx");
            yyjson_val* oidVal = yyjson_obj_get(filledObj, "oid");
            if (oidVal) {
                if (yyjson_is_str(oidVal))
                    strncpy_s(oid, yyjson_get_str(oidVal), _TRUNCATE);
                else if (yyjson_is_int(oidVal))
                    sprintf_s(oid, "%lld", (long long)yyjson_get_sint(oidVal));
                else if (yyjson_is_real(oidVal))
                    sprintf_s(oid, "%.0f", yyjson_get_real(oidVal));
            }
        } else if (restingObj) {
            isResting = true;
            yyjson_val* oidVal = yyjson_obj_get(restingObj, "oid");
            if (oidVal) {
                if (yyjson_is_str(oidVal))
                    strncpy_s(oid, yyjson_get_str(oidVal), _TRUNCATE);
                else if (yyjson_is_int(oidVal))
                    sprintf_s(oid, "%lld", (long long)yyjson_get_sint(oidVal));
                else if (yyjson_is_real(oidVal))
                    sprintf_s(oid, "%.0f", yyjson_get_real(oidVal));
            }
        }
    }
    yyjson_doc_free(doc);

    if (!*oid) {
        result.error = "No order ID in exchange response";
        logMsg(1, "placeOrder", "No order ID in exchange response");
        return result;
    }

    // STEP 7: Store order state with confirmed OID + fill data
    OrderState state;
    strncpy_s(state.cloid, cloid, _TRUNCATE);
    strncpy_s(state.orderId, oid, _TRUNCATE);
    strncpy_s(state.coin, request.coin.c_str(), _TRUNCATE);
    state.side = request.side;
    state.requestedSize = request.size;
    state.filledSize = filledSize;
    state.avgPrice = fillPrice;
    state.status = isFilled
        ? determineFilledStatus(filledSize, request.size) : OrderStatus::Open;
    state.zorroTradeId = tradeId;
    state.lastUpdate = (double)time(nullptr) / 86400.0 + 25569.0;

    storeOrder(tradeId, state);

    result.success = true;
    result.oid = oid;
    result.avgPrice = fillPrice;
    result.filledSize = filledSize;

    if (g_config.diagLevel >= 1) {
        char msg[256];
        if (isFilled) {
            sprintf_s(msg, "Order FILLED: oid=%s sz=%.6f px=%.2f", oid, filledSize, fillPrice);
        } else if (isResting) {
            sprintf_s(msg, "Order RESTING: oid=%s cloid=%s", oid, cloid);
        } else {
            sprintf_s(msg, "Order placed: oid=%s", oid);
        }
        logMsg(1, "placeOrder", msg);
    }

    return result;
}

} // namespace trading
} // namespace hl
