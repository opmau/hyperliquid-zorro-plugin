//=============================================================================
// hl_trading_margin.cpp - Per-asset leverage and isolated margin [OPM-1584]
//=============================================================================
// LAYER: Services | DEPENDENCIES: hl_meta.h, hl_crypto.h, hl_http.h
//
// updateLeverage and updateIsolatedMargin are L1 actions signed the same way
// as an order: msgpack(action) + nonce + vault -> connectionId -> EIP-712.
//=============================================================================

#include "hl_trading_margin.h"
#include "hl_trading_service.h"
#include "hl_trading_response.h"
#include "hl_meta.h"
#include "../foundation/hl_globals.h"
#include "../foundation/hl_crypto.h"
#include "../foundation/hl_eip712.h"
#include "../foundation/hl_msgpack.h"
#include "../transport/hl_http.h"
#include "../transport/json_helpers.h"
#include <cstdio>
#include <cstring>

namespace hl {
namespace trading {

static void logMsg(int level, const char* prefix, const char* msg) {
    if (g_config.diagLevel >= level) {
        char buf[512];
        sprintf_s(buf, "%s: %s", prefix, msg);
        g_logger.log(level, buf);
    }
}

/// Record a refusal where HL_GET_LAST_ORDER_ERROR can read it, and log it.
static bool fail(const char* prefix, const char* msg) {
    setLastOrderError(msg);
    logMsg(1, prefix, msg);
    return false;
}

/// Resolve a coin to its registry entry. Writes the refusal on failure.
static const AssetInfo* lookupAsset(const char* prefix, const char* coin) {
    if (!coin || !*coin) {
        fail(prefix, "No symbol selected (call SET_SYMBOL first)");
        return nullptr;
    }
    const AssetInfo* asset = g_assets.getByIndex(meta::findAssetIndex(coin));
    if (!asset) {
        char msg[96];
        sprintf_s(msg, "Asset not found in meta: %.48s", coin);
        fail(prefix, msg);
        return nullptr;
    }
    if (asset->isSpot) {
        fail(prefix, "Spot assets have no leverage or margin");
        return nullptr;
    }
    return asset;
}

/// Sign a packed L1 action, post it, and read the exchange's verdict.
/// @param actionJson  The same action as JSON, fields in the packed order
static bool submitL1(const char* prefix, const msgpack::ByteArray& packed,
                     const char* actionJson) {
    uint64_t nonce = generateNonce();
    std::string vault(g_config.vaultAddress);
    eip712::ByteArray msgHash = eip712::hashBatchModifyForSigning(
        packed, !g_config.isTestnet, nonce, vault);
    if (msgHash.size() != 32) {
        return fail(prefix, "Failed to generate EIP-712 message hash");
    }

    crypto::Signature sig;
    if (!crypto::signHash(msgHash.data(), g_config.privateKey, sig)) {
        return fail(prefix, "Failed to sign action");
    }

    char vaultJson[128];
    if (vault.empty()) {
        strcpy_s(vaultJson, "null");
    } else {
        sprintf_s(vaultJson, "\"%s\"", vault.c_str());
    }

    char json[768];
    sprintf_s(json, sizeof(json),
        "{\"action\":%s,\"nonce\":%llu,\"signature\":%s,"
        "\"vaultAddress\":%s,\"expiresAfter\":null}",
        actionJson, (unsigned long long)nonce, sig.toJson().c_str(), vaultJson);

    if (g_config.diagLevel >= 2) {
        char msg[320];
        sprintf_s(msg, "payload (first 256): %.256s", json);
        logMsg(2, prefix, msg);
    }

    http::Response resp = http::exchangePost(json);
    if (!resp.success()) {
        // Not retried: the action is signed and nonced, and may have landed.
        return fail(prefix, "HTTP request failed (outcome unknown)");
    }

    yyjson_doc* doc = yyjson_read(resp.body.c_str(), resp.body.size(), 0);
    if (!doc) {
        return fail(prefix, "Unparseable exchange response");
    }
    yyjson_val* root = yyjson_doc_get_root(doc);
    const char* st = json::getStringPtr(root, "status");
    bool ok = st && strcmp(st, "ok") == 0;
    if (!ok) {
        char errMsg[256];
        yyjson_val* respVal = yyjson_obj_get(root, "response");
        if (respVal && yyjson_is_str(respVal)) {
            sprintf_s(errMsg, "Exchange error: %.200s", yyjson_get_str(respVal));
        } else {
            sprintf_s(errMsg, "Exchange rejected the action");
        }
        yyjson_doc_free(doc);
        return fail(prefix, errMsg);
    }
    yyjson_doc_free(doc);
    return true;
}

// =============================================================================
// POSITION READ
// =============================================================================

struct MarginPosition {
    bool found = false;
    double szi = 0.0;          // Signed size: negative = short
    bool isolated = false;
    double marginUsed = 0.0;
};

/// Read one asset's position from clearinghouseState over HTTP. Fresh rather
/// than cached: the account cache does not carry the margin type, and a
/// margin change is rare enough that one request is cheap.
static bool readPosition(const AssetInfo& asset, MarginPosition& out) {
    const char* user = g_config.vaultAddress[0] ? g_config.vaultAddress
                                                : g_config.walletAddress;
    char body[256];
    sprintf_s(body, "{\"type\":\"clearinghouseState\",\"user\":\"%s\"}", user);
    http::Response resp = asset.isPerpDex
        ? http::infoPostPerpDex(body, asset.perpDex, false)
        : http::infoPost(body, false);
    if (!resp.success() || resp.body.empty()) return false;

    // Builder-dex positions are reported as "dex:COIN".
    char apiCoin[72];
    if (asset.isPerpDex) {
        sprintf_s(apiCoin, "%s:%s", asset.perpDex, asset.coin);
    } else {
        strcpy_s(apiCoin, asset.coin);
    }

    yyjson_doc* doc = yyjson_read(resp.body.c_str(), resp.body.size(), 0);
    if (!doc) return false;
    yyjson_val* positions = json::getArray(yyjson_doc_get_root(doc), "assetPositions");
    size_t idx, max;
    yyjson_val* item;
    yyjson_arr_foreach(positions, idx, max, item) {
        yyjson_val* pos = json::getObject(item, "position");
        const char* coin = json::getStringPtr(pos, "coin");
        if (!coin || _stricmp(coin, apiCoin) != 0) continue;
        out.found = true;
        out.szi = json::getDouble(pos, "szi");
        out.marginUsed = json::getDouble(pos, "marginUsed");
        const char* type = json::getStringPtr(json::getObject(pos, "leverage"), "type");
        out.isolated = type && strcmp(type, "isolated") == 0;
        break;
    }
    yyjson_doc_free(doc);
    return true;
}

// =============================================================================
// PUBLIC API
// =============================================================================

int getMarginMode(const char* coin) {
    if (!coin || !*coin) return MARGIN_MODE_UNKNOWN;
    const AssetInfo* asset = g_assets.getByIndex(meta::findAssetIndex(coin));
    if (!asset || asset->isSpot) return MARGIN_MODE_UNKNOWN;
    return marginModeOf(*asset);
}

bool updateLeverage(const char* coin, int leverage, bool isCross) {
    const char* prefix = "updateLeverage";
    clearLastOrderError();
    const AssetInfo* asset = lookupAsset(prefix, coin);
    if (!asset) return false;

    char msg[160];
    if (leverage < 1 || leverage > asset->maxLeverage) {
        sprintf_s(msg, "Leverage %d outside 1..%d for %s",
                  leverage, asset->maxLeverage, asset->name);
        return fail(prefix, msg);
    }
    int mode = marginModeOf(*asset);
    if (isCross && (mode == MARGIN_MODE_ISOLATED_ONLY ||
                    mode == MARGIN_MODE_STRICT_ISOLATED)) {
        sprintf_s(msg, "%s is isolated-only; cross margin refused", asset->name);
        return fail(prefix, msg);
    }

    int apiAsset = meta::getApiAssetId(meta::findAssetIndex(coin));
    msgpack::ByteArray packed =
        msgpack::packUpdateLeverageAction(apiAsset, isCross, leverage);
    char action[160];
    sprintf_s(action,
        "{\"type\":\"updateLeverage\",\"asset\":%d,\"isCross\":%s,\"leverage\":%d}",
        apiAsset, isCross ? "true" : "false", leverage);
    if (!submitL1(prefix, packed, action)) return false;

    sprintf_s(msg, "%s set to %dx %s", asset->name, leverage,
              isCross ? "cross" : "isolated");
    logMsg(1, prefix, msg);
    return true;
}

bool adjustIsolatedMargin(const char* coin, double usd) {
    const char* prefix = "updateIsolatedMargin";
    clearLastOrderError();
    const AssetInfo* asset = lookupAsset(prefix, coin);
    if (!asset) return false;

    char msg[160];
    int64_t ntli = usdToNtli(usd);
    if (ntli == 0) {
        return fail(prefix, "Amount rounds to zero");
    }
    if (ntli < 0 && marginModeOf(*asset) == MARGIN_MODE_STRICT_ISOLATED) {
        sprintf_s(msg, "%s is strictIsolated; margin cannot be removed", asset->name);
        return fail(prefix, msg);
    }

    MarginPosition pos;
    if (!readPosition(*asset, pos)) {
        return fail(prefix, "Position read (clearinghouseState) failed");
    }
    if (!pos.found || pos.szi == 0.0) {
        sprintf_s(msg, "No open position in %s", asset->name);
        return fail(prefix, msg);
    }
    if (!pos.isolated) {
        sprintf_s(msg, "%s position is cross margin, not isolated", asset->name);
        return fail(prefix, msg);
    }

    // isBuy names the position's side (the exchange ignores it outside
    // hedge mode, but it is part of the signed action).
    bool isBuy = pos.szi > 0;
    int apiAsset = meta::getApiAssetId(meta::findAssetIndex(coin));
    msgpack::ByteArray packed =
        msgpack::packUpdateIsolatedMarginAction(apiAsset, isBuy, ntli);
    char action[160];
    sprintf_s(action,
        "{\"type\":\"updateIsolatedMargin\",\"asset\":%d,\"isBuy\":%s,\"ntli\":%lld}",
        apiAsset, isBuy ? "true" : "false", (long long)ntli);
    if (!submitL1(prefix, packed, action)) return false;

    sprintf_s(msg, "%s isolated margin %+.2f USD (was %.2f)",
              asset->name, usd, pos.marginUsed);
    logMsg(1, prefix, msg);
    return true;
}

} // namespace trading
} // namespace hl
