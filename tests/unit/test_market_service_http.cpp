//=============================================================================
// test_market_service_http.cpp - Market service HTTP parsing tests [OPM-174]
//=============================================================================
// LAYER: Test | TESTS: hl_market_service (HTTP-dependent paths)
//
// Tests JSON parsing extracted from service functions that depend on HTTP:
//   1. l2Book parsing (getPrice HTTP fallback path)
//   2. candleSnapshot parsing (getCandles)
//   3. metaAndAssetCtxs parsing (getFundingRate)
//
// Uses canned JSON fixtures + yyjson for deterministic parsing tests.
//=============================================================================

#include "../test_framework.h"
#include "json_helpers.h"
#include <cstring>
#include <cmath>
#include <vector>
#include <cstdint>

using namespace hl::test;

//=============================================================================
// EXTRACTED PARSING: l2Book (from hl_market_service.cpp:232-252)
// getPrice() HTTP fallback parses l2Book JSON to extract bid/ask.
//=============================================================================

namespace MktHttp {

struct PriceResult {
    double bid = 0.0;
    double ask = 0.0;
    double mid = 0.0;
    bool valid = false;
    bool isNull = false;  // true if response was "null" (asset not found)
};

/// Parse l2Book HTTP response into bid/ask/mid.
/// Input format: {"levels":[[{"px":"50000","sz":"1.5"}],[{"px":"50001","sz":"0.8"}]]}
/// Returns valid=true if both bid and ask were extracted.
PriceResult parseL2Book(const char* json, size_t len) {
    PriceResult result;
    if (!json || len == 0) return result;

    // Check for "null" response (asset not found on exchange)
    if (len == 4 && strncmp(json, "null", 4) == 0) {
        result.isNull = true;
        return result;
    }

    yyjson_doc* doc = yyjson_read(json, len, 0);
    if (!doc) return result;

    yyjson_val* root = yyjson_doc_get_root(doc);
    yyjson_val* levels = hl::json::getArray(root, "levels");
    if (!levels) { yyjson_doc_free(doc); return result; }

    double httpBid = 0.0, httpAsk = 0.0;
    yyjson_val* bids = yyjson_arr_get(levels, 0);
    yyjson_val* asks = yyjson_arr_get(levels, 1);

    if (bids && yyjson_is_arr(bids)) {
        yyjson_val* topBid = yyjson_arr_get(bids, 0);
        if (topBid) httpBid = hl::json::getDouble(topBid, "px");
    }
    if (asks && yyjson_is_arr(asks)) {
        yyjson_val* topAsk = yyjson_arr_get(asks, 0);
        if (topAsk) httpAsk = hl::json::getDouble(topAsk, "px");
    }
    yyjson_doc_free(doc);

    if (httpBid > 0.0 && httpAsk > 0.0) {
        result.bid = httpBid;
        result.ask = httpAsk;
        result.mid = (httpBid + httpAsk) / 2.0;
        result.valid = true;
    } else if (httpBid > 0.0) {
        // Thin order book — only bid available
        result.bid = httpBid;
        result.ask = httpBid;
        result.mid = httpBid;
        result.valid = true;
    }

    return result;
}

//=============================================================================
// EXTRACTED PARSING: candleSnapshot (from hl_market_service.cpp:511-544)
// getCandles() parses candleSnapshot JSON to extract OHLCV data.
//=============================================================================

struct Candle {
    double open = 0.0;
    double high = 0.0;
    double low = 0.0;
    double close = 0.0;
    double volume = 0.0;
    int64_t timestamp = 0;  // Bar END time (ms)
};

/// Parse candleSnapshot HTTP response into candle vector.
/// Input: [{"t":1234567890000,"o":"100","h":"101","l":"99","c":"100.5","v":"1000"},...]
/// The "t" field is bar OPEN time; we convert to bar END time.
std::vector<Candle> parseCandleSnapshot(const char* json, size_t len,
                                         int barMinutes, int maxCandles = 5000) {
    std::vector<Candle> candles;
    if (!json || len == 0) return candles;

    yyjson_doc* doc = yyjson_read(json, len, 0);
    if (!doc) return candles;

    yyjson_val* root = yyjson_doc_get_root(doc);
    if (!yyjson_is_arr(root)) { yyjson_doc_free(doc); return candles; }

    candles.reserve(maxCandles);

    size_t idx, max;
    yyjson_val* item;
    yyjson_arr_foreach(root, idx, max, item) {
        if (candles.size() >= (size_t)maxCandles) break;

        int64_t tMs = hl::json::getInt64(item, "t");
        if (tMs == 0) continue;

        // Convert bar OPEN time to bar END time
        int64_t endTime = tMs + ((int64_t)barMinutes * 60 * 1000);

        Candle candle;
        candle.open   = hl::json::getDouble(item, "o");
        candle.high   = hl::json::getDouble(item, "h");
        candle.low    = hl::json::getDouble(item, "l");
        candle.close  = hl::json::getDouble(item, "c");
        candle.volume = hl::json::getDouble(item, "v");
        candle.timestamp = endTime;

        candles.push_back(candle);
    }
    yyjson_doc_free(doc);
    return candles;
}

//=============================================================================
// EXTRACTED PARSING: metaAndAssetCtxs (from hl_market_service.cpp:383-403)
// getFundingRate() parses metaAndAssetCtxs to extract funding rate.
//=============================================================================

struct FundingResult {
    double rate = 0.0;
    bool valid = false;
};

/// Parse metaAndAssetCtxs response to extract funding rate at given index.
/// Input: [metaObj, [assetCtx0, assetCtx1, ...]]
/// Each assetCtx has a "funding" field (string-encoded double).
FundingResult parseFundingRate(const char* json, size_t len, int assetIndex) {
    FundingResult result;
    if (!json || len == 0 || assetIndex < 0) return result;

    yyjson_doc* doc = yyjson_read(json, len, 0);
    if (!doc) return result;

    yyjson_val* root = yyjson_doc_get_root(doc);
    if (!yyjson_is_arr(root)) { yyjson_doc_free(doc); return result; }

    // Asset contexts array is the second element
    yyjson_val* ctxs = yyjson_arr_get(root, 1);
    if (!ctxs || !yyjson_is_arr(ctxs)) { yyjson_doc_free(doc); return result; }

    yyjson_val* ctx = yyjson_arr_get(ctxs, (size_t)assetIndex);
    if (!ctx) {
        yyjson_doc_free(doc);
        return result;
    }

    result.rate = hl::json::getDouble(ctx, "funding");
    result.valid = true;
    yyjson_doc_free(doc);
    return result;
}

} // namespace MktHttp

