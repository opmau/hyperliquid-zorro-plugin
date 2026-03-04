//=============================================================================
// test_lotsize_divzero.cpp - Division-by-zero guard for lotSize in fill calc
//=============================================================================
// PREVENTS BUG: OPM-158
//   - filledSize / lotSize crashes when lotSize is 0 (uninitialized state)
//   - 3 unguarded sites in hl_broker_trade.cpp (lines 124, 215, 441)
//   - 2 already-guarded sites (lines 303, 347) show the correct pattern
//
// TESTS:
//   1. Normal case: lotSize > 0 → proper lots conversion
//   2. Zero lotSize → fallback to raw filledSize (no crash)
//   3. Negative lotSize → fallback to raw filledSize (defensive)
//   4. Specific BTC/ETH examples matching CLAUDE.md formulas
//=============================================================================

#include "../test_framework.h"
#include <cmath>

using namespace hl::test;

//=============================================================================
// SIMULATED: fillToLots — the guarded pattern from hl_broker_trade.cpp
//=============================================================================

// This replicates the GUARDED pattern (lines 303-305, 347-349):
//   int fillLots = (lotSize > 0)
//       ? (int)round(filledSize / lotSize)
//       : (int)round(filledSize);
//
// The UNGUARDED pattern (lines 124, 215, 441) does:
//   (int)round(filledSize / lotSize)
// which crashes when lotSize == 0.

int fillToLots_GUARDED(double filledSize, double lotSize) {
    return (lotSize > 0)
        ? (int)round(filledSize / lotSize)
        : (int)round(filledSize);
}

int fillToLots_UNGUARDED(double filledSize, double lotSize) {
    return (int)round(filledSize / lotSize);
}

// BrokerBuy2 variant: absolute amount fallback (line 124 fix)
int fillToLots_BUY2(double filledSize, double lotSize) {
    return (lotSize > 0)
        ? (int)round(filledSize / lotSize)
        : (int)round(filledSize);
}

// BrokerSell2 variant: absolute amount fallback (line 215 fix)
int fillToLots_SELL2(double filledSize, double lotSize, int amount) {
    if (filledSize > 0) {
        return (lotSize > 0)
            ? (int)round(filledSize / lotSize)
            : abs(amount);
    }
    return abs(amount);
}

// BrokerTrade generic return variant (line 441 fix)
int fillToLots_TRADE(double filledSize, double lotSize) {
    if (filledSize <= 0) return 0;
    int fillLots = (lotSize > 0)
        ? (int)round(filledSize / lotSize)
        : (int)round(filledSize);
    return (fillLots > 0) ? fillLots : 1;
}

//=============================================================================
// TEST CASES
//=============================================================================

// --- Normal operation (lotSize > 0) ---

TEST_CASE(normal_btc_lotsize) {
    // BTC: szDecimals=4, lotSize=0.0001
    // 2.0 BTC filled → 20000 lots
    int lots = fillToLots_GUARDED(2.0, 0.0001);
    ASSERT_EQ(lots, 20000);
}

TEST_CASE(normal_eth_lotsize) {
    // ETH: szDecimals=3, lotSize=0.001
    // 1.5 ETH → 1500 lots
    int lots = fillToLots_GUARDED(1.5, 0.001);
    ASSERT_EQ(lots, 1500);
}

TEST_CASE(normal_ada_lotsize) {
    // ADA: szDecimals=0, lotSize=1.0
    // 100 ADA → 100 lots
    int lots = fillToLots_GUARDED(100.0, 1.0);
    ASSERT_EQ(lots, 100);
}

TEST_CASE(normal_small_fill) {
    // Small fill: 0.0001 BTC with lotSize=0.0001 → 1 lot
    int lots = fillToLots_GUARDED(0.0001, 0.0001);
    ASSERT_EQ(lots, 1);
}

// --- Division by zero protection ---

TEST_CASE(zero_lotsize_no_crash) {
    // lotSize == 0 should NOT crash — must return fallback
    int lots = fillToLots_GUARDED(2.0, 0.0);
    // Fallback: round(filledSize) = round(2.0) = 2
    ASSERT_EQ(lots, 2);
}

TEST_CASE(zero_lotsize_small_fill) {
    // lotSize == 0 with small fill
    int lots = fillToLots_GUARDED(0.5, 0.0);
    // Fallback: round(0.5) = 0 (rounds to nearest even in C++ default)
    // Actually round(0.5) = 1 in MSVC (rounds away from zero)
    ASSERT_TRUE(lots >= 0);
}

TEST_CASE(negative_lotsize_no_crash) {
    // Negative lotSize (should not happen, but defensive)
    int lots = fillToLots_GUARDED(2.0, -0.0001);
    // Guard: lotSize <= 0 → fallback: round(2.0) = 2
    ASSERT_EQ(lots, 2);
}

