//=============================================================================
// test_account_service_ws.cpp - Account service WebSocket cache tests [OPM-175]
//=============================================================================
// LAYER: Test | TESTS: hl_account_service WS cache interaction patterns
//
// Tests the service-layer logic that reads from PriceCache:
//   1. getBalance with various WS cache states (fresh, stale, missing)
//   2. hasRealtimeBalance freshness checks
//   3. getPosition / getAllPositions with pre-populated cache
//   4. ensurePositionData rate-limiting (HTTP fallback trigger)
//
// Strategy: Extract the WS-reading patterns from hl_account_service.cpp
// into test-local functions, then test with a real PriceCache instance.
// This avoids linking the full service (which has HTTP/WS dependencies).
//=============================================================================

#include "../test_framework.h"
#include "../../src/foundation/hl_types.h"
#include "../../src/foundation/hl_globals.h"
#include "../../src/transport/ws_price_cache.h"
#include <cstring>
#include <cmath>
#include <vector>

using namespace hl::test;
using namespace hl;

//=============================================================================
// EXTRACTED LOGIC (from hl_account_service.cpp)
// These replicate the service's WS-reading patterns for testability.
//=============================================================================

namespace AcctWs {

/// Matches hl::account::Balance
struct Balance {
    double accountValue = 0.0;
    double withdrawable = 0.0;
    double marginUsed = 0.0;
    double totalNotional = 0.0;
    uint32_t timestamp = 0;
    bool dataReceived = false;

    double unrealizedPnl() const { return accountValue - withdrawable; }
};

/// Extracted from account_service.cpp:getBalance() lines 45-90
/// Tests the WS-cache-reading path without HTTP fallback.
Balance getBalance(ws::PriceCache& cache, bool wsEnabled, uint32_t maxAgeMs) {
    Balance result;

    if (wsEnabled) {
        ws::AccountData accData = cache.getAccountData();
        DWORD age = cache.getAccountDataAge();

        if (age < maxAgeMs) {
            result.dataReceived = true;
            // [OPM-19] Perps equity + spot USDC = total balance
            result.accountValue = accData.accountValue + accData.spotUSDC;
            result.withdrawable = accData.withdrawable;
            result.marginUsed = accData.totalMarginUsed;
            result.totalNotional = accData.totalNtlPos;
            result.timestamp = GetTickCount();
            return result;
        }
    }

    // No fresh WS data - return empty
    return result;
}

/// Extracted from account_service.cpp:hasRealtimeBalance() lines 92-100
bool hasRealtimeBalance(ws::PriceCache& cache, bool wsEnabled, uint32_t maxAgeMs) {
    if (!wsEnabled) return false;

    ws::AccountData accData = cache.getAccountData();
    DWORD age = cache.getAccountDataAge();

    return (accData.accountValue > 0 && age < maxAgeMs);
}

/// Matches hl::account::PositionInfo
struct PositionInfo {
    std::string coin;
    double size = 0.0;
    double entryPrice = 0.0;
    double unrealizedPnl = 0.0;
    double leverage = 0.0;
    double liquidationPrice = 0.0;
    double marginUsed = 0.0;
    uint32_t timestamp = 0;

