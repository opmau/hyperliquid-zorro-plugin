//=============================================================================
// test_trade_continuity.cpp - Trade identity across logins and restarts [OPM-1552]
//=============================================================================
// LAYER: Test | TESTS: src/foundation/hl_trade_uuid.h and
//                      TradingState::beginSession (hl_globals.cpp) - production
//                      code, linked directly, not a re-implementation
//
// Zorro keeps the trade IDs the plugin handed it for as long as the trades are
// open. Two contracts follow:
//   - a logout/login on the same account must leave every int trade ID
//     resolvable, and must not reissue one that is already in use;
//   - a UUID identifier must carry everything needed to act on the trade, and
//     anything that is not exactly such an identifier must be refused.
//=============================================================================

#include "../test_framework.h"
#include "../../src/foundation/hl_trade_uuid.h"
#include "../../src/foundation/hl_globals.h"
#include <cstring>

using namespace hl::test;
using hl::OrderSide;
namespace uuid = hl::uuid;

static const char CLOID[] = "0x00000002000000006523a1f000000007";

//=============================================================================
// UUID FORMAT
//=============================================================================

TEST_CASE(format_then_parse_round_trips) {
    char buf[256];
    ASSERT_TRUE(uuid::format("BTC", OrderSide::Buy, CLOID, buf, sizeof(buf)));
    ASSERT_STREQ(buf, "HL1|BTC|L|0x00000002000000006523a1f000000007");

    uuid::TradeUuid u;
    ASSERT_TRUE(uuid::parse(buf, u));
    ASSERT_STREQ(u.coin, "BTC");
    ASSERT_TRUE(u.side == OrderSide::Buy);
    ASSERT_STREQ(u.cloid, CLOID);
}

TEST_CASE(short_side_round_trips) {
    char buf[256];
    ASSERT_TRUE(uuid::format("ETH", OrderSide::Sell, CLOID, buf, sizeof(buf)));
    ASSERT_STREQ(buf, "HL1|ETH|S|0x00000002000000006523a1f000000007");
    uuid::TradeUuid u;
    ASSERT_TRUE(uuid::parse(buf, u));
    ASSERT_TRUE(u.side == OrderSide::Sell);
}

TEST_CASE(coin_names_with_separators_round_trip) {
    const char* coins[] = { "xyz:TSLA", "PURR/USDC", "@107", "kPEPE" };
    for (const char* coin : coins) {
        char buf[256];
        ASSERT_TRUE(uuid::format(coin, OrderSide::Buy, CLOID, buf, sizeof(buf)));
        uuid::TradeUuid u;
        ASSERT_TRUE(uuid::parse(buf, u));
        ASSERT_STREQ(u.coin, coin);
        ASSERT_STREQ(u.cloid, CLOID);
    }
}

TEST_CASE(identifier_fits_zorro_uuid_buffer) {
    // The longest accepted coin (31 chars) still fits Zorro's 256-char buffer.
    char coin[32];
    memset(coin, 'A', 31);
    coin[31] = 0;
    char buf[256];
    ASSERT_TRUE(uuid::format(coin, OrderSide::Buy, CLOID, buf, sizeof(buf)));
    ASSERT_LT(strlen(buf), (size_t)256);
    uuid::TradeUuid u;
    ASSERT_TRUE(uuid::parse(buf, u));
    ASSERT_STREQ(u.coin, coin);
}

TEST_CASE(format_refuses_unusable_input) {
    char buf[256];
    ASSERT_FALSE(uuid::format("", OrderSide::Buy, CLOID, buf, sizeof(buf)));
    ASSERT_STREQ(buf, "");
    ASSERT_FALSE(uuid::format(nullptr, OrderSide::Buy, CLOID, buf, sizeof(buf)));
    ASSERT_FALSE(uuid::format("A|B", OrderSide::Buy, CLOID, buf, sizeof(buf)));
    ASSERT_FALSE(uuid::format("BTC", OrderSide::Buy, "", buf, sizeof(buf)));
    ASSERT_FALSE(uuid::format("BTC", OrderSide::Buy, nullptr, buf, sizeof(buf)));
    ASSERT_FALSE(uuid::format("BTC", OrderSide::Buy, "PENDING_1", buf, sizeof(buf)));
    ASSERT_FALSE(uuid::format("BTC", OrderSide::Buy, "0x1234", buf, sizeof(buf)));

    char coin[33];
    memset(coin, 'A', 32);
    coin[32] = 0;
    ASSERT_FALSE(uuid::format(coin, OrderSide::Buy, CLOID, buf, sizeof(buf)));

    char tiny[16];
    ASSERT_FALSE(uuid::format("BTC", OrderSide::Buy, CLOID, tiny, sizeof(tiny)));
    ASSERT_STREQ(tiny, "");
}

