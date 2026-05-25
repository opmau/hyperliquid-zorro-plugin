//=============================================================================
// test_imported_trades.cpp - IMPORTED trade position tracking
//=============================================================================
// PREVENTS BUGS:
//   - 18c287c — BrokerTrade returned stale import-time data; partial closes
//     (Zorro-driven or external) weren't reflected, causing wrong-size
//     decisions.
//   - OPM-680  — BrokerTrade returned the broker's live aggregate, which
//     double-counted when a same-side BrokerBuy2 created a new tradeID on
//     top of an IMPORTED_ trade (the new fill was reported by BOTH the
//     IMPORTED_ trade AND the new trade, breaking reconcile).
//
// IMPORTED_ TRADE CONTRACT (post-OPM-680):
//   `state.filledSize` records THIS tradeID's *share* of the broker position.
//   On import, share = full live position. BrokerTrade reports the share
//   under one of two regimes:
//
//     - SOLE TRACKER (no other Zorro-visible same-side tradeID on the asset):
//       report the broker's live aggregate and sync state.filledSize to it.
//       External partial closes/liquidations are auto-detected (the live
//       size shrinks → reported size shrinks). Preserves 18c287c semantics.
//
//     - MULTI TRACKER (another Zorro tradeID coexists same-asset same-side,
//       e.g. after a same-side BrokerBuy2 EXTEND): report state.filledSize
//       only. The other tradeID independently tracks its own fill. Sum of
//       shares = live aggregate; no double-counting. Fixes OPM-680.
//
//   Share is kept accurate across regime transitions by:
//     - BrokerBuy2 snapshot: just before extending the asset+side with a new
//       tradeID, capture the IMPORTED_'s share from the pre-extend live
//       position. This locks in any external deltas that occurred before
//       transitioning to multi-tracker mode.
//     - BrokerSell2 hook: on Zorro-driven close of the IMPORTED_, subtract
//       the actual close fill amount from state.filledSize.
//
//   Close/reverse detection always uses live (full close or direction flip
//   → trade returns 0 regardless of share).
//
//   NOT covered: external partial closes that occur after entering multi-
//   tracker mode. The plugin cannot attribute the delta to a specific
//   tradeID. Strategy-level reconcile handles this if needed.
//=============================================================================

#include "../test_framework.h"
#include <cstring>
#include <string>
#include <map>
#include <vector>

using namespace hl::test;

//=============================================================================
// SIMULATED TRADE TRACKING (mirrors hl_broker_trade.cpp post-OPM-680)
//=============================================================================

