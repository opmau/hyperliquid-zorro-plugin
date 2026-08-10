//=============================================================================
// test_alo_execution.cpp - ALO execution-path regression tests
//=============================================================================
// PREVENTS BUGS: OPM-792, OPM-796, OPM-797, OPM-798 (epic OPM-790)
//
// Background: post-only orders never reached the exchange. In live trading every
// order went out with "tif":"Ioc" even when the strategy had called
// brokerCommand(50012,"Alo"), because Zorro re-sends SET_ORDERTYPE at every
// order entry and can only derive a non-ALO type. These tests lock in each of
// the fixes that made maker execution reachable.
//
// TESTS (against the REAL implementations, not simulations, wherever the code
// is free of Zorro/HTTP dependencies):
//   CR-6 (OPM-796) Side-aware rounding + HL integer-price exemption
//   CR-2 (OPM-792) BrokerSell2 reports only what actually filled
//   CR-7 (OPM-797) Synthetic order IDs never produce a cancel for oid 0
//   CR-8 (OPM-798) Cancelled-with-partial reports the partial, not NAY-1
//   CR-9           Modify TIF must satisfy the always_place=false rule
//
// Sibling: test_alo_ordertype.cpp covers CR-1, CR-4 and CR-5 (sticky ALO
// order type, market TIF forcing + casing, exchange error surfacing).
//=============================================================================

#include "../test_framework.h"
#include "hl_utils.h"
#include "hl_types.h"
#include "hl_trading_service.h"
#include "hl_trading_response.h"
#include <cstring>
#include <cmath>

using namespace hl::test;
using namespace hl::utils;

//=============================================================================
// CR-6 (OPM-796): side-aware rounding + integer exemption  [real implementation]
//=============================================================================
// HL rule (Hyperliquid tick-and-lot-size docs): up to 5 significant
// figures and at most (MAX_DECIMALS - szDecimals) decimals, but "Integer
// prices are always allowed, regardless of the number of significant figures.
// E.g. 123456 is a valid price even though 12345.6 is not."

TEST_CASE(cr6_integer_price_passes_through_unchanged) {
    // The headline case: used to snap to 123460 (the 5-sig-fig $10 grid).
    ASSERT_FLOAT_EQ(roundPriceForExchange(123456.0, 4), 123456.0);
    ASSERT_FLOAT_EQ(roundPriceForExchange(123456.0, 4, 6, PriceRound::Down), 123456.0);
    ASSERT_FLOAT_EQ(roundPriceForExchange(123456.0, 4, 6, PriceRound::Up), 123456.0);
}

TEST_CASE(cr6_btc_above_100k_quotable_on_the_dollar_grid) {
    // Every $1 level above $100k must be expressible, or a maker strategy
    // cannot quote inside a $10 band.
    for (double px = 100001.0; px <= 100009.0; px += 1.0) {
        ASSERT_FLOAT_EQ(roundPriceForExchange(px, 4), px);
    }
    ASSERT_FLOAT_EQ(roundPriceForExchange(104999.0, 4), 104999.0);
}

TEST_CASE(cr6_buy_never_rounds_above_input) {
    // Rounding a passive buy UP can cross the spread -> post-only reject.
    const double prices[] = {100000.7, 1234.567, 99999.9, 42.4242, 0.00123456};
    const int szDecs[]    = {4,        4,        4,       2,       0};
    for (int i = 0; i < 5; ++i) {
        double r = roundPriceForExchange(prices[i], szDecs[i], 6, PriceRound::Down);
        ASSERT_TRUE(r <= prices[i] + 1e-12);
        ASSERT_TRUE(r > 0);
    }
}

TEST_CASE(cr6_sell_never_rounds_below_input) {
    const double prices[] = {100000.2, 1234.561, 99999.1, 42.4242, 0.00123451};
    const int szDecs[]    = {4,        4,        4,       2,       0};
    for (int i = 0; i < 5; ++i) {
        double r = roundPriceForExchange(prices[i], szDecs[i], 6, PriceRound::Up);
        ASSERT_TRUE(r >= prices[i] - 1e-12);
    }
}

