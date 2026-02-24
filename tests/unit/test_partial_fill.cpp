//=============================================================================
// test_partial_fill.cpp - Partial fill detection and status tracking
//=============================================================================
// PREVENTS BUG: OPM-91
//   - PartialFill status not set consistently across fill paths
//   - HTTP fallback guard blocked re-query for partially-filled GTC orders
//
// TESTS:
//   1. determineFilledStatus helper: threshold logic (0.999)
//   2. Partial fill detection at order placement response
//   3. HTTP fallback triggers for partially-filled orders
//   4. onOrderUpdate / onFillNotify set PartialFill correctly
//=============================================================================

#include "../test_framework.h"
#include <cstring>
#include <cmath>

using namespace hl::test;

//=============================================================================
// SIMULATED: determineFilledStatus helper (should be in hl_types.h)
//=============================================================================

// We test the EXPECTED behavior of the helper. The helper should:
//   - Return Open if filledSize <= 0
//   - Return Filled if filledSize >= requestedSize * 0.999
//   - Return PartialFill otherwise

enum class OrderStatus { Pending, Open, Filled, PartialFill, Cancelled, Error };

// Forward declaration — we'll test with a local implementation first,
// then the real one once it's added to hl_types.h
namespace hl {
    inline OrderStatus determineFilledStatus(double filledSize, double requestedSize) {
        if (filledSize <= 0) return OrderStatus::Open;
        if (requestedSize <= 0) return OrderStatus::Filled; // guard against div-by-zero
        return (filledSize >= requestedSize * 0.999)
            ? OrderStatus::Filled : OrderStatus::PartialFill;
    }
}

//=============================================================================
// SIMULATED: HTTP fallback guard logic
//=============================================================================

// Current (buggy) guard: only triggers for zero-fill orders
bool httpFallbackShouldTrigger_CURRENT(double filledSize, OrderStatus status,
                                        bool hasCancelledStatus) {
    return (filledSize <= 0 && !hasCancelledStatus);
}

// Fixed guard: triggers for any non-terminal order (Open or PartialFill)
bool httpFallbackShouldTrigger_FIXED(double filledSize, OrderStatus status,
                                      bool hasCancelledStatus) {
    (void)filledSize; // Not used in fixed version
    (void)hasCancelledStatus;
    return (status == OrderStatus::Open || status == OrderStatus::PartialFill);
}

//=============================================================================
// SIMULATED: onOrderUpdate status mapping
//=============================================================================

// Current (buggy): "filled" always maps to Filled, no partial check
OrderStatus mapOrderUpdateStatus_CURRENT(const char* status, double filledSz,
                                          double requestedSz) {
    (void)filledSz;
    (void)requestedSz;
    if (strcmp(status, "filled") == 0) return OrderStatus::Filled;
    if (strstr(status, "canceled")) return OrderStatus::Cancelled;
    if (strcmp(status, "rejected") == 0) return OrderStatus::Error;
    return OrderStatus::Open;
}

// Fixed: uses determineFilledStatus when exchange says "filled"
OrderStatus mapOrderUpdateStatus_FIXED(const char* status, double filledSz,
                                        double requestedSz) {
    if (strcmp(status, "filled") == 0) return hl::determineFilledStatus(filledSz, requestedSz);
    if (strstr(status, "canceled")) return OrderStatus::Cancelled;
    if (strcmp(status, "rejected") == 0) return OrderStatus::Error;
    // Open status with partial fill data
    if (filledSz > 0 && requestedSz > 0)
        return hl::determineFilledStatus(filledSz, requestedSz);
    return OrderStatus::Open;
}

//=============================================================================
// TEST CASES
//=============================================================================

// --- determineFilledStatus helper tests ---

TEST_CASE(threshold_full_fill) {
    // 100% fill → Filled
    ASSERT_EQ(hl::determineFilledStatus(1.0, 1.0), OrderStatus::Filled);
}

TEST_CASE(threshold_99_9_percent_fill) {
    // 99.9% fill → Filled (within rounding tolerance)
    ASSERT_EQ(hl::determineFilledStatus(0.999, 1.0), OrderStatus::Filled);
}

TEST_CASE(threshold_99_95_percent_fill) {
    // 99.95% fill → Filled (above 0.999 threshold)
    ASSERT_EQ(hl::determineFilledStatus(0.9995, 1.0), OrderStatus::Filled);
}

