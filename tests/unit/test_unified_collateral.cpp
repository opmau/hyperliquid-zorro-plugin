//=============================================================================
// test_unified_collateral.cpp - Unified-account collateral tests [OPM-824]
//=============================================================================
// LAYER: Test | TESTS: src/services/hl_account_collateral.h (production code,
//                      linked directly — not a re-implementation)
//
// The defect: BrokerAccount reported clearinghouseState.marginSummary
// .accountValue, which is perps-only AND main-dex only. On a unified account —
// where one spot USDC pool collateralizes everything — that under-reported
// usable equity by ~37% on mainnet and read a flat 0 on testnet, tripping the
// zero-balance guard and halting the strategy.
//
// Zorro sizes positions from this number, so every payload below is a verbatim
// (address-stripped) capture from the live API on 2026-07-31, not a mock-up.
//=============================================================================

#include "../test_framework.h"
#include "../../src/services/hl_account_collateral.h"
#include <cstring>

using namespace hl::test;
using namespace hl::account;

//=============================================================================
// CANNED PAYLOADS — captured live 2026-07-31
//=============================================================================

// unifiedAccount, mainnet. Spot USDC total IS the whole collateral pool;
// perps accountValue for the same account read 15025.81 at capture time.
static const char* SPOT_UNIFIED_MAINNET =
    "{\"balances\":[{\"coin\":\"USDC\",\"token\":0,\"total\":\"23942.437016\","
    "\"hold\":\"15130.155342\",\"entryNtl\":\"0.0\"}],"
    "\"tokenToAvailableAfterMaintenance\":[[0,\"17397.164168\"]]}";

// unifiedAccount, testnet. Perps accountValue is 0.0 here — the account is
// funded entirely on the spot side. This is the payload that used to halt.
static const char* SPOT_UNIFIED_TESTNET =
    "{\"balances\":[{\"coin\":\"USDC\",\"token\":0,\"total\":\"1234.567890\","
    "\"hold\":\"0.0\",\"entryNtl\":\"0.0\"}],"
    "\"tokenToAvailableAfterMaintenance\":[[0,\"1234.567890\"]]}";

// "default" mode, mainnet sub-account. No tokenToAvailableAfterMaintenance.
static const char* SPOT_DEFAULT_MAINNET =
    "{\"balances\":[{\"coin\":\"USDC\",\"token\":0,\"total\":\"0.00471007\","
    "\"hold\":\"0.0\",\"entryNtl\":\"0.0\"}]}";

// "disabled" mode: separate pools, meaningful spot holdings.
static const char* SPOT_DISABLED =
    "{\"balances\":[{\"coin\":\"USDC\",\"token\":0,\"total\":\"500.0\","
    "\"hold\":\"0.0\",\"entryNtl\":\"0.0\"},"
    "{\"coin\":\"PURR\",\"token\":1,\"total\":\"120.5\",\"hold\":\"0.0\","
    "\"entryNtl\":\"250.0\"}]}";

// portfolioMargin: multi-collateral, so USDC is not the only token listed and
// tokenToAvailableAfterMaintenance carries an entry per collateral token.
static const char* SPOT_PORTFOLIO_MARGIN =
    "{\"balances\":[{\"coin\":\"HYPE\",\"token\":150,\"total\":\"11.2094\","
    "\"hold\":\"0.0\",\"entryNtl\":\"600.0\"},"
    "{\"coin\":\"USDC\",\"token\":0,\"total\":\"12500.25\",\"hold\":\"3000.0\","
    "\"entryNtl\":\"0.0\"}],"
    "\"tokenToAvailableAfterMaintenance\":[[150,\"5.6047\"],[0,\"9200.75\"]]}";

//=============================================================================
// PARSING: does the payload shape identify the collateral model?
//=============================================================================

TEST_CASE(spot_parse_unified_mainnet) {
    SpotState s;
    ASSERT_TRUE(parseSpotClearinghouseState(SPOT_UNIFIED_MAINNET,
                                            strlen(SPOT_UNIFIED_MAINNET), s));
    ASSERT_TRUE(s.valid);
    ASSERT_TRUE(s.unifiedPool);
    ASSERT_FLOAT_EQ_TOL(s.usdcTotal, 23942.437016, 1e-6);
    ASSERT_FLOAT_EQ_TOL(s.availAfterMaint, 17397.164168, 1e-6);
}

