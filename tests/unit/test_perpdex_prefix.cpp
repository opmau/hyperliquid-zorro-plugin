//=============================================================================
// test_perpdex_prefix.cpp - Regression test for OPM-140
//=============================================================================
// PREVENTS BUG: Hyperliquid perpDex API returns coin names with a venue prefix
// (e.g., "xyz:TSLA") but fetchPerpDexMeta did NOT strip this prefix.
//
// This caused:
//   - Stored asset name = "xyz:TSLA-USDC.xyz" instead of "TSLA-USDC.xyz"
//   - Stored asset coin = "xyz:TSLA" instead of "TSLA"
//   - BrokerAsset price query couldn't find asset → fell to defaults
//   - Wrong PIP=1.0, LotAmount=0.0001, PIPCost=0.0001
//   - Correct PIPCost should be 0.000001 (constant for all perps)
//
// The fix: strip perpDex prefix from API coin names before storing.
//=============================================================================

#include "../test_framework.h"
#include "../../src/foundation/hl_utils.h"
#include <cstring>
#include <string>

using namespace hl::utils;
using namespace hl::test;

//=============================================================================
// TEST: stripApiCoinPrefix must strip perpDex prefix from API coin names
//=============================================================================

TEST_CASE(strip_prefix_xyz_tsla) {
    // API returns "xyz:TSLA" — must strip to "TSLA"
    char out[64];
    stripApiCoinPrefix("xyz:TSLA", out, sizeof(out));
    ASSERT_STREQ(out, "TSLA");
}

TEST_CASE(strip_prefix_xyz_gold) {
    char out[64];
    stripApiCoinPrefix("xyz:GOLD", out, sizeof(out));
    ASSERT_STREQ(out, "GOLD");
}

TEST_CASE(strip_prefix_hyna_btc) {
    char out[64];
    stripApiCoinPrefix("hyna:BTC", out, sizeof(out));
    ASSERT_STREQ(out, "BTC");
}

TEST_CASE(strip_prefix_no_prefix) {
    // Regular perp coin names have no prefix: "BTC" stays "BTC"
    char out[64];
    stripApiCoinPrefix("BTC", out, sizeof(out));
    ASSERT_STREQ(out, "BTC");
}

TEST_CASE(strip_prefix_xyz100) {
    // First xyz asset: "xyz:XYZ100" → "XYZ100"
    char out[64];
    stripApiCoinPrefix("xyz:XYZ100", out, sizeof(out));
    ASSERT_STREQ(out, "XYZ100");
}

//=============================================================================
// TEST: After stripping, buildCoinName produces correct display names
//=============================================================================

TEST_CASE(stripped_coin_builds_correct_name) {
    // Simulate fetchPerpDexMeta flow:
    // 1. API returns "xyz:TSLA"
    // 2. Strip prefix → "TSLA"
    // 3. buildCoinName("xyz", "TSLA", "USDC") → "TSLA-USDC.xyz"
    char bareCoin[64];
    stripApiCoinPrefix("xyz:TSLA", bareCoin, sizeof(bareCoin));

    std::string display = buildCoinName("xyz", bareCoin, "USDC");
    ASSERT_STREQ(display.c_str(), "TSLA-USDC.xyz");

    // CRITICAL: no colon in display name (breaks Zorro broker:symbol parsing)
    ASSERT_MSG(display.find(':') == std::string::npos,
               "display name must NOT contain colon");
}

TEST_CASE(unstripped_coin_produces_colon_in_name) {
    // This is the BUG scenario: using API name directly in buildCoinName
    std::string buggy = buildCoinName("xyz", "xyz:TSLA", "USDC");

    // This WOULD contain a colon — which is the bug we're preventing
    // After fix, fetchPerpDexMeta strips the prefix so this never happens
    ASSERT_MSG(buggy.find(':') != std::string::npos,
               "unstripped name should contain colon (proves bug exists)");
}

//=============================================================================
// MAIN
//=============================================================================
int main() {
    printf("=== OPM-140: PerpDex API Prefix Stripping Tests ===\n\n");

    RUN_TEST(strip_prefix_xyz_tsla);
    RUN_TEST(strip_prefix_xyz_gold);
    RUN_TEST(strip_prefix_hyna_btc);
    RUN_TEST(strip_prefix_no_prefix);
    RUN_TEST(strip_prefix_xyz100);
    RUN_TEST(stripped_coin_builds_correct_name);
    RUN_TEST(unstripped_coin_produces_colon_in_name);

    return printTestSummary();
}