namespace TradeTracker {

struct ImportedTrade {
    char orderId[64];
    char coin[32];
    bool isBuy;
    double share;            // state.filledSize — this trade's portion
};

// Other Zorro-visible tradeIDs (non-IMPORTED_) that coexist on an asset+side
struct RegularTrade {
    char coin[32];
    bool isBuy;
    double filledSize;       // self-reported fill
};

struct CurrentPosition {
    double size;     // Positive = long, negative = short, 0 = no position
    double entryPx;
};

std::map<std::string, ImportedTrade> importedTrades;
std::map<std::string, RegularTrade> regularTrades;
std::map<std::string, CurrentPosition> currentPositions;

void importTrade(const char* orderId, const char* coin, bool isBuy,
                 double size, double /*price*/) {
    ImportedTrade trade;
    strncpy_s(trade.orderId, orderId, sizeof(trade.orderId) - 1);
    strncpy_s(trade.coin, coin, sizeof(trade.coin) - 1);
    trade.isBuy = isBuy;
    trade.share = size;
    importedTrades[orderId] = trade;
}

void setCurrentPosition(const char* coin, double size, double entryPx) {
    CurrentPosition pos;
    pos.size = size;
    pos.entryPx = entryPx;
    currentPositions[coin] = pos;
}

// Helper used by tests: check if another Zorro-visible same-side tradeID
// exists for this asset (excluding the IMPORTED_ being queried).
bool hasOtherSameSideTracker(const char* coin, bool isBuy) {
    for (const auto& kv : regularTrades) {
        if (strcmp(kv.second.coin, coin) != 0) continue;
        if (kv.second.isBuy != isBuy) continue;
        if (kv.second.filledSize <= 0) continue;
        return true;
    }
    return false;
}

// Simulates BrokerBuy2's same-side extend snapshot AND the resulting new
// regular trade. The IMPORTED_'s share is captured from pre-extend live;
// then the regular trade tracks its own fill; then live is updated.
void brokerBuy2Extend(const char* newOrderId, const char* coin, bool isBuy,
                      double size) {
    // 1. Pre-extend snapshot of any IMPORTED_ same-asset same-side
    auto livePosIt = currentPositions.find(coin);
    double preLive = (livePosIt != currentPositions.end())
        ? ((livePosIt->second.size > 0) ? livePosIt->second.size
                                        : -livePosIt->second.size)
        : 0.0;
    for (auto& kv : importedTrades) {
        if (strcmp(kv.second.coin, coin) != 0) continue;
        if (kv.second.isBuy != isBuy) continue;
        kv.second.share = preLive;
    }

    // 2. Register the new regular trade
    RegularTrade rt;
    strncpy_s(rt.coin, coin, sizeof(rt.coin) - 1);
    rt.isBuy = isBuy;
    rt.filledSize = size;
    regularTrades[newOrderId] = rt;

    // 3. Mirror into live (applyFill)
    if (livePosIt != currentPositions.end()) {
        if (isBuy) livePosIt->second.size += size;
        else       livePosIt->second.size -= size;
    } else {
        CurrentPosition np;
        np.size = isBuy ? size : -size;
        np.entryPx = 0;
        currentPositions[coin] = np;
    }
}

// Simulates BrokerSell2 closing an IMPORTED_ trade: decrement share, mirror
// into live cache. (For closing regular trades the simulation is simpler:
// the regular trade's filledSize stays — Zorro removes it from its list.)
void coverImportedTrade(const char* orderId, double closeAmount) {
    auto it = importedTrades.find(orderId);
    if (it == importedTrades.end()) return;
    ImportedTrade& trade = it->second;

    trade.share -= closeAmount;
    if (trade.share < 0) trade.share = 0;

    auto posIt = currentPositions.find(trade.coin);
    if (posIt != currentPositions.end()) {
        if (trade.isBuy) posIt->second.size -= closeAmount;
        else             posIt->second.size += closeAmount;
    }
}

void reset() {
    importedTrades.clear();
    regularTrades.clear();
    currentPositions.clear();
}

// Implements the post-OPM-680 BrokerTrade contract:
// - close/reverse via live; else
// - sole tracker: live (sync share = live)
// - multi tracker: share
double getImportedTradeSize(const char* orderId) {
    auto it = importedTrades.find(orderId);
    if (it == importedTrades.end()) return 0;
    ImportedTrade& trade = it->second;

    auto posIt = currentPositions.find(trade.coin);
    if (posIt == currentPositions.end()) return 0;
    const CurrentPosition& live = posIt->second;

    if (live.size == 0) return 0;  // fully closed
    bool currentIsLong = live.size > 0;
    if (trade.isBuy != currentIsLong) return 0;  // direction reversed

    bool multi = hasOtherSameSideTracker(trade.coin, trade.isBuy);
    if (!multi) {
        // Sole tracker: live IS this trade's share. Sync.
        double liveAbs = (live.size >= 0) ? live.size : -live.size;
        trade.share = liveAbs;
        return liveAbs;
    }
    // Multi tracker: report share (set by import / snapshot / cover)
    return trade.share;
}

// Pre-OPM-680 buggy implementation: always returned live aggregate.
double getImportedTradeSize_PRE_OPM680(const char* orderId) {
    auto it = importedTrades.find(orderId);
    if (it == importedTrades.end()) return 0;
    const ImportedTrade& trade = it->second;

    auto posIt = currentPositions.find(trade.coin);
    if (posIt == currentPositions.end()) return 0;
    const CurrentPosition& live = posIt->second;

    if (live.size == 0) return 0;
    bool currentIsLong = live.size > 0;
    if (trade.isBuy != currentIsLong) return 0;

    return (live.size >= 0) ? live.size : -live.size;
}

} // namespace TradeTracker

//=============================================================================
// SOLE-TRACKER SCENARIOS (no extend — preserves 18c287c behavior)
//=============================================================================

void test_import_then_position_unchanged() {
    TradeTracker::reset();
    TradeTracker::importTrade("IMPORTED_12345", "BTC", true, 0.5, 40000.0);
    TradeTracker::setCurrentPosition("BTC", 0.5, 40000.0);

    ASSERT_FLOAT_EQ_TOL(TradeTracker::getImportedTradeSize("IMPORTED_12345"), 0.5, 1e-6);
}

void test_import_then_external_partial_close_auto_detected() {
    // Sole tracker: external partial close shrinks live → reported size shrinks.
    // (Preserves 18c287c.)
    TradeTracker::reset();
    TradeTracker::importTrade("IMPORTED_12345", "BTC", true, 0.5, 40000.0);
    TradeTracker::setCurrentPosition("BTC", 0.3, 41000.0);  // external reduce

    ASSERT_FLOAT_EQ_TOL(TradeTracker::getImportedTradeSize("IMPORTED_12345"), 0.3, 1e-6);
}

