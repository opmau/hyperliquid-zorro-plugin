//=============================================================================
// test_perpdex_separator.cpp - Regression test for OPM-132 / OPM-169
//=============================================================================
// Verifies the asset naming scheme across all three Hyperliquid asset types:
//   Perps:   BTC-USDC       (coin + collateral)
//   PerpDex: GOLD-USDC_xyz  (coin + collateral + underscore + venue suffix)
//   Spot:    BTC/USDC       (base/quote pair)
//
// OPM-169: Changed separator from dot to underscore so Zorro history file
// lookup works (Zorro strips dots from filenames).
//=============================================================================

#include "../test_framework.h"
#include "../../src/foundation/hl_utils.h"
#include <cstring>
#include <string>

using namespace hl::utils;
using namespace hl::test;

//=============================================================================
// FOUNDATION LAYER: parsePerpDex — underscore-suffix format [OPM-169]
//=============================================================================

TEST_CASE(parsePerpDex_underscore_suffix) {
    char perpDex[32], coin[64];

    // PerpDex with underscore suffix: GOLD-USDC_xyz -> perpDex="xyz", coin="GOLD-USDC"
    bool hasPerpDex = parsePerpDex("GOLD-USDC_xyz", perpDex, sizeof(perpDex), coin, sizeof(coin));
    ASSERT_TRUE(hasPerpDex);
    ASSERT_STREQ(perpDex, "xyz");
    ASSERT_STREQ(coin, "GOLD-USDC");
}

TEST_CASE(parsePerpDex_underscore_suffix_last_underscore) {
    char perpDex[32], coin[64];

    // Edge case: name with multiple underscores (hypothetical) — split at LAST underscore
    bool hasPerpDex = parsePerpDex("A_B-USDC_xyz", perpDex, sizeof(perpDex), coin, sizeof(coin));
    ASSERT_TRUE(hasPerpDex);
    ASSERT_STREQ(perpDex, "xyz");
    ASSERT_STREQ(coin, "A_B-USDC");
}

TEST_CASE(parsePerpDex_no_underscore_no_perpDex) {
    char perpDex[32], coin[64];

    // Main perp: BTC-USDC -> no perpDex, coin="BTC-USDC"
    bool hasPerpDex = parsePerpDex("BTC-USDC", perpDex, sizeof(perpDex), coin, sizeof(coin));
    ASSERT_FALSE(hasPerpDex);
    ASSERT_STREQ(perpDex, "");
    ASSERT_STREQ(coin, "BTC-USDC");
}

TEST_CASE(parsePerpDex_legacy_bare_coin) {
    char perpDex[32], coin[64];

    // Legacy bare coin: BTC -> no perpDex, coin="BTC"
    bool hasPerpDex = parsePerpDex("BTC", perpDex, sizeof(perpDex), coin, sizeof(coin));
    ASSERT_FALSE(hasPerpDex);
    ASSERT_STREQ(perpDex, "");
    ASSERT_STREQ(coin, "BTC");
}

TEST_CASE(parsePerpDex_spot_preserved) {
    char perpDex[32], coin[64];

    // Spot: BTC/USDC -> no perpDex, coin="BTC/USDC"
    bool hasPerpDex = parsePerpDex("BTC/USDC", perpDex, sizeof(perpDex), coin, sizeof(coin));
    ASSERT_FALSE(hasPerpDex);
    ASSERT_STREQ(coin, "BTC/USDC");
}

//=============================================================================
// FOUNDATION LAYER: buildCoinName — underscore format [OPM-169]
//=============================================================================

TEST_CASE(buildCoinName_perpdex_with_collateral) {
    // PerpDex: (perpDex="xyz", coin="GOLD", collateral="USDC") -> "GOLD-USDC_xyz"
    std::string name = buildCoinName("xyz", "GOLD", "USDC");
    ASSERT_STREQ(name.c_str(), "GOLD-USDC_xyz");
}

TEST_CASE(buildCoinName_perp_with_collateral) {
    // Main perp: (perpDex="", coin="BTC", collateral="USDC") -> "BTC-USDC"
    std::string name = buildCoinName("", "BTC", "USDC");
    ASSERT_STREQ(name.c_str(), "BTC-USDC");
}

TEST_CASE(buildCoinName_no_collateral_legacy) {
    // Legacy: (perpDex="", coin="BTC", collateral="") -> "BTC"
    std::string name = buildCoinName("", "BTC", "");
    ASSERT_STREQ(name.c_str(), "BTC");
}

TEST_CASE(buildCoinName_no_colon_in_output) {
    // CRITICAL: display names must NEVER contain colon (breaks Zorro broker:symbol)
    std::string name1 = buildCoinName("xyz", "GOLD", "USDC");
    ASSERT_MSG(name1.find(':') == std::string::npos,
               "perpDex display name must not contain colon");

    std::string name2 = buildCoinName("", "BTC", "USDC");
    ASSERT_MSG(name2.find(':') == std::string::npos,
               "perp display name must not contain colon");
}

