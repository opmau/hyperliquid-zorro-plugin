//=============================================================================
// test_get_position_after_fill.cpp - GET_POSITION after order fill
//=============================================================================
// PREVENTS BUG: OPM-85
//   - GET_POSITION returns 0 immediately after BrokerBuy2 confirms a fill
//   - Root cause: WS clearinghouseState hasn't arrived yet, nothing bridges
//     the fill response → position cache gap
//
// SCENARIO (from smoke test log):
//   1. BrokerBuy2 places BTC order → exchange confirms fill (0.00023 @ 67978)
//   2. Zorro immediately calls GET_POSITION("BTC")
//   3. BUG: returns 0.0 (WS cache empty, no bridge from fill)
//   4. FIX: applyFill() writes to cache immediately after fill
//
// TESTS:
//   1. Position is 0 before any fill (baseline)
//   2. After applyFill, position reflects fill size and entry price
//   3. Second fill on same coin merges with existing position
//   4. Sell fill on existing long reduces position
//   5. WS update overwrites synthetic fill data (WS is authoritative)
//=============================================================================

#include "../test_framework.h"
#include "../../src/transport/ws_price_cache.h"
#include <cstring>
#include <cmath>

using namespace hl::test;
using namespace hl::ws;

//=============================================================================
// FUNCTION UNDER TEST: applyFill
// Bridges the fill response → position cache gap.
// In production, this lives in hl_account_service.cpp.
// Here we test the logic in isolation against a real PriceCache.
//=============================================================================

namespace FillBridge {

/// Apply a fill to the position cache so GET_POSITION sees it immediately.
/// Handles both new positions and adding to existing ones.
///
/// @param cache    The PriceCache to update
/// @param coin     Asset name (e.g., "BTC")
/// @param fillSize Absolute size filled (always positive)
/// @param fillPx   Average fill price
/// @param isBuy    true=long, false=short
void applyFill(PriceCache& cache, const char* coin, double fillSize,
               double fillPx, bool isBuy) {
    if (!coin || fillSize <= 0) return;

    double signedFill = isBuy ? fillSize : -fillSize;

    PositionData existing = cache.getPosition(std::string(coin));

    if (existing.size == 0) {
        // New position
        PositionData pos;
        pos.coin = coin;
        pos.size = signedFill;
        pos.entryPx = fillPx;
        cache.setPosition(pos);
    } else {
        bool sameDirection = (existing.size > 0) == isBuy;

        if (sameDirection) {
            // Adding to position: weighted average entry price
            double oldNotional = fabs(existing.size) * existing.entryPx;
            double newNotional = fillSize * fillPx;
            double totalSize = fabs(existing.size) + fillSize;
            double avgEntry = (oldNotional + newNotional) / totalSize;

            PositionData pos = existing;
            pos.size = (existing.size > 0) ? totalSize : -totalSize;
            pos.entryPx = avgEntry;
            cache.setPosition(pos);
        } else {
            // Reducing position: keep original entry price
            double remaining = fabs(existing.size) - fillSize;

            if (remaining <= 1e-12) {
                // Position fully closed
                PositionData pos;
                pos.coin = coin;
                pos.size = 0;
                pos.entryPx = 0;
                cache.setPosition(pos);
            } else {
                PositionData pos = existing;
                pos.size = (existing.size > 0) ? remaining : -remaining;
                cache.setPosition(pos);
            }
        }
    }
}

} // namespace FillBridge

//=============================================================================
// TEST CASES
//=============================================================================

TEST_CASE(baseline_no_position) {
    PriceCache cache;
    PositionData pos = cache.getPosition("BTC");
    ASSERT_FLOAT_EQ(pos.size, 0.0);
    ASSERT_FLOAT_EQ(pos.entryPx, 0.0);
}

