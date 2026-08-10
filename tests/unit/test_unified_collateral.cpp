//=============================================================================
// test_unified_collateral.cpp - Unified-account collateral tests [OPM-824]
//=============================================================================
// LAYER: Test | TESTS: src/services/hl_account_collateral.h (production code,
//                      linked directly — not a re-implementation)
//
// The defect: BrokerAccount reported clearinghouseState.marginSummary
// .accountValue, which is perps-only AND main-dex only. On a unified account —
// where one spot USDC pool collateralizes everything — that under-reported
// usable equity on accounts with a shared collateral pool, and read a flat 0
// on an account funded entirely on the spot side, tripping the zero-balance
// guard and halting the strategy.
//
// Payload SHAPES below mirror the live API exactly; the values are illustrative.
//=============================================================================

#include "../test_framework.h"
#include "../../src/services/hl_account_collateral.h"
#include <cstring>

using namespace hl::test;
using namespace hl::account;

//=============================================================================
// CANNED PAYLOADS — live response shapes, illustrative values
//=============================================================================

// unifiedAccount. Spot USDC total IS the whole collateral pool; the perps
// accountValue for the same account is a different, smaller number.
static const char* SPOT_UNIFIED_MAINNET =
    "{\"balances\":[{\"coin\":\"USDC\",\"token\":0,\"total\":\"20000.000000\","
    "\"hold\":\"12000.000000\",\"entryNtl\":\"0.0\"}],"
    "\"tokenToAvailableAfterMaintenance\":[[0,\"15000.000000\"]]}";

// unifiedAccount, perps accountValue 0.0 — the account is
// funded entirely on the spot side. This is the payload that used to halt.
static const char* SPOT_UNIFIED_TESTNET =
    "{\"balances\":[{\"coin\":\"USDC\",\"token\":0,\"total\":\"1234.567890\","
    "\"hold\":\"0.0\",\"entryNtl\":\"0.0\"}],"
    "\"tokenToAvailableAfterMaintenance\":[[0,\"1234.567890\"]]}";

// "default" mode. No tokenToAvailableAfterMaintenance.
static const char* SPOT_DEFAULT_MAINNET =
    "{\"balances\":[{\"coin\":\"USDC\",\"token\":0,\"total\":\"0.00500000\","
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
    "{\"balances\":[{\"coin\":\"HYPE\",\"token\":150,\"total\":\"10.0000\","
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
    ASSERT_FLOAT_EQ_TOL(s.usdcTotal, 20000.000000, 1e-6);
    ASSERT_FLOAT_EQ_TOL(s.availAfterMaint, 15000.000000, 1e-6);
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
    ASSERT_FLOAT_EQ_TOL(s.usdcTotal, 0.00500000, 1e-8);
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

    CollateralView v = computeCollateral(12500.00, 10000.00, s);
    ASSERT_TRUE(v.unified);
    // NOT 12500.00 (the old, perps-only figure) and NOT 32500.00 (the sum).
    ASSERT_FLOAT_EQ_TOL(v.equity, 20000.000000, 1e-6);
    ASSERT_FLOAT_EQ_TOL(v.freeCollateral, 15000.000000, 1e-6);
}