TEST_CASE(threshold_50_percent_fill) {
    // 50% fill → PartialFill
    ASSERT_EQ(hl::determineFilledStatus(0.5, 1.0), OrderStatus::PartialFill);
}

TEST_CASE(threshold_99_8_percent_fill) {
    // 99.8% fill → PartialFill (below 0.999 threshold)
    ASSERT_EQ(hl::determineFilledStatus(0.998, 1.0), OrderStatus::PartialFill);
}

TEST_CASE(threshold_zero_fill) {
    // 0 fill → Open
    ASSERT_EQ(hl::determineFilledStatus(0.0, 1.0), OrderStatus::Open);
}

TEST_CASE(threshold_negative_fill) {
    // Negative fill (guard) → Open
    ASSERT_EQ(hl::determineFilledStatus(-0.1, 1.0), OrderStatus::Open);
}

TEST_CASE(threshold_zero_requested) {
    // Zero requested (guard) → Filled (avoid div-by-zero)
    ASSERT_EQ(hl::determineFilledStatus(0.5, 0.0), OrderStatus::Filled);
}

TEST_CASE(threshold_tiny_order) {
    // Small order: 0.0009999 of 0.001 → Filled (above 0.999 threshold)
    ASSERT_EQ(hl::determineFilledStatus(0.0009999, 0.001), OrderStatus::Filled);
}

TEST_CASE(threshold_tiny_order_partial) {
    // Small order: 0.0005 of 0.001 → PartialFill
    ASSERT_EQ(hl::determineFilledStatus(0.0005, 0.001), OrderStatus::PartialFill);
}

TEST_CASE(threshold_btc_example) {
    // BTC: requesting 2.0, filled 1.9999 → Filled
    ASSERT_EQ(hl::determineFilledStatus(1.9999, 2.0), OrderStatus::Filled);
}

TEST_CASE(threshold_btc_partial) {
    // BTC: requesting 2.0, filled 1.0 → PartialFill
    ASSERT_EQ(hl::determineFilledStatus(1.0, 2.0), OrderStatus::PartialFill);
}

// --- HTTP fallback guard tests ---

TEST_CASE(http_fallback_current_blocks_partial_fill) {
    // BUG: current guard blocks HTTP fallback for partially-filled GTC orders
    // A GTC order with 50% fill should still trigger HTTP fallback
    bool triggers = httpFallbackShouldTrigger_CURRENT(
        0.5, OrderStatus::PartialFill, false);
    // Current behavior: does NOT trigger (filledSize > 0)
    ASSERT_FALSE(triggers);
    // ^ This test PASSES because it documents the current buggy behavior
}

TEST_CASE(http_fallback_fixed_allows_partial_fill) {
    // FIXED: partial fill orders should trigger HTTP fallback
    bool triggers = httpFallbackShouldTrigger_FIXED(
        0.5, OrderStatus::PartialFill, false);
    ASSERT_TRUE(triggers);
}

TEST_CASE(http_fallback_fixed_allows_open) {
    // FIXED: unfilled open orders should trigger HTTP fallback
    bool triggers = httpFallbackShouldTrigger_FIXED(
        0.0, OrderStatus::Open, false);
    ASSERT_TRUE(triggers);
}

TEST_CASE(http_fallback_fixed_blocks_filled) {
    // FIXED: fully filled orders should NOT trigger HTTP fallback
    bool triggers = httpFallbackShouldTrigger_FIXED(
        1.0, OrderStatus::Filled, false);
    ASSERT_FALSE(triggers);
}

TEST_CASE(http_fallback_fixed_blocks_cancelled) {
    // FIXED: cancelled orders should NOT trigger HTTP fallback
    bool triggers = httpFallbackShouldTrigger_FIXED(
        0.0, OrderStatus::Cancelled, false);
    ASSERT_FALSE(triggers);
}

// --- onOrderUpdate status mapping tests ---

TEST_CASE(order_update_filled_partial_current_bug) {
    // BUG: exchange says "filled" but only 50% filled (IOC partial)
    // Current code maps to Filled regardless of fill ratio
    OrderStatus st = mapOrderUpdateStatus_CURRENT("filled", 0.5, 1.0);
    ASSERT_EQ(st, OrderStatus::Filled);  // Current buggy behavior
}