//=============================================================================
// TEST CASES: l2Book parsing
//=============================================================================

TEST_CASE(l2book_valid_bid_ask) {
    const char* json = R"({"levels":[[{"px":"91234.50","sz":"1.5","n":3}],[{"px":"91235.00","sz":"0.8","n":2}]]})";
    auto r = MktHttp::parseL2Book(json, strlen(json));
    ASSERT_TRUE(r.valid);
    ASSERT_FLOAT_EQ_TOL(r.bid, 91234.50, 0.01);
    ASSERT_FLOAT_EQ_TOL(r.ask, 91235.00, 0.01);
    ASSERT_FLOAT_EQ_TOL(r.mid, (91234.50 + 91235.00) / 2.0, 0.01);
}

TEST_CASE(l2book_numeric_prices) {
    // Hyperliquid usually sends string prices, but handle numeric too
    const char* json = R"({"levels":[[{"px":50000.0,"sz":"1.0"}],[{"px":50001.0,"sz":"0.5"}]]})";
    auto r = MktHttp::parseL2Book(json, strlen(json));
    ASSERT_TRUE(r.valid);
    ASSERT_FLOAT_EQ_TOL(r.bid, 50000.0, 0.01);
    ASSERT_FLOAT_EQ_TOL(r.ask, 50001.0, 0.01);
}