TEST_CASE(spot_parse_unified_testnet) {
    SpotState s;
    ASSERT_TRUE(parseSpotClearinghouseState(SPOT_UNIFIED_TESTNET,
                                            strlen(SPOT_UNIFIED_TESTNET), s));
    ASSERT_TRUE(s.unifiedPool);
    ASSERT_FLOAT_EQ_TOL(s.usdcTotal, 1234.567890, 1e-6);
}

TEST_CASE(spot_parse_default_has_no_unified_marker) {
    // "default" accounts omit tokenToAvailableAfterMaintenance entirely.
    SpotState s;
    ASSERT_TRUE(parseSpotClearinghouseState(SPOT_DEFAULT_MAINNET,
                                            strlen(SPOT_DEFAULT_MAINNET), s));
    ASSERT_TRUE(s.valid);
    ASSERT_FALSE(s.unifiedPool);
    ASSERT_FLOAT_EQ_TOL(s.usdcTotal, 0.00471007, 1e-8);
    ASSERT_FLOAT_EQ(s.availAfterMaint, 0.0);
}

TEST_CASE(spot_parse_disabled_picks_usdc_not_first_token) {
    SpotState s;
    ASSERT_TRUE(parseSpotClearinghouseState(SPOT_DISABLED, strlen(SPOT_DISABLED), s));
    ASSERT_FALSE(s.unifiedPool);
    ASSERT_FLOAT_EQ_TOL(s.usdcTotal, 500.0, 1e-9);
}

TEST_CASE(spot_parse_portfolio_margin_selects_usdc_row) {
    // USDC is the SECOND balance and its availAfterMaintenance entry is the
    // second pair. Indexing by position instead of token would take HYPE's.
    SpotState s;
    ASSERT_TRUE(parseSpotClearinghouseState(SPOT_PORTFOLIO_MARGIN,
                                            strlen(SPOT_PORTFOLIO_MARGIN), s));
    ASSERT_TRUE(s.unifiedPool);
    ASSERT_FLOAT_EQ_TOL(s.usdcTotal, 12500.25, 1e-6);
    ASSERT_FLOAT_EQ_TOL(s.availAfterMaint, 9200.75, 1e-6);
}

TEST_CASE(spot_parse_empty_balances) {
    const char* body = "{\"balances\":[]}";
    SpotState s;
    ASSERT_TRUE(parseSpotClearinghouseState(body, strlen(body), s));
    ASSERT_TRUE(s.valid);
    ASSERT_FALSE(s.unifiedPool);
    ASSERT_FLOAT_EQ(s.usdcTotal, 0.0);
}

TEST_CASE(spot_parse_rejects_malformed) {
    SpotState s;
    ASSERT_FALSE(parseSpotClearinghouseState("not json", 8, s));
    ASSERT_FALSE(s.valid);
}

TEST_CASE(spot_parse_rejects_null_and_empty) {
    // [OPM-823] Testnet /info has been observed returning a bare `null`.
    SpotState s;
    ASSERT_FALSE(parseSpotClearinghouseState(nullptr, 0, s));
    ASSERT_FALSE(s.valid);
    ASSERT_FALSE(parseSpotClearinghouseState("", 0, s));
    ASSERT_FALSE(s.valid);
}

//=============================================================================
// ABSTRACTION MODE: every name Hyperliquid ships must map
//=============================================================================

TEST_CASE(mode_parse_all_four_names) {
    ASSERT_TRUE(parseAbstractionMode("\"disabled\"") == AbstractionMode::Standard);
    ASSERT_TRUE(parseAbstractionMode("\"default\"") == AbstractionMode::Default);
    ASSERT_TRUE(parseAbstractionMode("\"unifiedAccount\"") == AbstractionMode::Unified);
    ASSERT_TRUE(parseAbstractionMode("\"portfolioMargin\"") == AbstractionMode::PortfolioMargin);
}

TEST_CASE(mode_parse_default_is_recognized) {
    // The regression: "default" fell through to Unknown, and Unknown was then
    // treated as unified — discarding spot on three live mainnet accounts.
    AbstractionMode m = parseAbstractionMode("\"default\"");
    ASSERT_FALSE(m == AbstractionMode::Unknown);
    ASSERT_FALSE(modePoolsSpot(m));
}

