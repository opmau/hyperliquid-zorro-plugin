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
    double filledSize;       // entry order's cumulative fill
    double closedSize;       // [OPM-733] Zorro-driven closes; reported net
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
        // [OPM-733] Net open size — a sibling fully closed by Zorro no
        // longer counts as a tracker (mirrors hasOtherSameSideTracker in
        // hl_broker_trade.cpp)
        if (kv.second.filledSize - kv.second.closedSize <= 0) continue;
        return true;
    }
    return false;
}

// Simulates BrokerBuy2's same-side extend snapshot AND the resulting new
// regular trade. The IMPORTED_'s share is captured from pre-extend live;
// then the regular trade tracks its own fill; then live is updated.
void brokerBuy2Extend(const char* newOrderId, const char* coin, bool isBuy,
                      double size) {
    // 1. Pre-extend snapshot of any IMPORTED_ same-asset same-side.
    //    [OPM-733] Only on the sole->multi transition: once another tradeID
    //    already tracks part of the position, live includes that sibling's
    //    fill and snapshotting would absorb it into the IMPORTED_ share.
    if (!hasOtherSameSideTracker(coin, isBuy)) {
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
    }

    // 2. Register the new regular trade
    RegularTrade rt;
    strncpy_s(rt.coin, coin, sizeof(rt.coin) - 1);
    rt.isBuy = isBuy;
    rt.filledSize = size;
    rt.closedSize = 0;
    regularTrades[newOrderId] = rt;

    // 3. Mirror into live (applyFill)
    auto posIt = currentPositions.find(coin);
    if (posIt != currentPositions.end()) {
        if (isBuy) posIt->second.size += size;
        else       posIt->second.size -= size;
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

// Simulates BrokerSell2 partially closing a REGULAR (mapped) trade: the
// reduce-only close order fills and the live position shrinks. Mirrors
// hl_broker_trade.cpp's per-trade close accounting for mapped trades.
void coverRegularTrade(const char* orderId, double closeAmount) {
    auto it = regularTrades.find(orderId);
    if (it == regularTrades.end()) return;
    RegularTrade& trade = it->second;

    // [OPM-733] Mirrors recordZorroClose: mapped trades accumulate closes in
    // closedSize (filledSize stays = entry order's cumulative fill, since
    // WS/HTTP lookups re-derive it from the exchange).
    trade.closedSize += closeAmount;

    auto posIt = currentPositions.find(trade.coin);
    if (posIt != currentPositions.end()) {
        if (trade.isBuy) posIt->second.size -= closeAmount;
        else             posIt->second.size += closeAmount;
    }
}

// Implements BrokerTrade's generic return path for REGULAR (mapped) trades:
// the value Zorro's automatic fill-poll writes into its ledger.
// [OPM-733] Net open amount = entry fill minus Zorro-driven closes.
double getRegularTradeSize(const char* orderId) {
    auto it = regularTrades.find(orderId);
    if (it == regularTrades.end()) return 0;
    double net = it->second.filledSize - it->second.closedSize;
    return (net > 0) ? net : 0;
}

// [OPM-733] Mirrors BrokerTrade's net-open LOT return (hl_broker_trade.cpp
// generic path). Returns the integer lot count Zorro writes into its ledger.
// A fully-closed mapped trade whose net is a tiny positive float residual
// (filledSize re-derived from the exchange as e.g. 0.1+0.2 vs closedSize 0.3)
// rounds to 0 lots but must NOT be floored up to a phantom 1-lot trade — that
// fails reconcile, the very symptom OPM-733 exists to prevent.
int getRegularTradeLots(const char* orderId, double lotSize) {
    auto it = regularTrades.find(orderId);
    if (it == regularTrades.end()) return 0;
    double openSize = it->second.filledSize - it->second.closedSize;
    double lotEps = (lotSize > 0) ? lotSize * 0.5 : 1e-9;
    if (openSize < lotEps) openSize = 0;  // fully closed within sub-lot residual
    if (openSize > 0) {
        int fillLots = (lotSize > 0)
            ? (int)(openSize / lotSize + 0.5)
            : (int)(openSize + 0.5);
        return (fillLots > 0) ? fillLots : 1;
    }
    return 0;
}

// Pre-fix buggy mirror: no sub-lot epsilon, legacy `:1` floor. A positive
// float residual after a full close returns a phantom 1 lot.
int getRegularTradeLots_PRE_OPM733_EPS(const char* orderId, double lotSize) {
    auto it = regularTrades.find(orderId);
    if (it == regularTrades.end()) return 0;
    double openSize = it->second.filledSize - it->second.closedSize;
    if (openSize < 0) openSize = 0;
    if (openSize > 0) {
        int fillLots = (lotSize > 0)
            ? (int)(openSize / lotSize + 0.5)
            : (int)(openSize + 0.5);
        return (fillLots > 0) ? fillLots : 1;
    }
    return 0;
}

// Inject exact filled/closed values to model float residuals that arise when
// filledSize is re-derived from the exchange (sum of fills) while closedSize
// accumulates separately. [OPM-733 hardening]
void setRegularRaw(const char* orderId, const char* coin, bool isBuy,
                   double filledSize, double closedSize) {
    RegularTrade rt;
    strncpy_s(rt.coin, coin, sizeof(rt.coin) - 1);
    rt.isBuy = isBuy;
    rt.filledSize = filledSize;
    rt.closedSize = closedSize;
    regularTrades[orderId] = rt;
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
// OPM-733 SCENARIOS (BrokerTrade fill-poll corrupts Zorro ledger)
//=============================================================================

void test_OPM733_double_extend_keeps_imported_share() {
    // OPM-733 incident 2 (ADA, YOLO V2 live log 2026-06-08): bootstrap import,
    // then same-side EXTENDs on two consecutive daily rebalances. The 2nd
    // extend's pre-extend snapshot ran while ALREADY in multi-tracker mode and
    // absorbed extend #1's fill into the IMPORTED_ share (81910 -> 87741).
    // Zorro's fill-poll then inflated its ledger by exactly 5831, failing the
    // strategy reconcile ($985 > $500 HALT) on every subsequent rebalance.
    // The snapshot must only run on the sole->multi transition.
    TradeTracker::reset();
    TradeTracker::importTrade("IMPORTED_S00002", "ADA", false, 81910.0, 0.222316);
    TradeTracker::setCurrentPosition("ADA", -81910.0, 0.222316);

    TradeTracker::brokerBuy2Extend("S00035", "ADA", false, 5831.0);   // live -87741
    TradeTracker::brokerBuy2Extend("S00084", "ADA", false, 11556.0);  // live -99297

    double importedShare = TradeTracker::getImportedTradeSize("IMPORTED_S00002");
    ASSERT_FLOAT_EQ_TOL(importedShare, 81910.0, 1e-6);

    // Ledger sum (imported share + both extends) must equal live aggregate
    double ledgerSum = importedShare + 5831.0 + 11556.0;
    ASSERT_FLOAT_EQ_TOL(ledgerSum, 99297.0, 1e-6);
}

void test_OPM733_triple_extend_keeps_imported_share() {
    // Share must stay pinned across ANY number of subsequent extends.
    TradeTracker::reset();
    TradeTracker::importTrade("IMPORTED_S00002", "ADA", false, 81910.0, 0.222316);
    TradeTracker::setCurrentPosition("ADA", -81910.0, 0.222316);

    TradeTracker::brokerBuy2Extend("S00035", "ADA", false, 5831.0);
    TradeTracker::brokerBuy2Extend("S00084", "ADA", false, 11556.0);
    TradeTracker::brokerBuy2Extend("S00099", "ADA", false, 1000.0);

    ASSERT_FLOAT_EQ_TOL(TradeTracker::getImportedTradeSize("IMPORTED_S00002"), 81910.0, 1e-6);
}

void test_OPM733_regular_partial_close_reports_net() {
    // OPM-733 incident 1 (TRX, YOLO V2 live log 2026-06-01): a mapped trade
    // (real oid) is partially closed via BrokerSell2 (61796 -> 58508 open).
    // BrokerTrade's fill-poll then re-reported the entry order's gross fill
    // (61796), and Zorro wrote it back into the ledger ("filled 61796 of
    // 58508"). BrokerTrade must report the NET open amount: entry fill minus
    // Zorro-driven closes.
    TradeTracker::reset();
    TradeTracker::brokerBuy2Extend("L00034", "TRX", true, 61796.0);  // live 61796

    TradeTracker::coverRegularTrade("L00034", 3288.0);  // live 58508

    ASSERT_FLOAT_EQ_TOL(TradeTracker::getRegularTradeSize("L00034"), 58508.0, 1e-6);
}

void test_OPM733_regular_full_close_reports_zero() {
    // Fully Zorro-closed mapped trade must report 0, not the gross entry fill.
    TradeTracker::reset();
    TradeTracker::brokerBuy2Extend("L00034", "TRX", true, 61796.0);

    TradeTracker::coverRegularTrade("L00034", 61796.0);  // live 0

    ASSERT_FLOAT_EQ_TOL(TradeTracker::getRegularTradeSize("L00034"), 0.0, 1e-6);
}

void test_OPM733_full_close_sublot_residual_no_phantom_lot() {
    // Pre-release review finding (2026-06-11): a mapped trade fully closed by
    // Zorro can leave a tiny POSITIVE float residual when filledSize is
    // re-derived from the exchange (0.1+0.2 = 0.30000000000000004) while
    // closedSize accumulated to 0.3. net = +4.4e-17 rounds to 0 lots, but the
    // legacy `:1` floor returned a phantom 1-lot trade -> reconcile mismatch,
    // the exact OPM-733 symptom. The sub-lot epsilon must report 0.
    TradeTracker::reset();
    const double BTC_LOT = 0.0001;  // szDecimals=4
    TradeTracker::setRegularRaw("L00034", "BTC", true, 0.1 + 0.2, 0.3);

    // Legacy behavior returned a phantom 1 lot...
    ASSERT_EQ(TradeTracker::getRegularTradeLots_PRE_OPM733_EPS("L00034", BTC_LOT), 1);
    // ...the epsilon-guarded path reports fully closed (0 lots).
    ASSERT_EQ(TradeTracker::getRegularTradeLots("L00034", BTC_LOT), 0);
}

void test_OPM733_genuine_sublot_position_still_reports_one_lot() {
    // Guard the other direction: a genuine open position smaller than one lot
    // but at least half a lot must still report 1 lot (not be swallowed by the
    // epsilon). 0.00007 BTC > 0.5*lot(0.00005) -> 1 lot.
    TradeTracker::reset();
    const double BTC_LOT = 0.0001;
    TradeTracker::setRegularRaw("L00040", "BTC", true, 0.00007, 0.0);

    ASSERT_EQ(TradeTracker::getRegularTradeLots("L00040", BTC_LOT), 1);
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
    RUN_TEST(OPM733_double_extend_keeps_imported_share);
    RUN_TEST(OPM733_triple_extend_keeps_imported_share);
    RUN_TEST(OPM733_regular_partial_close_reports_net);
    RUN_TEST(OPM733_regular_full_close_reports_zero);
    RUN_TEST(OPM733_full_close_sublot_residual_no_phantom_lot);
    RUN_TEST(OPM733_genuine_sublot_position_still_reports_one_lot);

    return printTestSummary();
}