TEST_CASE(l2book_null_response_asset_not_found) {
    const char* json = "null";
    auto r = MktHttp::parseL2Book(json, strlen(json));
    ASSERT_FALSE(r.valid);
    ASSERT_TRUE(r.isNull);
}

TEST_CASE(l2book_empty_levels) {
    const char* json = R"({"levels":[[],[]]})";
    auto r = MktHttp::parseL2Book(json, strlen(json));
    ASSERT_FALSE(r.valid);
    ASSERT_FLOAT_EQ(r.bid, 0.0);
    ASSERT_FLOAT_EQ(r.ask, 0.0);
}

TEST_CASE(l2book_missing_levels_key) {
    const char* json = R"({"something":"else"})";
    auto r = MktHttp::parseL2Book(json, strlen(json));
    ASSERT_FALSE(r.valid);
}

TEST_CASE(l2book_malformed_json) {
    const char* json = "not valid json at all";
    auto r = MktHttp::parseL2Book(json, strlen(json));
    ASSERT_FALSE(r.valid);
}

TEST_CASE(l2book_empty_input) {
    auto r = MktHttp::parseL2Book("", 0);
    ASSERT_FALSE(r.valid);
}

TEST_CASE(l2book_null_input) {
    auto r = MktHttp::parseL2Book(nullptr, 0);
    ASSERT_FALSE(r.valid);
}

TEST_CASE(l2book_bid_only_thin_book) {
    // Only bid side populated, no asks
    const char* json = R"({"levels":[[{"px":"100.50","sz":"10.0"}],[]]})";
    auto r = MktHttp::parseL2Book(json, strlen(json));
    ASSERT_TRUE(r.valid);
    ASSERT_FLOAT_EQ_TOL(r.bid, 100.50, 0.01);
    ASSERT_FLOAT_EQ_TOL(r.ask, 100.50, 0.01);  // Fallback: ask = bid
    ASSERT_FLOAT_EQ_TOL(r.mid, 100.50, 0.01);
}

TEST_CASE(l2book_multi_level_uses_top) {
    // Multiple bid/ask levels — should use top of book
    const char* json = R"({"levels":[
        [{"px":"50000","sz":"1"},{"px":"49999","sz":"2"},{"px":"49998","sz":"3"}],
        [{"px":"50001","sz":"0.5"},{"px":"50002","sz":"1.0"}]
    ]})";
    auto r = MktHttp::parseL2Book(json, strlen(json));
    ASSERT_TRUE(r.valid);
    ASSERT_FLOAT_EQ_TOL(r.bid, 50000.0, 0.01);
    ASSERT_FLOAT_EQ_TOL(r.ask, 50001.0, 0.01);
}

TEST_CASE(l2book_small_price_asset) {
    // Sub-penny asset (like PEPE)
    const char* json = R"({"levels":[[{"px":"0.00001234","sz":"1000000"}],[{"px":"0.00001235","sz":"500000"}]]})";
    auto r = MktHttp::parseL2Book(json, strlen(json));
    ASSERT_TRUE(r.valid);
    ASSERT_FLOAT_EQ_TOL(r.bid, 0.00001234, 1e-10);
    ASSERT_FLOAT_EQ_TOL(r.ask, 0.00001235, 1e-10);
}

//=============================================================================
// TEST CASES: candleSnapshot parsing
//=============================================================================

