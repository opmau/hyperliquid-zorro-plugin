//=============================================================================
// hl_meta_spot.cpp - Spot asset metadata fetching
//=============================================================================
// LAYER: Services | DEPENDENCIES: hl_meta.h, hl_globals.h, hl_http.h
//
// Fetches {"type":"spotMeta"} and registers spot trading pairs in the asset
// registry. Spot assets use index=10000+spotIndex, pxDecimals=8-szDecimals,
// and maxLeverage=1 (no leverage for spot).
//=============================================================================

#include "hl_meta.h"
#include "../foundation/hl_globals.h"
#include "../foundation/hl_utils.h"
#include "../transport/hl_http.h"
#include "../transport/json_helpers.h"
#include <cstring>
#include <cstdio>
#include <cmath>
#include <map>
#include <string>

namespace hl {
namespace meta {

// Track how many spot assets were loaded
static int s_spotAssetCount = 0;

/// Log a message using the global logger (matches hl_meta.cpp pattern)
static void logSpot(int level, const char* msg) {
    if (g_config.diagLevel >= level) {
        char buf[512];
        sprintf_s(buf, "fetchSpotMeta: %s", msg);
        g_logger.log(level, buf);
    }
}

// =============================================================================
// SPOT TOKEN INFO (internal)
// =============================================================================

struct SpotTokenInfo {
    char name[32] = {0};
    int szDecimals = 0;
    bool isCanonical = false;
};

// =============================================================================
// fetchSpotMeta IMPLEMENTATION
// =============================================================================

int fetchSpotMeta() {
    s_spotAssetCount = 0;

    http::Response resp = http::infoPost("{\"type\":\"spotMeta\"}", false);
    if (!resp.success()) {
        logSpot(1, "Failed to fetch spotMeta from API");
        return 0;
    }

    yyjson_doc* doc = yyjson_read(resp.body.c_str(), resp.body.size(), 0);
    if (!doc) {
        logSpot(1, "Failed to parse spotMeta JSON");
        return 0;
    }
    yyjson_val* root = yyjson_doc_get_root(doc);

    // --- Parse tokens[] array into a local map ---
    yyjson_val* tokens = json::getArray(root, "tokens");
    if (!tokens) {
        logSpot(1, "No tokens array in spotMeta");
        yyjson_doc_free(doc);
        return 0;
    }

    std::map<int, SpotTokenInfo> tokenMap;
    size_t idx, max;
    yyjson_val* item;
    yyjson_arr_foreach(tokens, idx, max, item) {
        int tokenIndex = (int)json::getInt64(item, "index");
        SpotTokenInfo info;
        json::getString(item, "name", info.name, sizeof(info.name));
        info.szDecimals = (int)json::getInt64(item, "szDecimals");
        info.isCanonical = json::getBool(item, "isCanonical", false);
        tokenMap[tokenIndex] = info;
    }

    if (g_config.diagLevel >= 2) {
        char msg[128];
        sprintf_s(msg, "Parsed %d tokens from spotMeta", (int)tokenMap.size());
        logSpot(2, msg);
    }

    // --- Parse universe[] array (trading pairs) ---
    yyjson_val* universe = json::getArray(root, "universe");
    if (!universe) {
        logSpot(1, "No universe array in spotMeta");
        yyjson_doc_free(doc);
        return 0;
    }

    int startCount = g_assets.count;
    int added = 0;

    // Check capacity
    if (startCount >= config::MAX_ASSETS - 50) {
        char msg[128];
        sprintf_s(msg, "Asset cache nearly full (%d/%d) - skipping spot",
                 startCount, config::MAX_ASSETS);
        logSpot(1, msg);
        yyjson_doc_free(doc);
        return 0;
    }

    yyjson_arr_foreach(universe, idx, max, item) {
        // Get pair index and name
        int spotIndex = (int)json::getInt64(item, "index");
        char pairName[64] = {0};
        json::getString(item, "name", pairName, sizeof(pairName));
        bool isCanonical = json::getBool(item, "isCanonical", false);

        // Get token indices: [baseTokenIndex, quoteTokenIndex]
        yyjson_val* tokensArr = json::getArray(item, "tokens");
        if (!tokensArr || yyjson_arr_size(tokensArr) < 2) continue;

        int baseTokenIdx = (int)yyjson_get_sint(yyjson_arr_get(tokensArr, 0));
        int quoteTokenIdx = (int)yyjson_get_sint(yyjson_arr_get(tokensArr, 1));

        // Resolve base token info for szDecimals
        auto baseIt = tokenMap.find(baseTokenIdx);
        if (baseIt == tokenMap.end()) continue;
        const SpotTokenInfo& baseToken = baseIt->second;

        // Resolve quote token name for display
        auto quoteIt = tokenMap.find(quoteTokenIdx);
        const char* quoteName = quoteIt != tokenMap.end() ? quoteIt->second.name : "USDC";

        // Build display name: "BASE/QUOTE" (e.g., "HYPE/USDC")
        char displayName[64];
        sprintf_s(displayName, "%s/%s", baseToken.name, quoteName);

        // API coin: canonical pairs use their name, non-canonical use @index
        char apiCoin[32];
        if (isCanonical) {
            strncpy_s(apiCoin, pairName, _TRUNCATE);
        } else {
            sprintf_s(apiCoin, "@%d", spotIndex);
        }

        // Spot MAX_DECIMALS = 8
        int szDecimals = baseToken.szDecimals;
        int pxDecimals = 8 - szDecimals;
        if (pxDecimals < 0) pxDecimals = 0;

        // Register asset
        AssetInfo info;
        strncpy_s(info.name, displayName, _TRUNCATE);
        strncpy_s(info.coin, apiCoin, _TRUNCATE);
        info.index = 10000 + spotIndex;
        info.szDecimals = szDecimals;
        info.pxDecimals = pxDecimals;
        info.minSize = pow(10.0, -szDecimals);
        info.tickSize = pow(10.0, -pxDecimals);
        info.maxLeverage = 1;  // No leverage for spot
        info.isPerpDex = false;
        info.isSpot = true;
        strncpy_s(info.spotCoin, apiCoin, _TRUNCATE);

        if (!g_assets.add(info)) {
            logSpot(1, "Asset registry full, stopping spot parse");
            break;
        }

        if (g_config.diagLevel >= 2 && (added < 3 || isCanonical ||
            strcmp(baseToken.name, "HYPE") == 0 ||
            strcmp(baseToken.name, "BTC") == 0)) {
            char msg[256];
            sprintf_s(msg, "Spot[%d]: %s coin=%s (szDec=%d, pxDec=%d, id=%d)",
                     startCount + added, displayName, apiCoin,
                     szDecimals, pxDecimals, 10000 + spotIndex);
            logSpot(2, msg);
        }

        ++added;
    }

    yyjson_doc_free(doc);
    s_spotAssetCount = added;

    if (g_config.diagLevel >= 1 && added > 0) {
        char msg[128];
        sprintf_s(msg, "Cached %d spot pairs (total assets=%d)", added, g_assets.count);
        logSpot(1, msg);
    }

    return added;
}

int getSpotAssetCount() {
    return s_spotAssetCount;
}

} // namespace meta
} // namespace hl