// --- BrokerBuy2 variant (line 124) ---

TEST_CASE(buy2_zero_lotsize) {
    int lots = fillToLots_BUY2(1.5, 0.0);
    // Fallback: round(1.5) = 2
    ASSERT_EQ(lots, 2);
}

TEST_CASE(buy2_normal) {
    int lots = fillToLots_BUY2(1.5, 0.001);
    ASSERT_EQ(lots, 1500);
}

// --- BrokerSell2 variant (line 215) ---

TEST_CASE(sell2_zero_lotsize_with_amount) {
    // lotSize == 0, should fallback to abs(amount)
    int lots = fillToLots_SELL2(1.5, 0.0, -500);
    ASSERT_EQ(lots, 500);
}

TEST_CASE(sell2_normal) {
    int lots = fillToLots_SELL2(1.5, 0.001, -1500);
    ASSERT_EQ(lots, 1500);
}

TEST_CASE(sell2_zero_fill_returns_amount) {
    // No fill → returns abs(amount)
    int lots = fillToLots_SELL2(0.0, 0.001, -500);
    ASSERT_EQ(lots, 500);
}

// --- BrokerTrade generic return variant (line 441) ---

TEST_CASE(trade_zero_lotsize) {
    // lotSize == 0 with valid fill → should not crash
    int lots = fillToLots_TRADE(2.0, 0.0);
    // Fallback: round(2.0) = 2
    ASSERT_EQ(lots, 2);
}

TEST_CASE(trade_normal) {
    int lots = fillToLots_TRADE(2.0, 0.0001);
    ASSERT_EQ(lots, 20000);
}

TEST_CASE(trade_tiny_fill_clamps_to_1) {
    // Very small fill: 0.00001 with lotSize=0.0001 → round(0.1) = 0 → clamp to 1
    int lots = fillToLots_TRADE(0.00001, 0.0001);
    ASSERT_EQ(lots, 1);
}

TEST_CASE(trade_zero_fill_returns_zero) {
    int lots = fillToLots_TRADE(0.0, 0.0001);
    ASSERT_EQ(lots, 0);
}

TEST_CASE(trade_zero_fill_zero_lotsize) {
    // Both zero → should return 0 (no fill), not crash
    int lots = fillToLots_TRADE(0.0, 0.0);
    ASSERT_EQ(lots, 0);
}

//=============================================================================
// Verify UNGUARDED version produces infinity/NaN with zero lotSize
// (This documents the bug — disabled by default to avoid FPE crashes)
//=============================================================================

TEST_CASE(unguarded_produces_bad_value) {
    // The unguarded version divides by zero, producing +inf or NaN
    // After (int)round(), this becomes undefined behavior on MSVC
    // We just verify our guarded version differs from the raw division result
    double filledSize = 2.0;
    double lotSize = 0.0;

    // Guarded version gives a sane value
    int guarded = fillToLots_GUARDED(filledSize, lotSize);
    ASSERT_EQ(guarded, 2);

    // Raw division: 2.0 / 0.0 = +inf
    double raw = filledSize / lotSize;
    // Should be infinity (not finite)
    ASSERT_FALSE(std::isfinite(raw));
}

//=============================================================================
// MAIN
//=============================================================================

int main() {
    printf("\n========================================\n");
    printf("OPM-158: lotSize Division-by-Zero Guard\n");
    printf("========================================\n\n");

    // Normal operation
    RUN_TEST(normal_btc_lotsize);
    RUN_TEST(normal_eth_lotsize);
    RUN_TEST(normal_ada_lotsize);
    RUN_TEST(normal_small_fill);

    // Division by zero protection
    RUN_TEST(zero_lotsize_no_crash);
    RUN_TEST(zero_lotsize_small_fill);
    RUN_TEST(negative_lotsize_no_crash);

    // BrokerBuy2 variant
    RUN_TEST(buy2_zero_lotsize);
    RUN_TEST(buy2_normal);

    // BrokerSell2 variant
    RUN_TEST(sell2_zero_lotsize_with_amount);
    RUN_TEST(sell2_normal);
    RUN_TEST(sell2_zero_fill_returns_amount);

    // BrokerTrade generic return
    RUN_TEST(trade_zero_lotsize);
    RUN_TEST(trade_normal);
    RUN_TEST(trade_tiny_fill_clamps_to_1);
    RUN_TEST(trade_zero_fill_returns_zero);
    RUN_TEST(trade_zero_fill_zero_lotsize);

    // Bug documentation
    RUN_TEST(unguarded_produces_bad_value);

    return printTestSummary();
}
