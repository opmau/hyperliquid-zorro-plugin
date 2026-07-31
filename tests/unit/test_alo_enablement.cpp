//=============================================================================
// test_alo_enablement.cpp - ALO (post-only maker) enablement regression tests
//=============================================================================
// PREVENTS BUGS: OPM-791..OPM-798 (epic OPM-790)
//
// Background: post-only orders never reached the exchange. In live trading every
// order went out with "tif":"Ioc" even when the strategy had called
// brokerCommand(50012,"Alo"), because Zorro re-sends SET_ORDERTYPE at every
// order entry and can only derive a non-ALO type. These tests lock in each of
// the fixes that made maker execution reachable.
//
// TESTS (against the REAL implementations, not simulations, wherever the code
// is free of Zorro/HTTP dependencies):
//   CR-1 (OPM-791) Sticky ALO survives Zorro's auto SET_ORDERTYPE
//   CR-4 (OPM-794) Market orders force Ioc; 50012 casing canonicalized
//   CR-5 (OPM-795) statuses[].error parsed and classified
//   CR-6 (OPM-796) Side-aware rounding + HL integer-price exemption
//   CR-2 (OPM-792) BrokerSell2 reports only what actually filled
//   CR-7 (OPM-797) Synthetic order IDs never produce a cancel for oid 0
//   CR-8 (OPM-798) Cancelled-with-partial reports the partial, not NAY-1
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
// CR-1 (OPM-791): sticky ALO order type
//=============================================================================
// Mirrors the SET_ORDERTYPE / HL_SET_ORDER_TYPE decision logic in
// hl_broker_commands.cpp. The API layer cannot be linked here (it pulls in the
// Zorro SDK), so the state machine is reproduced exactly; the assertions below
// are what the handler must implement.

struct OrderTypeState {
    char orderType[16] = "Ioc";
    bool sticky = false;

    // brokerCommand(50012, "<tif>") — returns false for an invalid TIF
    bool cmd50012(const char* tif) {
        const char* canonical = canonicalTif(tif);
        if (!canonical) return false;
        strcpy_s(orderType, canonical);
        sticky = (strcmp(canonical, "Alo") == 0);
        return true;
    }

    // SET_ORDERTYPE(n) — the call Zorro makes automatically at every entry
    void setOrderType(int n) {
        int baseType = n & 7;
        if (sticky) return;                       // [OPM-791] do not downgrade
        if (baseType == 0)      strcpy_s(orderType, "Ioc");
        else if (baseType == 2) strcpy_s(orderType, "Gtc");
        else if (baseType == 4) { strcpy_s(orderType, "Alo"); sticky = true; }
    }
};

TEST_CASE(cr1_auto_set_ordertype_used_to_clobber_alo) {
    // The pre-fix behaviour, kept as documentation of the defect: without
    // stickiness the auto-call wins and the order goes out as a taker.
    OrderTypeState s;
    s.cmd50012("Alo");
    s.sticky = false;              // simulate the old code (no override)
    s.setOrderType(0);
    ASSERT_STREQ("Ioc", s.orderType);
}

TEST_CASE(cr1_sticky_alo_survives_auto_set_ordertype_0) {
    OrderTypeState s;
    ASSERT_TRUE(s.cmd50012("Alo"));
    ASSERT_STREQ("Alo", s.orderType);
    ASSERT_TRUE(s.sticky);

    // Zorro auto-calls SET_ORDERTYPE(0) immediately before every order entry.
    s.setOrderType(0);
    ASSERT_STREQ("Alo", s.orderType);

    // ...and repeatedly, across many bars.
    for (int i = 0; i < 50; ++i) s.setOrderType(0);
    ASSERT_STREQ("Alo", s.orderType);
}

TEST_CASE(cr1_sticky_alo_survives_auto_set_ordertype_2_gtc) {
    OrderTypeState s;
    s.cmd50012("Alo");
    s.setOrderType(2);                    // TradeMode-derived GTC
    ASSERT_STREQ("Alo", s.orderType);
}

TEST_CASE(cr1_sticky_survives_stop_flag_variant) {
    // +8 is the STOP flag; the base type still must not downgrade ALO.
    OrderTypeState s;
    s.cmd50012("Alo");
    s.setOrderType(0 + 8);
    ASSERT_STREQ("Alo", s.orderType);
}

TEST_CASE(cr1_ioc_releases_the_sticky_override) {
    OrderTypeState s;
    s.cmd50012("Alo");
    ASSERT_TRUE(s.sticky);

    ASSERT_TRUE(s.cmd50012("Ioc"));       // explicit release
    ASSERT_FALSE(s.sticky);
    ASSERT_STREQ("Ioc", s.orderType);

    s.setOrderType(2);                    // auto-calls take effect again
    ASSERT_STREQ("Gtc", s.orderType);
}

