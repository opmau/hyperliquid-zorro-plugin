//=============================================================================
// hl_meta.cpp - Asset metadata caching implementation
//=============================================================================
// LAYER: Services | DEPENDENCIES: hl_globals.h, hl_http.h, hl_utils.h
//=============================================================================

#include "hl_meta.h"
#include "../foundation/hl_globals.h"
#include "../foundation/hl_utils.h"
#include "../transport/hl_http.h"
#include "../transport/json_helpers.h"
#include "../transport/ws_manager.h"
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cmath>

namespace hl {
namespace meta {

// =============================================================================
// INTERNAL STATE (module-level, protected by g_assets.cs)
// =============================================================================

// Map perpDex name -> offset (e.g., "xyz" -> 110000)
static std::map<std::string, int> s_perpDexOffsets;

// Map coin name -> PerpDexInfo (e.g., "XYZ100" -> {perpDex="xyz", localIndex=0, offset=110000})
static std::map<std::string, PerpDexInfo> s_perpDexMap;

// Track how many main perp assets were loaded (before perpDex assets)
static int s_mainAssetCount = 0;

// Track if meta has been loaded
static bool s_metaLoaded = false;

// Map token index -> token name (for collateral resolution, from spotMeta)
static std::map<int, std::string> s_tokenNames;

// =============================================================================
// INTERNAL HELPERS
// =============================================================================

// parseFieldBackwards and findArrayEnd removed — replaced by yyjson navigation

/// Log a message using the global logger
static void logMsg(int level, const char* prefix, const char* msg) {
    if (g_config.diagLevel >= level) {
        char buf[512];
        sprintf_s(buf, "%s: %s", prefix, msg);
        g_logger.log(level, buf);
    }
}

/// Fetch token index -> name mapping from spotMeta (lazy, fetches once)
/// Needed to resolve collateral token IDs to names (e.g., 0 -> "USDC")
static void ensureTokenNames() {
    if (!s_tokenNames.empty()) return;

    http::Response resp = http::infoPost("{\"type\":\"spotMeta\"}", false);
    if (!resp.success()) {
        // Fallback: at minimum, token 0 = USDC (always true for Hyperliquid)
        s_tokenNames[0] = "USDC";
        return;
    }

    yyjson_doc* doc = yyjson_read(resp.body.c_str(), resp.body.size(), 0);
    if (!doc) { s_tokenNames[0] = "USDC"; return; }

    yyjson_val* root = yyjson_doc_get_root(doc);
    yyjson_val* tokens = json::getArray(root, "tokens");
    if (tokens) {
        size_t idx, max;
        yyjson_val* item;
        yyjson_arr_foreach(tokens, idx, max, item) {
            int tokenIndex = (int)json::getInt64(item, "index");
            char tokenName[32] = {0};
            if (json::getString(item, "name", tokenName, sizeof(tokenName))) {
                s_tokenNames[tokenIndex] = tokenName;
            }
        }
    }
    yyjson_doc_free(doc);

    // Ensure USDC fallback
    if (s_tokenNames.find(0) == s_tokenNames.end()) {
        s_tokenNames[0] = "USDC";
    }

    if (g_config.diagLevel >= 2) {
        char msg[64];
        sprintf_s(msg, "Loaded %d token names for collateral resolution",
                 (int)s_tokenNames.size());
        logMsg(2, "ensureTokenNames", msg);
    }
}

/// Resolve token index to name, with "USDC" fallback
static const char* resolveTokenName(int tokenIndex) {
    auto it = s_tokenNames.find(tokenIndex);
    if (it != s_tokenNames.end()) return it->second.c_str();
    return "USDC";
}

// =============================================================================
// META FETCHING
// =============================================================================

int fetchMeta() {
    clearMeta();

    // Fetch main meta (crypto perps) - needs large buffer (response is ~16KB+)
    http::Response resp = http::infoPost("{\"type\":\"meta\"}", false);
    if (!resp.success()) {
        logMsg(1, "fetchMeta", "Failed to fetch meta from API");
        return 0;
    }

    yyjson_doc* doc = yyjson_read(resp.body.c_str(), resp.body.size(), 0);
    if (!doc) {
        logMsg(1, "fetchMeta", "Failed to parse meta JSON");
        return 0;
    }
    yyjson_val* root = yyjson_doc_get_root(doc);
    yyjson_val* universe = json::getArray(root, "universe");
    if (!universe) {
        logMsg(1, "fetchMeta", "No universe array found in meta response");
        yyjson_doc_free(doc);
        return 0;
    }

    int count = 0;
    size_t idx, max;
    yyjson_val* item;
    yyjson_arr_foreach(universe, idx, max, item) {
        char coinName[32] = {0};
        if (!json::getString(item, "name", coinName, sizeof(coinName)))
            continue;

        int szDecimals = (int)json::getInt64(item, "szDecimals");
        if (szDecimals < 0) szDecimals = 4;  // 0 is valid (ADA, DOGE, TRX, XRP)
        int maxLeverage = (int)json::getInt64(item, "maxLeverage");
        if (maxLeverage <= 0) maxLeverage = 50;

        // Calculate price decimals: Hyperliquid uses MAX_DECIMALS(6) - szDecimals
        int pxDecimals = 6 - szDecimals;
        if (pxDecimals < 0) pxDecimals = 0;

        // Create AssetInfo and add to registry
        // Main perps always use USDC collateral (token 0)
        std::string displayName = utils::buildCoinName("", coinName, "USDC");

        AssetInfo info;
        strncpy_s(info.name, displayName.c_str(), _TRUNCATE);
        strncpy_s(info.coin, coinName, _TRUNCATE);
        strncpy_s(info.collateral, "USDC", _TRUNCATE);
        info.index = count;
        info.szDecimals = szDecimals;
        info.pxDecimals = pxDecimals;
        info.minSize = pow(10.0, -szDecimals);
        info.tickSize = pow(10.0, -pxDecimals);
        info.maxLeverage = maxLeverage;
        info.isPerpDex = false;
        info.perpDex[0] = 0;
        info.localIndex = 0;
        info.perpDexOffset = 0;

        if (!g_assets.add(info)) {
            logMsg(1, "fetchMeta", "Asset registry full, stopping parse");
            break;
        }

        // Log sample assets at diag level 2+
        if (g_config.diagLevel >= 2 && (count < 3 ||
            strcmp(coinName, "BTC") == 0 || strcmp(coinName, "ETH") == 0)) {
            char msg[128];
            sprintf_s(msg, "Meta[%d]: %s (coin=%s, szDec=%d, maxLev=%d)",
                     count, displayName.c_str(), coinName, szDecimals, maxLeverage);
            logMsg(2, "fetchMeta", msg);
        }

        ++count;
    }

    yyjson_doc_free(doc);
    s_mainAssetCount = count;
    s_metaLoaded = true;

    if (g_config.diagLevel >= 2) {
        char msg[64];
        sprintf_s(msg, "Cached %d assets from meta universe", count);
        logMsg(2, "fetchMeta", msg);
    }

    return count;
}

void clearMeta() {
    g_assets.clear();
    s_perpDexOffsets.clear();
    s_perpDexMap.clear();
    s_tokenNames.clear();
    s_mainAssetCount = 0;
    s_metaLoaded = false;
}

int fetchPerpDexList() {
    s_perpDexOffsets.clear();
    s_perpDexOffsets[""] = 0; // Default perpDex has offset 0

    logMsg(2, "fetchPerpDexList", "Fetching perpDex list...");

    http::Response resp = http::infoPost("{\"type\":\"perpDexs\"}", false);
    if (!resp.success()) {
        logMsg(1, "fetchPerpDexList", "API request failed");
        return 0;
    }

    // Parse perpDex names from response
    // Format: [null, {"name":"xyz",...}, {"name":"flx",...}, ...]
    yyjson_doc* doc = yyjson_read(resp.body.c_str(), resp.body.size(), 0);
    if (!doc) return 0;
    yyjson_val* root = yyjson_doc_get_root(doc);
    if (!yyjson_is_arr(root)) { yyjson_doc_free(doc); return 0; }

    int perpDexIndex = 0;
    size_t idx, max;
    yyjson_val* item;
    yyjson_arr_foreach(root, idx, max, item) {
        if (!yyjson_is_obj(item)) continue; // skip null entries

        char perpDexName[64] = {0};
        if (!json::getString(item, "name", perpDexName, sizeof(perpDexName)))
            continue;

        // Calculate offset (first perpDex = 110000, second = 120000, etc.)
        int offset = 110000 + (perpDexIndex * 10000);
        s_perpDexOffsets[std::string(perpDexName)] = offset;

        if (g_config.diagLevel >= 2) {
            char msg[128];
            sprintf_s(msg, "Found '%s' at offset %d", perpDexName, offset);
            logMsg(2, "fetchPerpDexList", msg);
        }

        ++perpDexIndex;
    }
    yyjson_doc_free(doc);

    if (g_config.diagLevel >= 2) {
        char msg[64];
        sprintf_s(msg, "Total %d perpDex universes", perpDexIndex);
        logMsg(2, "fetchPerpDexList", msg);
    }

    return perpDexIndex;
}

int fetchPerpDexMeta(const char* perpDex) {
    if (!perpDex || !perpDex[0]) return 0;

    // Ensure perpDex offsets are initialized
    if (s_perpDexOffsets.empty()) {
        fetchPerpDexList();
    }

    // Ensure token names are loaded (for collateral resolution)
    ensureTokenNames();

    // Get offset for this perpDex
    int perpDexOffset = 0;
    auto it = s_perpDexOffsets.find(std::string(perpDex));
    if (it != s_perpDexOffsets.end()) {
        perpDexOffset = it->second;
    }

    // Build query with dex parameter
    char payload[256];
    sprintf_s(payload, "{\"type\":\"meta\",\"dex\":\"%s\"}", perpDex);

    http::Response resp = http::infoPost(payload, false);
    if (!resp.success()) {
        logMsg(1, "fetchPerpDexMeta", "API request failed");
        return 0;
    }

    if (g_config.diagLevel >= 2) {
        char msg[128];
        sprintf_s(msg, "Querying meta for perpDex: %s", perpDex);
        logMsg(2, "fetchPerpDexMeta", msg);
    }

    int startCount = g_assets.count;
    int localIdx = 0;
    int added = 0;

    // Check capacity
    if (startCount >= config::MAX_ASSETS - 50) {
        char msg[128];
        sprintf_s(msg, "Asset cache nearly full (%d/%d) - refusing perpDex '%s'",
                 startCount, config::MAX_ASSETS, perpDex);
        logMsg(1, "fetchPerpDexMeta", msg);
        return 0;
    }

    // Parse assets from perpDex meta using yyjson
    yyjson_doc* doc = yyjson_read(resp.body.c_str(), resp.body.size(), 0);
    if (!doc) return 0;
    yyjson_val* root = yyjson_doc_get_root(doc);
    yyjson_val* universe = json::getArray(root, "universe");
    if (!universe) { yyjson_doc_free(doc); return 0; }

    // Resolve collateral token name from meta response
    // The meta response includes "collateralToken" field (token index)
    int collateralTokenId = (int)json::getInt64(root, "collateralToken");
    const char* collateralName = resolveTokenName(collateralTokenId);

    if (g_config.diagLevel >= 2) {
        char msg[128];
        sprintf_s(msg, "PerpDex '%s' collateral: token %d = %s",
                 perpDex, collateralTokenId, collateralName);
        logMsg(2, "fetchPerpDexMeta", msg);
    }

    size_t idx, max;
    yyjson_val* item;
    yyjson_arr_foreach(universe, idx, max, item) {
        char rawCoinName[32] = {0};
        if (!json::getString(item, "name", rawCoinName, sizeof(rawCoinName)))
            continue;

        // Strip perpDex API prefix (e.g., "xyz:TSLA" -> "TSLA") [OPM-140]
        char coinName[32] = {0};
        utils::stripApiCoinPrefix(rawCoinName, coinName, sizeof(coinName));

        int szDecimals = (int)json::getInt64(item, "szDecimals");
        if (szDecimals < 0) szDecimals = 4;  // 0 is valid (whole-unit assets)
        int maxLeverage = (int)json::getInt64(item, "maxLeverage");
        if (maxLeverage <= 0) maxLeverage = 20;

        // Build display name: COIN-COLLATERAL.venue (e.g., "GOLD-USDC.xyz")
        std::string displayName = utils::buildCoinName(perpDex, coinName, collateralName);

        // PerpDex uses same formula as regular perps (MAX_DECIMALS=6)
        int pxDecimals = 6 - szDecimals;
        if (pxDecimals < 0) pxDecimals = 0;

        // Create AssetInfo
        AssetInfo info;
        strncpy_s(info.name, displayName.c_str(), _TRUNCATE);
        strncpy_s(info.coin, coinName, _TRUNCATE);
        strncpy_s(info.collateral, collateralName, _TRUNCATE);
        info.index = startCount + added;
        info.szDecimals = szDecimals;
        info.pxDecimals = pxDecimals;
        info.minSize = pow(10.0, -szDecimals);
        info.tickSize = pow(10.0, -pxDecimals);
        info.maxLeverage = maxLeverage;
        info.isPerpDex = true;
        strncpy_s(info.perpDex, perpDex, _TRUNCATE);
        info.localIndex = localIdx;
        info.perpDexOffset = perpDexOffset;

        if (!g_assets.add(info)) {
            logMsg(1, "fetchPerpDexMeta", "Asset registry full");
            break;
        }

        // Store perpDex mapping (coin name without prefix -> PerpDexInfo)
        PerpDexInfo pdInfo;
        pdInfo.perpDex = perpDex;
        pdInfo.localIndex = localIdx;
        pdInfo.offset = perpDexOffset;
        s_perpDexMap[std::string(coinName)] = pdInfo;

        if (g_config.diagLevel >= 2) {
            char msg[256];
            sprintf_s(msg, "Cached %s asset[%d]: %s (coin=%s, szDec=%d, maxLev=%d)",
                     perpDex, startCount + added, displayName.c_str(),
                     coinName, szDecimals, maxLeverage);
            logMsg(2, "fetchPerpDexMeta", msg);
        }

        ++added;
        ++localIdx;
    }
    yyjson_doc_free(doc);

    if (g_config.diagLevel >= 1 && added > 0) {
        char msg[128];
        sprintf_s(msg, "Added %d assets from perpDex '%s' collateral=%s (total=%d)",
                 added, perpDex, collateralName, g_assets.count);
        logMsg(1, "fetchPerpDexMeta", msg);
    }

    return added;
}

int fetchAllPerpDexMeta() {
    // Ensure perpDex list is fetched
    if (s_perpDexOffsets.empty() || s_perpDexOffsets.size() <= 1) {
        fetchPerpDexList();
    }

    int totalAdded = 0;
    for (const auto& entry : s_perpDexOffsets) {
        if (!entry.first.empty()) { // Skip default empty perpDex
            int added = fetchPerpDexMeta(entry.first.c_str());
            totalAdded += added;
        }
    }

    if (g_config.diagLevel >= 2 && totalAdded > 0) {
        char msg[64];
        sprintf_s(msg, "Total perpDex assets cached: %d", totalAdded);
        logMsg(2, "fetchAllPerpDexMeta", msg);
    }

    return totalAdded;
}

// =============================================================================
// ASSET LOOKUP
// =============================================================================

int findAssetIndex(const char* coin) {
    if (!coin || !*coin) return -1;

    // Try display name lookup (handles "BTC-USDC", "GOLD-USDC.xyz", "BTC/USDC")
    int idx = g_assets.findByName(coin);
    if (idx >= 0) return idx;

    // Try API coin name lookup (handles "BTC", "GOLD", "@107")
    idx = g_assets.findByCoin(coin);
    if (idx >= 0) return idx;

    // For perpDex assets, bare coin name fallback (e.g., "TSLA" -> first matching)
    auto it = s_perpDexMap.find(std::string(coin));
    if (it != s_perpDexMap.end()) {
        // Reconstruct display name and search
        const AssetInfo* asset = nullptr;
        for (int i = 0; i < g_assets.count; ++i) {
            asset = g_assets.getByIndex(i);
            if (asset && asset->isPerpDex &&
                strcmp(asset->coin, coin) == 0 &&
                strcmp(asset->perpDex, it->second.perpDex.c_str()) == 0) {
                return i;
            }
        }
    }

    return -1;
}

bool convertPerpDexIndexToCoin(const char* positionCoin, char* outCoin, size_t outSize) {
    if (!positionCoin || positionCoin[0] != '@' || !outCoin || outSize == 0) return false;

    int assetIndex = atoi(positionCoin + 1);
    if (assetIndex <= 0) return false;

    // Search perpDex map for matching index
    for (const auto& entry : s_perpDexMap) {
        int fullIndex = entry.second.offset + entry.second.localIndex;
        if (fullIndex == assetIndex) {
            strncpy_s(outCoin, outSize, entry.first.c_str(), _TRUNCATE);
            return true;
        }
    }

    return false;
}

bool getPerpDexInfo(const char* coin, PerpDexInfo& outInfo) {
    if (!coin || !*coin) return false;

    auto it = s_perpDexMap.find(std::string(coin));
    if (it != s_perpDexMap.end()) {
        outInfo = it->second;
        return true;
    }

    return false;
}

int getPerpDexOffset(const char* perpDex) {
    if (!perpDex) return 0;

    auto it = s_perpDexOffsets.find(std::string(perpDex));
    if (it != s_perpDexOffsets.end()) {
        return it->second;
    }

    return 0;
}

// =============================================================================
// WEBSOCKET INTEGRATION
// =============================================================================

void populateWsIndexMappings() {
    if (!g_wsManager) return;

    auto* mgr = reinterpret_cast<hl::ws::WebSocketManager*>(g_wsManager);
    mgr->clearIndexMappings();

    int mapped = 0;
    for (int i = 0; i < g_assets.count; ++i) {
        const AssetInfo* asset = g_assets.getByIndex(i);
        if (!asset) continue;

        if (asset->isSpot) {
            // Spot assets use 10000+spotIndex and spotCoin for WS/API lookups
            mgr->setIndexMapping(asset->index, std::string(asset->spotCoin));
        } else if (asset->isPerpDex) {
            // PerpDex assets use offset + localIndex (e.g., 110000 + 0 = 110000)
            int fullIndex = asset->perpDexOffset + asset->localIndex;
            mgr->setIndexMapping(fullIndex, std::string(asset->coin));
        } else {
            // Main perps use sequential index (0=BTC, 1=ETH, etc.)
            mgr->setIndexMapping(asset->index, std::string(asset->coin));
        }
        ++mapped;
    }

    if (g_config.diagLevel >= 2) {
        char msg[256];
        sprintf_s(msg, "Populated %d WS index mappings", mapped);
        logMsg(2, "populateWsIndexMappings", msg);

        // Log BTC/ETH indices for verification
        int btcIdx = -1, ethIdx = -1;
        for (int i = 0; i < g_assets.count; ++i) {
            const AssetInfo* asset = g_assets.getByIndex(i);
            if (asset) {
                if (strcmp(asset->coin, "BTC") == 0) btcIdx = asset->index;
                if (strcmp(asset->coin, "ETH") == 0) ethIdx = asset->index;
            }
        }
        sprintf_s(msg, "BTC at index %d, ETH at index %d", btcIdx, ethIdx);
        logMsg(2, "populateWsIndexMappings", msg);
    }
}

// =============================================================================
// STATS & DIAGNOSTICS
// =============================================================================

int getMainAssetCount() {
    return s_mainAssetCount;
}

int getPerpDexAssetCount() {
    return g_assets.count - s_mainAssetCount;
}

int getTotalAssetCount() {
    return g_assets.count;
}

bool isMetaLoaded() {
    return s_metaLoaded;
}

} // namespace meta
} // namespace hl
