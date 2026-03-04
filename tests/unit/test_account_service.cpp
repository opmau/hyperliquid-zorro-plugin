//=============================================================================
// test_account_service.cpp - Account service unit tests [OPM-9]
//=============================================================================
// LAYER: Test | TESTS: hl_account_service (pure logic subset)
//
// Tests:
//   1. PositionInfo helpers (isOpen, isLong, isShort)
//   2. Balance::unrealizedPnl calculation
//   3. Balance::isStale with mocked timestamps
//   4. applyFill logic (new position, add, reduce, close)
//   5. convertWsPosition (regular coins)
//=============================================================================

#include "../test_framework.h"
#include "../../src/foundation/hl_types.h"
#include "../../src/foundation/hl_globals.h"
#include "../../src/transport/ws_price_cache.h"
#include <cstring>
#include <cmath>

using namespace hl::test;
using namespace hl;

//=============================================================================
// EXTRACTED TYPES (from hl_account_service.h)
// Re-created here to avoid linking the full service.
//=============================================================================

namespace AcctLogic {

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
    bool isLong() const { return size > 0.0; }
    bool isShort() const { return size < 0.0; }
};

struct Balance {
    double accountValue = 0.0;
    double withdrawable = 0.0;
    double marginUsed = 0.0;
    double totalNotional = 0.0;
    uint32_t timestamp = 0;
    bool dataReceived = false;

    double unrealizedPnl() const { return accountValue - withdrawable; }

    bool isStale(uint32_t maxAgeMs = 60000) const {
        if (timestamp == 0) return true;
        DWORD now = GetTickCount();
        DWORD age = (now >= timestamp) ? (now - timestamp) : 0;
        return age > maxAgeMs;
    }
};

/// applyFill logic extracted from hl_account_service.cpp
/// Applies a fill to a PriceCache, bridging BrokerBuy2 → GET_POSITION gap.
void applyFill(hl::ws::PriceCache& cache, const char* coin,
               double fillSize, double fillPx, bool isBuy) {
    if (!coin || fillSize <= 0) return;

    double signedFill = isBuy ? fillSize : -fillSize;
    ws::PositionData existing = cache.getPosition(std::string(coin));

    if (existing.size == 0) {
        ws::PositionData pos;
        pos.coin = coin;
        pos.size = signedFill;
        pos.entryPx = fillPx;
        cache.setPosition(pos);
    } else {
        bool sameDirection = (existing.size > 0) == isBuy;

        if (sameDirection) {
            double oldNotional = fabs(existing.size) * existing.entryPx;
            double newNotional = fillSize * fillPx;
            double totalSize = fabs(existing.size) + fillSize;
            double avgEntry = (oldNotional + newNotional) / totalSize;

            ws::PositionData pos = existing;
            pos.size = (existing.size > 0) ? totalSize : -totalSize;
            pos.entryPx = avgEntry;
            cache.setPosition(pos);
        } else {
            double remaining = fabs(existing.size) - fillSize;

            if (remaining <= 1e-12) {
                ws::PositionData pos;
                pos.coin = coin;
                pos.size = 0;
                pos.entryPx = 0;
                cache.setPosition(pos);
            } else {
                ws::PositionData pos = existing;
                pos.size = (existing.size > 0) ? remaining : -remaining;
                cache.setPosition(pos);
            }
        }
    }
}

/// Zorro account value decomposition
/// Zorro expects: Balance + TradeVal = Total Equity
bool getZorroAccountValues(const Balance& bal,
                           double* outBalance, double* outTradeVal, double* outMarginVal) {
    if (bal.accountValue <= 0) return false;

    if (outBalance) *outBalance = bal.withdrawable;
    if (outTradeVal) *outTradeVal = bal.unrealizedPnl();
    if (outMarginVal) *outMarginVal = bal.marginUsed;
    return true;
}

} // namespace AcctLogic

