//=============================================================================
// test_trading_service.cpp - Trading service unit tests [OPM-9]
//=============================================================================
// LAYER: Test | TESTS: hl_trading_service (pure logic subset)
//
// Tests extracted pure-logic functions from hl_trading_service.cpp:
//   1. CLOID generation and parsing (generateCloid, extractTradeIdFromCloid)
//   2. Trade ID generation (generateTradeId via g_trading)
//   3. Nonce generation (generateNonce via g_trading)
//   4. Order storage & retrieval (g_trading trade map)
//   5. Fill status determination (determineFilledStatus)
//   6. Fill status string parsing (notifyFill logic)
//=============================================================================

#include "../test_framework.h"
#include "../../src/foundation/hl_types.h"
#include "../../src/foundation/hl_globals.h"
#include <cstring>
#include <cstdio>
#include <ctime>
#include <cstdint>

using namespace hl::test;
using namespace hl;

//=============================================================================
// EXTRACTED FUNCTIONS (from hl_trading_service.cpp)
// Isolated here to avoid linking heavy service dependencies.
//=============================================================================

namespace TradingLogic {

/// Generate a CLOID with embedded trade ID.
/// Format: "0x" + 8 hex (tradeId) + 16 hex (timestamp) + 8 hex (counter)
/// Total: 2 + 32 = 34 characters.
void generateCloid(int tradeId, char* outCloid, size_t outSize) {
    if (!outCloid || outSize == 0) return;

    static int s_cloidCounter = 0;
    uint64_t now = (uint64_t)time(nullptr);
    int counter = ++s_cloidCounter;

    sprintf_s(outCloid, outSize, "0x%08x%016llx%08x",
              (unsigned int)tradeId,
              (unsigned long long)now,
              (unsigned int)counter);
}

/// Extract trade ID from the first 8 hex chars after "0x" prefix.
int extractTradeIdFromCloid(const char* cloid) {
    if (!cloid || strlen(cloid) < 10) return 0;

    if (cloid[0] != '0' || (cloid[1] != 'x' && cloid[1] != 'X')) return 0;

    char hexBuf[9] = {0};
    strncpy_s(hexBuf, cloid + 2, 8);
    unsigned int tradeId = 0;
    if (sscanf_s(hexBuf, "%x", &tradeId) == 1) {
        return (int)tradeId;
    }
    return 0;
}

/// Parse fill status string into OrderStatus enum.
/// Extracted from notifyFill() for testability.
OrderStatus parseStatusString(const char* status) {
    if (!status) return OrderStatus::Open;
    if (strcmp(status, "filled") == 0) return OrderStatus::Filled;
    if (strcmp(status, "canceled") == 0 || strcmp(status, "cancelled") == 0)
        return OrderStatus::Cancelled;
    if (strcmp(status, "siblingFilledCanceled") == 0) return OrderStatus::Cancelled;
    if (strcmp(status, "partial") == 0) return OrderStatus::PartialFill;
    if (strcmp(status, "error") == 0) return OrderStatus::Error;
    return OrderStatus::Open;
}

} // namespace TradingLogic

//=============================================================================
// TEST CASES: CLOID Generation & Parsing
//=============================================================================

TEST_CASE(cloid_format_starts_with_0x) {
    char cloid[64];
    TradingLogic::generateCloid(42, cloid, sizeof(cloid));
    ASSERT_TRUE(cloid[0] == '0');
    ASSERT_TRUE(cloid[1] == 'x');
}

TEST_CASE(cloid_length_is_34_chars) {
    // "0x" + 8 (tradeId) + 16 (timestamp) + 8 (counter) = 34
    char cloid[64];
    TradingLogic::generateCloid(100, cloid, sizeof(cloid));
    ASSERT_EQ((int)strlen(cloid), 34);
}

TEST_CASE(cloid_roundtrip_extracts_trade_id) {
    int originalId = 12345;
    char cloid[64];
    TradingLogic::generateCloid(originalId, cloid, sizeof(cloid));

    int extracted = TradingLogic::extractTradeIdFromCloid(cloid);
    ASSERT_EQ(extracted, originalId);
}

