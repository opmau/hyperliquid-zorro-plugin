//=============================================================================
// test_market_service_ws.cpp - Market service WebSocket cache tests [OPM-175]
//=============================================================================
// LAYER: Test | TESTS: hl_market_service WS cache interaction patterns
//
// Tests the service-layer logic that reads from PriceCache:
//   1. hasRealtimePrice freshness checks
//   2. getPrice WS cache read (fresh, stale, missing)
//   3. Stale-data fallback logic (under PRICE_STALE_MS cap)
//
// Strategy: Extract the WS-reading patterns from hl_market_service.cpp
// into test-local functions, then test with a real PriceCache instance.
// This avoids linking the full service (which has HTTP/WS dependencies).
//=============================================================================

#include "../test_framework.h"
#include "../../src/foundation/hl_types.h"
#include "../../src/foundation/hl_globals.h"
#include "../../src/transport/ws_price_cache.h"
#include <cstring>
#include <cmath>

using namespace hl::test;
using namespace hl;

//=============================================================================
// EXTRACTED LOGIC (from hl_market_service.cpp)
// These replicate the service's WS-reading patterns for testability.
//=============================================================================

namespace MktWs {

/// Extracted from market_service.cpp:hasRealtimePrice() lines 414-419
bool hasRealtimePrice(ws::PriceCache& cache, bool wsEnabled, const char* coin, uint32_t maxAgeMs) {
    if (!coin || !wsEnabled) return false;
    return cache.isFresh(std::string(coin), maxAgeMs);
}

/// Return value for getPrice WS cache read
struct PriceResult {
    double bid = 0.0;
    double ask = 0.0;
    double mid = 0.0;
    uint32_t timestamp = 0;
    bool fromCache = false;     // True if returned from WS cache (fresh)
    bool staleAvailable = false; // True if stale data exists (< PRICE_STALE_MS)
    double staleBid = 0.0;
    double staleAsk = 0.0;
};

/// Extracted from market_service.cpp:getPrice() lines 105-194
/// Only the WS cache-reading portion — no HTTP fallback, no WS wait loop.
/// Returns what the cache has, plus stale-data availability info.
PriceResult getPriceFromCache(ws::PriceCache& cache, bool wsEnabled,
                              const char* coin, uint32_t maxAgeMs, uint32_t staleCapMs) {
    PriceResult result;
    if (!coin || !*coin) return result;

    if (!wsEnabled) return result;

    double bid = cache.getBid(std::string(coin));
    double ask = cache.getAsk(std::string(coin));
    DWORD age = cache.getAge(std::string(coin));

    // Fresh data — return immediately
    if (bid > 0.0 && ask > 0.0 && age < maxAgeMs) {
        result.bid = bid;
        result.ask = ask;
        result.mid = (bid + ask) / 2.0;
        result.timestamp = GetTickCount();
        result.fromCache = true;
        return result;
    }

    // Check for stale-but-usable data (under hard staleness cap)
    if (bid > 0.0 && ask > 0.0 && age < staleCapMs) {
        result.staleAvailable = true;
        result.staleBid = bid;
        result.staleAsk = ask;
    }

    return result;
}

/// Extracted from market_service.cpp lines 203-211
/// When HTTP cooldown blocks a refresh, return stale WS data rather than empty.
/// This prevents Error 053 (BrokerAsset returning 0 disables asset for session).
PriceData useStaleFallback(double staleBid, double staleAsk) {
    PriceData result;
    if (staleBid > 0.0 && staleAsk > 0.0) {
        result.bid = staleBid;
        result.ask = staleAsk;
        result.mid = (staleBid + staleAsk) / 2.0;
        result.timestamp = GetTickCount();
    }
    return result;
}

} // namespace MktWs

//=============================================================================
// TEST CASES: hasRealtimePrice
//=============================================================================

TEST_CASE(ws_hasprice_fresh_data) {
    ws::PriceCache cache;
    cache.setBidAsk("BTC", 65000.0, 65001.0);

    ASSERT_TRUE(MktWs::hasRealtimePrice(cache, true, "BTC", 5000));
}

TEST_CASE(ws_hasprice_stale_data) {
    ws::PriceCache cache;
    cache.setBidAsk("BTC", 65000.0, 65001.0);

    // maxAge=0 means any age is stale
    ASSERT_FALSE(MktWs::hasRealtimePrice(cache, true, "BTC", 0));
}

TEST_CASE(ws_hasprice_no_data) {
    ws::PriceCache cache;

    ASSERT_FALSE(MktWs::hasRealtimePrice(cache, true, "BTC", 5000));
}

