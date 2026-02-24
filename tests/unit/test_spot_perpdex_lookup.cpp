//=============================================================================
// test_spot_perpdex_lookup.cpp - Spot & PerpDex asset registry lookup tests
//=============================================================================
// PREVENTS: OPM-138 — Spot/PerpDex assets not found in metadata
//
// Validates that the AssetRegistry lookup chain used by BrokerAsset (lines
// 42-44 of hl_broker_market.cpp) correctly finds spot and perpDex assets
// using all three name formats: display name, normalized coin, API format.
//
// BrokerAsset lookup sequence:
//   1. getAsset(symbol)          — display name (e.g., "PURR/USDC")
//   2. getAsset(coin)            — normalized coin (e.g., "TSLA")
//   3. getAsset(coinForApi)      — API format (e.g., "xyz:TSLA")
//=============================================================================

#include "../test_framework.h"
#include "../../src/foundation/hl_globals.h"
#include <cstring>
#include <cmath>

using namespace hl::test;

//=============================================================================
// HELPER: Register test assets in the global registry
//=============================================================================

static void setupRegistry() {
    hl::g_assets.init();

    // Main perp: BTC-USDC
    {
        hl::AssetInfo info;
        strncpy_s(info.name, "BTC-USDC", _TRUNCATE);
        strncpy_s(info.coin, "BTC", _TRUNCATE);
        strncpy_s(info.collateral, "USDC", _TRUNCATE);
        info.index = 0;
        info.szDecimals = 4;
        info.pxDecimals = 2;
        info.maxLeverage = 50;
        info.isPerpDex = false;
        info.isSpot = false;
        hl::g_assets.add(info);
    }

    // Spot asset: PURR/USDC (canonical)
    {
        hl::AssetInfo info;
        strncpy_s(info.name, "PURR/USDC", _TRUNCATE);
        strncpy_s(info.coin, "PURR/USDC", _TRUNCATE);
        strncpy_s(info.collateral, "USDC", _TRUNCATE);
        info.index = 10004;  // 10000 + spotIndex(4)
        info.szDecimals = 0;
        info.pxDecimals = 8;
        info.maxLeverage = 1;
        info.isPerpDex = false;
        info.isSpot = true;
        strncpy_s(info.spotCoin, "PURR/USDC", _TRUNCATE);
        hl::g_assets.add(info);
    }

    // Spot asset: non-canonical (e.g., @107)
    {
        hl::AssetInfo info;
        strncpy_s(info.name, "JEFF/USDC", _TRUNCATE);
        strncpy_s(info.coin, "@107", _TRUNCATE);
        strncpy_s(info.collateral, "USDC", _TRUNCATE);
        info.index = 10107;
        info.szDecimals = 0;
        info.pxDecimals = 8;
        info.maxLeverage = 1;
        info.isPerpDex = false;
        info.isSpot = true;
        strncpy_s(info.spotCoin, "@107", _TRUNCATE);
        hl::g_assets.add(info);
    }

    // PerpDex asset: TSLA-USDC.xyz
    {
        hl::AssetInfo info;
        strncpy_s(info.name, "TSLA-USDC.xyz", _TRUNCATE);
        strncpy_s(info.coin, "TSLA", _TRUNCATE);
        strncpy_s(info.collateral, "USDC", _TRUNCATE);
        info.index = 3;  // sequential in registry
        info.szDecimals = 2;
        info.pxDecimals = 4;
        info.maxLeverage = 20;
        info.isPerpDex = true;
        strncpy_s(info.perpDex, "xyz", _TRUNCATE);
        info.localIndex = 0;
        info.perpDexOffset = 110000;
        hl::g_assets.add(info);
    }

    // PerpDex asset: GOLD-USDC.xyz
    {
        hl::AssetInfo info;
        strncpy_s(info.name, "GOLD-USDC.xyz", _TRUNCATE);
        strncpy_s(info.coin, "GOLD", _TRUNCATE);
        strncpy_s(info.collateral, "USDC", _TRUNCATE);
        info.index = 4;
        info.szDecimals = 2;
        info.pxDecimals = 4;
        info.maxLeverage = 20;
        info.isPerpDex = true;
        strncpy_s(info.perpDex, "xyz", _TRUNCATE);
        info.localIndex = 1;
        info.perpDexOffset = 110000;
        hl::g_assets.add(info);
    }
}

static void teardownRegistry() {
    hl::g_assets.cleanup();
}

//=============================================================================
// TEST CASES: Spot asset lookup (OPM-138 regression)
//=============================================================================

void test_spot_found_by_display_name() {
    // BrokerAsset line 42: getAsset(symbol) where symbol = "PURR/USDC"
    int idx = hl::g_assets.findByName("PURR/USDC");
    ASSERT_GE(idx, 0);

    const hl::AssetInfo* asset = hl::g_assets.getByIndex(idx);
    ASSERT_NOT_NULL(asset);
    ASSERT_TRUE(asset->isSpot);
    ASSERT_STREQ(asset->name, "PURR/USDC");
    ASSERT_EQ(asset->index, 10004);
}

void test_spot_found_by_coin() {
    // BrokerAsset line 43: getAsset(coin) where coin = "PURR/USDC"
    int idx = hl::g_assets.findByCoin("PURR/USDC");
    ASSERT_GE(idx, 0);

    const hl::AssetInfo* asset = hl::g_assets.getByIndex(idx);
    ASSERT_NOT_NULL(asset);
    ASSERT_TRUE(asset->isSpot);
}