TEST_CASE(cr6_on_grid_price_survives_directional_rounding) {
    // 1234.6 / 0.1 is 12345.999999999998 in binary floating point; a naive
    // floor() would silently drop a full tick off every already-valid quote.
    ASSERT_FLOAT_EQ(roundPriceForExchange(1234.6, 4, 6, PriceRound::Down), 1234.6);
    ASSERT_FLOAT_EQ(roundPriceForExchange(1234.6, 4, 6, PriceRound::Up), 1234.6);
    ASSERT_FLOAT_EQ(roundPriceForExchange(0.001234, 0, 6, PriceRound::Down), 0.001234);
    ASSERT_FLOAT_EQ(roundPriceForExchange(0.001234, 0, 6, PriceRound::Up), 0.001234);
}

TEST_CASE(cr6_five_sig_fig_rule_still_enforced_below_100k) {
    // 1234.56 has 6 significant figures — invalid; must snap to the 0.1 grid.
    ASSERT_FLOAT_EQ(roundPriceForExchange(1234.56, 4, 6, PriceRound::Down), 1234.5);
    ASSERT_FLOAT_EQ(roundPriceForExchange(1234.56, 4, 6, PriceRound::Up), 1234.6);
    ASSERT_FLOAT_EQ(roundPriceForExchange(1234.56, 4, 6, PriceRound::Nearest), 1234.6);
}

TEST_CASE(cr6_decimal_limit_still_enforced) {
    // szDecimals=4 -> at most 6-4 = 2 decimal places.
    ASSERT_FLOAT_EQ(roundPriceForExchange(42.4242, 4, 6, PriceRound::Nearest), 42.42);

    // szDecimals=1 -> at most 5 decimals; 0.01234 is valid, 0.0123456 is not.
    ASSERT_FLOAT_EQ(roundPriceForExchange(0.0123456, 1, 6, PriceRound::Down), 0.01234);
}

TEST_CASE(cr6_guards_unchanged) {
    ASSERT_EQ(0.0, roundPriceForExchange(0.0, 4));
    ASSERT_EQ(0.0, roundPriceForExchange(-1.0, 4));
    ASSERT_EQ(0.0, roundPriceForExchange(1e15, 4));
}

TEST_CASE(cr6_positive_price_never_rounds_to_zero) {
    // A positive price must never become "p":"0" on the wire. When an asset
    // allows no decimal places (szDecimals >= maxDecimals forces the integer
    // grid), flooring a sub-$1 buy would otherwise yield 0.
    //
    // Not reachable on any live perp today — max szDecimals across HL's 232
    // perps is 5, so maxDecPlaces >= 1 — but assets are listed continuously and
    // the failure would be silent, so the guard is load-bearing.
    const int szDecs[] = {6, 7, 8};
    const double prices[] = {0.5, 0.00012, 0.9999};
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            ASSERT_TRUE(roundPriceForExchange(prices[j], szDecs[i], 6, PriceRound::Down) > 0);
            ASSERT_TRUE(roundPriceForExchange(prices[j], szDecs[i], 6, PriceRound::Up) > 0);
            ASSERT_TRUE(roundPriceForExchange(prices[j], szDecs[i], 6, PriceRound::Nearest) > 0);
        }
    }

    // The guard must not perturb prices that are already representable.
    ASSERT_FLOAT_EQ(roundPriceForExchange(0.5, 4, 6, PriceRound::Down), 0.5);
    ASSERT_FLOAT_EQ(roundPriceForExchange(0.5, 4, 6, PriceRound::Up), 0.5);
}

TEST_CASE(cr6_format_price_uses_the_side) {
    ASSERT_STREQ("123456", formatPriceForExchange(123456.0, 4, 6, PriceRound::Down).c_str());
    ASSERT_STREQ("1234.5", formatPriceForExchange(1234.56, 4, 6, PriceRound::Down).c_str());
    ASSERT_STREQ("1234.6", formatPriceForExchange(1234.56, 4, 6, PriceRound::Up).c_str());
}

