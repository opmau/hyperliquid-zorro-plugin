//=============================================================================
// test_ws_parsers.cpp - Unit tests for WebSocket message parsers [OPM-10]
//=============================================================================
// Part of Hyperliquid Plugin for Zorro
//
// LAYER: Test
// PURPOSE: Deterministic unit tests for all 6 ws_parsers.cpp functions using
//          canned JSON fixtures. No network dependency.
//
// PARSERS TESTED:
//   parseL2Book, parsePostResponse, parseClearinghouseState,
//   parseOpenOrders, parseUserFills, parseOrderUpdates
//=============================================================================

#include "../test_framework.h"
#include "ws_parsers.h"
#include "ws_price_cache.h"

//=============================================================================
// HELPERS
//=============================================================================

// Capture struct for OrderUpdateCallback
static struct {
    char oid[64];
    char cloid[64];
    char status[64];
    double filledSz;
    double avgPx;
    int callCount;
} g_orderUpdateCapture;

static void resetOrderUpdateCapture() {
    memset(&g_orderUpdateCapture, 0, sizeof(g_orderUpdateCapture));
}

static void captureOrderUpdate(const char* oid, const char* cloid,
                                const char* status, double filledSz,
                                double avgPx) {
    if (oid) strncpy_s(g_orderUpdateCapture.oid, oid, _TRUNCATE);
    if (cloid) strncpy_s(g_orderUpdateCapture.cloid, cloid, _TRUNCATE);
    if (status) strncpy_s(g_orderUpdateCapture.status, status, _TRUNCATE);
    g_orderUpdateCapture.filledSz = filledSz;
    g_orderUpdateCapture.avgPx = avgPx;
    g_orderUpdateCapture.callCount++;
}

//=============================================================================
// parseL2Book TESTS
//=============================================================================

TEST_CASE(l2book_valid_ws_message) {
    const char* json = R"({
        "channel":"l2Book",
        "data":{
            "coin":"BTC",
            "levels":[
                [{"px":"91234.50","sz":"1.5","n":3}],
                [{"px":"91235.00","sz":"0.8","n":2}]
            ]
        }
    })";
    auto r = hl::ws::parseL2Book(json, 0, nullptr);
    ASSERT_TRUE(r.valid);
    ASSERT_STREQ(r.coin, "BTC");
    ASSERT_FLOAT_EQ_TOL(r.bid, 91234.50, 0.01);
    ASSERT_FLOAT_EQ_TOL(r.ask, 91235.00, 0.01);
}

TEST_CASE(l2book_valid_http_path) {
    // HTTP path: no "channel"/"data" wrapper — coin+levels at root
    const char* json = R"({
        "coin":"ETH",
        "levels":[
            [{"px":"3100.25","sz":"10.0"}],
            [{"px":"3100.75","sz":"5.0"}]
        ]
    })";
    auto r = hl::ws::parseL2Book(json, 0, nullptr);
    ASSERT_TRUE(r.valid);
    ASSERT_STREQ(r.coin, "ETH");
    ASSERT_FLOAT_EQ_TOL(r.bid, 3100.25, 0.01);
    ASSERT_FLOAT_EQ_TOL(r.ask, 3100.75, 0.01);
}

TEST_CASE(l2book_missing_coin) {
    const char* json = R"({
        "data":{
            "levels":[[{"px":"100","sz":"1"}],[{"px":"101","sz":"1"}]]
        }
    })";
    auto r = hl::ws::parseL2Book(json, 0, nullptr);
    ASSERT_FALSE(r.valid);
}

TEST_CASE(l2book_empty_levels) {
    const char* json = R"({
        "data":{"coin":"SOL","levels":[[],[]]}
    })";
    auto r = hl::ws::parseL2Book(json, 0, nullptr);
    // bid/ask remain 0 → not valid
    ASSERT_FALSE(r.valid);
}

TEST_CASE(l2book_malformed_json) {
    auto r = hl::ws::parseL2Book("{not valid json", 0, nullptr);
    ASSERT_FALSE(r.valid);
}

//=============================================================================
// parsePostResponse TESTS
//=============================================================================

TEST_CASE(post_response_filled) {
    const char* json = R"({
        "channel":"post",
        "data":{
            "id":42,
            "response":{
                "type":"order",
                "data":{
                    "statuses":[{"filled":{"totalSz":"1.0","avgPx":"91000.0","oid":12345}}]
                }
            }
        }
    })";
    auto r = hl::ws::parsePostResponse(json, 0, nullptr);
    ASSERT_EQ(r.requestId, 42);
    ASSERT_TRUE(r.success);
    ASSERT_STREQ(r.status.c_str(), "filled");
}

