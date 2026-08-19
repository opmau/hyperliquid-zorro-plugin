//=============================================================================
// test_order_oid.cpp - Exchange order ID for a Zorro trade ID [OPM-1085]
//=============================================================================
// Part of Hyperliquid Plugin for Zorro
//
// LAYER: Test
// DEPENDENCIES: hl_utils.h, hl_globals.h, test_framework.h
// THREAD SAFETY: Not thread-safe (single-threaded test execution)
//=============================================================================
//
// HL_GET_ORDER_OID (50045) answers "which exchange order is this trade?" for a
// Zorro trade ID. A strategy uses the answer as a join key between its own
// records and the fills the exchange reports back. Two ways of getting that
// wrong are worse than returning nothing:
//
//   1. Returning a number parsed out of a synthetic ID. The trade map also
//      holds "PENDING_<cloid>", "RESUMED_<n>", "IMPORTED_<n>" and "DRY_RUN",
//      none of which name an order on the exchange. _atoi64 turns them into 0
//      or into the trailing digits without complaint [OPM-797].
//
//   2. Returning an ID that a double cannot carry exactly. The command's
//      return type is a scalar, and a silently rounded key matches the wrong
//      order, or no order, with no way for the caller to tell.
//
// Both must report "no ID" (0) instead, which is also what an untracked trade
// returns and what an older plugin returns for an unknown command code - so a
// caller written against this contract behaves sanely everywhere.
//
// The command itself lives in the API layer and needs the Zorro SDK headers to
// compile, so this exercises the two pieces it is built from - the real
// conversion (hl::utils::exchangeOrderIdToDouble) and the real trade-map
// lookup (hl::TradingState::getOrder) - through a local mirror of its four
// lines of glue.
//=============================================================================

#include "../test_framework.h"
#include "hl_utils.h"
#include "hl_globals.h"

using namespace hl::test;

//-----------------------------------------------------------------------------
// Mirror of the HL_GET_ORDER_OID case in hl_broker_commands_hl.cpp.
//-----------------------------------------------------------------------------
static double getOrderOid(int tradeId) {
    hl::OrderState state;
    if (tradeId <= 0 || !hl::g_trading.getOrder(tradeId, state)) return 0;
    return hl::utils::exchangeOrderIdToDouble(state.orderId);
}

static void track(int tradeId, const char* orderId) {
    hl::OrderState state;
    strncpy_s(state.orderId, orderId, _TRUNCATE);
    strncpy_s(state.coin, "BTC", _TRUNCATE);
    state.zorroTradeId = tradeId;
    hl::g_trading.setOrder(tradeId, state);
}

//-----------------------------------------------------------------------------
// TEST 1: An exchange order ID comes back unchanged.
//
// Order IDs are compared for equality against the exchange's own records, so
// the value must survive the round trip through a double bit for bit - not
// merely land close to it.
//-----------------------------------------------------------------------------
void test_exchange_id_round_trips_exactly() {
    ASSERT_FLOAT_EQ(hl::utils::exchangeOrderIdToDouble("1"), 1.0);
    ASSERT_FLOAT_EQ(hl::utils::exchangeOrderIdToDouble("58231947265"), 58231947265.0);
    ASSERT_FLOAT_EQ(hl::utils::exchangeOrderIdToDouble("9007199254740992"), 9007199254740992.0);  // 2^53
}

//-----------------------------------------------------------------------------
// TEST 2: A synthetic ID reports no ID, never a parsed prefix.
//
// Each of these is a real form the trade map holds. "RESUMED_7" must not come
// back as 7: some other order on the exchange has that ID.
//-----------------------------------------------------------------------------
void test_synthetic_ids_report_no_id() {
    ASSERT_FLOAT_EQ(hl::utils::exchangeOrderIdToDouble("PENDING_0x1234"), 0.0);
    ASSERT_FLOAT_EQ(hl::utils::exchangeOrderIdToDouble("RESUMED_7"), 0.0);
    ASSERT_FLOAT_EQ(hl::utils::exchangeOrderIdToDouble("IMPORTED_7"), 0.0);
    ASSERT_FLOAT_EQ(hl::utils::exchangeOrderIdToDouble("DRY_RUN"), 0.0);
    ASSERT_FLOAT_EQ(hl::utils::exchangeOrderIdToDouble(""), 0.0);
    ASSERT_FLOAT_EQ(hl::utils::exchangeOrderIdToDouble(nullptr), 0.0);
    ASSERT_FLOAT_EQ(hl::utils::exchangeOrderIdToDouble("0"), 0.0);
}

//-----------------------------------------------------------------------------
// TEST 3: An ID a double cannot carry exactly reports no ID.
//
// 2^53 + 1 is the first integer a double cannot represent; it rounds to 2^53
// and would silently name a different order. An over-long digit string
// saturates _atoi64 at _I64_MAX, which the same bound catches.
//-----------------------------------------------------------------------------
void test_inexact_ids_report_no_id() {
    ASSERT_FLOAT_EQ(hl::utils::exchangeOrderIdToDouble("9007199254740993"), 0.0);   // 2^53 + 1
    ASSERT_FLOAT_EQ(hl::utils::exchangeOrderIdToDouble("18446744073709551615"), 0.0);  // > int64
}

//-----------------------------------------------------------------------------
// TEST 4: A tracked trade yields its order's ID.
//-----------------------------------------------------------------------------
void test_tracked_trade_yields_its_order_id() {
    track(2, "58231947265");
    ASSERT_FLOAT_EQ(getOrderOid(2), 58231947265.0);
}

//-----------------------------------------------------------------------------
// TEST 5: An untracked trade ID reports no ID.
//
// Includes the values Zorro itself can supply: 0 before a trade is opened, and
// -1 for a trade identified by a UUID string rather than a number. Neither may
// be passed through to the caller as if it were an order ID.
//-----------------------------------------------------------------------------
void test_untracked_trade_reports_no_id() {
    ASSERT_FLOAT_EQ(getOrderOid(9999), 0.0);
    ASSERT_FLOAT_EQ(getOrderOid(0), 0.0);
    ASSERT_FLOAT_EQ(getOrderOid(-1), 0.0);
}

//-----------------------------------------------------------------------------
// TEST 6: A resumed or imported position reports no ID.
//
// These are tracked trades - the lookup succeeds - so only the conversion
// stands between the caller and a number that names somebody else's order.
//-----------------------------------------------------------------------------
void test_resumed_and_imported_trades_report_no_id() {
    track(3, "RESUMED_7");
    track(4, "IMPORTED_7");
    track(5, "PENDING_0xabc");

    ASSERT_FLOAT_EQ(getOrderOid(3), 0.0);
    ASSERT_FLOAT_EQ(getOrderOid(4), 0.0);
    ASSERT_FLOAT_EQ(getOrderOid(5), 0.0);
}

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------
int main() {
    printf("\n===========================================\n");
    printf(" Exchange order ID for a trade ID [OPM-1085]\n");
    printf("===========================================\n\n");

    hl::g_trading.init();

    RUN_TEST(exchange_id_round_trips_exactly);
    RUN_TEST(synthetic_ids_report_no_id);
    RUN_TEST(inexact_ids_report_no_id);
    RUN_TEST(tracked_trade_yields_its_order_id);
    RUN_TEST(untracked_trade_reports_no_id);
    RUN_TEST(resumed_and_imported_trades_report_no_id);

    int result = printTestSummary();
    hl::g_trading.cleanup();
    return result;
}