//=============================================================================
// CR-2 (OPM-792): BrokerSell2 reports only what actually filled
//=============================================================================
// The old code reported abs(amount) — a FULL close — whenever filledSize was
// 0, which is exactly what a resting (ALO/GTC) close returns. Zorro wrote the
// position off its books while the contracts were still on the exchange.

struct SellResult {
    int fillLots = 0;
    double closePx = 0.0;
    double profit = 0.0;
    bool appliedToCache = false;   // applyFill / recordZorroClose ran
    int linkedCloseTradeId = 0;    // DO_CANCEL target while resting
};

static SellResult brokerSell2_FIXED(double requestedSize, double filledSize,
                                    double avgPrice, double entryPrice,
                                    double lotSize, int closeTradeId) {
    SellResult r;
    double closeFillSize = (filledSize > 0) ? filledSize : 0.0;

    r.fillLots = (closeFillSize > 0 && lotSize > 0)
        ? (int)floor(closeFillSize / lotSize + 0.5) : 0;
    r.closePx = (closeFillSize > 0 && avgPrice > 0) ? avgPrice : 0.0;
    if (entryPrice > 0 && r.closePx > 0)
        r.profit = (r.closePx - entryPrice) * closeFillSize;
    r.appliedToCache = (closeFillSize > 0 && r.closePx > 0);

    bool fullyClosed = (closeFillSize >= requestedSize - 1e-12);
    r.linkedCloseTradeId = fullyClosed ? 0 : closeTradeId;
    return r;
}

TEST_CASE(cr2_resting_close_reports_nothing_closed) {
    // 1.0 BTC close requested, exchange says "resting", nothing filled.
    SellResult r = brokerSell2_FIXED(1.0, 0.0, 0.0, 40000.0, 0.0001, 777);

    ASSERT_EQ(0, r.fillLots);            // was 10000 lots (a full phantom close)
    ASSERT_EQ(0.0, r.closePx);
    ASSERT_EQ(0.0, r.profit);
    ASSERT_FALSE(r.appliedToCache);      // caches must not move
    ASSERT_EQ(777, r.linkedCloseTradeId);// and the order stays cancellable
}

TEST_CASE(cr2_partial_close_reports_the_partial) {
    // 0.4 of a 1.0 BTC close filled at 41000.
    SellResult r = brokerSell2_FIXED(1.0, 0.4, 41000.0, 40000.0, 0.0001, 778);

    ASSERT_EQ(4000, r.fillLots);
    ASSERT_FLOAT_EQ_TOL(r.closePx, 41000.0, 1e-9);
    ASSERT_FLOAT_EQ_TOL(r.profit, 400.0, 1e-6);  // (41000-40000) * 0.4
    ASSERT_TRUE(r.appliedToCache);
    ASSERT_EQ(778, r.linkedCloseTradeId);// remainder still working
}

TEST_CASE(cr2_full_close_reports_full_and_unlinks) {
    SellResult r = brokerSell2_FIXED(1.0, 1.0, 41000.0, 40000.0, 0.0001, 779);

    ASSERT_EQ(10000, r.fillLots);
    ASSERT_FLOAT_EQ_TOL(r.closePx, 41000.0, 1e-9);
    ASSERT_FLOAT_EQ_TOL(r.profit, 1000.0, 1e-6);
    ASSERT_TRUE(r.appliedToCache);
    ASSERT_EQ(0, r.linkedCloseTradeId);  // nothing left to cancel
}

TEST_CASE(cr2_short_close_profit_sign) {
    // Closing a short at a LOWER price is a profit; the caller negates for
    // OrderSide::Sell. Verified here on the magnitude the helper produces.
    SellResult r = brokerSell2_FIXED(1.0, 1.0, 39000.0, 40000.0, 0.0001, 0);
    ASSERT_FLOAT_EQ_TOL(r.profit, -1000.0, 1e-6);   // long-convention; inverted for shorts
}