TEST_CASE(buildCoinName_no_dot_in_output) {
    // OPM-169: display names must NOT contain dots (Zorro strips them from filenames)
    std::string name1 = buildCoinName("xyz", "GOLD", "USDC");
    ASSERT_MSG(name1.find('.') == std::string::npos,
               "perpDex display name must not contain dot [OPM-169]");
}

//=============================================================================
// ROUND-TRIP: build -> parse -> normalizeCoin -> API coin
//=============================================================================

TEST_CASE(roundtrip_perpdex) {
    // Build display name
    std::string display = buildCoinName("xyz", "TSLA", "USDC");
    ASSERT_STREQ(display.c_str(), "TSLA-USDC_xyz");

    // Parse it back
    char perpDex[32], coin[64];
    bool hasPerpDex = parsePerpDex(display.c_str(), perpDex, sizeof(perpDex), coin, sizeof(coin));
    ASSERT_TRUE(hasPerpDex);
    ASSERT_STREQ(perpDex, "xyz");
    ASSERT_STREQ(coin, "TSLA-USDC");

    // normalizeCoin strips the collateral suffix
    char apiCoin[32];
    normalizeCoin(coin, apiCoin, sizeof(apiCoin));
    ASSERT_STREQ(apiCoin, "TSLA");
}

TEST_CASE(roundtrip_perp) {
    // Build display name
    std::string display = buildCoinName("", "BTC", "USDC");
    ASSERT_STREQ(display.c_str(), "BTC-USDC");

    // Parse it
    char perpDex[32], coin[64];
    bool hasPerpDex = parsePerpDex(display.c_str(), perpDex, sizeof(perpDex), coin, sizeof(coin));
    ASSERT_FALSE(hasPerpDex);
    ASSERT_STREQ(coin, "BTC-USDC");

    // normalizeCoin strips the collateral suffix
    char apiCoin[32];
    normalizeCoin(coin, apiCoin, sizeof(apiCoin));
    ASSERT_STREQ(apiCoin, "BTC");
}

//=============================================================================
// HYNA OVERLAP: hyna:BTC vs main BTC — different display names
//=============================================================================

TEST_CASE(hyna_btc_vs_main_btc) {
    std::string mainPerp = buildCoinName("", "BTC", "USDC");
    std::string hynaBtc = buildCoinName("hyna", "BTC", "USDE");

    ASSERT_STREQ(mainPerp.c_str(), "BTC-USDC");
    ASSERT_STREQ(hynaBtc.c_str(), "BTC-USDE_hyna");

    // They must be different
    ASSERT_STRNE(mainPerp.c_str(), hynaBtc.c_str());
}

//=============================================================================
// FOUR GOLD INSTRUMENTS (matching HL UI screenshot)
//=============================================================================

TEST_CASE(four_golds) {
    std::string g1 = buildCoinName("xyz", "GOLD", "USDC");
    std::string g2 = buildCoinName("cash", "GOLD", "USDT0");
    std::string g3 = buildCoinName("flx", "GOLD", "USDH");
    std::string g4 = buildCoinName("km", "GOLD", "USDH");

    ASSERT_STREQ(g1.c_str(), "GOLD-USDC_xyz");
    ASSERT_STREQ(g2.c_str(), "GOLD-USDT0_cash");
    ASSERT_STREQ(g3.c_str(), "GOLD-USDH_flx");
    ASSERT_STREQ(g4.c_str(), "GOLD-USDH_km");

    // All unique
    ASSERT_STRNE(g1.c_str(), g2.c_str());
    ASSERT_STRNE(g3.c_str(), g4.c_str());
}

//=============================================================================
// MAIN
//=============================================================================
int main() {
    printf("=== OPM-132/OPM-169: Asset Naming Tests ===\n\n");

    RUN_TEST(parsePerpDex_underscore_suffix);
    RUN_TEST(parsePerpDex_underscore_suffix_last_underscore);
    RUN_TEST(parsePerpDex_no_underscore_no_perpDex);
    RUN_TEST(parsePerpDex_legacy_bare_coin);
    RUN_TEST(parsePerpDex_spot_preserved);
    RUN_TEST(buildCoinName_perpdex_with_collateral);
    RUN_TEST(buildCoinName_perp_with_collateral);
    RUN_TEST(buildCoinName_no_collateral_legacy);
    RUN_TEST(buildCoinName_no_colon_in_output);
    RUN_TEST(buildCoinName_no_dot_in_output);
    RUN_TEST(roundtrip_perpdex);
    RUN_TEST(roundtrip_perp);
    RUN_TEST(hyna_btc_vs_main_btc);
    RUN_TEST(four_golds);

    return printTestSummary();
}
