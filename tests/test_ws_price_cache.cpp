//=============================================================================
// test_ws_price_cache.cpp - Unit tests for PriceCache
//=============================================================================

#include "ws_price_cache.h"
#include <cassert>
#include <cstdio>

using namespace hl::ws;

void test_price_operations() {
    printf("  test_price_operations...");

    PriceCache cache;

    // Set and get bid/ask
    cache.setBidAsk("BTC", 50000.0, 50001.0);
    assert(cache.getBid("BTC") == 50000.0);
    assert(cache.getAsk("BTC") == 50001.0);
    assert(cache.getPrice("BTC") == 50000.5);  // Mid

    // Check age is reasonable
    DWORD age = cache.getAge("BTC");
    assert(age < 1000);  // Should be very recent

    // Non-existent coin returns 0
    assert(cache.getBid("NOTEXIST") == 0.0);
    assert(cache.getAsk("NOTEXIST") == 0.0);

    printf(" PASSED\n");
}

void test_account_data() {
    printf("  test_account_data...");

    PriceCache cache;

    cache.setAccountData(10000.0, 2000.0, 8000.0, 5000.0);
    AccountData data = cache.getAccountData();

    assert(data.accountValue == 10000.0);
    assert(data.totalMarginUsed == 2000.0);
    assert(data.withdrawable == 8000.0);
    assert(data.totalNtlPos == 5000.0);

    printf(" PASSED\n");
}

void test_positions() {
    printf("  test_positions...");

    PriceCache cache;

    PositionData pos;
    pos.coin = "ETH";
    pos.size = 1.5;
    pos.entryPx = 3000.0;
    pos.unrealizedPnl = 100.0;

    cache.setPosition(pos);

    PositionData retrieved = cache.getPosition("ETH");
    assert(retrieved.coin == "ETH");
    assert(retrieved.size == 1.5);
    assert(retrieved.entryPx == 3000.0);

    // Get all positions
    auto all = cache.getAllPositions();
    assert(all.size() == 1);

    // Clear
    cache.clearPositions();
    all = cache.getAllPositions();
    assert(all.size() == 0);

    printf(" PASSED\n");
}

void test_open_orders() {
    printf("  test_open_orders...");

    PriceCache cache;

    OpenOrderData order;
    order.oid = "order123";
    order.coin = "BTC";
    order.isBuy = true;
    order.limitPx = 50000.0;
    order.sz = 0.1;

    cache.setOpenOrder(order);

    auto retrieved = cache.getOpenOrder("order123");
    assert(retrieved.oid == "order123");
    assert(retrieved.isBuy == true);

    // Get orders for coin
    auto forBtc = cache.getOpenOrdersForCoin("BTC");
    assert(forBtc.size() == 1);

    // Remove order
    cache.removeOpenOrder("order123");
    retrieved = cache.getOpenOrder("order123");
    assert(retrieved.oid.empty());

    printf(" PASSED\n");
}

void test_fills() {
    printf("  test_fills...");

    PriceCache cache;

    FillData fill;
    fill.oid = "order123";
    fill.coin = "BTC";
    fill.px = 50000.0;
    fill.sz = 0.05;
    fill.isBuy = true;

    cache.addFill(fill);

    auto recent = cache.getRecentFills(10);
    assert(recent.size() == 1);
    assert(recent[0].px == 50000.0);

    auto forOrder = cache.getFillsForOrder("order123");
    assert(forOrder.size() == 1);

    printf(" PASSED\n");
}

void test_clear_all() {
    printf("  test_clear_all...");

    PriceCache cache;

    // Add some data
    cache.setBidAsk("BTC", 50000.0, 50001.0);
    cache.setAccountData(10000.0, 2000.0, 8000.0, 5000.0);

    PositionData pos;
    pos.coin = "ETH";
    pos.size = 1.0;
    cache.setPosition(pos);

    // Clear all
    cache.clear();

    // Verify everything is cleared
    assert(cache.getBid("BTC") == 0.0);
    assert(cache.getAccountData().accountValue == 0.0);
    assert(cache.getAllPositions().size() == 0);

    printf(" PASSED\n");
}

int main() {
    printf("=== ws_price_cache tests ===\n");

    test_price_operations();
    test_account_data();
    test_positions();
    test_open_orders();
    test_fills();
    test_clear_all();

    printf("\nAll ws_price_cache tests PASSED!\n");
    return 0;
}
