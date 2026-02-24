//=============================================================================
// hl_meta.h - Asset metadata caching from Hyperliquid API
//=============================================================================
// Part of Hyperliquid Plugin for Zorro
//
// LAYER: Services
// DEPENDENCIES: hl_types.h, hl_globals.h, hl_http.h
// THREAD SAFETY: All public functions are thread-safe (use g_assets mutex)
//
// This module handles:
// - Fetching asset metadata from /info?type=meta endpoint
// - Fetching perpDex (equity perps) metadata
// - Asset lookup by coin name with perpDex prefix support
// - Syncing asset indices with WebSocket manager
//=============================================================================

#pragma once

#include "../foundation/hl_types.h"
#include <string>
#include <map>

namespace hl {
namespace meta {

// =============================================================================
// PERPDEX TYPES
// =============================================================================

/// Information about an asset on a perpDex (equity perps platform)
/// PerpDex assets use different index offsets (110000, 120000, etc.)
struct PerpDexInfo {
    std::string perpDex;    // e.g., "xyz", "flx"
    int localIndex = 0;     // Index within this perpDex universe (0, 1, 2...)
    int offset = 0;         // Offset for this perpDex (e.g., 110000)

    /// Calculate the full asset index for API calls
    int fullIndex() const { return offset + localIndex; }
};

// =============================================================================
// META FETCHING
// =============================================================================

/// Fetch main (crypto perps) metadata from API and populate asset registry
/// @return Number of assets cached, or 0 on failure
///
/// This fetches {"type":"meta"} and parses the "universe" array to extract:
/// - Asset names (BTC, ETH, etc.)
/// - Size decimals (szDecimals)
/// - Max leverage (maxLeverage)
/// - Calculated price decimals (6 - szDecimals for perps)
int fetchMeta();

/// Clear all cached metadata (both main and perpDex)
/// Resets g_assets and internal perpDex maps
void clearMeta();

/// Initialize perpDex offset map by fetching perpDex list
/// @return Number of perpDexes found
///
/// Fetches {"type":"perpDexs"} and calculates offsets:
/// - First perpDex: 110000
/// - Second perpDex: 120000 (etc.)
int fetchPerpDexList();

/// Fetch metadata for a specific perpDex and append to asset registry
/// @param perpDex PerpDex name (e.g., "xyz")
/// @return Number of assets added, or 0 on failure
int fetchPerpDexMeta(const char* perpDex);

/// Fetch all perpDex metadata (calls fetchPerpDexMeta for each known perpDex)
/// @return Total number of perpDex assets cached
int fetchAllPerpDexMeta();

/// Fetch spot token/pair metadata from API and append to asset registry
/// @return Number of spot pairs cached, or 0 on failure
///
/// Fetches {"type":"spotMeta"}, parses tokens[] and universe[] arrays.
/// Spot assets use: index=10000+spotIndex, pxDecimals=8-szDecimals, maxLeverage=1
int fetchSpotMeta();

// =============================================================================
// ASSET LOOKUP
// =============================================================================

/// Find asset index by coin name (case-insensitive)
/// Supports both main perps ("BTC") and perpDex assets ("xyz:XYZ100" or just "XYZ100")
/// @param coin Coin name to look up
/// @return Asset index, or -1 if not found
int findAssetIndex(const char* coin);

/// Convert perpDex position index (e.g., "@110001") to friendly coin name (e.g., "XYZ100")
/// @param positionCoin Position coin from API (e.g., "@110001")
/// @param outCoin Output buffer for friendly name
/// @param outSize Size of output buffer
/// @return true if conversion succeeded
bool convertPerpDexIndexToCoin(const char* positionCoin, char* outCoin, size_t outSize);

/// Get perpDex info for a coin (if it's a perpDex asset)
/// @param coin Coin name (without perpDex prefix, e.g., "XYZ100")
/// @param outInfo Output for perpDex info
/// @return true if coin is a perpDex asset
bool getPerpDexInfo(const char* coin, PerpDexInfo& outInfo);

/// Get the perpDex offset for a perpDex name
/// @param perpDex PerpDex name (e.g., "xyz")
/// @return Offset (e.g., 110000), or 0 if not found or default perpDex
int getPerpDexOffset(const char* perpDex);

// =============================================================================
// WEBSOCKET INTEGRATION
// =============================================================================

/// Populate WebSocket manager's index-to-coin mapping from cached meta
/// Must be called after fetchMeta() and fetchAllPerpDexMeta() to enable allMids parsing
void populateWsIndexMappings();

// =============================================================================
// STATS & DIAGNOSTICS
// =============================================================================

/// Get count of cached assets (main perps only)
int getMainAssetCount();

/// Get count of cached perpDex assets
int getPerpDexAssetCount();

/// Get count of cached spot assets
int getSpotAssetCount();

/// Get total cached assets (main + perpDex + spot)
int getTotalAssetCount();

/// Check if meta has been fetched
bool isMetaLoaded();

} // namespace meta
} // namespace hl