TEST_CASE(cloid_roundtrip_large_trade_id) {
    // Max int that fits in 8 hex chars = 0xFFFFFFFF = 4294967295
    // But Zorro trade IDs are signed int, so test up to ~2 billion
    int largeId = 2000000000;
    char cloid[64];
    TradingLogic::generateCloid(largeId, cloid, sizeof(cloid));

    int extracted = TradingLogic::extractTradeIdFromCloid(cloid);
    ASSERT_EQ(extracted, largeId);
}

TEST_CASE(cloid_roundtrip_small_ids) {
    for (int id = 2; id <= 10; ++id) {
        char cloid[64];
        TradingLogic::generateCloid(id, cloid, sizeof(cloid));
        int extracted = TradingLogic::extractTradeIdFromCloid(cloid);
        ASSERT_EQ(extracted, id);
    }
}

TEST_CASE(cloid_consecutive_are_unique) {
    char cloid1[64], cloid2[64];
    TradingLogic::generateCloid(5, cloid1, sizeof(cloid1));
    TradingLogic::generateCloid(5, cloid2, sizeof(cloid2));
    // Same trade ID but different counter → different CLOIDs
    ASSERT_STRNE(cloid1, cloid2);
}

TEST_CASE(extract_null_returns_zero) {
    ASSERT_EQ(TradingLogic::extractTradeIdFromCloid(nullptr), 0);
}

TEST_CASE(extract_empty_returns_zero) {
    ASSERT_EQ(TradingLogic::extractTradeIdFromCloid(""), 0);
}

TEST_CASE(extract_short_string_returns_zero) {
    ASSERT_EQ(TradingLogic::extractTradeIdFromCloid("0x12"), 0);
}

TEST_CASE(extract_no_prefix_returns_zero) {
    ASSERT_EQ(TradingLogic::extractTradeIdFromCloid("1234567890abcdef"), 0);
}

TEST_CASE(extract_uppercase_0X_prefix_works) {
    // Build a cloid with uppercase prefix
    char cloid[64] = "0X0000002a0000000000000000ffffffff";
    int extracted = TradingLogic::extractTradeIdFromCloid(cloid);
    ASSERT_EQ(extracted, 42);  // 0x2a = 42
}

TEST_CASE(generate_cloid_null_buffer_no_crash) {
    // Should not crash
    TradingLogic::generateCloid(1, nullptr, 0);
    TradingLogic::generateCloid(1, nullptr, 64);
}

TEST_CASE(generate_cloid_zero_size_no_crash) {
    char buf[64];
    TradingLogic::generateCloid(1, buf, 0);
}

//=============================================================================
// TEST CASES: Trade ID Generation (via g_trading)
//=============================================================================

TEST_CASE(trade_id_starts_at_2) {
    // Zorro treats BrokerBuy2 return 0 or 1 as failure, so IDs start at 2
    g_trading.init();
    int first = g_trading.generateTradeId();
    ASSERT_EQ(first, 2);
    g_trading.cleanup();
}

TEST_CASE(trade_id_increments_sequentially) {
    g_trading.init();
    int id1 = g_trading.generateTradeId();
    int id2 = g_trading.generateTradeId();
    int id3 = g_trading.generateTradeId();
    ASSERT_EQ(id2, id1 + 1);
    ASSERT_EQ(id3, id2 + 1);
    g_trading.cleanup();
}

TEST_CASE(trade_id_never_returns_zero_or_one) {
    g_trading.init();
    for (int i = 0; i < 100; ++i) {
        int id = g_trading.generateTradeId();
        ASSERT_GT(id, 1);
    }
    g_trading.cleanup();
}

//=============================================================================
// TEST CASES: Nonce Generation (via g_trading)
//=============================================================================

TEST_CASE(nonce_is_nonzero) {
    g_trading.init();
    uint64_t n = g_trading.generateNonce();
    ASSERT_TRUE(n > 0);
    g_trading.cleanup();
}