    bool isOpen() const { return size != 0.0; }
};

/// Extracted from account_service.cpp:getPosition() lines 239-267
/// Only the cache-reading part (assumes ensurePositionData already called).
PositionInfo getPosition(ws::PriceCache& cache, const char* coin) {
    PositionInfo result;
    if (!coin) return result;

    ws::PositionData wsPos = cache.getPosition(std::string(coin));
    if (wsPos.size != 0) {
        result.coin = wsPos.coin;
        result.size = wsPos.size;
        result.entryPrice = wsPos.entryPx;
        result.unrealizedPnl = wsPos.unrealizedPnl;
        result.leverage = wsPos.leverage;
        result.liquidationPrice = wsPos.liquidationPx;
        result.marginUsed = wsPos.marginUsed;
        result.timestamp = wsPos.timestamp;
        return result;
    }

    result.coin = coin;
    return result;
}

/// Extracted from account_service.cpp:getAllPositions() lines 269-296
/// Only the cache-reading part (assumes ensurePositionData already called).
std::vector<PositionInfo> getAllPositions(ws::PriceCache& cache) {
    std::vector<PositionInfo> positions;

    std::vector<ws::PositionData> wsPositions = cache.getAllPositions();
    for (const auto& wsPos : wsPositions) {
        if (wsPos.size == 0) continue;

        PositionInfo pos;
        pos.coin = wsPos.coin;
        pos.size = wsPos.size;
        pos.entryPrice = wsPos.entryPx;
        pos.unrealizedPnl = wsPos.unrealizedPnl;
        pos.leverage = wsPos.leverage;
        pos.liquidationPrice = wsPos.liquidationPx;
        pos.marginUsed = wsPos.marginUsed;
        pos.timestamp = wsPos.timestamp;
        positions.push_back(pos);
    }

    return positions;
}

/// Extracted from account_service.cpp:ensurePositionData() lines 217-237
/// Tests the rate-limiting logic. Returns true if cache has data already.
bool ensurePositionData(ws::PriceCache& cache, DWORD& lastAttempt, int cachePeriodMs) {
    DWORD posAge = cache.getPositionsAge();

    // If we have a position snapshot (even if empty), no fallback needed
    if (posAge != MAXDWORD) return true;

    // No position snapshot — rate-limited HTTP fallback
    DWORD now = GetTickCount();
    DWORD elapsed = (now >= lastAttempt) ? (now - lastAttempt) : MAXDWORD;
    if (lastAttempt != 0 && elapsed < (DWORD)cachePeriodMs) {
        return false;  // Too soon since last HTTP attempt
    }

    // Would trigger HTTP fallback here — return false since we don't mock HTTP
    lastAttempt = now;
    return false;
}

} // namespace AcctWs

//=============================================================================
// TEST CASES: getBalance with WS cache states
//=============================================================================

TEST_CASE(ws_getbalance_fresh_data) {
    ws::PriceCache cache;
    cache.setAccountData(10500.0, 2000.0, 10000.0, 50000.0);

    AcctWs::Balance bal = AcctWs::getBalance(cache, true, 60000);
    ASSERT_TRUE(bal.dataReceived);
    ASSERT_FLOAT_EQ_TOL(bal.accountValue, 10500.0, 0.01);
    ASSERT_FLOAT_EQ_TOL(bal.withdrawable, 10000.0, 0.01);
    ASSERT_FLOAT_EQ_TOL(bal.marginUsed, 2000.0, 0.01);
    ASSERT_FLOAT_EQ_TOL(bal.totalNotional, 50000.0, 0.01);
    ASSERT_GT(bal.timestamp, (uint32_t)0);
}

TEST_CASE(ws_getbalance_with_spot_usdc) {
    // [OPM-19] Total = perps equity + spot USDC
    ws::PriceCache cache;
    cache.setAccountData(10000.0, 1000.0, 9000.0, 30000.0);
    cache.setSpotUSDC(500.0);

    AcctWs::Balance bal = AcctWs::getBalance(cache, true, 60000);
    ASSERT_TRUE(bal.dataReceived);
    // accountValue should be perps(10000) + spot(500) = 10500
    ASSERT_FLOAT_EQ_TOL(bal.accountValue, 10500.0, 0.01);
    // withdrawable is perps-only
    ASSERT_FLOAT_EQ_TOL(bal.withdrawable, 9000.0, 0.01);
}

TEST_CASE(ws_getbalance_stale_data) {
    ws::PriceCache cache;
    cache.setAccountData(10000.0, 1000.0, 9000.0, 30000.0);

    // Use a very short maxAge to make fresh data appear "stale"
    // Data was just set (age ~0ms), so maxAge=0 should consider it stale
    AcctWs::Balance bal = AcctWs::getBalance(cache, true, 0);
    ASSERT_FALSE(bal.dataReceived);
    ASSERT_FLOAT_EQ(bal.accountValue, 0.0);
}

TEST_CASE(ws_getbalance_missing_data) {
    // Empty cache — no account data set
    ws::PriceCache cache;

    AcctWs::Balance bal = AcctWs::getBalance(cache, true, 60000);
    // getAccountDataAge() returns MAXDWORD when no data set
    ASSERT_FALSE(bal.dataReceived);
    ASSERT_FLOAT_EQ(bal.accountValue, 0.0);
}

TEST_CASE(ws_getbalance_ws_disabled) {
    ws::PriceCache cache;
    cache.setAccountData(10000.0, 1000.0, 9000.0, 30000.0);

    AcctWs::Balance bal = AcctWs::getBalance(cache, false, 60000);
    // WebSocket disabled — should not read from cache
    ASSERT_FALSE(bal.dataReceived);
    ASSERT_FLOAT_EQ(bal.accountValue, 0.0);
}