TEST_CASE(post_response_resting) {
    const char* json = R"({
        "channel":"post",
        "data":{
            "id":7,
            "response":{
                "type":"order",
                "data":{
                    "statuses":[{"resting":{"oid":99999}}]
                }
            }
        }
    })";
    auto r = hl::ws::parsePostResponse(json, 0, nullptr);
    ASSERT_EQ(r.requestId, 7);
    ASSERT_TRUE(r.success);
    ASSERT_STREQ(r.status.c_str(), "open");
}

TEST_CASE(post_response_error) {
    const char* json = R"({
        "channel":"post",
        "data":{
            "id":99,
            "error":"Insufficient margin"
        }
    })";
    auto r = hl::ws::parsePostResponse(json, 0, nullptr);
    ASSERT_EQ(r.requestId, 99);
    ASSERT_FALSE(r.success);
    ASSERT_STREQ(r.error.c_str(), "Insufficient margin");
}

TEST_CASE(post_response_missing_id) {
    const char* json = R"({"data":{"response":{"type":"order"}}})";
    auto r = hl::ws::parsePostResponse(json, 0, nullptr);
    ASSERT_EQ(r.requestId, 0);
    ASSERT_FALSE(r.success);
}

TEST_CASE(post_response_malformed) {
    auto r = hl::ws::parsePostResponse("{{{{", 0, nullptr);
    ASSERT_EQ(r.requestId, 0);
    ASSERT_FALSE(r.success);
}

//=============================================================================
// parseClearinghouseState TESTS
//=============================================================================

TEST_CASE(clearinghouse_ws_path) {
    hl::ws::PriceCache cache;
    const char* json = R"({
        "channel":"clearinghouseState",
        "data":{
            "clearinghouseState":{
                "assetPositions":[
                    {"position":{"coin":"BTC","szi":"0.5","entryPx":"90000","unrealizedPnl":"500","marginUsed":"4500","leverage":{"type":"cross","value":"20"}}}
                ],
                "marginSummary":{
                    "accountValue":"10000.0",
                    "totalMarginUsed":"4500.0",
                    "totalNtlPos":"45000.0"
                },
                "withdrawable":"5500.0"
            }
        }
    })";
    hl::ws::parseClearinghouseState(cache, json, 0, nullptr);

    // Check position
    auto pos = cache.getPosition("BTC");
    ASSERT_FLOAT_EQ_TOL(pos.size, 0.5, 1e-6);
    ASSERT_FLOAT_EQ_TOL(pos.entryPx, 90000.0, 0.01);
    ASSERT_FLOAT_EQ_TOL(pos.unrealizedPnl, 500.0, 0.01);
    ASSERT_FLOAT_EQ_TOL(pos.leverage, 20.0, 0.01);

    // Check account data
    auto acct = cache.getAccountData();
    ASSERT_FLOAT_EQ_TOL(acct.accountValue, 10000.0, 0.01);
    ASSERT_FLOAT_EQ_TOL(acct.totalMarginUsed, 4500.0, 0.01);
    ASSERT_FLOAT_EQ_TOL(acct.withdrawable, 5500.0, 0.01);
    ASSERT_FLOAT_EQ_TOL(acct.totalNtlPos, 45000.0, 0.01);
}

TEST_CASE(clearinghouse_http_path) {
    hl::ws::PriceCache cache;
    // HTTP path: root IS the clearinghouseState directly
    const char* json = R"({
        "assetPositions":[
            {"position":{"coin":"ETH","szi":"-2.0","entryPx":"3100","unrealizedPnl":"-50","marginUsed":"620","leverage":"5"}}
        ],
        "marginSummary":{
            "accountValue":"8000.0",
            "totalMarginUsed":"620.0",
            "totalNtlPos":"6200.0"
        },
        "withdrawable":"7380.0"
    })";
    hl::ws::parseClearinghouseState(cache, json, 0, nullptr);

    auto pos = cache.getPosition("ETH");
    ASSERT_FLOAT_EQ_TOL(pos.size, -2.0, 1e-6);
    ASSERT_FLOAT_EQ_TOL(pos.leverage, 5.0, 0.01);

    auto acct = cache.getAccountData();
    ASSERT_FLOAT_EQ_TOL(acct.accountValue, 8000.0, 0.01);
    ASSERT_FLOAT_EQ_TOL(acct.withdrawable, 7380.0, 0.01);
}

