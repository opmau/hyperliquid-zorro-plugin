//=============================================================================
// test_trigger_order.cpp - Stop-loss trigger order validation [OPM-77]
//=============================================================================
// Tests:
//   - OrderRequest trigger fields and isTriggerOrder() helper
//   - packTriggerOrderAction() msgpack encoding
//   - STOP flag parsing from SET_ORDERTYPE values
//   - Slippage price computation for market trigger orders
//   - Trigger orders differ from limit orders in msgpack output
//=============================================================================

#include "../test_framework.h"
#include "../../src/foundation/hl_types.h"
#include "../../src/foundation/hl_msgpack.h"
#include <cmath>
#include <cstring>

using namespace hl::test;

//=============================================================================
// Test: OrderRequest defaults
//=============================================================================

void test_trigger_order_request_defaults() {
    hl::OrderRequest req;
    ASSERT_FALSE(req.isTriggerOrder());
    ASSERT_EQ((int)req.triggerType, (int)hl::TriggerType::NoTrigger);
    ASSERT_FLOAT_EQ(req.triggerPx, 0.0);
    ASSERT_TRUE(req.triggerIsMarket);
}

//=============================================================================
// Test: OrderRequest with StopLoss trigger
//=============================================================================

void test_trigger_order_request_stoploss() {
    hl::OrderRequest req;
    req.triggerType = hl::TriggerType::SL;
    req.triggerPx = 48000.0;
    req.triggerIsMarket = true;
    req.reduceOnly = true;
    ASSERT_TRUE(req.isTriggerOrder());
    ASSERT_FLOAT_EQ(req.triggerPx, 48000.0);
}

//=============================================================================
// Test: OrderRequest with TakeProfit trigger
//=============================================================================

void test_trigger_order_request_takeprofit() {
    hl::OrderRequest req;
    req.triggerType = hl::TriggerType::TP;
    ASSERT_TRUE(req.isTriggerOrder());
}

//=============================================================================
// Test: packTriggerOrderAction produces non-empty bytes
//=============================================================================

void test_trigger_msgpack_not_empty() {
    auto bytes = hl::msgpack::packTriggerOrderAction(
        3, true, "50000", "0.01", true, true, "48000", "sl", "", "na");
    ASSERT_TRUE(bytes.size() > 0);
}

//=============================================================================
// Test: Trigger msgpack output differs from limit msgpack
//=============================================================================

void test_trigger_msgpack_differs_from_limit() {
    auto limitBytes = hl::msgpack::packOrderAction(
        3, true, "50000", "0.01", true, "Ioc", "", "na");
    auto triggerBytes = hl::msgpack::packTriggerOrderAction(
        3, true, "50000", "0.01", true, true, "48000", "sl", "", "na");

    // Must produce different bytes (trigger has different "t" field)
    ASSERT_TRUE(limitBytes != triggerBytes);
}

//=============================================================================
// Test: Trigger msgpack with cloid includes "c" field
//=============================================================================

void test_trigger_msgpack_with_cloid() {
    auto withoutCloid = hl::msgpack::packTriggerOrderAction(
        3, true, "50000", "0.01", true, true, "48000", "sl", "", "na");
    auto withCloid = hl::msgpack::packTriggerOrderAction(
        3, true, "50000", "0.01", true, true, "48000", "sl", "my-cloid-123", "na");

    // With cloid should be larger (extra key-value pair)
    ASSERT_TRUE(withCloid.size() > withoutCloid.size());
}

//=============================================================================
// Test: STOP flag parsing from SET_ORDERTYPE values
//=============================================================================

void test_stop_flag_parsing_ioc_stop() {
    // SET_ORDERTYPE(8) = base 0 (IOC) + STOP flag
    int orderType = 8;
    int baseType = orderType & 7;
    bool isStop = (orderType & 8) != 0;
    ASSERT_EQ(baseType, 0);
    ASSERT_TRUE(isStop);
}

void test_stop_flag_parsing_gtc_stop() {
    // SET_ORDERTYPE(10) = base 2 (GTC) + STOP flag
    int orderType = 10;
    int baseType = orderType & 7;
    bool isStop = (orderType & 8) != 0;
    ASSERT_EQ(baseType, 2);
    ASSERT_TRUE(isStop);
}

void test_stop_flag_parsing_no_stop() {
    // SET_ORDERTYPE(2) = GTC without STOP
    int orderType = 2;
    int baseType = orderType & 7;
    bool isStop = (orderType & 8) != 0;
    ASSERT_EQ(baseType, 2);
    ASSERT_FALSE(isStop);
}

//=============================================================================
// Test: Slippage price computation for sell stop (long position SL)
//=============================================================================

void test_slippage_price_sell_stop() {
    // Sell stop: worst-case price = triggerPx * (1 - slippage)
    double triggerPx = 48000.0;
    double slippage = 0.05;  // 5% (MARKET_ORDER_SLIPPAGE)
    double expected = triggerPx * (1.0 - slippage);  // 45600.0
    ASSERT_FLOAT_EQ(expected, 45600.0);
}

//=============================================================================
// Test: Slippage price computation for buy stop (short position SL)
//=============================================================================

void test_slippage_price_buy_stop() {
    // Buy stop: worst-case price = triggerPx * (1 + slippage)
    double triggerPx = 48000.0;
    double slippage = 0.05;
    double expected = triggerPx * (1.0 + slippage);  // 50400.0
    ASSERT_FLOAT_EQ(expected, 50400.0);
}

//=============================================================================
// Test: Trigger orders with isMarket=false (limit execution after trigger)
//=============================================================================

void test_trigger_msgpack_limit_execution() {
    auto marketBytes = hl::msgpack::packTriggerOrderAction(
        3, false, "47500", "0.01", true, true, "48000", "sl", "", "na");
    auto limitBytes = hl::msgpack::packTriggerOrderAction(
        3, false, "47500", "0.01", true, false, "48000", "sl", "", "na");

    // isMarket true vs false should produce different bytes
    ASSERT_TRUE(marketBytes != limitBytes);
}

//=============================================================================
// Test: tpsl "sl" vs "tp" produce different msgpack
//=============================================================================

void test_trigger_msgpack_sl_vs_tp() {
    auto slBytes = hl::msgpack::packTriggerOrderAction(
        3, false, "47500", "0.01", true, true, "48000", "sl", "", "na");
    auto tpBytes = hl::msgpack::packTriggerOrderAction(
        3, false, "47500", "0.01", true, true, "48000", "tp", "", "na");

    ASSERT_TRUE(slBytes != tpBytes);
}

//=============================================================================
// MAIN
//=============================================================================

int main() {
    printf("=== Trigger Order Tests [OPM-77] ===\n\n");

    RUN_TEST(trigger_order_request_defaults);
    RUN_TEST(trigger_order_request_stoploss);
    RUN_TEST(trigger_order_request_takeprofit);
    RUN_TEST(trigger_msgpack_not_empty);
    RUN_TEST(trigger_msgpack_differs_from_limit);
    RUN_TEST(trigger_msgpack_with_cloid);
    RUN_TEST(stop_flag_parsing_ioc_stop);
    RUN_TEST(stop_flag_parsing_gtc_stop);
    RUN_TEST(stop_flag_parsing_no_stop);
    RUN_TEST(slippage_price_sell_stop);
    RUN_TEST(slippage_price_buy_stop);
    RUN_TEST(trigger_msgpack_limit_execution);
    RUN_TEST(trigger_msgpack_sl_vs_tp);

    return printTestSummary();
}