TEST_CASE(cr1_gtc_releases_the_sticky_override) {
    OrderTypeState s;
    s.cmd50012("Alo");
    ASSERT_TRUE(s.cmd50012("Gtc"));
    ASSERT_FALSE(s.sticky);
    ASSERT_STREQ("Gtc", s.orderType);

    s.setOrderType(0);
    ASSERT_STREQ("Ioc", s.orderType);
}

TEST_CASE(cr1_set_ordertype_4_is_also_sticky) {
    // Zorro never sends 4, but a script calling brokerCommand(157,4) means it.
    OrderTypeState s;
    s.setOrderType(4);
    ASSERT_STREQ("Alo", s.orderType);
    s.setOrderType(0);
    ASSERT_STREQ("Alo", s.orderType);
}

//=============================================================================
// CR-4a (OPM-794): the request enum is the only source of truth for wire TIF
//=============================================================================
// Reproduces the TIF switch in hl_trading_service.cpp placeOrderWithId().

static const char* wireTif_FIXED(hl::OrderType t, const char* globalOrderType) {
    (void)globalOrderType;                 // deliberately unused after the fix
    switch (t) {
        case hl::OrderType::Gtc: return "Gtc";
        case hl::OrderType::Alo: return "Alo";
        case hl::OrderType::Ioc: return "Ioc";
        default:                 return "Ioc";
    }
}

static const char* wireTif_BUGGY(hl::OrderType t, const char* globalOrderType) {
    switch (t) {
        case hl::OrderType::Gtc: return "Gtc";
        case hl::OrderType::Alo: return "Alo";
        default:                 return globalOrderType;   // fell through
    }
}

TEST_CASE(cr4_market_order_used_to_inherit_alo) {
    // BrokerBuy2 downgrades a market order to OrderType::Ioc at a deliberately
    // CROSSING price (ask * 1.05). Sending that as "Alo" is a guaranteed
    // post-only reject — the booby trap this test locks out.
    ASSERT_STREQ("Alo", wireTif_BUGGY(hl::OrderType::Ioc, "Alo"));
}

TEST_CASE(cr4_market_order_forces_ioc_even_under_global_alo) {
    ASSERT_STREQ("Ioc", wireTif_FIXED(hl::OrderType::Ioc, "Alo"));
    ASSERT_STREQ("Ioc", wireTif_FIXED(hl::OrderType::Ioc, "Gtc"));
}

TEST_CASE(cr4_explicit_types_still_reach_the_wire) {
    ASSERT_STREQ("Alo", wireTif_FIXED(hl::OrderType::Alo, "Ioc"));
    ASSERT_STREQ("Gtc", wireTif_FIXED(hl::OrderType::Gtc, "Ioc"));
}

//=============================================================================
// CR-4b (OPM-794): TIF casing canonicalization  [real implementation]
//=============================================================================
// Only the msgpack signing path normalized casing. A 50012("ioc") therefore
// signed "Ioc" but serialized "ioc": the hashes disagree, HL recovers a
// phantom signer address and rejects with "User or API Wallet does not
// exist". This is the OPM-677 failure class.

TEST_CASE(cr4_canonical_tif_accepts_any_casing) {
    ASSERT_STREQ("Ioc", canonicalTif("ioc"));
    ASSERT_STREQ("Ioc", canonicalTif("IOC"));
    ASSERT_STREQ("Ioc", canonicalTif("Ioc"));
    ASSERT_STREQ("Ioc", canonicalTif("iOc"));

    ASSERT_STREQ("Gtc", canonicalTif("gtc"));
    ASSERT_STREQ("Gtc", canonicalTif("GTC"));

    ASSERT_STREQ("Alo", canonicalTif("alo"));
    ASSERT_STREQ("Alo", canonicalTif("ALO"));
    ASSERT_STREQ("Alo", canonicalTif("Alo"));
}

TEST_CASE(cr4_canonical_tif_output_is_byte_identical) {
    // Every casing must produce the SAME bytes, or JSON and signature diverge.
    const char* variants[] = {"alo", "ALO", "Alo", "aLO", "AlO"};
    for (int i = 0; i < 5; ++i) {
        const char* c = canonicalTif(variants[i]);
        ASSERT_TRUE(c != nullptr);
        ASSERT_EQ(0, strcmp(c, "Alo"));
    }
}

TEST_CASE(cr4_canonical_tif_rejects_invalid) {
    ASSERT_TRUE(canonicalTif("Fok") == nullptr);
    ASSERT_TRUE(canonicalTif("") == nullptr);
    ASSERT_TRUE(canonicalTif(nullptr) == nullptr);
    ASSERT_TRUE(canonicalTif("Alo ") == nullptr);   // no silent trimming
}