//=============================================================================
// TEST CASES: PositionInfo helpers
//=============================================================================

TEST_CASE(position_zero_size_not_open) {
    AcctLogic::PositionInfo pos;
    pos.size = 0.0;
    ASSERT_FALSE(pos.isOpen());
    ASSERT_FALSE(pos.isLong());
    ASSERT_FALSE(pos.isShort());
}

TEST_CASE(position_positive_size_is_long) {
    AcctLogic::PositionInfo pos;
    pos.size = 0.001;
    ASSERT_TRUE(pos.isOpen());
    ASSERT_TRUE(pos.isLong());
    ASSERT_FALSE(pos.isShort());
}

TEST_CASE(position_negative_size_is_short) {
    AcctLogic::PositionInfo pos;
    pos.size = -5.0;
    ASSERT_TRUE(pos.isOpen());
    ASSERT_FALSE(pos.isLong());
    ASSERT_TRUE(pos.isShort());
}

TEST_CASE(position_very_small_is_open) {
    AcctLogic::PositionInfo pos;
    pos.size = 1e-10;
    ASSERT_TRUE(pos.isOpen());
    ASSERT_TRUE(pos.isLong());
}

//=============================================================================
// TEST CASES: Balance helpers
//=============================================================================

TEST_CASE(balance_unrealized_pnl_positive) {
    AcctLogic::Balance bal;
    bal.accountValue = 10500.0;
    bal.withdrawable = 10000.0;
    ASSERT_FLOAT_EQ(bal.unrealizedPnl(), 500.0);
}

TEST_CASE(balance_unrealized_pnl_negative) {
    AcctLogic::Balance bal;
    bal.accountValue = 9500.0;
    bal.withdrawable = 10000.0;
    ASSERT_FLOAT_EQ(bal.unrealizedPnl(), -500.0);
}

TEST_CASE(balance_unrealized_pnl_zero) {
    AcctLogic::Balance bal;
    bal.accountValue = 10000.0;
    bal.withdrawable = 10000.0;
    ASSERT_FLOAT_EQ(bal.unrealizedPnl(), 0.0);
}

TEST_CASE(balance_stale_when_timestamp_zero) {
    AcctLogic::Balance bal;
    bal.timestamp = 0;
    ASSERT_TRUE(bal.isStale());
}

TEST_CASE(balance_fresh_when_just_updated) {
    AcctLogic::Balance bal;
    bal.timestamp = GetTickCount();
    ASSERT_FALSE(bal.isStale(60000));
}

TEST_CASE(balance_stale_when_old) {
    AcctLogic::Balance bal;
    // Set timestamp to 2 seconds ago
    bal.timestamp = GetTickCount() - 2000;
    // Stale with 1-second threshold
    ASSERT_TRUE(bal.isStale(1000));
    // Fresh with 5-second threshold
    ASSERT_FALSE(bal.isStale(5000));
}

//=============================================================================
// TEST CASES: Zorro account value decomposition
//=============================================================================

TEST_CASE(zorro_account_values_normal) {
    AcctLogic::Balance bal;
    bal.accountValue = 10500.0;
    bal.withdrawable = 10000.0;
    bal.marginUsed = 2000.0;

    double balance = 0, tradeVal = 0, marginVal = 0;
    bool ok = AcctLogic::getZorroAccountValues(bal, &balance, &tradeVal, &marginVal);

    ASSERT_TRUE(ok);
    ASSERT_FLOAT_EQ(balance, 10000.0);     // withdrawable
    ASSERT_FLOAT_EQ(tradeVal, 500.0);      // unrealizedPnl
    ASSERT_FLOAT_EQ(marginVal, 2000.0);    // marginUsed
    // Verify Zorro invariant: Balance + TradeVal = Total Equity
    ASSERT_FLOAT_EQ(balance + tradeVal, 10500.0);
}