TEST_CASE(ws_getbalance_zero_equity_still_received) {
    // Edge case: account value is 0 but data WAS received (new account)
    ws::PriceCache cache;
    cache.setAccountData(0.0, 0.0, 0.0, 0.0);

    AcctWs::Balance bal = AcctWs::getBalance(cache, true, 60000);
    // age should be < maxAgeMs, so dataReceived = true even with 0 values
    ASSERT_TRUE(bal.dataReceived);
    ASSERT_FLOAT_EQ(bal.accountValue, 0.0);
}

TEST_CASE(ws_getbalance_unrealized_pnl) {
    ws::PriceCache cache;
    cache.setAccountData(10500.0, 2000.0, 10000.0, 50000.0);

    AcctWs::Balance bal = AcctWs::getBalance(cache, true, 60000);
    // unrealizedPnl = accountValue - withdrawable = 10500 - 10000 = 500
    ASSERT_FLOAT_EQ_TOL(bal.unrealizedPnl(), 500.0, 0.01);
}

//=============================================================================
// TEST CASES: hasRealtimeBalance
//=============================================================================

TEST_CASE(ws_hasbalance_fresh) {
    ws::PriceCache cache;
    cache.setAccountData(10000.0, 1000.0, 9000.0, 30000.0);

    ASSERT_TRUE(AcctWs::hasRealtimeBalance(cache, true, 60000));
}

TEST_CASE(ws_hasbalance_zero_equity_returns_false) {
    // hasRealtimeBalance requires accountValue > 0
    ws::PriceCache cache;
    cache.setAccountData(0.0, 0.0, 0.0, 0.0);

    ASSERT_FALSE(AcctWs::hasRealtimeBalance(cache, true, 60000));
}

TEST_CASE(ws_hasbalance_ws_disabled) {
    ws::PriceCache cache;
    cache.setAccountData(10000.0, 1000.0, 9000.0, 30000.0);

    ASSERT_FALSE(AcctWs::hasRealtimeBalance(cache, false, 60000));
}

TEST_CASE(ws_hasbalance_stale) {
    ws::PriceCache cache;
    cache.setAccountData(10000.0, 1000.0, 9000.0, 30000.0);

    // maxAge=0 means data is always stale
    ASSERT_FALSE(AcctWs::hasRealtimeBalance(cache, true, 0));
}

TEST_CASE(ws_hasbalance_no_data) {
    ws::PriceCache cache;

    ASSERT_FALSE(AcctWs::hasRealtimeBalance(cache, true, 60000));
}

//=============================================================================
// TEST CASES: getPosition with pre-populated cache
//=============================================================================

TEST_CASE(ws_getposition_existing_long) {
    ws::PriceCache cache;
    ws::PositionData wsPos;
    wsPos.coin = "BTC";
    wsPos.size = 0.5;
    wsPos.entryPx = 65000.0;
    wsPos.unrealizedPnl = 250.0;
    wsPos.leverage = 5.0;
    wsPos.liquidationPx = 55000.0;
    wsPos.marginUsed = 6500.0;
    cache.setPosition(wsPos);

    AcctWs::PositionInfo pos = AcctWs::getPosition(cache, "BTC");
    ASSERT_TRUE(pos.isOpen());
    ASSERT_STREQ(pos.coin.c_str(), "BTC");
    ASSERT_FLOAT_EQ_TOL(pos.size, 0.5, 1e-10);
    ASSERT_FLOAT_EQ_TOL(pos.entryPrice, 65000.0, 0.01);
    ASSERT_FLOAT_EQ_TOL(pos.unrealizedPnl, 250.0, 0.01);
    ASSERT_FLOAT_EQ_TOL(pos.leverage, 5.0, 0.01);
    ASSERT_FLOAT_EQ_TOL(pos.liquidationPrice, 55000.0, 0.01);
    ASSERT_FLOAT_EQ_TOL(pos.marginUsed, 6500.0, 0.01);
}

TEST_CASE(ws_getposition_existing_short) {
    ws::PriceCache cache;
    ws::PositionData wsPos;
    wsPos.coin = "ETH";
    wsPos.size = -10.0;
    wsPos.entryPx = 3500.0;
    wsPos.unrealizedPnl = -200.0;
    cache.setPosition(wsPos);

    AcctWs::PositionInfo pos = AcctWs::getPosition(cache, "ETH");
    ASSERT_TRUE(pos.isOpen());
    ASSERT_FLOAT_EQ_TOL(pos.size, -10.0, 1e-10);
    ASSERT_FLOAT_EQ_TOL(pos.entryPrice, 3500.0, 0.01);
    ASSERT_FLOAT_EQ_TOL(pos.unrealizedPnl, -200.0, 0.01);
}

