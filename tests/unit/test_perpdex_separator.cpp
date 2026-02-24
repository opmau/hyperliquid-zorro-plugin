//=============================================================================
// test_perpdex_separator.cpp - Regression test for OPM-132
//=============================================================================
// Verifies the asset naming scheme across all three Hyperliquid asset types:
//   Perps:   BTC-USDC    (coin + collateral)
//   PerpDex: GOLD-USDC.xyz  (coin + collateral + venue suffix)
//   Spot:    BTC/USDC    (base/quote pair)
//
// The naming matches the Hyperliquid UI and avoids Zorro's broker:symbol
// colon conflict.
//=============================================================================

#include "../test_framework.h"
#include "../../src/foundation/hl_utils.h"
#include <cstring>
#include <string>

using namespace hl::utils;
using namespace hl::test;

//=============================================================================
// FOUNDATION LAYER: parsePerpDex — new dot-suffix format
//=============================================================================

TEST_CASE(parsePerpDex_dot_suffix) {
    char perpDex[32], coin[64];

    // PerpDex with dot suffix: GOLD-USDC.xyz → perpDex="xyz", coin="GOLD-USDC"
    bool hasPerpDex = parsePerpDex("GOLD-USDC.xyz", perpDex, sizeof(perpDex), coin, sizeof(coin));
    ASSERT_TRUE(hasPerpDex);
    ASSERT_STREQ(perpDex, "xyz");
    ASSERT_STREQ(coin, "GOLD-USDC");
}

TEST_CASE(parsePerpDex_dot_suffix_last_dot) {
    char perpDex[32], coin[64];

    // Edge case: coin name with dots (hypothetical) — split at LAST dot
    bool hasPerpDex = parsePerpDex("A.B-USDC.xyz", perpDex, sizeof(perpDex), coin, sizeof(coin));
    ASSERT_TRUE(hasPerpDex);
    ASSERT_STREQ(perpDex, "xyz");
    ASSERT_STREQ(coin, "A.B-USDC");
}

TEST_CASE(parsePerpDex_no_dot_no_perpDex) {
    char perpDex[32], coin[64];

    // Main perp: BTC-USDC → no perpDex, coin="BTC-USDC"
    bool hasPerpDex = parsePerpDex("BTC-USDC", perpDex, sizeof(perpDex), coin, sizeof(coin));
    ASSERT_FALSE(hasPerpDex);
    ASSERT_STREQ(perpDex, "");
    ASSERT_STREQ(coin, "BTC-USDC");
}

TEST_CASE(parsePerpDex_legacy_bare_coin) {
    char perpDex[32], coin[64];

    // Legacy bare coin: BTC → no perpDex, coin="BTC"
    bool hasPerpDex = parsePerpDex("BTC", perpDex, sizeof(perpDex), coin, sizeof(coin));
    ASSERT_FALSE(hasPerpDex);
    ASSERT_STREQ(perpDex, "");
    ASSERT_STREQ(coin, "BTC");
}

TEST_CASE(parsePerpDex_spot_preserved) {
    char perpDex[32], coin[64];

    // Spot: BTC/USDC → no perpDex, coin="BTC/USDC"
    bool hasPerpDex = parsePerpDex("BTC/USDC", perpDex, sizeof(perpDex), coin, sizeof(coin));
    ASSERT_FALSE(hasPerpDex);
    ASSERT_STREQ(coin, "BTC/USDC");
}

//=============================================================================
// FOUNDATION LAYER: buildCoinName — new format
//=============================================================================

TEST_CASE(buildCoinName_perpdex_with_collateral) {
    // PerpDex: (perpDex="xyz", coin="GOLD", collateral="USDC") → "GOLD-USDC.xyz"
    std::string name = buildCoinName("xyz", "GOLD", "USDC");
    ASSERT_STREQ(name.c_str(), "GOLD-USDC.xyz");
}

TEST_CASE(buildCoinName_perp_with_collateral) {
    // Main perp: (perpDex="", coin="BTC", collateral="USDC") → "BTC-USDC"
    std::string name = buildCoinName("", "BTC", "USDC");
    ASSERT_STREQ(name.c_str(), "BTC-USDC");
}

TEST_CASE(buildCoinName_no_collateral_legacy) {
    // Legacy: (perpDex="", coin="BTC", collateral="") → "BTC"
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

//=============================================================================
// ROUND-TRIP: build → parse → normalizeCoin → API coin
//=============================================================================

TEST_CASE(roundtrip_perpdex) {
    // Build display name
    std::string display = buildCoinName("xyz", "TSLA", "USDC");
    ASSERT_STREQ(display.c_str(), "TSLA-USDC.xyz");

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
    ASSERT_STREQ(hynaBtc.c_str(), "BTC-USDE.hyna");

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

    ASSERT_STREQ(g1.c_str(), "GOLD-USDC.xyz");
    ASSERT_STREQ(g2.c_str(), "GOLD-USDT0.cash");
    ASSERT_STREQ(g3.c_str(), "GOLD-USDH.flx");
    ASSERT_STREQ(g4.c_str(), "GOLD-USDH.km");

    // All unique
    ASSERT_STRNE(g1.c_str(), g2.c_str());
    ASSERT_STRNE(g3.c_str(), g4.c_str());
}

//=============================================================================
// MAIN
//=============================================================================
int main() {
    printf("=== OPM-132: Asset Naming Tests ===\n\n");

    RUN_TEST(parsePerpDex_dot_suffix);
    RUN_TEST(parsePerpDex_dot_suffix_last_dot);
    RUN_TEST(parsePerpDex_no_dot_no_perpDex);
    RUN_TEST(parsePerpDex_legacy_bare_coin);
    RUN_TEST(parsePerpDex_spot_preserved);
    RUN_TEST(buildCoinName_perpdex_with_collateral);
    RUN_TEST(buildCoinName_perp_with_collateral);
    RUN_TEST(buildCoinName_no_collateral_legacy);
    RUN_TEST(buildCoinName_no_colon_in_output);
    RUN_TEST(roundtrip_perpdex);
    RUN_TEST(roundtrip_perp);
    RUN_TEST(hyna_btc_vs_main_btc);
    RUN_TEST(four_golds);

    return printTestSummary();
}
