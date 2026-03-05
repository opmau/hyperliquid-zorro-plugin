//=============================================================================
// hl_trading_bracket.cpp - Bracket order placement (entry + TP + SL) [OPM-79]
//=============================================================================
// LAYER: Services
// DEPENDENCIES: hl_trading_service.h, hl_eip712.h, hl_crypto.h, hl_http.h
// THREAD SAFETY: Thread-safe (uses g_config, g_logger — read-only globals)
//
// Bracket orders send entry + TP + SL in a single API call with "normalTpsl"
// grouping. When TP fills, SL is auto-cancelled (and vice versa) with status
// "siblingFilledCanceled".
//
// Wire format: {"type":"order","orders":[{entry},{tp},{sl}],"grouping":"normalTpsl"}
// Reuses packOrderWire() for each order in the array.
// Signing uses hashBatchModifyForSigning() — generic pre-packed-bytes hasher.
//=============================================================================

#include "hl_trading_bracket.h"
#include "hl_trading_service.h"
#include "hl_meta.h"
#include "../foundation/hl_globals.h"
#include "../foundation/hl_utils.h"
#include "../foundation/hl_crypto.h"
#include "../foundation/hl_eip712.h"
#include "../foundation/hl_msgpack.h"
#include "../transport/hl_http.h"
#include "../transport/json_helpers.h"
#include <cstdio>
#include <cstring>