//=============================================================================
// CR-5 (OPM-795): exchange error surfacing  [real implementation]
//=============================================================================
// HL returns per-order rejects under a TOP-LEVEL status:"ok", so the err
// branch never saw them and every reject collapsed into "No order ID in
// exchange response". Error strings below are copied verbatim from
// Hyperliquid's documented API error responses.

TEST_CASE(cr5_classify_post_only_reject) {
    ASSERT_EQ(hl::trading::ORDER_ERR_POST_ONLY,
              hl::trading::classifyOrderError(
                  "Post only order would have immediately matched, bbo was 42.5"));
}

TEST_CASE(cr5_classify_margin_reject) {
    ASSERT_EQ(hl::trading::ORDER_ERR_MARGIN,
              hl::trading::classifyOrderError("Insufficient margin to place order."));
}

TEST_CASE(cr5_classify_other_rejects) {
    ASSERT_EQ(hl::trading::ORDER_ERR_OTHER,
              hl::trading::classifyOrderError("Price must be divisible by tick size."));
    ASSERT_EQ(hl::trading::ORDER_ERR_OTHER,
              hl::trading::classifyOrderError("Order must have minimum value of $10."));
    ASSERT_EQ(hl::trading::ORDER_ERR_OTHER,
              hl::trading::classifyOrderError("Reduce only order would increase position."));
}

TEST_CASE(cr5_classify_none_for_empty) {
    ASSERT_EQ(hl::trading::ORDER_ERR_NONE, hl::trading::classifyOrderError(""));
    ASSERT_EQ(hl::trading::ORDER_ERR_NONE, hl::trading::classifyOrderError(nullptr));
}

TEST_CASE(cr5_post_only_and_margin_are_distinguishable) {
    // The whole point: a reprice-aware strategy must branch on these.
    int postOnly = hl::trading::classifyOrderError(
        "Post only order would have immediately matched, bbo was 100.0");
    int margin = hl::trading::classifyOrderError("Insufficient margin to place order.");
    ASSERT_TRUE(postOnly != margin);
    ASSERT_TRUE(postOnly != hl::trading::ORDER_ERR_NONE);
    ASSERT_TRUE(margin != hl::trading::ORDER_ERR_NONE);
}

TEST_CASE(cr5_parse_alo_reject_from_canned_response) {
    // A deliberately-crossing ALO, exactly as HL returns it.
    const char* body =
        "{\"status\":\"ok\",\"response\":{\"type\":\"order\",\"data\":{\"statuses\":"
        "[{\"error\":\"Post only order would have immediately matched, bbo was 42.5\"}]}}}";

    hl::trading::OrderResponseStatus st;
    ASSERT_TRUE(hl::trading::parseOrderStatusResponse(body, strlen(body), st));
    ASSERT_TRUE(st.valid);
    ASSERT_FALSE(st.filled);
    ASSERT_FALSE(st.resting);
    ASSERT_FALSE(st.topLevelErr);              // reported under status:"ok"
    ASSERT_TRUE(st.error.find("Post only") != std::string::npos);
    ASSERT_EQ(hl::trading::ORDER_ERR_POST_ONLY,
              hl::trading::classifyOrderError(st.error.c_str()));
}

TEST_CASE(cr5_parse_margin_reject_from_canned_response) {
    const char* body =
        "{\"status\":\"ok\",\"response\":{\"type\":\"order\",\"data\":{\"statuses\":"
        "[{\"error\":\"Insufficient margin to place order.\"}]}}}";

    hl::trading::OrderResponseStatus st;
    ASSERT_TRUE(hl::trading::parseOrderStatusResponse(body, strlen(body), st));
    ASSERT_EQ(hl::trading::ORDER_ERR_MARGIN,
              hl::trading::classifyOrderError(st.error.c_str()));
}

TEST_CASE(cr5_parse_resting_response) {
    const char* body =
        "{\"status\":\"ok\",\"response\":{\"type\":\"order\",\"data\":{\"statuses\":"
        "[{\"resting\":{\"oid\":123456789}}]}}}";

    hl::trading::OrderResponseStatus st;
    ASSERT_TRUE(hl::trading::parseOrderStatusResponse(body, strlen(body), st));
    ASSERT_TRUE(st.resting);
    ASSERT_FALSE(st.filled);
    ASSERT_TRUE(st.error.empty());
    ASSERT_STREQ("123456789", st.oid);
    ASSERT_EQ(0.0, st.filledSize);             // a resting order filled nothing
}