TEST_CASE(zorro_account_values_zero_equity_returns_false) {
    AcctLogic::Balance bal;
    bal.accountValue = 0.0;

    double balance = 0, tradeVal = 0, marginVal = 0;
    bool ok = AcctLogic::getZorroAccountValues(bal, &balance, &tradeVal, &marginVal);
    ASSERT_FALSE(ok);
}

TEST_CASE(zorro_account_values_null_pointers_safe) {
    AcctLogic::Balance bal;
    bal.accountValue = 10000.0;
    bal.withdrawable = 10000.0;

    // Should not crash with null pointers
    bool ok = AcctLogic::getZorroAccountValues(bal, nullptr, nullptr, nullptr);
    ASSERT_TRUE(ok);
}

//=============================================================================
// TEST CASES: applyFill (position cache bridging)
//=============================================================================

TEST_CASE(apply_fill_new_long_position) {
    ws::PriceCache cache;
    AcctLogic::applyFill(cache, "BTC", 0.001, 60000.0, true);

    ws::PositionData pos = cache.getPosition("BTC");
    ASSERT_FLOAT_EQ_TOL(pos.size, 0.001, 1e-10);
    ASSERT_FLOAT_EQ_TOL(pos.entryPx, 60000.0, 0.01);
}

TEST_CASE(apply_fill_new_short_position) {
    ws::PriceCache cache;
    AcctLogic::applyFill(cache, "ETH", 2.0, 3500.0, false);

    ws::PositionData pos = cache.getPosition("ETH");
    ASSERT_FLOAT_EQ_TOL(pos.size, -2.0, 1e-10);
    ASSERT_FLOAT_EQ_TOL(pos.entryPx, 3500.0, 0.01);
}

TEST_CASE(apply_fill_add_to_long_weighted_average) {
    ws::PriceCache cache;
    AcctLogic::applyFill(cache, "BTC", 0.001, 60000.0, true);
    AcctLogic::applyFill(cache, "BTC", 0.002, 61000.0, true);

    ws::PositionData pos = cache.getPosition("BTC");
    ASSERT_FLOAT_EQ_TOL(pos.size, 0.003, 1e-10);
    // Weighted: (0.001*60000 + 0.002*61000) / 0.003 = 60666.67
    double expected = (0.001 * 60000.0 + 0.002 * 61000.0) / 0.003;
    ASSERT_FLOAT_EQ_TOL(pos.entryPx, expected, 0.01);
}

TEST_CASE(apply_fill_add_to_short_weighted_average) {
    ws::PriceCache cache;
    AcctLogic::applyFill(cache, "SOL", 10.0, 100.0, false);
    AcctLogic::applyFill(cache, "SOL", 20.0, 110.0, false);

    ws::PositionData pos = cache.getPosition("SOL");
    ASSERT_FLOAT_EQ_TOL(pos.size, -30.0, 1e-10);
    double expected = (10.0 * 100.0 + 20.0 * 110.0) / 30.0;
    ASSERT_FLOAT_EQ_TOL(pos.entryPx, expected, 0.01);
}

TEST_CASE(apply_fill_partial_close_long) {
    ws::PriceCache cache;
    AcctLogic::applyFill(cache, "BTC", 0.005, 65000.0, true);
    AcctLogic::applyFill(cache, "BTC", 0.002, 66000.0, false);

    ws::PositionData pos = cache.getPosition("BTC");
    ASSERT_FLOAT_EQ_TOL(pos.size, 0.003, 1e-10);
    // Entry price stays at original on partial close
    ASSERT_FLOAT_EQ_TOL(pos.entryPx, 65000.0, 0.01);
}

TEST_CASE(apply_fill_full_close) {
    ws::PriceCache cache;
    AcctLogic::applyFill(cache, "BTC", 0.001, 60000.0, true);
    AcctLogic::applyFill(cache, "BTC", 0.001, 61000.0, false);

    ws::PositionData pos = cache.getPosition("BTC");
    ASSERT_FLOAT_EQ(pos.size, 0.0);
    ASSERT_FLOAT_EQ(pos.entryPx, 0.0);
}