TEST_CASE(candle_single_bar) {
    const char* json = R"([{"t":1700000000000,"o":"100.0","h":"105.0","l":"99.0","c":"103.0","v":"5000.0"}])";
    auto candles = MktHttp::parseCandleSnapshot(json, strlen(json), 1);
    ASSERT_EQ((int)candles.size(), 1);
    ASSERT_FLOAT_EQ_TOL(candles[0].open, 100.0, 0.01);
    ASSERT_FLOAT_EQ_TOL(candles[0].high, 105.0, 0.01);
    ASSERT_FLOAT_EQ_TOL(candles[0].low, 99.0, 0.01);
    ASSERT_FLOAT_EQ_TOL(candles[0].close, 103.0, 0.01);
    ASSERT_FLOAT_EQ_TOL(candles[0].volume, 5000.0, 0.01);
    // Timestamp should be bar END = open + 1 minute
    ASSERT_EQ(candles[0].timestamp, (int64_t)(1700000000000LL + 60000LL));
}

TEST_CASE(candle_multiple_bars) {
    const char* json = R"([
        {"t":1700000000000,"o":"100","h":"101","l":"99","c":"100.5","v":"1000"},
        {"t":1700000060000,"o":"100.5","h":"102","l":"100","c":"101","v":"1500"},
        {"t":1700000120000,"o":"101","h":"103","l":"100.5","c":"102","v":"2000"}
    ])";
    auto candles = MktHttp::parseCandleSnapshot(json, strlen(json), 1);
    ASSERT_EQ((int)candles.size(), 3);

    // First candle
    ASSERT_FLOAT_EQ_TOL(candles[0].open, 100.0, 0.01);
    ASSERT_EQ(candles[0].timestamp, (int64_t)(1700000000000LL + 60000LL));

    // Last candle
    ASSERT_FLOAT_EQ_TOL(candles[2].close, 102.0, 0.01);
    ASSERT_EQ(candles[2].timestamp, (int64_t)(1700000120000LL + 60000LL));
}

TEST_CASE(candle_hourly_timestamp_conversion) {
    // 1-hour candles: bar END = open + 3600000ms
    const char* json = R"([{"t":1700000000000,"o":"100","h":"101","l":"99","c":"100","v":"500"}])";
    auto candles = MktHttp::parseCandleSnapshot(json, strlen(json), 60);
    ASSERT_EQ((int)candles.size(), 1);
    ASSERT_EQ(candles[0].timestamp, (int64_t)(1700000000000LL + 3600000LL));
}

TEST_CASE(candle_daily_timestamp_conversion) {
    // 1-day candles: bar END = open + 86400000ms
    const char* json = R"([{"t":1700000000000,"o":"100","h":"101","l":"99","c":"100","v":"500"}])";
    auto candles = MktHttp::parseCandleSnapshot(json, strlen(json), 1440);
    ASSERT_EQ((int)candles.size(), 1);
    ASSERT_EQ(candles[0].timestamp, (int64_t)(1700000000000LL + 86400000LL));
}

TEST_CASE(candle_empty_array) {
    const char* json = "[]";
    auto candles = MktHttp::parseCandleSnapshot(json, strlen(json), 1);
    ASSERT_EQ((int)candles.size(), 0);
}

TEST_CASE(candle_not_array) {
    const char* json = R"({"error":"something"})";
    auto candles = MktHttp::parseCandleSnapshot(json, strlen(json), 1);
    ASSERT_EQ((int)candles.size(), 0);
}

TEST_CASE(candle_malformed_json) {
    auto candles = MktHttp::parseCandleSnapshot("garbage", 7, 1);
    ASSERT_EQ((int)candles.size(), 0);
}

TEST_CASE(candle_null_input) {
    auto candles = MktHttp::parseCandleSnapshot(nullptr, 0, 1);
    ASSERT_EQ((int)candles.size(), 0);
}

TEST_CASE(candle_skip_zero_timestamp) {
    // Items with t=0 should be skipped
    const char* json = R"([
        {"t":0,"o":"100","h":"101","l":"99","c":"100","v":"500"},
        {"t":1700000000000,"o":"200","h":"201","l":"199","c":"200","v":"600"}
    ])";
    auto candles = MktHttp::parseCandleSnapshot(json, strlen(json), 1);
    ASSERT_EQ((int)candles.size(), 1);
    ASSERT_FLOAT_EQ_TOL(candles[0].open, 200.0, 0.01);
}