void test_import_then_zorro_cover_partial() {
    TradeTracker::reset();
    TradeTracker::importTrade("IMPORTED_12345", "BTC", true, 0.5, 40000.0);
    TradeTracker::setCurrentPosition("BTC", 0.5, 40000.0);

    TradeTracker::coverImportedTrade("IMPORTED_12345", 0.2);

    ASSERT_FLOAT_EQ_TOL(TradeTracker::getImportedTradeSize("IMPORTED_12345"), 0.3, 1e-6);
}

void test_import_then_zorro_cover_full() {
    TradeTracker::reset();
    TradeTracker::importTrade("IMPORTED_12345", "BTC", true, 0.5, 40000.0);
    TradeTracker::setCurrentPosition("BTC", 0.5, 40000.0);

    TradeTracker::coverImportedTrade("IMPORTED_12345", 0.5);

    ASSERT_FLOAT_EQ_TOL(TradeTracker::getImportedTradeSize("IMPORTED_12345"), 0.0, 1e-6);
}

void test_import_then_position_closed() {
    TradeTracker::reset();
    TradeTracker::importTrade("IMPORTED_12345", "BTC", true, 0.5, 40000.0);
    TradeTracker::setCurrentPosition("BTC", 0.0, 0.0);

    ASSERT_FLOAT_EQ_TOL(TradeTracker::getImportedTradeSize("IMPORTED_12345"), 0.0, 1e-6);
}

void test_import_then_position_reversed() {
    TradeTracker::reset();
    TradeTracker::importTrade("IMPORTED_12345", "BTC", true, 0.5, 40000.0);
    TradeTracker::setCurrentPosition("BTC", -0.2, 42000.0);

    ASSERT_FLOAT_EQ_TOL(TradeTracker::getImportedTradeSize("IMPORTED_12345"), 0.0, 1e-6);
}

void test_import_short_position() {
    TradeTracker::reset();
    TradeTracker::importTrade("IMPORTED_67890", "BTC", false, 0.3, 45000.0);
    TradeTracker::setCurrentPosition("BTC", -0.3, 45000.0);

    ASSERT_FLOAT_EQ_TOL(TradeTracker::getImportedTradeSize("IMPORTED_67890"), 0.3, 1e-6);
}

void test_import_short_then_reversed_to_long() {
    TradeTracker::reset();
    TradeTracker::importTrade("IMPORTED_67890", "BTC", false, 0.3, 45000.0);
    TradeTracker::setCurrentPosition("BTC", 0.5, 44000.0);

    ASSERT_FLOAT_EQ_TOL(TradeTracker::getImportedTradeSize("IMPORTED_67890"), 0.0, 1e-6);
}

void test_multiple_assets_imported() {
    TradeTracker::reset();
    TradeTracker::importTrade("IMPORTED_BTC", "BTC", true, 0.5, 40000.0);
    TradeTracker::importTrade("IMPORTED_ETH", "ETH", true, 5.0, 2000.0);
    TradeTracker::setCurrentPosition("BTC", 0.5, 40000.0);
    TradeTracker::setCurrentPosition("ETH", 5.0, 2000.0);
    TradeTracker::coverImportedTrade("IMPORTED_BTC", 0.2);

    ASSERT_FLOAT_EQ_TOL(TradeTracker::getImportedTradeSize("IMPORTED_BTC"), 0.3, 1e-6);
    ASSERT_FLOAT_EQ_TOL(TradeTracker::getImportedTradeSize("IMPORTED_ETH"), 5.0, 1e-6);
}

void test_unknown_order_id() {
    TradeTracker::reset();
    ASSERT_FLOAT_EQ_TOL(TradeTracker::getImportedTradeSize("UNKNOWN_ID"), 0.0, 1e-6);
}

//=============================================================================
// MULTI-TRACKER SCENARIOS (OPM-680 regression)
//=============================================================================

void test_OPM680_import_then_same_side_extend() {
    // YOLO BTC scenario: IMPORTED_ 12045 short + new BrokerBuy2 9015 short.
    // Sum must equal 21060 live, not 30075.
    TradeTracker::reset();
    TradeTracker::importTrade("IMPORTED_S00005", "BTC", false, 12045.0, 77916.50);
    TradeTracker::setCurrentPosition("BTC", -12045.0, 77916.50);

    TradeTracker::brokerBuy2Extend("S00032", "BTC", false, 9015.0);

    double importedShare = TradeTracker::getImportedTradeSize("IMPORTED_S00005");
    ASSERT_FLOAT_EQ_TOL(importedShare, 12045.0, 1e-6);

    // Pre-OPM-680 buggy implementation returns the doubled value
    double buggy = TradeTracker::getImportedTradeSize_PRE_OPM680("IMPORTED_S00005");
    ASSERT_FLOAT_EQ_TOL(buggy, 21060.0, 1e-6);
    ASSERT_FLOAT_NE(importedShare, buggy);
}

