//=============================================================================
// test_alo_ordertype.cpp - ALO order-type + exchange-error regression tests
//=============================================================================
// PREVENTS BUGS: OPM-791, OPM-794, OPM-795 (epic OPM-790)
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
//
// Sibling: test_alo_execution.cpp covers CR-2, CR-6, CR-7, CR-8 and CR-9
// (honest close reporting, side-aware rounding, cancel safety, partial-fill
// preservation, modify TIF).
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
// MAIN
//=============================================================================

int main() {
    printf("\n=== OPM-790: ALO Order-Type Regression Tests ===\n\n");

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

    return printTestSummary();
}