TEST_CASE(cr2_resting_close_never_debits_the_ledger) {
    // recordZorroClose must see 0, or BrokerTrade's fill-poll reports the
    // position closed and Zorro's ledger permanently desyncs (OPM-733 class).
    SellResult r = brokerSell2_FIXED(2.0, 0.0, 0.0, 100.0, 1.0, 42);
    ASSERT_FALSE(r.appliedToCache);
    ASSERT_EQ(0, r.fillLots);
}

//=============================================================================
// CR-7 (OPM-797): synthetic order IDs must never cancel oid 0
//=============================================================================

TEST_CASE(cr7_synthetic_ids_are_not_exchange_order_ids) {
    ASSERT_FALSE(isExchangeOrderId("PENDING_0x1234abcd"));
    ASSERT_FALSE(isExchangeOrderId("RESUMED_17"));
    ASSERT_FALSE(isExchangeOrderId("IMPORTED_42"));
    ASSERT_FALSE(isExchangeOrderId("DRY_RUN"));
    ASSERT_FALSE(isExchangeOrderId("UNKNOWN"));
    ASSERT_FALSE(isExchangeOrderId(""));
    ASSERT_FALSE(isExchangeOrderId(nullptr));
    ASSERT_FALSE(isExchangeOrderId("0"));        // _atoi64 -> 0: the bug itself
    ASSERT_FALSE(isExchangeOrderId("12a34"));
    ASSERT_FALSE(isExchangeOrderId(" 1234"));
    ASSERT_FALSE(isExchangeOrderId("-5"));
}

TEST_CASE(cr7_real_oids_are_accepted) {
    ASSERT_TRUE(isExchangeOrderId("1"));
    ASSERT_TRUE(isExchangeOrderId("123456789"));
    ASSERT_TRUE(isExchangeOrderId("98765432109876"));
}

TEST_CASE(cr7_coin_matching_across_naming_variants) {
    // Exchange spelling vs the API form buildCoinForApi() produces.
    ASSERT_TRUE(coinMatches("BTC", "BTC"));
    ASSERT_TRUE(coinMatches("BTC", "BTC-USDC"));
    ASSERT_TRUE(coinMatches("xyz:TSLA", "xyz:TSLA-USDC"));
    ASSERT_TRUE(coinMatches("xyz:TSLA", "xyz:TSLA"));
    ASSERT_TRUE(coinMatches("btc", "BTC"));            // case-insensitive
}

TEST_CASE(cr7_coin_matching_respects_the_venue) {
    // A main-dex BTC position must never be confused with a perpDex BTC —
    // the OPM-600 class of bug.
    ASSERT_FALSE(coinMatches("BTC", "xyz:BTC"));
    ASSERT_FALSE(coinMatches("xyz:BTC", "BTC"));
    ASSERT_FALSE(coinMatches("abc:BTC", "xyz:BTC"));
    ASSERT_FALSE(coinMatches("ETH", "BTC"));
    ASSERT_FALSE(coinMatches("", "BTC"));
    ASSERT_FALSE(coinMatches("BTC", ""));
}

TEST_CASE(cr7_coin_matching_preserves_spot_names) {
    ASSERT_TRUE(coinMatches("PURR/USDC", "PURR/USDC"));
    ASSERT_TRUE(coinMatches("@107", "@107"));
    ASSERT_FALSE(coinMatches("@107", "@108"));
}

//=============================================================================
// CR-8 (OPM-798): a cancelled order that partially filled is not NAY-1
//=============================================================================
// NAY-1 tells Zorro the order never existed, so it discards the partial from
// its ledger — but the exchange still holds those contracts. A reprice loop
// (cancel/replace with partials in flight) hits this on every iteration.

#define TEST_NAY (-1000000)