TEST_CASE(nonce_is_monotonically_increasing) {
    g_trading.init();
    uint64_t prev = g_trading.generateNonce();
    for (int i = 0; i < 50; ++i) {
        uint64_t curr = g_trading.generateNonce();
        ASSERT_GT(curr, prev);
        prev = curr;
    }
    g_trading.cleanup();
}

TEST_CASE(nonce_no_collisions_in_tight_loop) {
    // Rapid-fire nonces should never collide
    g_trading.init();
    uint64_t nonces[100];
    for (int i = 0; i < 100; ++i) {
        nonces[i] = g_trading.generateNonce();
    }
    // Check all unique
    for (int i = 0; i < 100; ++i) {
        for (int j = i + 1; j < 100; ++j) {
            ASSERT_NE(nonces[i], nonces[j]);
        }
    }
    g_trading.cleanup();
}

//=============================================================================
// TEST CASES: Order Storage & Retrieval (via g_trading)
//=============================================================================

TEST_CASE(store_and_get_order) {
    g_trading.init();

    OrderState state;
    strcpy_s(state.orderId, "OID_123");
    strcpy_s(state.cloid, "0x000000050000000000000000ffffffff");
    strcpy_s(state.coin, "BTC");
    state.side = OrderSide::Buy;
    state.requestedSize = 1.0;
    state.filledSize = 0.5;
    state.avgPrice = 50000.0;
    state.status = OrderStatus::PartialFill;
    state.zorroTradeId = 5;

    g_trading.setOrder(5, state);

    OrderState retrieved;
    bool found = g_trading.getOrder(5, retrieved);
    ASSERT_TRUE(found);
    ASSERT_STREQ(retrieved.orderId, "OID_123");
    ASSERT_STREQ(retrieved.cloid, "0x000000050000000000000000ffffffff");
    ASSERT_STREQ(retrieved.coin, "BTC");
    ASSERT_EQ((int)retrieved.side, (int)OrderSide::Buy);
    ASSERT_FLOAT_EQ(retrieved.requestedSize, 1.0);
    ASSERT_FLOAT_EQ(retrieved.filledSize, 0.5);
    ASSERT_FLOAT_EQ(retrieved.avgPrice, 50000.0);
    ASSERT_EQ((int)retrieved.status, (int)OrderStatus::PartialFill);
    ASSERT_EQ(retrieved.zorroTradeId, 5);

    g_trading.cleanup();
}

TEST_CASE(get_nonexistent_order_returns_false) {
    g_trading.init();
    OrderState out;
    bool found = g_trading.getOrder(999, out);
    ASSERT_FALSE(found);
    g_trading.cleanup();
}

TEST_CASE(update_order_changes_fields) {
    g_trading.init();

    OrderState state;
    strcpy_s(state.orderId, "OID_456");
    strcpy_s(state.coin, "ETH");
    state.status = OrderStatus::Open;
    state.filledSize = 0.0;
    g_trading.setOrder(10, state);

    // Update
    OrderState updated;
    g_trading.getOrder(10, updated);
    updated.status = OrderStatus::Filled;
    updated.filledSize = 1.5;
    updated.avgPrice = 3200.0;
    g_trading.updateOrder(10, updated);

    // Verify
    OrderState check;
    g_trading.getOrder(10, check);
    ASSERT_EQ((int)check.status, (int)OrderStatus::Filled);
    ASSERT_FLOAT_EQ(check.filledSize, 1.5);
    ASSERT_FLOAT_EQ(check.avgPrice, 3200.0);

    g_trading.cleanup();
}

TEST_CASE(update_nonexistent_order_returns_false) {
    g_trading.init();
    OrderState state;
    state.status = OrderStatus::Filled;
    bool ok = g_trading.updateOrder(999, state);
    ASSERT_FALSE(ok);
    g_trading.cleanup();
}

