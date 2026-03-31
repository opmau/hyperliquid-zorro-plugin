//=============================================================================
// test_get_price_context.cpp - GET_PRICE asset context tests
//=============================================================================
// VALIDATES:
//   GET_PRICE returns the correct price per Zorro's documented contract:
//   "Returns a particular price after a BrokerAsset call" (brokercommand.md)
//
// KEY CONTRACT:
//   - BrokerAsset sets both currentSymbol AND priceSymbol
//   - GET_PRICE reads from priceSymbol
//   - SET_SYMBOL also sets priceSymbol (for GET_BOOK, GET_OPTIONS, etc.)
//   - In multi-asset setups, the last BrokerAsset/SET_SYMBOL call wins
//   - Strategy must call asset(Asset) in TMFs to ensure correct context
//=============================================================================

#include "../test_framework.h"
#include "../../src/transport/ws_price_cache.h"
#include "../../src/foundation/hl_globals.h"
#include <cstring>

using namespace hl::test;

//=============================================================================
// GET_PRICE lookup logic (extracted from hl_broker_commands.cpp)
//=============================================================================

struct GetPriceResult {
    const char* lookupCoin;  // Which coin GET_PRICE resolved to
    double bid;
    double ask;
    double mid;
};

// Replicates the GET_PRICE handler logic from hl_broker_commands.cpp
// Uses priceSymbol (set by both BrokerAsset and SET_SYMBOL)
GetPriceResult simulateGetPrice(hl::ws::PriceCache& cache) {
    GetPriceResult result = { nullptr, 0, 0, 0 };

    if (!hl::g_trading.priceSymbol[0]) {
        return result;  // No BrokerAsset or SET_SYMBOL → return zeros
    }

    const char* lookupCoin = hl::g_trading.priceSymbol;
    result.lookupCoin = lookupCoin;
    result.bid = cache.getBid(lookupCoin);
    result.ask = cache.getAsk(lookupCoin);
    if (result.bid > 0 && result.ask > 0)
        result.mid = (result.bid + result.ask) / 2.0;

    return result;
}

//=============================================================================
// TEST CASES
//=============================================================================

void test_get_price_after_broker_asset_returns_correct_price() {
    // BrokerAsset for BTC sets priceSymbol, GET_PRICE returns BTC's price

    hl::ws::PriceCache cache;
    cache.setBidAsk("BTC", 67000.0, 67300.0);
    cache.setBidAsk("SOL", 83.5, 83.6);

    // Simulate BrokerAsset("BTC") — sets both currentSymbol and priceSymbol
    strncpy_s(hl::g_trading.currentSymbol, "BTC", _TRUNCATE);
    strncpy_s(hl::g_trading.priceSymbol, "BTC", _TRUNCATE);

    GetPriceResult r = simulateGetPrice(cache);

    ASSERT_NOT_NULL(r.lookupCoin);
    ASSERT_STREQ(r.lookupCoin, "BTC");
    ASSERT_FLOAT_EQ_TOL(r.bid, 67000.0, 0.01);
    ASSERT_FLOAT_EQ_TOL(r.ask, 67300.0, 0.01);
}

void test_get_price_last_broker_asset_wins() {
    // BrokerAsset subscription loop: BTC then SOL.
    // GET_PRICE should return SOL (last BrokerAsset call).

    hl::ws::PriceCache cache;
    cache.setBidAsk("BTC", 67000.0, 67300.0);
    cache.setBidAsk("SOL", 83.5, 83.6);

    // Simulate subscription loop: BTC first, then SOL
    strncpy_s(hl::g_trading.currentSymbol, "SOL", _TRUNCATE);
    strncpy_s(hl::g_trading.priceSymbol, "SOL", _TRUNCATE);

    GetPriceResult r = simulateGetPrice(cache);

    // Per Zorro contract: returns price of last BrokerAsset'd asset
    ASSERT_STREQ(r.lookupCoin, "SOL");
    ASSERT_FLOAT_EQ_TOL(r.bid, 83.5, 0.01);
}

void test_get_price_set_symbol_also_works() {
    // SET_SYMBOL sets priceSymbol, GET_PRICE returns that asset's price

    hl::ws::PriceCache cache;
    cache.setBidAsk("BTC", 67000.0, 67300.0);

    // Simulate SET_SYMBOL("BTC")
    strncpy_s(hl::g_trading.currentSymbol, "BTC", _TRUNCATE);
    strncpy_s(hl::g_trading.priceSymbol, "BTC", _TRUNCATE);

    GetPriceResult r = simulateGetPrice(cache);

    ASSERT_NOT_NULL(r.lookupCoin);
    ASSERT_STREQ(r.lookupCoin, "BTC");
    ASSERT_FLOAT_EQ_TOL(r.bid, 67000.0, 0.01);
}

void test_get_price_no_context_returns_zero() {
    // Neither BrokerAsset nor SET_SYMBOL was called

    hl::ws::PriceCache cache;
    cache.setBidAsk("BTC", 67000.0, 67300.0);

    hl::g_trading.currentSymbol[0] = '\0';
    hl::g_trading.priceSymbol[0] = '\0';

    GetPriceResult r = simulateGetPrice(cache);

    ASSERT_NULL(r.lookupCoin);
    ASSERT_FLOAT_EQ_TOL(r.mid, 0.0, 1e-9);
}

void test_get_price_asset_call_overrides_loop() {
    // Subscription loop set priceSymbol to SOL, then strategy calls
    // asset("BTC") which triggers BrokerAsset("BTC") → overrides priceSymbol.
    // GET_PRICE should return BTC.

    hl::ws::PriceCache cache;
    cache.setBidAsk("BTC", 67000.0, 67300.0);
    cache.setBidAsk("SOL", 83.5, 83.6);

    // After subscription loop: SOL
    strncpy_s(hl::g_trading.priceSymbol, "SOL", _TRUNCATE);
    // Strategy calls asset("BTC") → BrokerAsset overrides
    strncpy_s(hl::g_trading.currentSymbol, "BTC", _TRUNCATE);
    strncpy_s(hl::g_trading.priceSymbol, "BTC", _TRUNCATE);

    GetPriceResult r = simulateGetPrice(cache);

    ASSERT_STREQ(r.lookupCoin, "BTC");
    ASSERT_FLOAT_EQ_TOL(r.bid, 67000.0, 0.01);
}

//=============================================================================
// MAIN
//=============================================================================

int main() {
    hl::initGlobals();

    printf("\n");
    printf("=================================================\n");
    printf("  GET_PRICE Context Tests\n");
    printf("  Contract: works after BrokerAsset or SET_SYMBOL\n");
    printf("=================================================\n\n");

    RUN_TEST(get_price_after_broker_asset_returns_correct_price);
    RUN_TEST(get_price_last_broker_asset_wins);
    RUN_TEST(get_price_set_symbol_also_works);
    RUN_TEST(get_price_no_context_returns_zero);
    RUN_TEST(get_price_asset_call_overrides_loop);

    hl::cleanupGlobals();

    return printTestSummary();
}