static int brokerTradeReturn_FIXED(double filledSize, double closedSize,
                                   bool cancelled, double lotSize) {
    double openSize = filledSize - closedSize;
    double lotEps = (lotSize > 0) ? lotSize * 0.5 : 1e-9;
    if (openSize < lotEps) openSize = 0;

    if (cancelled && openSize <= 0) return TEST_NAY - 1;
    if (openSize > 0) {
        int lots = (lotSize > 0) ? (int)floor(openSize / lotSize + 0.5)
                                 : (int)floor(openSize + 0.5);
        return (lots > 0) ? lots : 1;
    }
    return 0;
}

TEST_CASE(cr8_cancelled_with_partial_reports_the_partial) {
    // 0.3 BTC filled, then cancelled. Those contracts are real.
    int r = brokerTradeReturn_FIXED(0.3, 0.0, true, 0.0001);
    ASSERT_EQ(3000, r);
    ASSERT_TRUE(r != TEST_NAY - 1);
}

TEST_CASE(cr8_cancelled_with_no_fill_is_still_nay_minus_1) {
    ASSERT_EQ(TEST_NAY - 1, brokerTradeReturn_FIXED(0.0, 0.0, true, 0.0001));
}

TEST_CASE(cr8_cancelled_after_full_close_is_nay_minus_1) {
    // Partial filled and then closed by Zorro — nothing open remains.
    ASSERT_EQ(TEST_NAY - 1, brokerTradeReturn_FIXED(0.3, 0.3, true, 0.0001));
}

TEST_CASE(cr8_sublot_residual_does_not_resurrect_a_trade) {
    // OPM-733 guard must survive the reordering: a float residual smaller than
    // half a lot is not a phantom 1-lot trade.
    ASSERT_EQ(TEST_NAY - 1, brokerTradeReturn_FIXED(0.30000001, 0.3, true, 0.0001));
    ASSERT_EQ(0, brokerTradeReturn_FIXED(0.30000001, 0.3, false, 0.0001));
}

TEST_CASE(cr8_open_order_unaffected_by_the_reorder) {
    ASSERT_EQ(3000, brokerTradeReturn_FIXED(0.3, 0.0, false, 0.0001));
    ASSERT_EQ(0, brokerTradeReturn_FIXED(0.0, 0.0, false, 0.0001));
}

//=============================================================================
// CR-9: modify TIF must satisfy the always_place=false rule  [real impl]
//=============================================================================
// packBatchModifyAction omits the action's `a` (always_place) field, because
// the exchange docs require it: "a must be skipped if false, i.e. actions
// hashed with a: false will be rejected". Omitting it means always_place is
// false, and that branch constrains the replacement order:
//
//   "When always_place = false the new order must be a non-trigger order, and
//    must have TIF = ALO or a non-executable order with TIF = GTC. In the
//    latter case, TIF of the new order is overridden to ALO."
//     -- Hyperliquid exchange-endpoint docs, batchModify
//
// So IOC is rejected by the exchange unconditionally. 50044 hardcodes ALO, but
// 50042 (HL_MODIFY_ORDER) forwards a caller-supplied TIF whose struct default
// is Gtc — an IOC modify used to go out and come back as an opaque failure.

TEST_CASE(cr9_ioc_is_never_valid_for_modify) {
    // Unconditionally rejected by HL: guard it before spending a round trip.
    ASSERT_FALSE(isTifValidForModify("Ioc"));
    ASSERT_FALSE(isTifValidForModify("ioc"));   // casing must not smuggle it through
    ASSERT_FALSE(isTifValidForModify("IOC"));
}

TEST_CASE(cr9_alo_is_valid_for_modify) {
    // The reprice path. Explicitly allowed by the always_place=false branch.
    ASSERT_TRUE(isTifValidForModify("Alo"));
    ASSERT_TRUE(isTifValidForModify("alo"));
    ASSERT_TRUE(isTifValidForModify("ALO"));
}