TEST_CASE(parse_refuses_anything_else) {
    const char* bad[] = {
        "",
        "12345",
        "HL1|",
        "HL1||L|0x00000002000000006523a1f000000007",            // no coin
        "HL1|BTC|X|0x00000002000000006523a1f000000007",         // bad side
        "HL1|BTC|L0x00000002000000006523a1f000000007",          // missing bar
        "HL1|BTC|L|0x00000002000000006523a1f00000000",          // short cloid
        "HL1|BTC|L|0x00000002000000006523a1f0000000070",        // long cloid
        "HL1|BTC|L|0x00000002000000006523a1f00000000g",         // non-hex
        "HL1|BTC|L|0x00000002000000006523a1f000000007|extra",   // trailing data
        "HL2|BTC|L|0x00000002000000006523a1f000000007",         // other version
        "hl1|BTC|L|0x00000002000000006523a1f000000007",         // wrong case
    };
    for (const char* s : bad) {
        uuid::TradeUuid u;
        ASSERT_FALSE(uuid::parse(s, u));
        ASSERT_STREQ(u.coin, "");
        ASSERT_STREQ(u.cloid, "");
    }
    uuid::TradeUuid u;
    ASSERT_FALSE(uuid::parse(nullptr, u));
}

TEST_CASE(parse_refuses_oversized_coin) {
    char s[128];
    sprintf_s(s, "HL1|%s|L|%s", "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789", CLOID);
    uuid::TradeUuid u;
    ASSERT_FALSE(uuid::parse(s, u));
}

//=============================================================================
// SESSION CONTINUITY
//=============================================================================

static hl::OrderState order(const char* oid) {
    hl::OrderState st;
    strncpy_s(st.orderId, oid, _TRUNCATE);
    strncpy_s(st.coin, "BTC", _TRUNCATE);
    st.filledSize = 0.01;
    return st;
}

TEST_CASE(same_account_relogin_keeps_trades_and_counter) {
    hl::TradingState ts;
    ts.init();
    ASSERT_FALSE(ts.beginSession("0xabc@testnet"));

    int a = ts.generateTradeId();
    int b = ts.generateTradeId();
    ts.setOrder(a, order("101"));
    ts.setOrder(b, order("102"));
    ts.uuidTrades = true;

    // Logout + login on the same account; the address casing may differ.
    ASSERT_TRUE(ts.beginSession("0xABC@testnet"));

    hl::OrderState out;
    ASSERT_TRUE(ts.getOrder(a, out));
    ASSERT_STREQ(out.orderId, "101");
    ASSERT_TRUE(ts.getOrder(b, out));
    ASSERT_STREQ(out.orderId, "102");
    ASSERT_TRUE(ts.uuidTrades);

    // The next ID must not collide with one Zorro still holds.
    ASSERT_GT(ts.generateTradeId(), b);
    ts.cleanup();
}

TEST_CASE(different_account_starts_empty) {
    hl::TradingState ts;
    ts.init();
    ts.beginSession("0xabc@testnet");
    int a = ts.generateTradeId();
    ts.setOrder(a, order("101"));
    ts.uuidTrades = true;
    uint64_t nonce = ts.generateNonce();

    ASSERT_FALSE(ts.beginSession("0xdef@testnet"));

    hl::OrderState out;
    ASSERT_FALSE(ts.getOrder(a, out));
    ASSERT_FALSE(ts.uuidTrades);
    ASSERT_EQ(ts.generateTradeId(), 2);
    // The nonce never goes backwards, whichever account signs next.
    ASSERT_GT(ts.generateNonce(), nonce);
    ts.cleanup();
}

TEST_CASE(same_wallet_other_network_starts_empty) {
    hl::TradingState ts;
    ts.init();
    ts.beginSession("0xabc@testnet");
    int a = ts.generateTradeId();
    ts.setOrder(a, order("101"));

    ASSERT_FALSE(ts.beginSession("0xabc@mainnet"));
    hl::OrderState out;
    ASSERT_FALSE(ts.getOrder(a, out));
    ts.cleanup();
}

TEST_CASE(empty_owner_never_matches) {
    hl::TradingState ts;
    ts.init();
    ASSERT_FALSE(ts.beginSession(""));
    int a = ts.generateTradeId();
    ts.setOrder(a, order("101"));
    ASSERT_FALSE(ts.beginSession(""));
    hl::OrderState out;
    ASSERT_FALSE(ts.getOrder(a, out));
    ASSERT_FALSE(ts.beginSession(nullptr));
    ts.cleanup();
}

int main() {
    printf("=== Trade identity across logins and restarts [OPM-1552] ===\n");

    printf("\n-- UUID format --\n");
    RUN_TEST(format_then_parse_round_trips);
    RUN_TEST(short_side_round_trips);
    RUN_TEST(coin_names_with_separators_round_trip);
    RUN_TEST(identifier_fits_zorro_uuid_buffer);
    RUN_TEST(format_refuses_unusable_input);
    RUN_TEST(parse_refuses_anything_else);
    RUN_TEST(parse_refuses_oversized_coin);

    printf("\n-- session continuity --\n");
    RUN_TEST(same_account_relogin_keeps_trades_and_counter);
    RUN_TEST(different_account_starts_empty);
    RUN_TEST(same_wallet_other_network_starts_empty);
    RUN_TEST(empty_owner_never_matches);

    return printTestSummary();
}