TEST_CASE(get_position_nonzero_after_buy_fill) {
    // Reproduces OPM-85: BrokerBuy2 fills 0.00023 BTC @ 67978
    PriceCache cache;

    FillBridge::applyFill(cache, "BTC", 0.00023, 67978.0, true);

    PositionData pos = cache.getPosition("BTC");
    ASSERT_FLOAT_EQ_TOL(pos.size, 0.00023, 1e-10);
    ASSERT_FLOAT_EQ_TOL(pos.entryPx, 67978.0, 0.01);
}

TEST_CASE(get_position_nonzero_after_sell_fill) {
    PriceCache cache;

    FillBridge::applyFill(cache, "ETH", 1.5, 3200.0, false);

    PositionData pos = cache.getPosition("ETH");
    ASSERT_FLOAT_EQ_TOL(pos.size, -1.5, 1e-10);  // Short = negative
    ASSERT_FLOAT_EQ_TOL(pos.entryPx, 3200.0, 0.01);
}

TEST_CASE(second_fill_merges_with_existing_position) {
    // User buys 0.001 BTC, then buys another 0.002 BTC
    PriceCache cache;

    FillBridge::applyFill(cache, "BTC", 0.001, 60000.0, true);
    FillBridge::applyFill(cache, "BTC", 0.002, 61000.0, true);

    PositionData pos = cache.getPosition("BTC");
    // Total size: 0.001 + 0.002 = 0.003
    ASSERT_FLOAT_EQ_TOL(pos.size, 0.003, 1e-10);
    // Weighted average entry: (0.001*60000 + 0.002*61000) / 0.003 = 60666.67
    double expectedEntry = (0.001 * 60000.0 + 0.002 * 61000.0) / 0.003;
    ASSERT_FLOAT_EQ_TOL(pos.entryPx, expectedEntry, 0.01);
}

TEST_CASE(sell_fill_reduces_long_position) {
    // User has 0.005 BTC long, sells 0.002
    PriceCache cache;

    // Seed initial position
    FillBridge::applyFill(cache, "BTC", 0.005, 65000.0, true);

    // Partial close (sell)
    FillBridge::applyFill(cache, "BTC", 0.002, 66000.0, false);

    PositionData pos = cache.getPosition("BTC");
    // Remaining: 0.005 - 0.002 = 0.003 (still long)
    ASSERT_FLOAT_EQ_TOL(pos.size, 0.003, 1e-10);
    // Entry price stays at original (exchange doesn't change entry on partial close)
    ASSERT_FLOAT_EQ_TOL(pos.entryPx, 65000.0, 0.01);
}

TEST_CASE(ws_update_overwrites_synthetic_fill) {
    // After applyFill, a WS clearinghouseState arrives with authoritative data
    PriceCache cache;

    // Synthetic fill from BrokerBuy2
    FillBridge::applyFill(cache, "BTC", 0.00023, 67978.0, true);

    // WS update arrives (slightly different due to fees, etc.)
    PositionData wsPos;
    wsPos.coin = "BTC";
    wsPos.size = 0.00023;
    wsPos.entryPx = 67978.5;  // Slightly different (exchange rounding)
    wsPos.unrealizedPnl = 0.01;
    wsPos.leverage = 5.0;
    cache.setPosition(wsPos);

    // WS data is now authoritative
    PositionData pos = cache.getPosition("BTC");
    ASSERT_FLOAT_EQ_TOL(pos.entryPx, 67978.5, 0.01);
    ASSERT_FLOAT_EQ_TOL(pos.leverage, 5.0, 0.01);
}

//=============================================================================
// MAIN
//=============================================================================

int main() {
    printf("=== OPM-85: GET_POSITION after fill ===\n\n");

    RUN_TEST(baseline_no_position);
    RUN_TEST(get_position_nonzero_after_buy_fill);
    RUN_TEST(get_position_nonzero_after_sell_fill);
    RUN_TEST(second_fill_merges_with_existing_position);
    RUN_TEST(sell_fill_reduces_long_position);
    RUN_TEST(ws_update_overwrites_synthetic_fill);

    return printTestSummary();
}