TEST_CASE(cr9_gtc_is_valid_for_modify) {
    // Conditionally accepted — HL takes a non-executable GTC and overrides it
    // to ALO. Executability is not knowable client-side, so this is not a
    // client-side reject; the exchange decides.
    ASSERT_TRUE(isTifValidForModify("Gtc"));
    ASSERT_TRUE(isTifValidForModify("gtc"));
}

TEST_CASE(cr9_invalid_tif_is_rejected_for_modify) {
    ASSERT_FALSE(isTifValidForModify("Fok"));
    ASSERT_FALSE(isTifValidForModify(""));
    ASSERT_FALSE(isTifValidForModify(nullptr));
    ASSERT_FALSE(isTifValidForModify("Alo "));  // no silent trimming, as canonicalTif
}

TEST_CASE(cr9_modify_validity_agrees_with_canonical_tif) {
    // Anything canonicalTif rejects must also be invalid for modify — the guard
    // narrows the valid set, it must never widen it.
    const char* junk[] = {"Fok", "", "Alo ", "Post", "gtc "};
    for (int i = 0; i < 5; ++i) {
        if (canonicalTif(junk[i]) == nullptr) {
            ASSERT_FALSE(isTifValidForModify(junk[i]));
        }
    }
}

//=============================================================================
// MAIN
//=============================================================================

int main() {
    printf("\n=== OPM-790: ALO Execution Regression Tests ===\n\n");

    printf("--- CR-6 (OPM-796): side-aware rounding ---\n");
    RUN_TEST(cr6_integer_price_passes_through_unchanged);
    RUN_TEST(cr6_btc_above_100k_quotable_on_the_dollar_grid);
    RUN_TEST(cr6_buy_never_rounds_above_input);
    RUN_TEST(cr6_sell_never_rounds_below_input);
    RUN_TEST(cr6_on_grid_price_survives_directional_rounding);
    RUN_TEST(cr6_five_sig_fig_rule_still_enforced_below_100k);
    RUN_TEST(cr6_decimal_limit_still_enforced);
    RUN_TEST(cr6_guards_unchanged);
    RUN_TEST(cr6_positive_price_never_rounds_to_zero);
    RUN_TEST(cr6_format_price_uses_the_side);

    printf("\n--- CR-2 (OPM-792): honest close reporting ---\n");
    RUN_TEST(cr2_resting_close_reports_nothing_closed);
    RUN_TEST(cr2_partial_close_reports_the_partial);
    RUN_TEST(cr2_full_close_reports_full_and_unlinks);
    RUN_TEST(cr2_short_close_profit_sign);
    RUN_TEST(cr2_resting_close_never_debits_the_ledger);

    printf("\n--- CR-7 (OPM-797): cancel-all safety ---\n");
    RUN_TEST(cr7_synthetic_ids_are_not_exchange_order_ids);
    RUN_TEST(cr7_real_oids_are_accepted);
    RUN_TEST(cr7_coin_matching_across_naming_variants);
    RUN_TEST(cr7_coin_matching_respects_the_venue);
    RUN_TEST(cr7_coin_matching_preserves_spot_names);

    printf("\n--- CR-8 (OPM-798): partial-fill preservation ---\n");
    RUN_TEST(cr8_cancelled_with_partial_reports_the_partial);
    RUN_TEST(cr8_cancelled_with_no_fill_is_still_nay_minus_1);
    RUN_TEST(cr8_cancelled_after_full_close_is_nay_minus_1);
    RUN_TEST(cr8_sublot_residual_does_not_resurrect_a_trade);
    RUN_TEST(cr8_open_order_unaffected_by_the_reorder);

    printf("\n--- CR-9: modify TIF vs always_place=false ---\n");
    RUN_TEST(cr9_ioc_is_never_valid_for_modify);
    RUN_TEST(cr9_alo_is_valid_for_modify);
    RUN_TEST(cr9_gtc_is_valid_for_modify);
    RUN_TEST(cr9_invalid_tif_is_rejected_for_modify);
    RUN_TEST(cr9_modify_validity_agrees_with_canonical_tif);

    return printTestSummary();
}