TEST_CASE(ws_getposition_empty_cache) {
    ws::PriceCache cache;

    AcctWs::PositionInfo pos = AcctWs::getPosition(cache, "BTC");
    ASSERT_FALSE(pos.isOpen());
    ASSERT_FLOAT_EQ(pos.size, 0.0);
    ASSERT_STREQ(pos.coin.c_str(), "BTC");
}

TEST_CASE(ws_getposition_wrong_coin) {
    ws::PriceCache cache;
    ws::PositionData wsPos;
    wsPos.coin = "BTC";
    wsPos.size = 1.0;
    wsPos.entryPx = 60000.0;
    cache.setPosition(wsPos);

    // Query for ETH, should not find BTC's position
    AcctWs::PositionInfo pos = AcctWs::getPosition(cache, "ETH");
    ASSERT_FALSE(pos.isOpen());
    ASSERT_FLOAT_EQ(pos.size, 0.0);
}

TEST_CASE(ws_getposition_null_coin) {
    ws::PriceCache cache;
    AcctWs::PositionInfo pos = AcctWs::getPosition(cache, nullptr);
    ASSERT_FALSE(pos.isOpen());
}

//=============================================================================
// TEST CASES: getAllPositions with pre-populated cache
//=============================================================================

TEST_CASE(ws_getallpositions_multiple) {
    ws::PriceCache cache;

    ws::PositionData btc;
    btc.coin = "BTC";
    btc.size = 0.5;
    btc.entryPx = 65000.0;
    cache.setPosition(btc);

    ws::PositionData eth;
    eth.coin = "ETH";
    eth.size = -5.0;
    eth.entryPx = 3500.0;
    cache.setPosition(eth);

    ws::PositionData sol;
    sol.coin = "SOL";
    sol.size = 100.0;
    sol.entryPx = 150.0;
    cache.setPosition(sol);

    std::vector<AcctWs::PositionInfo> positions = AcctWs::getAllPositions(cache);
    ASSERT_EQ((int)positions.size(), 3);

    // Verify we got all three coins (order may vary due to map ordering)
    bool foundBTC = false, foundETH = false, foundSOL = false;
    for (const auto& p : positions) {
        if (p.coin == "BTC") { foundBTC = true; ASSERT_FLOAT_EQ_TOL(p.size, 0.5, 1e-10); }
        if (p.coin == "ETH") { foundETH = true; ASSERT_FLOAT_EQ_TOL(p.size, -5.0, 1e-10); }
        if (p.coin == "SOL") { foundSOL = true; ASSERT_FLOAT_EQ_TOL(p.size, 100.0, 1e-10); }
    }
    ASSERT_TRUE(foundBTC);
    ASSERT_TRUE(foundETH);
    ASSERT_TRUE(foundSOL);
}

TEST_CASE(ws_getallpositions_filters_zero_size) {
    ws::PriceCache cache;

    ws::PositionData btc;
    btc.coin = "BTC";
    btc.size = 0.5;
    btc.entryPx = 65000.0;
    cache.setPosition(btc);

    // Closed position (size=0) should be filtered out
    ws::PositionData eth;
    eth.coin = "ETH";
    eth.size = 0.0;
    eth.entryPx = 3500.0;
    cache.setPosition(eth);

    std::vector<AcctWs::PositionInfo> positions = AcctWs::getAllPositions(cache);
    ASSERT_EQ((int)positions.size(), 1);
    ASSERT_STREQ(positions[0].coin.c_str(), "BTC");
}

TEST_CASE(ws_getallpositions_empty_cache) {
    ws::PriceCache cache;

    std::vector<AcctWs::PositionInfo> positions = AcctWs::getAllPositions(cache);
    ASSERT_EQ((int)positions.size(), 0);
}

//=============================================================================
// TEST CASES: ensurePositionData rate-limiting
//=============================================================================

