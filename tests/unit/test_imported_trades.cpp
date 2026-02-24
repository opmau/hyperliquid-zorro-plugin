//=============================================================================
// test_imported_trades.cpp - IMPORTED trade position tracking
//=============================================================================
// PREVENTS BUG: 18c287c
//   - BrokerTrade returned stale import-time data, not current position
//   - Risk: Wrong trade decisions based on outdated position size
//
// ZORRO BEHAVIOR:
//   - When a trade is "imported" via GET_POSITION, Zorro assigns a synthetic ID
//   - Later calls to BrokerTrade with that ID should return CURRENT position
//   - If position reversed (long->short), the imported trade should show as closed
//
// WHY THIS MATTERS:
//   - If user has 0.5 BTC position imported at $40,000
//   - Position later reduced to 0.3 BTC at $41,000
//   - BrokerTrade must return 0.3 (current), not 0.5 (stale import-time)
//   - Otherwise Zorro makes wrong decisions about position sizing
//=============================================================================

#include "../test_framework.h"
#include <cstring>
#include <string>
#include <map>

using namespace hl::test;

//=============================================================================
// SIMULATED IMPORTED TRADE TRACKING (extracted from plugin pattern)
//=============================================================================

namespace TradeTracker {

// Represents an imported trade state
struct ImportedTrade {
    char orderId[64];
    char coin[32];
    bool isBuy;
    double importedSize;   // Size at import time
    double importedPrice;  // Price at import time
    int64_t importTimestamp;
};

// Represents current position state (from exchange)
struct CurrentPosition {
    double size;      // Positive = long, negative = short, 0 = no position
    double entryPx;
    int64_t timestamp;
};

// Storage for imported trades
std::map<std::string, ImportedTrade> importedTrades;

// Simulated current position (would come from WebSocket/HTTP in real code)
std::map<std::string, CurrentPosition> currentPositions;

// Register an imported trade (called when Zorro imports an existing position)
void importTrade(const char* orderId, const char* coin, bool isBuy,
                 double size, double price) {
    ImportedTrade trade;
    strncpy_s(trade.orderId, orderId, sizeof(trade.orderId) - 1);
    strncpy_s(trade.coin, coin, sizeof(trade.coin) - 1);
    trade.isBuy = isBuy;
    trade.importedSize = size;
    trade.importedPrice = price;
    trade.importTimestamp = 1700000000000LL;  // Simulated timestamp

    importedTrades[orderId] = trade;
}

// Set current position (simulates WebSocket update)
void setCurrentPosition(const char* coin, double size, double entryPx) {
    CurrentPosition pos;
    pos.size = size;
    pos.entryPx = entryPx;
    pos.timestamp = 1700000001000LL;  // Later timestamp

    currentPositions[coin] = pos;
}

// Clear all data
void reset() {
    importedTrades.clear();
    currentPositions.clear();
}

// CORRECT implementation: Returns CURRENT position size
// This is what BrokerTrade should do for imported trades
double getImportedTradeSize_CORRECT(const char* orderId) {
    auto it = importedTrades.find(orderId);
    if (it == importedTrades.end()) return 0;

    const ImportedTrade& trade = it->second;

    // Look up CURRENT position for this coin
    auto posIt = currentPositions.find(trade.coin);
    if (posIt == currentPositions.end()) {
        // No position = trade closed
        return 0;
    }

    const CurrentPosition& current = posIt->second;

    // Check if position direction matches imported trade
    bool currentIsLong = current.size > 0;
    bool currentIsShort = current.size < 0;

    if (trade.isBuy && currentIsShort) {
        // Imported a LONG but now we're SHORT = position reversed
        return 0;
    }
    if (!trade.isBuy && currentIsLong) {
        // Imported a SHORT but now we're LONG = position reversed
        return 0;
    }

    // Return current absolute size (Zorro expects positive lots)
    return (current.size >= 0) ? current.size : -current.size;
}

// BUGGY implementation: Returns STALE import-time size
// This is what bug 18c287c did
double getImportedTradeSize_BUGGY(const char* orderId) {
    auto it = importedTrades.find(orderId);
    if (it == importedTrades.end()) return 0;

    // BUG: Returns import-time size, ignoring current position!
    return it->second.importedSize;
}

} // namespace TradeTracker

//=============================================================================
// TEST CASES
//=============================================================================

void test_import_then_position_unchanged() {
    TradeTracker::reset();

    // Import 0.5 BTC long at $40,000
    TradeTracker::importTrade("IMPORTED_12345", "BTC", true, 0.5, 40000.0);

    // Current position is still 0.5 BTC (unchanged)
    TradeTracker::setCurrentPosition("BTC", 0.5, 40000.0);

    double size = TradeTracker::getImportedTradeSize_CORRECT("IMPORTED_12345");
    ASSERT_FLOAT_EQ_TOL(size, 0.5, 1e-6);
}

