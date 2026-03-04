//=============================================================================
// hl_trading_twap.cpp - TWAP order placement and cancellation [OPM-81]
//=============================================================================
// LAYER: Services
// DEPENDENCIES: hl_trading_service.h, hl_eip712.h, hl_crypto.h, hl_http.h
// THREAD SAFETY: Thread-safe (uses g_config, g_logger — read-only globals)
//
// TWAP (Time-Weighted Average Price) orders split a large order into smaller
// suborders executed at regular intervals (~30 seconds) to minimize market
// impact. The exchange manages all suborder execution; the plugin only
// submits the initial request and can cancel via TWAP ID.
//
// Wire format verified against:
//   - Chainstack TWAP guide (action dict structure)
//   - Hyperliquid API docs (06-exchange-endpoint.md:1349-1501)
//   - Python SDK signing.py (sign_l1_action flow)
//=============================================================================

#include "hl_trading_twap.h"
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

namespace hl {
namespace trading {

// =============================================================================
// LOGGING HELPER
// =============================================================================

static void logTwap(int level, const char* prefix, const char* msg) {
    if (g_config.diagLevel >= level) {
        char buf[512];
        sprintf_s(buf, "TWAP %s: %s", prefix, msg);
        g_logger.log(level, buf);
    }
}

// =============================================================================
// TWAP ORDER PLACEMENT
// =============================================================================

TwapResult placeTwapOrder(const TwapRequest& request) {
    TwapResult result;

    // Validate
    if (request.coin.empty()) {
        result.error = "Coin is required";
        return result;
    }
    if (request.size <= 0) {
        result.error = "Size must be positive";
        return result;
    }
    if (request.durationMinutes <= 0) {
        result.error = "Duration must be positive";
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

    // Format size
    const AssetInfo* assetInfo = g_assets.getByIndex(assetIndex);
    std::string sizeStr;
    if (assetInfo) {
        sizeStr = utils::formatSize(request.size, assetInfo->szDecimals);
    } else {
        sizeStr = eip712::formatNumber(request.size);
    }

    bool isBuy = (request.side == OrderSide::Buy);

    if (g_config.diagLevel >= 1) {
        char msg[256];
        sprintf_s(msg, "Placing: %s %s %s (%d min, %s)",
                 isBuy ? "BUY" : "SELL", request.coin.c_str(), sizeStr.c_str(),
                 request.durationMinutes, request.randomize ? "randomized" : "fixed");
        logTwap(1, "place", msg);
    }

    // Build EIP-712 TWAP order action
    eip712::TwapOrderAction twapAction;
    twapAction.asset = meta::getApiAssetId(assetIndex);
    twapAction.isBuy = isBuy;
    twapAction.size = sizeStr;
    twapAction.reduceOnly = request.reduceOnly;
    twapAction.minutes = request.durationMinutes;
    twapAction.randomize = request.randomize;

    // Hash with EIP-712 for signing
    uint64_t nonce = generateNonce();
    bool isMainnet = !g_config.isTestnet;
    eip712::ByteArray msgHash = eip712::hashTwapOrderForSigning(
        twapAction, isMainnet, nonce, "");

    if (msgHash.empty() || msgHash.size() != 32) {
        result.error = "Failed to generate EIP-712 message hash";
        logTwap(1, "place", "EIP-712 hash failed");
        return result;
    }

    // Sign the hash
    crypto::Signature sig;
    if (!crypto::signHash(msgHash.data(), g_config.privateKey, sig)) {
        result.error = "Failed to sign TWAP order";
        logTwap(1, "place", "Signing failed");
        return result;
    }

    // Build signed TWAP order JSON
    char json[1024];
    sprintf_s(json, sizeof(json),
        "{"
            "\"action\":{"
                "\"type\":\"twapOrder\","
                "\"twap\":{"
                    "\"a\":%d,"
                    "\"b\":%s,"
                    "\"s\":\"%s\","
                    "\"r\":%s,"
                    "\"m\":%d,"
                    "\"t\":%s"
                "}"
            "},"
            "\"nonce\":%llu,"
            "\"signature\":%s,"
            "\"vaultAddress\":null,"
            "\"expiresAfter\":null"
        "}",
        assetIndex,
        isBuy ? "true" : "false",
        sizeStr.c_str(),
        request.reduceOnly ? "true" : "false",
        request.durationMinutes,
        request.randomize ? "true" : "false",
        nonce,
        sig.toJson().c_str()
    );

    if (g_config.diagLevel >= 2) {
        char msg[512];
        sprintf_s(msg, "JSON (first 200): %.200s", json);
        logTwap(2, "place", msg);
    }

    // Submit to exchange
    http::Response resp = http::exchangePost(json);
    if (!resp.success() || resp.body.empty()) {
        result.error = "HTTP request failed";
        logTwap(1, "place", "HTTP failed");
        return result;
    }

    // Parse response
    if (g_config.diagLevel >= 2) {
        char msg[512];
        sprintf_s(msg, "Response (first 400): %.400s", resp.body.c_str());
        logTwap(2, "place", msg);
    }

    yyjson_doc* doc = yyjson_read(resp.body.c_str(), resp.body.size(), 0);
    if (!doc) {
        result.error = "Failed to parse exchange response";
        logTwap(1, "place", "JSON parse failed");
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
            result.error = "Exchange rejected TWAP order";
        }
        logTwap(1, "place", result.error.c_str());
        yyjson_doc_free(doc);
        return result;
    }

    // Extract TWAP ID from response
    // Response format: {"status":"ok","response":{"type":"twapOrder","data":{"id":12345}}}
    yyjson_val* response = json::getObject(root, "response");
    yyjson_val* rdata = response ? json::getObject(response, "data") : nullptr;

    if (rdata) {
        yyjson_val* idVal = yyjson_obj_get(rdata, "id");
        if (idVal) {
            if (yyjson_is_int(idVal))
                result.twapId = (uint64_t)yyjson_get_sint(idVal);
            else if (yyjson_is_real(idVal))
                result.twapId = (uint64_t)yyjson_get_real(idVal);
        }
    }

    yyjson_doc_free(doc);

    result.success = true;

    char msg[128];
    sprintf_s(msg, "OK: twapId=%llu", result.twapId);
    logTwap(1, "place", msg);

    return result;
}

// =============================================================================
// TWAP ORDER CANCELLATION
// =============================================================================

bool cancelTwapOrder(const char* coin, uint64_t twapId) {
    if (!coin || twapId == 0) return false;

    int assetIndex = meta::findAssetIndex(coin);
    if (assetIndex < 0) {
        logTwap(1, "cancel", "Asset not found");
        return false;
    }

    if (g_config.diagLevel >= 1) {
        char msg[128];
        sprintf_s(msg, "Cancelling: %s twapId=%llu", coin, twapId);
        logTwap(1, "cancel", msg);
    }

    // Build EIP-712 TWAP cancel action
    eip712::TwapCancelAction cancelAction;
    cancelAction.asset = meta::getApiAssetId(assetIndex);
    cancelAction.twapId = twapId;

    uint64_t nonce = generateNonce();
    bool isMainnet = !g_config.isTestnet;
    eip712::ByteArray msgHash = eip712::hashTwapCancelForSigning(
        cancelAction, isMainnet, nonce, "");

    if (msgHash.empty() || msgHash.size() != 32) {
        logTwap(1, "cancel", "EIP-712 hash failed");
        return false;
    }

    crypto::Signature sig;
    if (!crypto::signHash(msgHash.data(), g_config.privateKey, sig)) {
        logTwap(1, "cancel", "Signing failed");
        return false;
    }

    // Build signed cancel JSON
    char json[512];
    sprintf_s(json, sizeof(json),
        "{"
            "\"action\":{"
                "\"type\":\"twapCancel\","
                "\"a\":%d,"
                "\"t\":%llu"
            "},"
            "\"nonce\":%llu,"
            "\"signature\":%s,"
            "\"vaultAddress\":null,"
            "\"expiresAfter\":null"
        "}",
        assetIndex,
        twapId,
        nonce,
        sig.toJson().c_str()
    );

    http::Response resp = http::exchangePost(json);
    if (!resp.success()) {
        logTwap(1, "cancel", "HTTP failed");
        return false;
    }

    // Check for error in response
    if (!resp.body.empty()) {
        yyjson_doc* doc = yyjson_read(resp.body.c_str(), resp.body.size(), 0);
        if (doc) {
            yyjson_val* root = yyjson_doc_get_root(doc);
            const char* st = json::getStringPtr(root, "status");
            if (st && strncmp(st, "err", 3) == 0) {
                logTwap(1, "cancel", "Exchange rejected cancel");
                yyjson_doc_free(doc);
                return false;
            }
            yyjson_doc_free(doc);
        }
    }

    logTwap(1, "cancel", "OK");
    return true;
}

} // namespace trading
} // namespace hl