TEST_CASE(remove_order_makes_it_unfindable) {
    g_trading.init();

    OrderState state;
    strcpy_s(state.coin, "SOL");
    g_trading.setOrder(20, state);

    // Verify it exists
    OrderState out;
    ASSERT_TRUE(g_trading.getOrder(20, out));

    // Remove
    g_trading.removeOrder(20);

    // Verify gone
    ASSERT_FALSE(g_trading.getOrder(20, out));

    g_trading.cleanup();
}

TEST_CASE(multiple_orders_coexist) {
    g_trading.init();

    OrderState btc;
    strcpy_s(btc.coin, "BTC");
    btc.requestedSize = 0.001;
    g_trading.setOrder(100, btc);

    OrderState eth;
    strcpy_s(eth.coin, "ETH");
    eth.requestedSize = 2.0;
    g_trading.setOrder(101, eth);

    OrderState sol;
    strcpy_s(sol.coin, "SOL");
    sol.requestedSize = 50.0;
    g_trading.setOrder(102, sol);

    // All three retrievable
    OrderState out;
    ASSERT_TRUE(g_trading.getOrder(100, out));
    ASSERT_STREQ(out.coin, "BTC");
    ASSERT_TRUE(g_trading.getOrder(101, out));
    ASSERT_STREQ(out.coin, "ETH");
    ASSERT_TRUE(g_trading.getOrder(102, out));
    ASSERT_STREQ(out.coin, "SOL");

    // Remove middle one
    g_trading.removeOrder(101);
    ASSERT_TRUE(g_trading.getOrder(100, out));
    ASSERT_FALSE(g_trading.getOrder(101, out));
    ASSERT_TRUE(g_trading.getOrder(102, out));

    g_trading.cleanup();
}

//=============================================================================
// TEST CASES: determineFilledStatus (inline in hl_types.h)
//=============================================================================

TEST_CASE(filled_status_zero_fill_is_open) {
    ASSERT_EQ((int)determineFilledStatus(0.0, 1.0), (int)OrderStatus::Open);
}

TEST_CASE(filled_status_negative_fill_is_open) {
    ASSERT_EQ((int)determineFilledStatus(-0.5, 1.0), (int)OrderStatus::Open);
}

TEST_CASE(filled_status_exact_fill_is_filled) {
    ASSERT_EQ((int)determineFilledStatus(1.0, 1.0), (int)OrderStatus::Filled);
}

TEST_CASE(filled_status_overfill_is_filled) {
    ASSERT_EQ((int)determineFilledStatus(1.1, 1.0), (int)OrderStatus::Filled);
}

TEST_CASE(filled_status_999_threshold) {
    // 0.999 * 1.0 = 0.999 → anything >= 0.999 is "Filled"
    ASSERT_EQ((int)determineFilledStatus(0.999, 1.0), (int)OrderStatus::Filled);
    ASSERT_EQ((int)determineFilledStatus(0.9989, 1.0), (int)OrderStatus::PartialFill);
}

TEST_CASE(filled_status_partial_fill) {
    ASSERT_EQ((int)determineFilledStatus(0.5, 1.0), (int)OrderStatus::PartialFill);
}

TEST_CASE(filled_status_zero_requested_is_filled) {
    // Edge case: if requestedSize is 0, treat as filled
    ASSERT_EQ((int)determineFilledStatus(0.5, 0.0), (int)OrderStatus::Filled);
}

TEST_CASE(filled_status_btc_rounding) {
    // Real-world: ordered 0.00023, filled 0.000229977 (exchange rounding)
    ASSERT_EQ((int)determineFilledStatus(0.000229977, 0.00023), (int)OrderStatus::Filled);
}

//=============================================================================
// TEST CASES: Fill Status String Parsing
//=============================================================================

TEST_CASE(parse_status_filled) {
    ASSERT_EQ((int)TradingLogic::parseStatusString("filled"), (int)OrderStatus::Filled);
}

TEST_CASE(parse_status_canceled_us_spelling) {
    ASSERT_EQ((int)TradingLogic::parseStatusString("canceled"), (int)OrderStatus::Cancelled);
}