void test_import_then_position_reduced() {
    TradeTracker::reset();

    // Import 0.5 BTC long at $40,000
    TradeTracker::importTrade("IMPORTED_12345", "BTC", true, 0.5, 40000.0);

    // Position reduced to 0.3 BTC
    TradeTracker::setCurrentPosition("BTC", 0.3, 41000.0);

    // CORRECT: Returns current 0.3, not import-time 0.5
    double sizeCorrect = TradeTracker::getImportedTradeSize_CORRECT("IMPORTED_12345");
    ASSERT_FLOAT_EQ_TOL(sizeCorrect, 0.3, 1e-6);

    // BUGGY: Would return stale 0.5
    double sizeBuggy = TradeTracker::getImportedTradeSize_BUGGY("IMPORTED_12345");
    ASSERT_FLOAT_EQ_TOL(sizeBuggy, 0.5, 1e-6);

    // The values must differ
    ASSERT_FLOAT_NE(sizeCorrect, sizeBuggy);
}

void test_import_then_position_closed() {
    TradeTracker::reset();

    // Import 0.5 BTC long at $40,000
    TradeTracker::importTrade("IMPORTED_12345", "BTC", true, 0.5, 40000.0);

    // Position completely closed (0 size)
    TradeTracker::setCurrentPosition("BTC", 0.0, 0.0);

    double size = TradeTracker::getImportedTradeSize_CORRECT("IMPORTED_12345");
    ASSERT_FLOAT_EQ_TOL(size, 0.0, 1e-6);  // Trade is closed
}

void test_import_then_position_reversed() {
    TradeTracker::reset();

    // Import 0.5 BTC LONG at $40,000
    TradeTracker::importTrade("IMPORTED_12345", "BTC", true, 0.5, 40000.0);

    // Position REVERSED to -0.2 BTC (SHORT)
    TradeTracker::setCurrentPosition("BTC", -0.2, 42000.0);

    // Original LONG trade is now closed (position reversed)
    double size = TradeTracker::getImportedTradeSize_CORRECT("IMPORTED_12345");
    ASSERT_FLOAT_EQ_TOL(size, 0.0, 1e-6);
}

void test_import_short_position() {
    TradeTracker::reset();

    // Import -0.3 BTC SHORT at $45,000
    TradeTracker::importTrade("IMPORTED_67890", "BTC", false, 0.3, 45000.0);

    // Current position is -0.3 BTC (still short)
    TradeTracker::setCurrentPosition("BTC", -0.3, 45000.0);

    double size = TradeTracker::getImportedTradeSize_CORRECT("IMPORTED_67890");
    ASSERT_FLOAT_EQ_TOL(size, 0.3, 1e-6);  // Returns absolute size
}

void test_import_short_then_reversed_to_long() {
    TradeTracker::reset();

    // Import -0.3 BTC SHORT
    TradeTracker::importTrade("IMPORTED_67890", "BTC", false, 0.3, 45000.0);

    // Position reversed to +0.5 LONG
    TradeTracker::setCurrentPosition("BTC", 0.5, 44000.0);

    // Original SHORT trade is now closed
    double size = TradeTracker::getImportedTradeSize_CORRECT("IMPORTED_67890");
    ASSERT_FLOAT_EQ_TOL(size, 0.0, 1e-6);
}

void test_multiple_assets_imported() {
    TradeTracker::reset();

    // Import BTC and ETH positions
    TradeTracker::importTrade("IMPORTED_BTC", "BTC", true, 0.5, 40000.0);
    TradeTracker::importTrade("IMPORTED_ETH", "ETH", true, 5.0, 2000.0);

    // BTC reduced, ETH unchanged
    TradeTracker::setCurrentPosition("BTC", 0.3, 41000.0);
    TradeTracker::setCurrentPosition("ETH", 5.0, 2000.0);

    double btcSize = TradeTracker::getImportedTradeSize_CORRECT("IMPORTED_BTC");
    double ethSize = TradeTracker::getImportedTradeSize_CORRECT("IMPORTED_ETH");

    ASSERT_FLOAT_EQ_TOL(btcSize, 0.3, 1e-6);
    ASSERT_FLOAT_EQ_TOL(ethSize, 5.0, 1e-6);
}

void test_unknown_order_id() {
    TradeTracker::reset();

    double size = TradeTracker::getImportedTradeSize_CORRECT("UNKNOWN_ID");
    ASSERT_FLOAT_EQ_TOL(size, 0.0, 1e-6);
}

//=============================================================================
// MAIN
//=============================================================================

int main() {
    printf("\n");
    printf("=================================================\n");
    printf("  IMPORTED Trade Position Tracking Tests\n");
    printf("  Prevents bug: 18c287c (stale import data)\n");
    printf("=================================================\n\n");

    RUN_TEST(import_then_position_unchanged);
    RUN_TEST(import_then_position_reduced);
    RUN_TEST(import_then_position_closed);
    RUN_TEST(import_then_position_reversed);
    RUN_TEST(import_short_position);
    RUN_TEST(import_short_then_reversed_to_long);
    RUN_TEST(multiple_assets_imported);
    RUN_TEST(unknown_order_id);

    return printTestSummary();
}