TEST_CASE(candle_max_candles_limit) {
    // Create JSON with 5 candles but limit to 3
    const char* json = R"([
        {"t":1700000000000,"o":"100","h":"101","l":"99","c":"100","v":"500"},
        {"t":1700000060000,"o":"101","h":"102","l":"100","c":"101","v":"500"},
        {"t":1700000120000,"o":"102","h":"103","l":"101","c":"102","v":"500"},
        {"t":1700000180000,"o":"103","h":"104","l":"102","c":"103","v":"500"},
        {"t":1700000240000,"o":"104","h":"105","l":"103","c":"104","v":"500"}
    ])";
    auto candles = MktHttp::parseCandleSnapshot(json, strlen(json), 1, 3);
    ASSERT_EQ((int)candles.size(), 3);
}

TEST_CASE(candle_numeric_ohlcv) {
    // Handle numeric OHLCV values (not just string-encoded)
    const char* json = R"([{"t":1700000000000,"o":100.5,"h":101.0,"l":99.5,"c":100.0,"v":1234.5}])";
    auto candles = MktHttp::parseCandleSnapshot(json, strlen(json), 1);
    ASSERT_EQ((int)candles.size(), 1);
    ASSERT_FLOAT_EQ_TOL(candles[0].open, 100.5, 0.01);
    ASSERT_FLOAT_EQ_TOL(candles[0].volume, 1234.5, 0.1);
}

//=============================================================================
// TEST CASES: metaAndAssetCtxs parsing (funding rate)
//=============================================================================

TEST_CASE(funding_rate_btc_index_0) {
    // Response: [metaObj, [ctx0, ctx1, ...]]
    // BTC is typically at index 0
    const char* json = R"([
        {"universe":[{"name":"BTC","szDecimals":4}]},
        [{"funding":"0.0000125","openInterest":"50000","premium":"0.001"}]
    ])";
    auto r = MktHttp::parseFundingRate(json, strlen(json), 0);
    ASSERT_TRUE(r.valid);
    ASSERT_FLOAT_EQ_TOL(r.rate, 0.0000125, 1e-10);
}

TEST_CASE(funding_rate_eth_index_1) {
    const char* json = R"([
        {"universe":[{"name":"BTC"},{"name":"ETH"}]},
        [
            {"funding":"0.0000100","openInterest":"50000"},
            {"funding":"-0.0000200","openInterest":"30000"}
        ]
    ])";
    auto r = MktHttp::parseFundingRate(json, strlen(json), 1);
    ASSERT_TRUE(r.valid);
    ASSERT_FLOAT_EQ_TOL(r.rate, -0.0000200, 1e-10);
}

TEST_CASE(funding_rate_negative) {
    const char* json = R"([
        {"universe":[]},
        [{"funding":"-0.0000500"}]
    ])";
    auto r = MktHttp::parseFundingRate(json, strlen(json), 0);
    ASSERT_TRUE(r.valid);
    ASSERT_FLOAT_EQ_TOL(r.rate, -0.0000500, 1e-10);
}

TEST_CASE(funding_rate_zero) {
    const char* json = R"([
        {"universe":[]},
        [{"funding":"0.0"}]
    ])";
    auto r = MktHttp::parseFundingRate(json, strlen(json), 0);
    ASSERT_TRUE(r.valid);
    ASSERT_FLOAT_EQ_TOL(r.rate, 0.0, 1e-10);
}

TEST_CASE(funding_rate_index_out_of_range) {
    const char* json = R"([
        {"universe":[]},
        [{"funding":"0.0000100"}]
    ])";
    // Only 1 context but requesting index 5
    auto r = MktHttp::parseFundingRate(json, strlen(json), 5);
    ASSERT_FALSE(r.valid);
}