TEST_CASE(ws_hasprice_ws_disabled) {
    ws::PriceCache cache;
    cache.setBidAsk("BTC", 65000.0, 65001.0);

    ASSERT_FALSE(MktWs::hasRealtimePrice(cache, false, "BTC", 5000));
}

TEST_CASE(ws_hasprice_null_coin) {
    ws::PriceCache cache;

    ASSERT_FALSE(MktWs::hasRealtimePrice(cache, true, nullptr, 5000));
}

TEST_CASE(ws_hasprice_wrong_coin) {
    ws::PriceCache cache;
    cache.setBidAsk("BTC", 65000.0, 65001.0);

    // ETH not in cache
    ASSERT_FALSE(MktWs::hasRealtimePrice(cache, true, "ETH", 5000));
}

TEST_CASE(ws_hasprice_multiple_coins) {
    ws::PriceCache cache;
    cache.setBidAsk("BTC", 65000.0, 65001.0);
    cache.setBidAsk("ETH", 3500.0, 3501.0);
    cache.setBidAsk("SOL", 150.0, 150.05);

    ASSERT_TRUE(MktWs::hasRealtimePrice(cache, true, "BTC", 5000));
    ASSERT_TRUE(MktWs::hasRealtimePrice(cache, true, "ETH", 5000));
    ASSERT_TRUE(MktWs::hasRealtimePrice(cache, true, "SOL", 5000));
    ASSERT_FALSE(MktWs::hasRealtimePrice(cache, true, "DOGE", 5000));
}

//=============================================================================
// TEST CASES: getPrice WS cache read
//=============================================================================

TEST_CASE(ws_getprice_fresh_bidask) {
    ws::PriceCache cache;
    cache.setBidAsk("BTC", 65000.0, 65002.0);

    MktWs::PriceResult pr = MktWs::getPriceFromCache(cache, true, "BTC", 1500, 5000);
    ASSERT_TRUE(pr.fromCache);
    ASSERT_FLOAT_EQ_TOL(pr.bid, 65000.0, 0.01);
    ASSERT_FLOAT_EQ_TOL(pr.ask, 65002.0, 0.01);
    ASSERT_FLOAT_EQ_TOL(pr.mid, 65001.0, 0.01);
    ASSERT_GT(pr.timestamp, (uint32_t)0);
}

TEST_CASE(ws_getprice_stale_under_cap) {
    ws::PriceCache cache;
    cache.setBidAsk("BTC", 65000.0, 65002.0);

    // maxAge=0 makes data stale, but stale cap=5000 allows fallback
    MktWs::PriceResult pr = MktWs::getPriceFromCache(cache, true, "BTC", 0, 5000);
    ASSERT_FALSE(pr.fromCache);
    ASSERT_TRUE(pr.staleAvailable);
    ASSERT_FLOAT_EQ_TOL(pr.staleBid, 65000.0, 0.01);
    ASSERT_FLOAT_EQ_TOL(pr.staleAsk, 65002.0, 0.01);
}

TEST_CASE(ws_getprice_stale_beyond_cap) {
    ws::PriceCache cache;
    cache.setBidAsk("BTC", 65000.0, 65002.0);

    // Both maxAge and staleCap are 0 — data is stale AND beyond cap
    MktWs::PriceResult pr = MktWs::getPriceFromCache(cache, true, "BTC", 0, 0);
    ASSERT_FALSE(pr.fromCache);
    ASSERT_FALSE(pr.staleAvailable);
}

TEST_CASE(ws_getprice_empty_cache) {
    ws::PriceCache cache;

    MktWs::PriceResult pr = MktWs::getPriceFromCache(cache, true, "BTC", 1500, 5000);
    ASSERT_FALSE(pr.fromCache);
    ASSERT_FALSE(pr.staleAvailable);
    ASSERT_FLOAT_EQ(pr.bid, 0.0);
    ASSERT_FLOAT_EQ(pr.ask, 0.0);
}

TEST_CASE(ws_getprice_ws_disabled) {
    ws::PriceCache cache;
    cache.setBidAsk("BTC", 65000.0, 65002.0);

    MktWs::PriceResult pr = MktWs::getPriceFromCache(cache, false, "BTC", 1500, 5000);
    ASSERT_FALSE(pr.fromCache);
    ASSERT_FLOAT_EQ(pr.bid, 0.0);
}

TEST_CASE(ws_getprice_null_coin) {
    ws::PriceCache cache;

    MktWs::PriceResult pr = MktWs::getPriceFromCache(cache, true, nullptr, 1500, 5000);
    ASSERT_FALSE(pr.fromCache);
}

TEST_CASE(ws_getprice_empty_string) {
    ws::PriceCache cache;

    MktWs::PriceResult pr = MktWs::getPriceFromCache(cache, true, "", 1500, 5000);
    ASSERT_FALSE(pr.fromCache);
}