void test_spot_noncanonical_found_by_display_name() {
    // Non-canonical spot: name = "JEFF/USDC", coin = "@107"
    int idx = hl::g_assets.findByName("JEFF/USDC");
    ASSERT_GE(idx, 0);

    const hl::AssetInfo* asset = hl::g_assets.getByIndex(idx);
    ASSERT_NOT_NULL(asset);
    ASSERT_STREQ(asset->coin, "@107");
}

void test_spot_noncanonical_found_by_at_format() {
    // Non-canonical lookup by @-format coin
    int idx = hl::g_assets.findByCoin("@107");
    ASSERT_GE(idx, 0);

    const hl::AssetInfo* asset = hl::g_assets.getByIndex(idx);
    ASSERT_NOT_NULL(asset);
    ASSERT_STREQ(asset->name, "JEFF/USDC");
}

//=============================================================================
// TEST CASES: PerpDex asset lookup (OPM-138 regression)
//=============================================================================

void test_perpdex_found_by_display_name() {
    // BrokerAsset line 42: getAsset(symbol) where symbol = "TSLA-USDC.xyz"
    int idx = hl::g_assets.findByName("TSLA-USDC.xyz");
    ASSERT_GE(idx, 0);

    const hl::AssetInfo* asset = hl::g_assets.getByIndex(idx);
    ASSERT_NOT_NULL(asset);
    ASSERT_TRUE(asset->isPerpDex);
    ASSERT_STREQ(asset->perpDex, "xyz");
}

void test_perpdex_found_by_bare_coin() {
    // BrokerAsset line 43: getAsset(coin) where coin = "TSLA"
    int idx = hl::g_assets.findByCoin("TSLA");
    ASSERT_GE(idx, 0);

    const hl::AssetInfo* asset = hl::g_assets.getByIndex(idx);
    ASSERT_NOT_NULL(asset);
    ASSERT_TRUE(asset->isPerpDex);
    ASSERT_STREQ(asset->name, "TSLA-USDC.xyz");
}

void test_perpdex_gold_found_by_display_name() {
    int idx = hl::g_assets.findByName("GOLD-USDC.xyz");
    ASSERT_GE(idx, 0);

    const hl::AssetInfo* asset = hl::g_assets.getByIndex(idx);
    ASSERT_NOT_NULL(asset);
    ASSERT_TRUE(asset->isPerpDex);
}

//=============================================================================
// TEST CASES: Main perp still works
//=============================================================================

void test_perp_found_by_display_name() {
    int idx = hl::g_assets.findByName("BTC-USDC");
    ASSERT_GE(idx, 0);

    const hl::AssetInfo* asset = hl::g_assets.getByIndex(idx);
    ASSERT_NOT_NULL(asset);
    ASSERT_FALSE(asset->isPerpDex);
    ASSERT_FALSE(asset->isSpot);
}

void test_perp_found_by_coin() {
    int idx = hl::g_assets.findByCoin("BTC");
    ASSERT_GE(idx, 0);
}

//=============================================================================
// TEST CASES: Negative cases
//=============================================================================

void test_nonexistent_asset_returns_negative() {
    ASSERT_EQ(hl::g_assets.findByName("DOESNOTEXIST"), -1);
    ASSERT_EQ(hl::g_assets.findByCoin("DOESNOTEXIST"), -1);
}

void test_api_format_not_in_registry() {
    // "xyz:TSLA" is the API format, NOT stored as coin or name
    // Registry stores coin="TSLA", name="TSLA-USDC.xyz"
    ASSERT_EQ(hl::g_assets.findByName("xyz:TSLA"), -1);
    ASSERT_EQ(hl::g_assets.findByCoin("xyz:TSLA"), -1);
}

//=============================================================================
// TEST CASES: Case-insensitive lookup
//=============================================================================

void test_lookup_case_insensitive() {
    // findByName and findByCoin use _stricmp
    ASSERT_GE(hl::g_assets.findByName("purr/usdc"), 0);
    ASSERT_GE(hl::g_assets.findByName("tsla-usdc.xyz"), 0);
    ASSERT_GE(hl::g_assets.findByCoin("btc"), 0);
}

//=============================================================================
// MAIN
//=============================================================================

int main() {
    printf("\n");
    printf("=================================================\n");
    printf("  Spot & PerpDex Asset Lookup Tests [OPM-138]\n");
    printf("  Validates: BrokerAsset lookup chain for all\n");
    printf("             three asset types (perp/spot/perpDex)\n");
    printf("=================================================\n\n");

    setupRegistry();

    // Spot lookups
    RUN_TEST(spot_found_by_display_name);
    RUN_TEST(spot_found_by_coin);
    RUN_TEST(spot_noncanonical_found_by_display_name);
    RUN_TEST(spot_noncanonical_found_by_at_format);

    // PerpDex lookups
    RUN_TEST(perpdex_found_by_display_name);
    RUN_TEST(perpdex_found_by_bare_coin);
    RUN_TEST(perpdex_gold_found_by_display_name);

    // Main perp (regression: shouldn't break)
    RUN_TEST(perp_found_by_display_name);
    RUN_TEST(perp_found_by_coin);

    // Negative cases
    RUN_TEST(nonexistent_asset_returns_negative);
    RUN_TEST(api_format_not_in_registry);

    // Case-insensitive
    RUN_TEST(lookup_case_insensitive);

    teardownRegistry();

    return printTestSummary();
}