TEST_CASE(clearinghouse_empty_positions) {
    hl::ws::PriceCache cache;
    const char* json = R"({
        "assetPositions":[],
        "marginSummary":{
            "accountValue":"500.0",
            "totalMarginUsed":"0",
            "totalNtlPos":"0"
        },
        "withdrawable":"500.0"
    })";
    hl::ws::parseClearinghouseState(cache, json, 0, nullptr);

    auto positions = cache.getAllPositions();
    ASSERT_EQ((int)positions.size(), 0);

    auto acct = cache.getAccountData();
    ASSERT_FLOAT_EQ_TOL(acct.accountValue, 500.0, 0.01);
}

TEST_CASE(clearinghouse_cross_margin_higher) {
    hl::ws::PriceCache cache;
    // crossMarginSummary has higher accountValue → should be preferred
    const char* json = R"({
        "assetPositions":[],
        "marginSummary":{
            "accountValue":"5000.0",
            "totalMarginUsed":"100.0",
            "totalNtlPos":"1000.0"
        },
        "crossMarginSummary":{
            "accountValue":"8000.0",
            "totalMarginUsed":"200.0",
            "totalNtlPos":"2000.0"
        },
        "withdrawable":"7800.0"
    })";
    hl::ws::parseClearinghouseState(cache, json, 0, nullptr);

    auto acct = cache.getAccountData();
    ASSERT_FLOAT_EQ_TOL(acct.accountValue, 8000.0, 0.01);
    ASSERT_FLOAT_EQ_TOL(acct.totalMarginUsed, 200.0, 0.01);
    ASSERT_FLOAT_EQ_TOL(acct.totalNtlPos, 2000.0, 0.01);
}

TEST_CASE(clearinghouse_leverage_as_number) {
    hl::ws::PriceCache cache;
    // leverage as plain number (not object)
    const char* json = R"({
        "assetPositions":[
            {"position":{"coin":"SOL","szi":"100","entryPx":"150","unrealizedPnl":"0","marginUsed":"750","leverage":"10"}}
        ],
        "marginSummary":{"accountValue":"1000","totalMarginUsed":"750","totalNtlPos":"15000"},
        "withdrawable":"250"
    })";
    hl::ws::parseClearinghouseState(cache, json, 0, nullptr);

    auto pos = cache.getPosition("SOL");
    ASSERT_FLOAT_EQ_TOL(pos.leverage, 10.0, 0.01);
}

//=============================================================================
// parseOpenOrders TESTS
//=============================================================================

TEST_CASE(open_orders_data_orders_path) {
    hl::ws::PriceCache cache;
    // WS path: data.orders is the array
    const char* json = R"({
        "channel":"openOrders",
        "data":{
            "dex":"",
            "user":"0xabc",
            "orders":[
                {"oid":"111","coin":"BTC","side":"Buy","limitPx":"90000","sz":"0.1","origSz":"0.2"},
                {"oid":"222","coin":"ETH","side":"Sell","limitPx":"3200","sz":"5.0","origSz":"5.0"}
            ]
        }
    })";
    hl::ws::parseOpenOrders(cache, json, 0, nullptr);

    auto btcOrder = cache.getOpenOrder("111");
    ASSERT_STREQ(btcOrder.oid.c_str(), "111");
    ASSERT_STREQ(btcOrder.coin.c_str(), "BTC");
    ASSERT_TRUE(btcOrder.isBuy);
    ASSERT_FLOAT_EQ_TOL(btcOrder.limitPx, 90000.0, 0.01);
    ASSERT_FLOAT_EQ_TOL(btcOrder.sz, 0.1, 1e-6);
    ASSERT_FLOAT_EQ_TOL(btcOrder.origSz, 0.2, 1e-6);

    auto ethOrder = cache.getOpenOrder("222");
    ASSERT_STREQ(ethOrder.coin.c_str(), "ETH");
    ASSERT_FALSE(ethOrder.isBuy);
}

TEST_CASE(open_orders_data_as_array) {
    hl::ws::PriceCache cache;
    // Alternative WS path: data IS the array directly
    const char* json = R"({
        "channel":"openOrders",
        "data":[
            {"oid":"333","coin":"SOL","side":"Buy","limitPx":"150","sz":"10","origSz":"10"}
        ]
    })";
    hl::ws::parseOpenOrders(cache, json, 0, nullptr);

    auto order = cache.getOpenOrder("333");
    ASSERT_STREQ(order.coin.c_str(), "SOL");
    ASSERT_TRUE(order.isBuy);
}