TEST_CASE(cr5_parse_filled_response) {
    const char* body =
        "{\"status\":\"ok\",\"response\":{\"type\":\"order\",\"data\":{\"statuses\":"
        "[{\"filled\":{\"oid\":987,\"totalSz\":\"0.5\",\"avgPx\":\"42000.5\"}}]}}}";

    hl::trading::OrderResponseStatus st;
    ASSERT_TRUE(hl::trading::parseOrderStatusResponse(body, strlen(body), st));
    ASSERT_TRUE(st.filled);
    ASSERT_TRUE(st.error.empty());
    ASSERT_STREQ("987", st.oid);
    ASSERT_FLOAT_EQ_TOL(st.filledSize, 0.5, 1e-9);
    ASSERT_FLOAT_EQ_TOL(st.avgPrice, 42000.5, 1e-9);
}

TEST_CASE(cr5_parse_top_level_error) {
    const char* body = "{\"status\":\"err\",\"response\":\"User or API Wallet does not exist\"}";

    hl::trading::OrderResponseStatus st;
    ASSERT_TRUE(hl::trading::parseOrderStatusResponse(body, strlen(body), st));
    ASSERT_TRUE(st.topLevelErr);
    ASSERT_TRUE(st.error.find("does not exist") != std::string::npos);
}

TEST_CASE(cr5_parse_batchmodify_success) {
    const char* body =
        "{\"status\":\"ok\",\"response\":{\"type\":\"order\",\"data\":{\"statuses\":"
        "[\"success\"]}}}";

    hl::trading::OrderResponseStatus st;
    ASSERT_TRUE(hl::trading::parseOrderStatusResponse(body, strlen(body), st));
    ASSERT_TRUE(st.success);
    ASSERT_TRUE(st.error.empty());
}

TEST_CASE(cr5_last_order_error_roundtrip) {
    hl::trading::clearLastOrderError();
    ASSERT_EQ(hl::trading::ORDER_ERR_NONE, hl::trading::getLastOrderErrorClass());
    ASSERT_STREQ("", hl::trading::getLastOrderErrorText());

    hl::trading::setLastOrderError("Post only order would have immediately matched, bbo was 7");
    ASSERT_EQ(hl::trading::ORDER_ERR_POST_ONLY, hl::trading::getLastOrderErrorClass());
    ASSERT_TRUE(strstr(hl::trading::getLastOrderErrorText(), "Post only") != nullptr);

    // A successful order must clear it, or the next reprice reads a stale class.
    hl::trading::clearLastOrderError();
    ASSERT_EQ(hl::trading::ORDER_ERR_NONE, hl::trading::getLastOrderErrorClass());
}

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
// MAIN
//=============================================================================

int main() {
    printf("\n=== OPM-790: ALO Enablement Regression Tests ===\n\n");

    printf("--- CR-1 (OPM-791): sticky ALO order type ---\n");
    RUN_TEST(cr1_auto_set_ordertype_used_to_clobber_alo);
    RUN_TEST(cr1_sticky_alo_survives_auto_set_ordertype_0);
    RUN_TEST(cr1_sticky_alo_survives_auto_set_ordertype_2_gtc);
    RUN_TEST(cr1_sticky_survives_stop_flag_variant);
    RUN_TEST(cr1_ioc_releases_the_sticky_override);
    RUN_TEST(cr1_gtc_releases_the_sticky_override);
    RUN_TEST(cr1_set_ordertype_4_is_also_sticky);

    printf("\n--- CR-4 (OPM-794): market TIF + casing ---\n");
    RUN_TEST(cr4_market_order_used_to_inherit_alo);
    RUN_TEST(cr4_market_order_forces_ioc_even_under_global_alo);
    RUN_TEST(cr4_explicit_types_still_reach_the_wire);
    RUN_TEST(cr4_canonical_tif_accepts_any_casing);
    RUN_TEST(cr4_canonical_tif_output_is_byte_identical);
    RUN_TEST(cr4_canonical_tif_rejects_invalid);

    printf("\n--- CR-5 (OPM-795): exchange error surfacing ---\n");
    RUN_TEST(cr5_classify_post_only_reject);
    RUN_TEST(cr5_classify_margin_reject);
    RUN_TEST(cr5_classify_other_rejects);
    RUN_TEST(cr5_classify_none_for_empty);
    RUN_TEST(cr5_post_only_and_margin_are_distinguishable);
    RUN_TEST(cr5_parse_alo_reject_from_canned_response);
    RUN_TEST(cr5_parse_margin_reject_from_canned_response);
    RUN_TEST(cr5_parse_resting_response);
    RUN_TEST(cr5_parse_filled_response);
    RUN_TEST(cr5_parse_top_level_error);
    RUN_TEST(cr5_parse_batchmodify_success);
    RUN_TEST(cr5_last_order_error_roundtrip);

    printf("\n--- CR-6 (OPM-796): side-aware rounding ---\n");
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

    return printTestSummary();
}