TEST_CASE(parse_status_cancelled_uk_spelling) {
    ASSERT_EQ((int)TradingLogic::parseStatusString("cancelled"), (int)OrderStatus::Cancelled);
}

TEST_CASE(parse_status_sibling_filled_canceled) {
    // OPM-79: bracket order sibling cancellation
    ASSERT_EQ((int)TradingLogic::parseStatusString("siblingFilledCanceled"),
              (int)OrderStatus::Cancelled);
}

TEST_CASE(parse_status_partial) {
    ASSERT_EQ((int)TradingLogic::parseStatusString("partial"), (int)OrderStatus::PartialFill);
}

TEST_CASE(parse_status_error) {
    ASSERT_EQ((int)TradingLogic::parseStatusString("error"), (int)OrderStatus::Error);
}

TEST_CASE(parse_status_null_defaults_to_open) {
    ASSERT_EQ((int)TradingLogic::parseStatusString(nullptr), (int)OrderStatus::Open);
}

TEST_CASE(parse_status_unknown_defaults_to_open) {
    ASSERT_EQ((int)TradingLogic::parseStatusString("something_else"), (int)OrderStatus::Open);
}

//=============================================================================
// MAIN
//=============================================================================

int main() {
    printf("=== OPM-9: Trading Service Tests ===\n\n");

    // CLOID generation & parsing
    RUN_TEST(cloid_format_starts_with_0x);
    RUN_TEST(cloid_length_is_34_chars);
    RUN_TEST(cloid_roundtrip_extracts_trade_id);
    RUN_TEST(cloid_roundtrip_large_trade_id);
    RUN_TEST(cloid_roundtrip_small_ids);
    RUN_TEST(cloid_consecutive_are_unique);
    RUN_TEST(extract_null_returns_zero);
    RUN_TEST(extract_empty_returns_zero);
    RUN_TEST(extract_short_string_returns_zero);
    RUN_TEST(extract_no_prefix_returns_zero);
    RUN_TEST(extract_uppercase_0X_prefix_works);
    RUN_TEST(generate_cloid_null_buffer_no_crash);
    RUN_TEST(generate_cloid_zero_size_no_crash);

    // Trade ID generation
    RUN_TEST(trade_id_starts_at_2);
    RUN_TEST(trade_id_increments_sequentially);
    RUN_TEST(trade_id_never_returns_zero_or_one);

    // Nonce generation
    RUN_TEST(nonce_is_nonzero);
    RUN_TEST(nonce_is_monotonically_increasing);
    RUN_TEST(nonce_no_collisions_in_tight_loop);

    // Order storage & retrieval
    RUN_TEST(store_and_get_order);
    RUN_TEST(get_nonexistent_order_returns_false);
    RUN_TEST(update_order_changes_fields);
    RUN_TEST(update_nonexistent_order_returns_false);
    RUN_TEST(remove_order_makes_it_unfindable);
    RUN_TEST(multiple_orders_coexist);

    // Fill status determination
    RUN_TEST(filled_status_zero_fill_is_open);
    RUN_TEST(filled_status_negative_fill_is_open);
    RUN_TEST(filled_status_exact_fill_is_filled);
    RUN_TEST(filled_status_overfill_is_filled);
    RUN_TEST(filled_status_999_threshold);
    RUN_TEST(filled_status_partial_fill);
    RUN_TEST(filled_status_zero_requested_is_filled);
    RUN_TEST(filled_status_btc_rounding);

    // Fill status string parsing
    RUN_TEST(parse_status_filled);
    RUN_TEST(parse_status_canceled_us_spelling);
    RUN_TEST(parse_status_cancelled_uk_spelling);
    RUN_TEST(parse_status_sibling_filled_canceled);
    RUN_TEST(parse_status_partial);
    RUN_TEST(parse_status_error);
    RUN_TEST(parse_status_null_defaults_to_open);
    RUN_TEST(parse_status_unknown_defaults_to_open);

    return printTestSummary();
}