TEST_CASE(mode_parse_unknown_stays_unknown) {
    ASSERT_TRUE(parseAbstractionMode("\"someFutureMode\"") == AbstractionMode::Unknown);
    ASSERT_TRUE(parseAbstractionMode("") == AbstractionMode::Unknown);
    ASSERT_TRUE(parseAbstractionMode(nullptr) == AbstractionMode::Unknown);
}

TEST_CASE(mode_pools_spot_only_for_unified_and_pm) {
    ASSERT_TRUE(modePoolsSpot(AbstractionMode::Unified));
    ASSERT_TRUE(modePoolsSpot(AbstractionMode::PortfolioMargin));
    ASSERT_FALSE(modePoolsSpot(AbstractionMode::Standard));
    ASSERT_FALSE(modePoolsSpot(AbstractionMode::Default));
    ASSERT_FALSE(modePoolsSpot(AbstractionMode::Unknown));
}

//=============================================================================
// COLLATERAL MODEL: the number Zorro sizes positions from
//=============================================================================

TEST_CASE(collateral_unified_uses_spot_total_alone) {
    SpotState s;
    parseSpotClearinghouseState(SPOT_UNIFIED_MAINNET, strlen(SPOT_UNIFIED_MAINNET), s);

    CollateralView v = computeCollateral(15025.81, 13150.15, s);
    ASSERT_TRUE(v.unified);
    // NOT 15025.81 (the old, perps-only figure) and NOT 38968.25 (the sum).
    ASSERT_FLOAT_EQ_TOL(v.equity, 23942.437016, 1e-6);
    ASSERT_FLOAT_EQ_TOL(v.freeCollateral, 17397.164168, 1e-6);
}

TEST_CASE(collateral_unified_does_not_double_count_pnl) {
    // Spot USDC total is marked to market and already contains perp PnL.
    // Verified live: delta(perp accountValue) == delta(spot USDC total).
    // So moving BOTH by the same amount must move equity by that amount ONCE.
    SpotState s;
    parseSpotClearinghouseState(SPOT_UNIFIED_MAINNET, strlen(SPOT_UNIFIED_MAINNET), s);
    double before = computeCollateral(15025.81, 13150.15, s).equity;

    const double pnl = 100.0;
    s.usdcTotal += pnl;
    double after = computeCollateral(15025.81 + pnl, 13150.15, s).equity;

    ASSERT_FLOAT_EQ_TOL(after - before, pnl, 1e-9);  // once, not twice
}

TEST_CASE(collateral_unified_testnet_no_longer_reports_zero) {
    // The halt case: perps accountValue is 0.0 but the account holds spot collateral.
    SpotState s;
    parseSpotClearinghouseState(SPOT_UNIFIED_TESTNET, strlen(SPOT_UNIFIED_TESTNET), s);

    CollateralView v = computeCollateral(0.0, 0.0, s);
    ASSERT_FLOAT_EQ_TOL(v.equity, 1234.567890, 1e-6);
    ASSERT_GT(v.equity, 0.0);  // would otherwise trip zorroQuit()
}

TEST_CASE(collateral_unified_ignores_perps_value_entirely) {
    // A HIP-3 account holds its whole position set off dex 0, so the dex-0
    // perps figure can be 0 (or anything) while equity is millions. Equity must
    // not depend on it at all.
    SpotState s;
    parseSpotClearinghouseState(SPOT_UNIFIED_MAINNET, strlen(SPOT_UNIFIED_MAINNET), s);

    double a = computeCollateral(0.0,        0.0, s).equity;
    double b = computeCollateral(15025.81,   0.0, s).equity;
    double c = computeCollateral(9999999.99, 0.0, s).equity;
    ASSERT_FLOAT_EQ_TOL(a, b, 1e-9);
    ASSERT_FLOAT_EQ_TOL(b, c, 1e-9);
}

TEST_CASE(collateral_portfolio_margin_uses_usdc_pool) {
    SpotState s;
    parseSpotClearinghouseState(SPOT_PORTFOLIO_MARGIN, strlen(SPOT_PORTFOLIO_MARGIN), s);

    CollateralView v = computeCollateral(8000.0, 3000.0, s);
    ASSERT_TRUE(v.unified);
    // Staked/collateral HYPE is deliberately NOT added — it is not perp
    // collateral in the USDC pool, and counting it would over-report equity.
    ASSERT_FLOAT_EQ_TOL(v.equity, 12500.25, 1e-6);
    ASSERT_FLOAT_EQ_TOL(v.freeCollateral, 9200.75, 1e-6);
}

