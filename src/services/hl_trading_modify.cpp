//=============================================================================
// hl_trading_modify.cpp - Atomic order modification (batchModify) [OPM-80]
//=============================================================================
// LAYER: Services | DEPENDENCIES: hl_globals.h, hl_crypto.h, hl_http.h
//
// Implements Hyperliquid's batchModify API action for atomic order modification.
// Verified against Python SDK bulk_modify_orders_new() in exchange.py:195-223.
//=============================================================================

#include "hl_trading_modify.h"
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

// --- Logging helper (matches other trading modules) ---

static void logMsg(int level, const char* prefix, const char* msg) {
    if (g_config.diagLevel >= level) {
        char buf[512];
        sprintf_s(buf, "%s: %s", prefix, msg);
        g_logger.log(level, buf);
    }
}

ModifyResult modifyOrder(const ModifyRequest& request) {
    ModifyResult result;

    // === Validate request ===
    if (request.coin.empty()) {
        result.error = "Coin is required";
        return result;
    }
    if (request.size <= 0) {
        result.error = "Size must be positive";
        return result;
    }
    if (request.limitPrice <= 0) {
        result.error = "Limit price must be positive";
        return result;
    }
    if (!request.useCloid && request.oid == 0) {
        result.error = "Target order ID (oid or cloid) is required";
        return result;
    }
    if (request.useCloid && request.oidCloid.empty()) {
        result.error = "Target cloid is required when useCloid=true";
        return result;
    }

    // === Find asset index ===
    int assetIndex = request.assetIndex;
    if (assetIndex == 0) {
        assetIndex = meta::findAssetIndex(request.coin.c_str());
        if (assetIndex < 0) {
            result.error = "Asset not found in meta";
            return result;
        }
    }

    if (g_config.diagLevel >= 1) {
        char msg[256];
        if (request.useCloid) {
            sprintf_s(msg, "Modify order: cloid=%s %s %s %.6f @ %.2f",
                     request.oidCloid.c_str(),
                     request.side == OrderSide::Buy ? "BUY" : "SELL",
                     request.coin.c_str(), request.size, request.limitPrice);
        } else {
            sprintf_s(msg, "Modify order: oid=%llu %s %s %.6f @ %.2f",
                     (unsigned long long)request.oid,
                     request.side == OrderSide::Buy ? "BUY" : "SELL",
                     request.coin.c_str(), request.size, request.limitPrice);
        }
        logMsg(1, "modifyOrder", msg);
    }

    // === Format price/size per Hyperliquid rules ===
    const AssetInfo* assetInfo = g_assets.getByIndex(assetIndex);
    std::string priceStr, sizeStr;
    if (assetInfo) {
        priceStr = utils::formatPriceForExchange(request.limitPrice, assetInfo->szDecimals);
        sizeStr = utils::formatSize(request.size, assetInfo->szDecimals);
    } else {
        priceStr = eip712::formatNumber(request.limitPrice);
        sizeStr = eip712::formatNumber(request.size);
    }

    // === Determine TIF string ===
    std::string tif;
    switch (request.orderType) {
        case OrderType::Gtc: tif = "Gtc"; break;
        case OrderType::Alo: tif = "Alo"; break;
        default: tif = "Ioc"; break;
    }

    // === STEP 1: Pack action with msgpack ===
    eip712::ByteArray packedAction = msgpack::packBatchModifyAction(
        request.oid,
        request.oidCloid,
        request.useCloid,
        assetIndex,
        (request.side == OrderSide::Buy),
        priceStr,
        sizeStr,
        request.reduceOnly,
        tif,
        request.cloid
    );

    // === STEP 2: Hash with EIP-712 for signing ===
    uint64_t nonce = generateNonce();
    bool isMainnet = !g_config.isTestnet;
    eip712::ByteArray msgHash = eip712::hashBatchModifyForSigning(
        packedAction, isMainnet, nonce, "");

    if (msgHash.empty() || msgHash.size() != 32) {
        result.error = "Failed to generate EIP-712 message hash for modify";
        logMsg(1, "modifyOrder", result.error.c_str());
        return result;
    }

    // === STEP 3: Sign the hash ===
    crypto::Signature sig;
    if (!crypto::signHash(msgHash.data(), g_config.privateKey, sig)) {
        result.error = "Failed to sign modify action";
        logMsg(1, "modifyOrder", result.error.c_str());
        return result;
    }

    // === STEP 4: Build signed JSON payload ===
    // Must match the HTTP wire format for batchModify
    char oidJson[128];
    if (request.useCloid) {
        sprintf_s(oidJson, "\"%s\"", request.oidCloid.c_str());
    } else {
        sprintf_s(oidJson, "%llu", (unsigned long long)request.oid);
    }

    char cloidField[128] = "";
    if (!request.cloid.empty()) {
        sprintf_s(cloidField, ",\"c\":\"%s\"", request.cloid.c_str());
    }

    char modifyJson[2048];
    sprintf_s(modifyJson, sizeof(modifyJson),
        "{"
            "\"action\":{"
                "\"type\":\"batchModify\","
                "\"modifies\":[{"
                    "\"oid\":%s,"
                    "\"order\":{"
                        "\"a\":%d,"
                        "\"b\":%s,"
                        "\"p\":\"%s\","
                        "\"s\":\"%s\","
                        "\"r\":%s,"
                        "\"t\":{\"limit\":{\"tif\":\"%s\"}}"
                        "%s"
                    "}"
                "}]"
            "},"
            "\"nonce\":%llu,"
            "\"signature\":%s,"
            "\"vaultAddress\":null,"
            "\"expiresAfter\":null"
        "}",
        oidJson,
        assetIndex,
        (request.side == OrderSide::Buy) ? "true" : "false",
        priceStr.c_str(),
        sizeStr.c_str(),
        request.reduceOnly ? "true" : "false",
        tif.c_str(),
        cloidField,
        (unsigned long long)nonce,
        sig.toJson().c_str()
    );

    if (g_config.diagLevel >= 2) {
        char msg[512];
        sprintf_s(msg, "Modify JSON (first 200): %.200s", modifyJson);
        logMsg(2, "modifyOrder", msg);
    }

    // === STEP 5: Submit to exchange ===
    http::Response resp = http::exchangePost(modifyJson);
    if (!resp.success()) {
        result.error = "HTTP request to exchange failed";
        logMsg(1, "modifyOrder", result.error.c_str());
        return result;
    }

    // === STEP 6: Parse response ===
    if (resp.body.empty()) {
        result.error = "Exchange response is empty";
        logMsg(1, "modifyOrder", result.error.c_str());
        return result;
    }

    if (g_config.diagLevel >= 2) {
        char msg[512];
        sprintf_s(msg, "Modify response (first 400): %.400s", resp.body.c_str());
        logMsg(2, "modifyOrder", msg);
    }

    yyjson_doc* doc = yyjson_read(resp.body.c_str(), resp.body.size(), 0);
    if (!doc) {
        result.error = "Failed to parse exchange response JSON";
        logMsg(1, "modifyOrder", result.error.c_str());
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
            sprintf_s(errMsg, "Exchange rejected modify (no detail)");
        }
        result.error = errMsg;
        logMsg(1, "modifyOrder", errMsg);
        yyjson_doc_free(doc);
        return result;
    }

    yyjson_doc_free(doc);

    result.success = true;
    logMsg(1, "modifyOrder", "Modify submitted successfully");
    return result;
}

} // namespace trading
} // namespace hl