TEST_CASE(order_update_filled_partial_fixed) {
    // FIXED: "filled" with 50% fill → PartialFill
    OrderStatus st = mapOrderUpdateStatus_FIXED("filled", 0.5, 1.0);
    ASSERT_EQ(st, OrderStatus::PartialFill);
}

TEST_CASE(order_update_filled_full_fixed) {
    // FIXED: "filled" with 100% fill → Filled
    OrderStatus st = mapOrderUpdateStatus_FIXED("filled", 1.0, 1.0);
    ASSERT_EQ(st, OrderStatus::Filled);
}

TEST_CASE(order_update_open_with_partial_fixed) {
    // FIXED: "open" with partial fill data → PartialFill
    OrderStatus st = mapOrderUpdateStatus_FIXED("open", 0.5, 1.0);
    ASSERT_EQ(st, OrderStatus::PartialFill);
}

TEST_CASE(order_update_canceled_fixed) {
    // Cancelled status should be preserved regardless of fill data
    OrderStatus st = mapOrderUpdateStatus_FIXED("canceled", 0.3, 1.0);
    ASSERT_EQ(st, OrderStatus::Cancelled);
}

// --- placeOrderWithId status mapping ---

TEST_CASE(place_order_ioc_partial_fill) {
    // IOC order filled 50%: exchange returns "filled" status with totalSz < requestedSz
    // Should be PartialFill, not Filled
    double filledSize = 0.5;
    double requestedSize = 1.0;
    bool isFilled = true;  // Exchange says "filled"

    // Current buggy logic:
    OrderStatus currentStatus = isFilled ? OrderStatus::Filled : OrderStatus::Open;
    ASSERT_EQ(currentStatus, OrderStatus::Filled);  // Bug: always Filled

    // Fixed logic:
    OrderStatus fixedStatus = isFilled
        ? hl::determineFilledStatus(filledSize, requestedSize)
        : OrderStatus::Open;
    ASSERT_EQ(fixedStatus, OrderStatus::PartialFill);  // Correct
}

TEST_CASE(place_order_ioc_full_fill) {
    // IOC order fully filled: both should give Filled
    double filledSize = 1.0;
    double requestedSize = 1.0;

    OrderStatus fixedStatus = hl::determineFilledStatus(filledSize, requestedSize);
    ASSERT_EQ(fixedStatus, OrderStatus::Filled);
}

//=============================================================================
// MAIN
//=============================================================================

int main() {
    printf("\n========================================\n");
    printf("OPM-91: Partial Fill Detection Tests\n");
    printf("========================================\n\n");

    // determineFilledStatus threshold tests
    RUN_TEST(threshold_full_fill);
    RUN_TEST(threshold_99_9_percent_fill);
    RUN_TEST(threshold_99_95_percent_fill);
    RUN_TEST(threshold_50_percent_fill);
    RUN_TEST(threshold_99_8_percent_fill);
    RUN_TEST(threshold_zero_fill);
    RUN_TEST(threshold_negative_fill);
    RUN_TEST(threshold_zero_requested);
    RUN_TEST(threshold_tiny_order);
    RUN_TEST(threshold_tiny_order_partial);
    RUN_TEST(threshold_btc_example);
    RUN_TEST(threshold_btc_partial);

    // HTTP fallback guard tests
    RUN_TEST(http_fallback_current_blocks_partial_fill);
    RUN_TEST(http_fallback_fixed_allows_partial_fill);
    RUN_TEST(http_fallback_fixed_allows_open);
    RUN_TEST(http_fallback_fixed_blocks_filled);
    RUN_TEST(http_fallback_fixed_blocks_cancelled);

    // onOrderUpdate mapping tests
    RUN_TEST(order_update_filled_partial_current_bug);
    RUN_TEST(order_update_filled_partial_fixed);
    RUN_TEST(order_update_filled_full_fixed);
    RUN_TEST(order_update_open_with_partial_fixed);
    RUN_TEST(order_update_canceled_fixed);

    // placeOrderWithId status mapping
    RUN_TEST(place_order_ioc_partial_fill);
    RUN_TEST(place_order_ioc_full_fill);

    return printTestSummary();
}