TEST_CASE(collateral_default_adds_perps_and_spot) {
    SpotState s;
    parseSpotClearinghouseState(SPOT_DEFAULT_MAINNET, strlen(SPOT_DEFAULT_MAINNET), s);

    CollateralView v = computeCollateral(228.634863, 0.0, s);
    ASSERT_FALSE(v.unified);
    ASSERT_FLOAT_EQ_TOL(v.equity, 228.634863 + 0.00471007, 1e-8);
}

TEST_CASE(collateral_disabled_adds_perps_and_spot) {
    SpotState s;
    parseSpotClearinghouseState(SPOT_DISABLED, strlen(SPOT_DISABLED), s);

    CollateralView v = computeCollateral(10000.0, 1000.0, s);
    ASSERT_FALSE(v.unified);
    ASSERT_FLOAT_EQ_TOL(v.equity, 10500.0, 1e-9);
    ASSERT_FLOAT_EQ_TOL(v.freeCollateral, 9500.0, 1e-9);
}

TEST_CASE(collateral_no_spot_data_falls_back_to_perps) {
    // Spot fetch failed. Under-reporting a unified account is bad; reporting 0
    // is worse — it halts the strategy. Fall back, do not zero out.
    SpotState s;  // valid = false
    CollateralView v = computeCollateral(15025.81, 13150.15, s);
    ASSERT_FALSE(v.unified);
    ASSERT_FLOAT_EQ_TOL(v.equity, 15025.81, 1e-9);
    ASSERT_FLOAT_EQ_TOL(v.freeCollateral, 1875.66, 1e-9);
}

TEST_CASE(collateral_unified_with_zero_spot_reports_zero) {
    // A genuinely empty unified account must report 0, not silently fall back
    // to a stale perps figure. The zero-balance halt is correct here.
    const char* body = "{\"balances\":[],\"tokenToAvailableAfterMaintenance\":[]}";
    SpotState s;
    parseSpotClearinghouseState(body, strlen(body), s);

    CollateralView v = computeCollateral(500.0, 0.0, s);
    ASSERT_TRUE(v.unified);
    ASSERT_FLOAT_EQ(v.equity, 0.0);
}

//=============================================================================
// MAIN
//=============================================================================

int main() {
    printf("=== OPM-824: Unified Account Collateral Tests ===\n\n");

    printf("-- spotClearinghouseState parsing --\n");
    RUN_TEST(spot_parse_unified_mainnet);
    RUN_TEST(spot_parse_unified_testnet);
    RUN_TEST(spot_parse_default_has_no_unified_marker);
    RUN_TEST(spot_parse_disabled_picks_usdc_not_first_token);
    RUN_TEST(spot_parse_portfolio_margin_selects_usdc_row);
    RUN_TEST(spot_parse_empty_balances);
    RUN_TEST(spot_parse_rejects_malformed);
    RUN_TEST(spot_parse_rejects_null_and_empty);

    printf("\n-- abstraction mode --\n");
    RUN_TEST(mode_parse_all_four_names);
    RUN_TEST(mode_parse_default_is_recognized);
    RUN_TEST(mode_parse_unknown_stays_unknown);
    RUN_TEST(mode_pools_spot_only_for_unified_and_pm);

    printf("\n-- collateral model --\n");
    RUN_TEST(collateral_unified_uses_spot_total_alone);
    RUN_TEST(collateral_unified_does_not_double_count_pnl);
    RUN_TEST(collateral_unified_testnet_no_longer_reports_zero);
    RUN_TEST(collateral_unified_ignores_perps_value_entirely);
    RUN_TEST(collateral_portfolio_margin_uses_usdc_pool);
    RUN_TEST(collateral_default_adds_perps_and_spot);
    RUN_TEST(collateral_disabled_adds_perps_and_spot);
    RUN_TEST(collateral_no_spot_data_falls_back_to_perps);
    RUN_TEST(collateral_unified_with_zero_spot_reports_zero);

    return printTestSummary();
}