namespace hl {
namespace trading {

// =============================================================================
// LOGGING HELPER
// =============================================================================

static void logBracket(int level, const char* prefix, const char* msg) {
    if (g_config.diagLevel >= level) {
        char buf[512];
        sprintf_s(buf, "BRACKET %s: %s", prefix, msg);
        g_logger.log(level, buf);
    }
}

// =============================================================================
// BRACKET ORDER PLACEMENT
// =============================================================================

BracketResult placeBracketOrder(const BracketRequest& request) {
    BracketResult result;

    // --- Validate ---
    if (request.coin.empty()) {
        result.error = "Coin is required";
        return result;
    }
    if (request.size <= 0) {
        result.error = "Size must be positive";
        return result;
    }
    if (!request.hasTP() && !request.hasSL()) {
        result.error = "At least one of TP or SL is required for bracket order";
        return result;
    }

    // --- Find asset ---
    int assetIndex = request.assetIndex;
    if (assetIndex == 0) {
        assetIndex = meta::findAssetIndex(request.coin.c_str());
        if (assetIndex < 0) {
            result.error = "Asset not found in meta";
            return result;
        }
    }
    const AssetInfo* assetInfo = g_assets.getByIndex(assetIndex);

    // --- Format prices and sizes ---
    auto fmtPrice = [&](double px) -> std::string {
        return assetInfo
            ? utils::formatPriceForExchange(px, assetInfo->szDecimals)
            : eip712::formatNumber(px);
    };
    auto fmtSize = [&](double sz) -> std::string {
        return assetInfo
            ? utils::formatSize(sz, assetInfo->szDecimals)
            : eip712::formatNumber(sz);
    };

    std::string sizeStr = fmtSize(request.size);
    bool isBuyEntry = (request.side == OrderSide::Buy);

    // --- Generate trade IDs and CLOIDs ---
    int entryTradeId = generateTradeId();
    int tpTradeId = request.hasTP() ? generateTradeId() : 0;
    int slTradeId = request.hasSL() ? generateTradeId() : 0;

    char entryCloid[64], tpCloid[64], slCloid[64];
    generateCloid(entryTradeId, entryCloid, sizeof(entryCloid));
    if (tpTradeId) generateCloid(tpTradeId, tpCloid, sizeof(tpCloid));
    if (slTradeId) generateCloid(slTradeId, slCloid, sizeof(slCloid));

    if (g_config.diagLevel >= 1) {
        char msg[512];
        sprintf_s(msg, "Placing: %s %s %s @ %s (TP=%.2f SL=%.2f)",
                 isBuyEntry ? "BUY" : "SELL", request.coin.c_str(), sizeStr.c_str(),
                 fmtPrice(request.limitPrice).c_str(),
                 request.tpTriggerPx, request.slTriggerPx);
        logBracket(1, "place", msg);
    }

    // --- Build order wires ---
    std::vector<msgpack::BracketOrderWire> wires;

    // Entry order TIF
    std::string entryTif;
    switch (request.orderType) {
        case OrderType::Gtc: entryTif = "Gtc"; break;
        case OrderType::Alo: entryTif = "Alo"; break;
        default:             entryTif = "Ioc"; break;
    }

    // 1. Entry order (limit)
    {
        msgpack::BracketOrderWire w;
        w.asset = meta::getApiAssetId(assetIndex);
        w.isBuy = isBuyEntry;
        w.price = fmtPrice(request.limitPrice);
        w.size = sizeStr;
        w.reduceOnly = false;
        w.tif = entryTif;
        w.cloid = entryCloid;
        w.isTrigger = false;
        w.triggerIsMarket = false;
        wires.push_back(w);
    }

    double slippage = config::MARKET_ORDER_SLIPPAGE;

    // 2. TP order (trigger, opposite side, reduceOnly)
    if (request.hasTP()) {
        // TP: for long entry, sell at higher price; for short, buy at lower price
        bool tpIsBuy = !isBuyEntry;
        double tpSlippagePrice = tpIsBuy
            ? request.tpTriggerPx * (1.0 + slippage)   // Buy: accept above trigger
            : request.tpTriggerPx * (1.0 - slippage);  // Sell: accept below trigger

        msgpack::BracketOrderWire w;
        w.asset = meta::getApiAssetId(assetIndex);
        w.isBuy = tpIsBuy;
        w.price = fmtPrice(tpSlippagePrice);
        w.size = sizeStr;
        w.reduceOnly = true;
        w.tif = "";  // ignored for triggers
        w.cloid = tpCloid;
        w.isTrigger = true;
        w.triggerIsMarket = true;
        w.triggerPx = fmtPrice(request.tpTriggerPx);
        w.tpsl = "tp";
        wires.push_back(w);
    }

    // 3. SL order (trigger, opposite side, reduceOnly)
    if (request.hasSL()) {
        bool slIsBuy = !isBuyEntry;
        double slSlippagePrice = slIsBuy
            ? request.slTriggerPx * (1.0 + slippage)
            : request.slTriggerPx * (1.0 - slippage);

        msgpack::BracketOrderWire w;
        w.asset = meta::getApiAssetId(assetIndex);
        w.isBuy = slIsBuy;
        w.price = fmtPrice(slSlippagePrice);
        w.size = sizeStr;
        w.reduceOnly = true;
        w.tif = "";
        w.cloid = slCloid;
        w.isTrigger = true;
        w.triggerIsMarket = true;
        w.triggerPx = fmtPrice(request.slTriggerPx);
        w.tpsl = "sl";
        wires.push_back(w);
    }

    // --- Pack with msgpack for signing ---
    eip712::ByteArray packedAction = msgpack::packBracketOrderAction(wires, "normalTpsl");

    // --- Hash with EIP-712 ---
    // Reuse hashBatchModifyForSigning: generic packed-bytes → EIP-712 hash
    uint64_t nonce = generateNonce();
    bool isMainnet = !g_config.isTestnet;
    std::string vault(g_config.vaultAddress);  // [OPM-202]
    eip712::ByteArray msgHash = eip712::hashBatchModifyForSigning(
        packedAction, isMainnet, nonce, vault);

    if (msgHash.empty() || msgHash.size() != 32) {
        result.error = "Failed to generate EIP-712 message hash";
        logBracket(1, "place", "EIP-712 hash failed");
        return result;
    }

    // --- Sign ---
    crypto::Signature sig;
    if (!crypto::signHash(msgHash.data(), g_config.privateKey, sig)) {
        result.error = "Failed to sign bracket order";
        logBracket(1, "place", "Signing failed");
        return result;
    }

    // --- Build JSON payload ---
    // Construct each order's JSON manually (entry + TP + SL)
    std::string ordersJson;
    for (size_t i = 0; i < wires.size(); i++) {
        const auto& w = wires[i];
        char tField[256];
        if (w.isTrigger) {
            sprintf_s(tField, sizeof(tField),
                "\"t\":{\"trigger\":{\"isMarket\":%s,\"triggerPx\":\"%s\",\"tpsl\":\"%s\"}}",
                w.triggerIsMarket ? "true" : "false",
                w.triggerPx.c_str(), w.tpsl.c_str());
        } else {
            sprintf_s(tField, sizeof(tField),
                "\"t\":{\"limit\":{\"tif\":\"%s\"}}", w.tif.c_str());
        }

        char orderBuf[512];
        sprintf_s(orderBuf, sizeof(orderBuf),
            "{\"a\":%d,\"b\":%s,\"p\":\"%s\",\"s\":\"%s\",\"r\":%s,%s,\"c\":\"%s\"}",
            w.asset, w.isBuy ? "true" : "false",
            w.price.c_str(), w.size.c_str(),
            w.reduceOnly ? "true" : "false",
            tField, w.cloid.c_str());

        if (i > 0) ordersJson += ",";
        ordersJson += orderBuf;
    }

    // [OPM-202] Format vaultAddress for JSON payload
    char vaultJson[128];
    if (vault.empty()) {
        strcpy_s(vaultJson, "null");
    } else {
        sprintf_s(vaultJson, "\"%s\"", vault.c_str());
    }

    char json[4096];
    sprintf_s(json, sizeof(json),
        "{"
            "\"action\":{"
                "\"type\":\"order\","
                "\"orders\":[%s],"
                "\"grouping\":\"normalTpsl\""
            "},"
            "\"nonce\":%llu,"
            "\"signature\":%s,"
            "\"vaultAddress\":%s,"
            "\"expiresAfter\":null"
        "}",
        ordersJson.c_str(),
        nonce,
        sig.toJson().c_str(),
        vaultJson
    );

    if (g_config.diagLevel >= 2) {
        char msg[512];
        sprintf_s(msg, "JSON (first 300): %.300s", json);
        logBracket(2, "place", msg);
    }

    // --- Submit to exchange ---
    http::Response resp = http::exchangePost(json);
    if (!resp.success() || resp.body.empty()) {
        result.error = "HTTP request failed";
        logBracket(1, "place", "HTTP failed");
        return result;
    }

    // --- Parse response ---
    if (g_config.diagLevel >= 2) {
        char msg[512];
        sprintf_s(msg, "Response (first 400): %.400s", resp.body.c_str());
        logBracket(2, "place", msg);
    }

    yyjson_doc* doc = yyjson_read(resp.body.c_str(), resp.body.size(), 0);
    if (!doc) {
        result.error = "Failed to parse exchange response";
        logBracket(1, "place", "JSON parse failed");
        return result;
    }
    yyjson_val* root = yyjson_doc_get_root(doc);

    // Check for error
    const char* statusVal = json::getStringPtr(root, "status");
    if (statusVal && strncmp(statusVal, "err", 3) == 0) {
        yyjson_val* respVal = yyjson_obj_get(root, "response");
        if (respVal && yyjson_is_str(respVal)) {
            result.error = std::string("Exchange error: ") + yyjson_get_str(respVal);
        } else {
            result.error = "Exchange rejected bracket order";
        }
        logBracket(1, "place", result.error.c_str());
        yyjson_doc_free(doc);
        return result;
    }

    // Parse statuses array — one per order in the bracket
    yyjson_val* response = json::getObject(root, "response");
    yyjson_val* rdata = response ? json::getObject(response, "data") : nullptr;
    yyjson_val* statuses = rdata ? json::getArray(rdata, "statuses") : nullptr;

    // Store entry order state
    {
        OrderState state;
        strncpy_s(state.cloid, entryCloid, _TRUNCATE);
        strncpy_s(state.coin, request.coin.c_str(), _TRUNCATE);
        state.side = request.side;
        state.requestedSize = request.size;
        state.zorroTradeId = entryTradeId;
        state.lastUpdate = (double)time(nullptr) / 86400.0 + 25569.0;

        // Extract entry OID and fill from first status
        yyjson_val* s0 = statuses ? yyjson_arr_get(statuses, 0) : nullptr;
        if (s0) {
            yyjson_val* filledObj = json::getObject(s0, "filled");
            yyjson_val* restingObj = json::getObject(s0, "resting");
            if (filledObj) {
                result.entryFilledSize = json::getDouble(filledObj, "totalSz");
                result.entryAvgPrice = json::getDouble(filledObj, "avgPx");
                state.filledSize = result.entryFilledSize;
                state.avgPrice = result.entryAvgPrice;
                state.status = determineFilledStatus(state.filledSize, request.size);
                const char* oid = json::getStringPtr(filledObj, "oid");
                if (oid) strncpy_s(state.orderId, oid, _TRUNCATE);
            } else if (restingObj) {
                state.status = OrderStatus::Open;
                const char* oid = json::getStringPtr(restingObj, "oid");
                if (oid) strncpy_s(state.orderId, oid, _TRUNCATE);
            }
        }
        storeOrder(entryTradeId, state);
    }

    // Store TP order state (if present)
    if (tpTradeId) {
        OrderState state;
        strncpy_s(state.cloid, tpCloid, _TRUNCATE);
        strncpy_s(state.coin, request.coin.c_str(), _TRUNCATE);
        state.side = isBuyEntry ? OrderSide::Sell : OrderSide::Buy;
        state.requestedSize = request.size;
        state.zorroTradeId = tpTradeId;
        state.status = OrderStatus::Open;
        state.lastUpdate = (double)time(nullptr) / 86400.0 + 25569.0;

        yyjson_val* s1 = statuses ? yyjson_arr_get(statuses, 1) : nullptr;
        if (s1) {
            yyjson_val* restingObj = json::getObject(s1, "resting");
            if (restingObj) {
                const char* oid = json::getStringPtr(restingObj, "oid");
                if (oid) strncpy_s(state.orderId, oid, _TRUNCATE);
            }
        }
        storeOrder(tpTradeId, state);
    }

    // Store SL order state (if present)
    if (slTradeId) {
        int slStatusIdx = request.hasTP() ? 2 : 1;  // SL is 2nd or 3rd in array
        OrderState state;
        strncpy_s(state.cloid, slCloid, _TRUNCATE);
        strncpy_s(state.coin, request.coin.c_str(), _TRUNCATE);
        state.side = isBuyEntry ? OrderSide::Sell : OrderSide::Buy;
        state.requestedSize = request.size;
        state.zorroTradeId = slTradeId;
        state.status = OrderStatus::Open;
        state.lastUpdate = (double)time(nullptr) / 86400.0 + 25569.0;

        yyjson_val* sN = statuses ? yyjson_arr_get(statuses, slStatusIdx) : nullptr;
        if (sN) {
            yyjson_val* restingObj = json::getObject(sN, "resting");
            if (restingObj) {
                const char* oid = json::getStringPtr(restingObj, "oid");
                if (oid) strncpy_s(state.orderId, oid, _TRUNCATE);
            }
        }
        storeOrder(slTradeId, state);
    }

    yyjson_doc_free(doc);

    result.success = true;
    result.entryTradeId = entryTradeId;
    result.tpTradeId = tpTradeId;
    result.slTradeId = slTradeId;

    char msg[256];
    sprintf_s(msg, "OK: entry=%d tp=%d sl=%d filled=%.6f @ %.2f",
              entryTradeId, tpTradeId, slTradeId,
              result.entryFilledSize, result.entryAvgPrice);
    logBracket(1, "place", msg);

    return result;
}

} // namespace trading
} // namespace hl