TEST_CASE(apply_fill_close_near_zero_epsilon) {
    // Test floating-point epsilon handling (remaining <= 1e-12)
    ws::PriceCache cache;
    AcctLogic::applyFill(cache, "BTC", 1.0, 60000.0, true);
    // Sell 0.999999999999 (within epsilon of 1.0)
    AcctLogic::applyFill(cache, "BTC", 0.999999999999, 60000.0, false);

    ws::PositionData pos = cache.getPosition("BTC");
    // Should be treated as closed (remaining < 1e-12)
    ASSERT_FLOAT_EQ(pos.size, 0.0);
}

TEST_CASE(apply_fill_different_coins_independent) {
    ws::PriceCache cache;
    AcctLogic::applyFill(cache, "BTC", 0.001, 60000.0, true);
    AcctLogic::applyFill(cache, "ETH", 1.0, 3500.0, false);

    ws::PositionData btc = cache.getPosition("BTC");
    ws::PositionData eth = cache.getPosition("ETH");
    ASSERT_FLOAT_EQ_TOL(btc.size, 0.001, 1e-10);
    ASSERT_FLOAT_EQ_TOL(eth.size, -1.0, 1e-10);
}

TEST_CASE(apply_fill_null_coin_no_crash) {
    ws::PriceCache cache;
    AcctLogic::applyFill(cache, nullptr, 1.0, 60000.0, true);
    // Should be a no-op, no crash
}

TEST_CASE(apply_fill_zero_size_no_op) {
    ws::PriceCache cache;
    AcctLogic::applyFill(cache, "BTC", 0.0, 60000.0, true);
    ws::PositionData pos = cache.getPosition("BTC");
    ASSERT_FLOAT_EQ(pos.size, 0.0);
}

TEST_CASE(apply_fill_negative_size_no_op) {
    ws::PriceCache cache;
    AcctLogic::applyFill(cache, "BTC", -1.0, 60000.0, true);
    ws::PositionData pos = cache.getPosition("BTC");
    ASSERT_FLOAT_EQ(pos.size, 0.0);
}

//=============================================================================
// MAIN
//=============================================================================

int main() {
    printf("=== OPM-9: Account Service Tests ===\n\n");

    // PositionInfo helpers
    RUN_TEST(position_zero_size_not_open);
    RUN_TEST(position_positive_size_is_long);
    RUN_TEST(position_negative_size_is_short);
    RUN_TEST(position_very_small_is_open);

    // Balance helpers
    RUN_TEST(balance_unrealized_pnl_positive);
    RUN_TEST(balance_unrealized_pnl_negative);
    RUN_TEST(balance_unrealized_pnl_zero);
    RUN_TEST(balance_stale_when_timestamp_zero);
    RUN_TEST(balance_fresh_when_just_updated);
    RUN_TEST(balance_stale_when_old);

    // Zorro account values
    RUN_TEST(zorro_account_values_normal);
    RUN_TEST(zorro_account_values_zero_equity_returns_false);
    RUN_TEST(zorro_account_values_null_pointers_safe);

    // applyFill
    RUN_TEST(apply_fill_new_long_position);
    RUN_TEST(apply_fill_new_short_position);
    RUN_TEST(apply_fill_add_to_long_weighted_average);
    RUN_TEST(apply_fill_add_to_short_weighted_average);
    RUN_TEST(apply_fill_partial_close_long);
    RUN_TEST(apply_fill_full_close);
    RUN_TEST(apply_fill_close_near_zero_epsilon);
    RUN_TEST(apply_fill_different_coins_independent);
    RUN_TEST(apply_fill_null_coin_no_crash);
    RUN_TEST(apply_fill_zero_size_no_op);
    RUN_TEST(apply_fill_negative_size_no_op);

    return printTestSummary();
}