void test_OPM680_yolo_xrp_scenario() {
    TradeTracker::reset();
    TradeTracker::importTrade("IMPORTED_S00011", "XRP", false, 2927.0, 1.37409);
    TradeTracker::setCurrentPosition("XRP", -2927.0, 1.37409);

    TradeTracker::brokerBuy2Extend("S00042", "XRP", false, 4815.0);

    ASSERT_FLOAT_EQ_TOL(TradeTracker::getImportedTradeSize("IMPORTED_S00011"), 2927.0, 1e-6);
    ASSERT_FLOAT_EQ_TOL(TradeTracker::getImportedTradeSize_PRE_OPM680("IMPORTED_S00011"), 7742.0, 1e-6);
}

void test_OPM680_extend_then_cover_imported() {
    // After EXTEND, partially Cover the IMPORTED_. IMPORTED_'s share drops;
    // extend trade is unaffected. Sum = live.
    TradeTracker::reset();
    TradeTracker::importTrade("IMPORTED_S00005", "BTC", false, 12045.0, 77916.50);
    TradeTracker::setCurrentPosition("BTC", -12045.0, 77916.50);
    TradeTracker::brokerBuy2Extend("S00032", "BTC", false, 9015.0);  // live = -21060

    TradeTracker::coverImportedTrade("IMPORTED_S00005", 5000.0);
    // Live now -16060; IMPORTED_ share = 7045

    ASSERT_FLOAT_EQ_TOL(TradeTracker::getImportedTradeSize("IMPORTED_S00005"), 7045.0, 1e-6);
}

void test_OPM680_extend_then_full_cover_imported() {
    // Fully cover IMPORTED_ while extend trade remains alive.
    TradeTracker::reset();
    TradeTracker::importTrade("IMPORTED_S00005", "BTC", false, 12045.0, 77916.50);
    TradeTracker::setCurrentPosition("BTC", -12045.0, 77916.50);
    TradeTracker::brokerBuy2Extend("S00032", "BTC", false, 9015.0);

    TradeTracker::coverImportedTrade("IMPORTED_S00005", 12045.0);
    // Live now -9015 (the extend); IMPORTED_ share = 0

    ASSERT_FLOAT_EQ_TOL(TradeTracker::getImportedTradeSize("IMPORTED_S00005"), 0.0, 1e-6);
}

void test_OPM680_external_close_before_extend_captured_by_snapshot() {
    // 18c287c + OPM-680 interaction: external partial close happens BEFORE
    // any BrokerTrade poll, then a BrokerBuy2 extends. The pre-extend
    // snapshot must capture the post-external-close share so multi-tracker
    // reporting is accurate without relying on a strategy reconcile.
    TradeTracker::reset();
    TradeTracker::importTrade("IMPORTED_S00005", "BTC", true, 0.5, 40000.0);
    TradeTracker::setCurrentPosition("BTC", 0.5, 40000.0);

    // External partial close (3 lots), without anyone calling BrokerTrade
    TradeTracker::setCurrentPosition("BTC", 0.2, 41000.0);

    // BrokerBuy2 extends with 0.3 → live = 0.5
    TradeTracker::brokerBuy2Extend("S00032", "BTC", true, 0.3);

    // Snapshot captured IMPORTED_'s share as 0.2 (pre-extend live).
    // Multi-tracker now: IMPORTED_ should report 0.2, NOT 0.5.
    ASSERT_FLOAT_EQ_TOL(TradeTracker::getImportedTradeSize("IMPORTED_S00005"), 0.2, 1e-6);
}

//=============================================================================
// MAIN
//=============================================================================

int main() {
    printf("\n");
    printf("=================================================\n");
    printf("  IMPORTED Trade Position Tracking Tests\n");
    printf("  Prevents bugs: 18c287c (stale import data)\n");
    printf("                 OPM-680 (same-side EXTEND double-count)\n");
    printf("=================================================\n\n");

    RUN_TEST(import_then_position_unchanged);
    RUN_TEST(import_then_external_partial_close_auto_detected);
    RUN_TEST(import_then_zorro_cover_partial);
    RUN_TEST(import_then_zorro_cover_full);
    RUN_TEST(import_then_position_closed);
    RUN_TEST(import_then_position_reversed);
    RUN_TEST(import_short_position);
    RUN_TEST(import_short_then_reversed_to_long);
    RUN_TEST(multiple_assets_imported);
    RUN_TEST(unknown_order_id);
    RUN_TEST(OPM680_import_then_same_side_extend);
    RUN_TEST(OPM680_yolo_xrp_scenario);
    RUN_TEST(OPM680_extend_then_cover_imported);
    RUN_TEST(OPM680_extend_then_full_cover_imported);
    RUN_TEST(OPM680_external_close_before_extend_captured_by_snapshot);

    return printTestSummary();
}