//=============================================================================
// TEST CASES: Stale fallback logic
//=============================================================================

TEST_CASE(ws_stale_fallback_returns_data) {
    PriceData pd = MktWs::useStaleFallback(65000.0, 65002.0);
    ASSERT_FLOAT_EQ_TOL(pd.bid, 65000.0, 0.01);
    ASSERT_FLOAT_EQ_TOL(pd.ask, 65002.0, 0.01);
    ASSERT_FLOAT_EQ_TOL(pd.mid, 65001.0, 0.01);
    ASSERT_GT(pd.timestamp, (DWORD)0);
}

TEST_CASE(ws_stale_fallback_zero_returns_empty) {
    PriceData pd = MktWs::useStaleFallback(0.0, 0.0);
    ASSERT_FLOAT_EQ(pd.bid, 0.0);
    ASSERT_FLOAT_EQ(pd.ask, 0.0);
    ASSERT_EQ(pd.timestamp, (DWORD)0);
}

TEST_CASE(ws_stale_fallback_partial_zero_returns_empty) {
    // If either bid or ask is 0, return empty
    PriceData pd = MktWs::useStaleFallback(65000.0, 0.0);
    ASSERT_FLOAT_EQ(pd.bid, 0.0);

    pd = MktWs::useStaleFallback(0.0, 65002.0);
    ASSERT_FLOAT_EQ(pd.bid, 0.0);
}

//=============================================================================
// TEST CASES: Price updates and freshness transitions
//=============================================================================

TEST_CASE(ws_price_update_refreshes_age) {
    ws::PriceCache cache;
    cache.setBidAsk("BTC", 60000.0, 60001.0);

    // Immediately after set, should be fresh
    ASSERT_TRUE(MktWs::hasRealtimePrice(cache, true, "BTC", 5000));

    MktWs::PriceResult pr = MktWs::getPriceFromCache(cache, true, "BTC", 1500, 5000);
    ASSERT_TRUE(pr.fromCache);

    // Update price
    cache.setBidAsk("BTC", 61000.0, 61001.0);
    pr = MktWs::getPriceFromCache(cache, true, "BTC", 1500, 5000);
    ASSERT_TRUE(pr.fromCache);
    ASSERT_FLOAT_EQ_TOL(pr.bid, 61000.0, 0.01);
    ASSERT_FLOAT_EQ_TOL(pr.ask, 61001.0, 0.01);
}

TEST_CASE(ws_price_after_clear) {
    ws::PriceCache cache;
    cache.setBidAsk("BTC", 65000.0, 65001.0);

    // Verify data exists
    ASSERT_TRUE(MktWs::hasRealtimePrice(cache, true, "BTC", 5000));

    // Clear all data
    cache.clear();

    // Now should be empty
    ASSERT_FALSE(MktWs::hasRealtimePrice(cache, true, "BTC", 5000));
    MktWs::PriceResult pr = MktWs::getPriceFromCache(cache, true, "BTC", 1500, 5000);
    ASSERT_FALSE(pr.fromCache);
    ASSERT_FLOAT_EQ(pr.bid, 0.0);
}

//=============================================================================
// MAIN
//=============================================================================

int main() {
    printf("=== OPM-175: Market Service WS Cache Tests ===\n\n");

    // hasRealtimePrice
    RUN_TEST(ws_hasprice_fresh_data);
    RUN_TEST(ws_hasprice_stale_data);
    RUN_TEST(ws_hasprice_no_data);
    RUN_TEST(ws_hasprice_ws_disabled);
    RUN_TEST(ws_hasprice_null_coin);
    RUN_TEST(ws_hasprice_wrong_coin);
    RUN_TEST(ws_hasprice_multiple_coins);

    // getPrice WS cache read
    RUN_TEST(ws_getprice_fresh_bidask);
    RUN_TEST(ws_getprice_stale_under_cap);
    RUN_TEST(ws_getprice_stale_beyond_cap);
    RUN_TEST(ws_getprice_empty_cache);
    RUN_TEST(ws_getprice_ws_disabled);
    RUN_TEST(ws_getprice_null_coin);
    RUN_TEST(ws_getprice_empty_string);

    // Stale fallback
    RUN_TEST(ws_stale_fallback_returns_data);
    RUN_TEST(ws_stale_fallback_zero_returns_empty);
    RUN_TEST(ws_stale_fallback_partial_zero_returns_empty);

    // Price updates and transitions
    RUN_TEST(ws_price_update_refreshes_age);
    RUN_TEST(ws_price_after_clear);

    return printTestSummary();
}