TEST_CASE(funding_rate_negative_index) {
    const char* json = R"([{"universe":[]},[ {"funding":"0.0001"}]])";
    auto r = MktHttp::parseFundingRate(json, strlen(json), -1);
    ASSERT_FALSE(r.valid);
}

TEST_CASE(funding_rate_not_array) {
    const char* json = R"({"error":"something went wrong"})";
    auto r = MktHttp::parseFundingRate(json, strlen(json), 0);
    ASSERT_FALSE(r.valid);
}

TEST_CASE(funding_rate_missing_ctxs_array) {
    // Root is array but only has 1 element (no contexts)
    const char* json = R"([{"universe":[]}])";
    auto r = MktHttp::parseFundingRate(json, strlen(json), 0);
    ASSERT_FALSE(r.valid);
}

TEST_CASE(funding_rate_ctxs_not_array) {
    const char* json = R"([{"universe":[]}, "not_an_array"])";
    auto r = MktHttp::parseFundingRate(json, strlen(json), 0);
    ASSERT_FALSE(r.valid);
}

TEST_CASE(funding_rate_malformed_json) {
    auto r = MktHttp::parseFundingRate("broken", 6, 0);
    ASSERT_FALSE(r.valid);
}

TEST_CASE(funding_rate_null_input) {
    auto r = MktHttp::parseFundingRate(nullptr, 0, 0);
    ASSERT_FALSE(r.valid);
}

TEST_CASE(funding_rate_numeric_value) {
    // Handle numeric funding value (not string-encoded)
    const char* json = R"([{"universe":[]}, [{"funding":0.0000300}]])";
    auto r = MktHttp::parseFundingRate(json, strlen(json), 0);
    ASSERT_TRUE(r.valid);
    ASSERT_FLOAT_EQ_TOL(r.rate, 0.0000300, 1e-10);
}

//=============================================================================
// MAIN
//=============================================================================

int main() {
    printf("=== OPM-174: Market Service HTTP Parsing Tests ===\n\n");

    // l2Book parsing
    RUN_TEST(l2book_valid_bid_ask);
    RUN_TEST(l2book_numeric_prices);
    RUN_TEST(l2book_null_response_asset_not_found);
    RUN_TEST(l2book_empty_levels);
    RUN_TEST(l2book_missing_levels_key);
    RUN_TEST(l2book_malformed_json);
    RUN_TEST(l2book_empty_input);
    RUN_TEST(l2book_null_input);
    RUN_TEST(l2book_bid_only_thin_book);
    RUN_TEST(l2book_multi_level_uses_top);
    RUN_TEST(l2book_small_price_asset);

    // candleSnapshot parsing
    RUN_TEST(candle_single_bar);
    RUN_TEST(candle_multiple_bars);
    RUN_TEST(candle_hourly_timestamp_conversion);
    RUN_TEST(candle_daily_timestamp_conversion);
    RUN_TEST(candle_empty_array);
    RUN_TEST(candle_not_array);
    RUN_TEST(candle_malformed_json);
    RUN_TEST(candle_null_input);
    RUN_TEST(candle_skip_zero_timestamp);
    RUN_TEST(candle_max_candles_limit);
    RUN_TEST(candle_numeric_ohlcv);

    // metaAndAssetCtxs funding rate parsing
    RUN_TEST(funding_rate_btc_index_0);
    RUN_TEST(funding_rate_eth_index_1);
    RUN_TEST(funding_rate_negative);
    RUN_TEST(funding_rate_zero);
    RUN_TEST(funding_rate_index_out_of_range);
    RUN_TEST(funding_rate_negative_index);
    RUN_TEST(funding_rate_not_array);
    RUN_TEST(funding_rate_missing_ctxs_array);
    RUN_TEST(funding_rate_ctxs_not_array);
    RUN_TEST(funding_rate_malformed_json);
    RUN_TEST(funding_rate_null_input);
    RUN_TEST(funding_rate_numeric_value);

    return printTestSummary();
}