TEST_CASE(open_orders_empty) {
    hl::ws::PriceCache cache;
    // Pre-populate then clear via empty orders
    hl::ws::OpenOrderData seed;
    seed.oid = "old";
    seed.coin = "BTC";
    cache.setOpenOrder(seed);

    const char* json = R"({"data":{"orders":[]}})";
    hl::ws::parseOpenOrders(cache, json, 0, nullptr);

    // clearOpenOrders() is called at the start of parseOpenOrders
    auto old = cache.getOpenOrder("old");
    ASSERT_TRUE(old.oid.empty());
}

//=============================================================================
// parseUserFills TESTS
//=============================================================================

TEST_CASE(user_fills_data_as_array) {
    hl::ws::PriceCache cache;
    const char* json = R"({
        "channel":"userFills",
        "data":[
            {"oid":"aaa","coin":"BTC","side":"Buy","px":"91000","sz":"0.5","fee":"0.45","time":1700000000000,"tid":"t1"},
            {"oid":"bbb","coin":"ETH","side":"Sell","px":"3100","sz":"2.0","fee":"0.31","time":1700000001000,"tid":"t2"}
        ]
    })";
    hl::ws::parseUserFills(cache, json, 0, nullptr);

    auto fills = cache.getRecentFills(10);
    ASSERT_EQ((int)fills.size(), 2);

    // Fills are ordered by insertion
    bool foundBTC = false, foundETH = false;
    for (auto& f : fills) {
        if (f.coin == "BTC") {
            ASSERT_TRUE(f.isBuy);
            ASSERT_FLOAT_EQ_TOL(f.px, 91000.0, 0.01);
            ASSERT_FLOAT_EQ_TOL(f.sz, 0.5, 1e-6);
            ASSERT_FLOAT_EQ_TOL(f.fee, 0.45, 0.001);
            foundBTC = true;
        }
        if (f.coin == "ETH") {
            ASSERT_FALSE(f.isBuy);
            ASSERT_FLOAT_EQ_TOL(f.px, 3100.0, 0.01);
            foundETH = true;
        }
    }
    ASSERT_TRUE(foundBTC);
    ASSERT_TRUE(foundETH);
}

TEST_CASE(user_fills_zero_size_skipped) {
    hl::ws::PriceCache cache;
    const char* json = R"({
        "data":[
            {"oid":"x","coin":"BTC","side":"Buy","px":"90000","sz":"0","fee":"0","time":0,"tid":"t0"}
        ]
    })";
    hl::ws::parseUserFills(cache, json, 0, nullptr);

    auto fills = cache.getRecentFills(10);
    ASSERT_EQ((int)fills.size(), 0);
}

TEST_CASE(user_fills_data_fills_path) {
    hl::ws::PriceCache cache;
    // Alternative: data.fills
    const char* json = R"({
        "data":{
            "fills":[
                {"oid":"fff","coin":"DOGE","side":"Buy","px":"0.15","sz":"1000","fee":"0.01","time":123456,"tid":"t9"}
            ]
        }
    })";
    hl::ws::parseUserFills(cache, json, 0, nullptr);

    auto fills = cache.getRecentFills(10);
    ASSERT_EQ((int)fills.size(), 1);
    ASSERT_STREQ(fills[0].coin.c_str(), "DOGE");
}

//=============================================================================
// parseOrderUpdates TESTS
//=============================================================================

TEST_CASE(order_updates_filled) {
    resetOrderUpdateCapture();
    const char* json = R"({
        "channel":"orderUpdates",
        "data":[
            {
                "order":{"oid":"12345","coin":"BTC","side":"Buy","limitPx":"91000","origSz":"1.0","sz":"0.0","cloid":"my-cloid-1"},
                "status":"filled"
            }
        ]
    })";
    hl::ws::parseOrderUpdates(json, captureOrderUpdate, 0, nullptr);

    ASSERT_EQ(g_orderUpdateCapture.callCount, 1);
    ASSERT_STREQ(g_orderUpdateCapture.oid, "12345");
    ASSERT_STREQ(g_orderUpdateCapture.cloid, "my-cloid-1");
    ASSERT_STREQ(g_orderUpdateCapture.status, "filled");
    ASSERT_FLOAT_EQ_TOL(g_orderUpdateCapture.filledSz, 1.0, 1e-6);
    ASSERT_FLOAT_EQ_TOL(g_orderUpdateCapture.avgPx, 91000.0, 0.01);
}