TEST_CASE(collateral_unified_does_not_double_count_pnl) {
    // Spot USDC total is marked to market and already contains perp PnL —
    // it moves one-for-one with the perp accountValue. So moving BOTH by the
    // same amount must move equity by that amount ONCE.
    SpotState s;
    parseSpotClearinghouseState(SPOT_UNIFIED_MAINNET, strlen(SPOT_UNIFIED_MAINNET), s);
    double before = computeCollateral(12500.00, 10000.00, s).equity;

    const double pnl = 100.0;
    s.usdcTotal += pnl;
    double after = computeCollateral(12500.00 + pnl, 10000.00, s).equity;

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
    double b = computeCollateral(12500.00,   0.0, s).equity;
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

    CollateralView v = computeCollateral(250.000000, 0.0, s);
    ASSERT_FALSE(v.unified);
    ASSERT_FLOAT_EQ_TOL(v.equity, 250.000000 + 0.00500000, 1e-8);
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
    CollateralView v = computeCollateral(12500.00, 10000.00, s);
    ASSERT_FALSE(v.unified);
    ASSERT_FLOAT_EQ_TOL(v.equity, 12500.00, 1e-9);
    ASSERT_FLOAT_EQ_TOL(v.freeCollateral, 2500.00, 1e-9);
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
// TEST CASES: Zorro account split [OPM-876]
//
// Zorro derives equity from the balance plus the profit of the trades it
// tracks, so the balance handed over must be a cash basis, not a
// marked-to-market equity.
//=============================================================================

TEST_CASE(zorro_split_reconstructs_equity) {
    ZorroAccountSplit s = splitEquityForZorro(25000.00, 3700.00);
    ASSERT_FLOAT_EQ_TOL(s.balance + s.tradeVal, 25000.00, 1e-9);
}

TEST_CASE(zorro_split_removes_open_pnl_from_balance) {
    // The reported balance is a cash basis: equity less the open profit that
    // Zorro is going to add back on.
    ZorroAccountSplit s = splitEquityForZorro(25000.00, 3700.00);
    ASSERT_FLOAT_EQ_TOL(s.balance, 21300.00, 1e-9);
    ASSERT_FLOAT_EQ_TOL(s.tradeVal, 3700.00, 1e-9);
}

TEST_CASE(zorro_split_equity_moves_once_per_pnl_move) {
    // PnL moves land entirely in tradeVal; the cash basis is unmoved.
    const double cash = 21300.00;
    double pnlBefore = 3700.00, pnlAfter = 3800.00;

    ZorroAccountSplit a = splitEquityForZorro(cash + pnlBefore, pnlBefore);
    ZorroAccountSplit b = splitEquityForZorro(cash + pnlAfter,  pnlAfter);

    // Cash basis is unmoved by PnL; the whole move lands in tradeVal, once.
    ASSERT_FLOAT_EQ_TOL(a.balance, b.balance, 1e-9);
    ASSERT_FLOAT_EQ_TOL(b.tradeVal - a.tradeVal, 100.00, 1e-9);
    ASSERT_FLOAT_EQ_TOL((b.balance + b.tradeVal) - (a.balance + a.tradeVal), 100.00, 1e-9);
}

TEST_CASE(zorro_split_flat_account_is_all_balance) {
    ZorroAccountSplit s = splitEquityForZorro(25000.00, 0.0);
    ASSERT_FLOAT_EQ_TOL(s.balance, 25000.00, 1e-9);
    ASSERT_FLOAT_EQ(s.tradeVal, 0.0);
}

TEST_CASE(zorro_split_losing_book_raises_balance) {
    // Open losses are already deducted from equity, so the cash basis is higher
    // than equity and tradeVal is negative.
    ZorroAccountSplit s = splitEquityForZorro(18000.00, -2500.00);
    ASSERT_FLOAT_EQ_TOL(s.balance, 20500.00, 1e-9);
    ASSERT_FLOAT_EQ_TOL(s.tradeVal, -2500.00, 1e-9);
    ASSERT_FLOAT_EQ_TOL(s.balance + s.tradeVal, 18000.00, 1e-9);
}

TEST_CASE(zorro_split_profit_above_capital_gives_negative_balance) {
    // Profit can exceed deposited capital. A negative cash basis is the honest
    // figure and still reconstructs the right equity.
    ZorroAccountSplit s = splitEquityForZorro(1000.00, 1500.00);
    ASSERT_LT(s.balance, 0.0);
    ASSERT_FLOAT_EQ_TOL(s.balance + s.tradeVal, 1000.00, 1e-9);
}

TEST_CASE(zorro_split_feeds_from_unified_equity) {
    // End to end: the unified equity computeCollateral() derives is what gets
    // split, so the two modules agree on the number Zorro finally sees.
    SpotState st;
    parseSpotClearinghouseState(SPOT_UNIFIED_MAINNET, strlen(SPOT_UNIFIED_MAINNET), st);
    CollateralView v = computeCollateral(12500.00, 10000.00, st);

    ZorroAccountSplit s = splitEquityForZorro(v.equity, 4000.00);
    ASSERT_FLOAT_EQ_TOL(s.balance, 16000.000000, 1e-6);
    ASSERT_FLOAT_EQ_TOL(s.balance + s.tradeVal, v.equity, 1e-9);
}

//=============================================================================
// TEST CASES: Spot refresh schedule [OPM-878]
//
// Spot state has no WS channel, so its age is bounded by this predicate. The
// wraparound cases matter: GetTickCount() wraps to 0 after ~49.7 days, and a
// signed comparison would wrongly report "due" or "fresh" across the wrap.
//=============================================================================

TEST_CASE(spot_refresh_due_when_never_fetched) {
    ASSERT_TRUE(spotRefreshDue(1000, 0, false, 60000));
}

TEST_CASE(spot_refresh_not_due_within_ttl) {
    ASSERT_FALSE(spotRefreshDue(59999, 0, true, 60000));
}

TEST_CASE(spot_refresh_due_at_ttl_boundary) {
    ASSERT_TRUE(spotRefreshDue(60000, 0, true, 60000));
}

TEST_CASE(spot_refresh_survives_tick_wraparound) {
    // Fetched just before the wrap, checked just after: elapsed is small, so
    // no refresh is due even though nowTick < lastFetchTick numerically.
    unsigned long last = 0xFFFFFF00ul;   // 256 ticks before the wrap
    unsigned long now = 0x00000100ul;    // 256 ticks after it (elapsed 512ms)
    ASSERT_FALSE(spotRefreshDue(now, last, true, 60000));

    // And a full TTL after the wrap, the refresh IS due.
    ASSERT_TRUE(spotRefreshDue(last + 60000ul, last, true, 60000));
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

    printf("\n-- Zorro account split --\n");
    RUN_TEST(zorro_split_reconstructs_equity);
    RUN_TEST(zorro_split_removes_open_pnl_from_balance);
    RUN_TEST(zorro_split_equity_moves_once_per_pnl_move);
    RUN_TEST(zorro_split_flat_account_is_all_balance);
    RUN_TEST(zorro_split_losing_book_raises_balance);
    RUN_TEST(zorro_split_profit_above_capital_gives_negative_balance);
    RUN_TEST(zorro_split_feeds_from_unified_equity);

    printf("\n-- spot refresh schedule --\n");
    RUN_TEST(spot_refresh_due_when_never_fetched);
    RUN_TEST(spot_refresh_not_due_within_ttl);
    RUN_TEST(spot_refresh_due_at_ttl_boundary);
    RUN_TEST(spot_refresh_survives_tick_wraparound);

    return printTestSummary();
}