TEST_CASE(ws_ensureposdata_has_snapshot_returns_true) {
    ws::PriceCache cache;
    // clearPositions() sets lastPositionsUpdate_ = GetTickCount()
    // This simulates WS having delivered a position snapshot
    cache.clearPositions();

    DWORD lastAttempt = 0;
    bool result = AcctWs::ensurePositionData(cache, lastAttempt, 2000);
    ASSERT_TRUE(result);
    // lastAttempt should NOT be updated (no HTTP needed)
    ASSERT_EQ(lastAttempt, (DWORD)0);
}

TEST_CASE(ws_ensureposdata_no_snapshot_first_attempt) {
    ws::PriceCache cache;
    // Don't call clearPositions — positionsAge() will be MAXDWORD

    DWORD lastAttempt = 0;
    bool result = AcctWs::ensurePositionData(cache, lastAttempt, 2000);
    // Should return false (would need HTTP), but lastAttempt updated
    ASSERT_FALSE(result);
    ASSERT_GT(lastAttempt, (DWORD)0);
}

TEST_CASE(ws_ensureposdata_rate_limited) {
    ws::PriceCache cache;
    // No position snapshot

    DWORD lastAttempt = GetTickCount();
    bool result = AcctWs::ensurePositionData(cache, lastAttempt, 2000);
    // Should be rate-limited (too soon since last attempt)
    ASSERT_FALSE(result);
}

TEST_CASE(ws_ensureposdata_rate_limit_expired) {
    ws::PriceCache cache;
    // No position snapshot

    // Set lastAttempt to 3 seconds ago
    DWORD lastAttempt = GetTickCount() - 3000;
    DWORD oldAttempt = lastAttempt;
    bool result = AcctWs::ensurePositionData(cache, lastAttempt, 2000);
    // Rate limit expired (3s > 2s), should attempt and update lastAttempt
    ASSERT_FALSE(result);  // Still false (no HTTP available)
    ASSERT_GT(lastAttempt, oldAttempt);  // Updated to now
}

TEST_CASE(ws_ensureposdata_with_positions_set) {
    ws::PriceCache cache;
    // Set a position directly (not via clearPositions)
    ws::PositionData pos;
    pos.coin = "BTC";
    pos.size = 1.0;
    pos.entryPx = 60000.0;
    cache.setPosition(pos);

    // Note: setPosition alone doesn't update lastPositionsUpdate_
    // Only clearPositions() does that. So positionsAge() is still MAXDWORD.
    // This tests the edge case where individual positions exist but no
    // full snapshot was received.
    DWORD lastAttempt = 0;
    bool result = AcctWs::ensurePositionData(cache, lastAttempt, 2000);
    // MAXDWORD age means no snapshot — triggers fallback
    ASSERT_FALSE(result);
}

//=============================================================================
// MAIN
//=============================================================================

int main() {
    printf("=== OPM-175: Account Service WS Cache Tests ===\n\n");

    // getBalance with WS cache states
    RUN_TEST(ws_getbalance_fresh_data);
    RUN_TEST(ws_getbalance_with_spot_usdc);
    RUN_TEST(ws_getbalance_stale_data);
    RUN_TEST(ws_getbalance_missing_data);
    RUN_TEST(ws_getbalance_ws_disabled);
    RUN_TEST(ws_getbalance_zero_equity_still_received);
    RUN_TEST(ws_getbalance_unrealized_pnl);

    // hasRealtimeBalance
    RUN_TEST(ws_hasbalance_fresh);
    RUN_TEST(ws_hasbalance_zero_equity_returns_false);
    RUN_TEST(ws_hasbalance_ws_disabled);
    RUN_TEST(ws_hasbalance_stale);
    RUN_TEST(ws_hasbalance_no_data);

    // getPosition
    RUN_TEST(ws_getposition_existing_long);
    RUN_TEST(ws_getposition_existing_short);
    RUN_TEST(ws_getposition_empty_cache);
    RUN_TEST(ws_getposition_wrong_coin);
    RUN_TEST(ws_getposition_null_coin);

    // getAllPositions
    RUN_TEST(ws_getallpositions_multiple);
    RUN_TEST(ws_getallpositions_filters_zero_size);
    RUN_TEST(ws_getallpositions_empty_cache);

    // ensurePositionData
    RUN_TEST(ws_ensureposdata_has_snapshot_returns_true);
    RUN_TEST(ws_ensureposdata_no_snapshot_first_attempt);
    RUN_TEST(ws_ensureposdata_rate_limited);
    RUN_TEST(ws_ensureposdata_rate_limit_expired);
    RUN_TEST(ws_ensureposdata_with_positions_set);

    return printTestSummary();
}