TEST_CASE(order_updates_canceled) {
    resetOrderUpdateCapture();
    const char* json = R"({
        "channel":"orderUpdates",
        "data":[
            {
                "order":{"oid":"999","coin":"ETH","side":"Sell","limitPx":"3200","origSz":"5.0","sz":"5.0"},
                "status":"canceled"
            }
        ]
    })";
    hl::ws::parseOrderUpdates(json, captureOrderUpdate, 0, nullptr);

    ASSERT_EQ(g_orderUpdateCapture.callCount, 1);
    ASSERT_STREQ(g_orderUpdateCapture.status, "canceled");
    // filledSz = origSz - sz = 5.0 - 5.0 = 0
    ASSERT_FLOAT_EQ_TOL(g_orderUpdateCapture.filledSz, 0.0, 1e-6);
    // cloid not present → empty
    ASSERT_STREQ(g_orderUpdateCapture.cloid, "");
}

TEST_CASE(order_updates_oid_as_number) {
    resetOrderUpdateCapture();
    const char* json = R"({
        "channel":"orderUpdates",
        "data":[
            {
                "order":{"oid":77777,"coin":"SOL","side":"Buy","limitPx":"150","origSz":"10","sz":"0"},
                "status":"filled"
            }
        ]
    })";
    hl::ws::parseOrderUpdates(json, captureOrderUpdate, 0, nullptr);

    ASSERT_EQ(g_orderUpdateCapture.callCount, 1);
    ASSERT_STREQ(g_orderUpdateCapture.oid, "77777");
}

TEST_CASE(order_updates_null_callback_no_crash) {
    // Should not crash when callback is NULL
    const char* json = R"({
        "channel":"orderUpdates",
        "data":[{"order":{"oid":"1","coin":"BTC","origSz":"1","sz":"0","limitPx":"90000"},"status":"filled"}]
    })";
    hl::ws::parseOrderUpdates(json, nullptr, 0, nullptr);
    // If we get here, no crash — pass
    ASSERT_TRUE(true);
}

TEST_CASE(order_updates_multiple) {
    resetOrderUpdateCapture();
    const char* json = R"({
        "channel":"orderUpdates",
        "data":[
            {"order":{"oid":"a1","coin":"BTC","origSz":"1","sz":"0","limitPx":"90000"},"status":"filled"},
            {"order":{"oid":"a2","coin":"ETH","origSz":"2","sz":"2","limitPx":"3000"},"status":"canceled"}
        ]
    })";
    hl::ws::parseOrderUpdates(json, captureOrderUpdate, 0, nullptr);

    // Callback called twice; capture has the LAST one
    ASSERT_EQ(g_orderUpdateCapture.callCount, 2);
    ASSERT_STREQ(g_orderUpdateCapture.oid, "a2");
    ASSERT_STREQ(g_orderUpdateCapture.status, "canceled");
}

//=============================================================================
// MAIN
//=============================================================================

int main() {
    printf("=== WebSocket Parser Unit Tests [OPM-10] ===\n\n");

    // parseL2Book
    RUN_TEST(l2book_valid_ws_message);
    RUN_TEST(l2book_valid_http_path);
    RUN_TEST(l2book_missing_coin);
    RUN_TEST(l2book_empty_levels);
    RUN_TEST(l2book_malformed_json);

    // parsePostResponse
    RUN_TEST(post_response_filled);
    RUN_TEST(post_response_resting);
    RUN_TEST(post_response_error);
    RUN_TEST(post_response_missing_id);
    RUN_TEST(post_response_malformed);

    // parseClearinghouseState
    RUN_TEST(clearinghouse_ws_path);
    RUN_TEST(clearinghouse_http_path);
    RUN_TEST(clearinghouse_empty_positions);
    RUN_TEST(clearinghouse_cross_margin_higher);
    RUN_TEST(clearinghouse_leverage_as_number);

    // parseOpenOrders
    RUN_TEST(open_orders_data_orders_path);
    RUN_TEST(open_orders_data_as_array);
    RUN_TEST(open_orders_empty);

    // parseUserFills
    RUN_TEST(user_fills_data_as_array);
    RUN_TEST(user_fills_zero_size_skipped);
    RUN_TEST(user_fills_data_fills_path);

    // parseOrderUpdates
    RUN_TEST(order_updates_filled);
    RUN_TEST(order_updates_canceled);
    RUN_TEST(order_updates_oid_as_number);
    RUN_TEST(order_updates_null_callback_no_crash);
    RUN_TEST(order_updates_multiple);

    return hl::test::printTestSummary();
}
