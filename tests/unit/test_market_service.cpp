//=============================================================================
// test_market_service.cpp - Market service unit tests [OPM-9]
//=============================================================================
// LAYER: Test | TESTS: hl_market_service (pure logic subset)
//
// Tests:
//   1. Candle interval conversion (minutesToInterval, intervalToString, intervalToMinutes)
//   2. Interval roundtrip consistency
//   3. HTTP seed cooldown logic (canSeedHttp, recordSeed, clearSeedCooldowns)
//=============================================================================

#include "../test_framework.h"
#include "../../src/foundation/hl_types.h"
#include "../../src/foundation/hl_globals.h"
#include <cstring>
#include <map>

using namespace hl::test;
using namespace hl;

//=============================================================================
// EXTRACTED TYPES & FUNCTIONS (from hl_market_service.h/.cpp)
//=============================================================================

namespace MktLogic {

enum class CandleInterval { M1, M3, M5, M15, M30, H1, H2, H4, D1 };

CandleInterval minutesToInterval(int minutes) {
    switch (minutes) {
        case 1:    return CandleInterval::M1;
        case 3:    return CandleInterval::M3;
        case 5:    return CandleInterval::M5;
        case 15:   return CandleInterval::M15;
        case 30:   return CandleInterval::M30;
        case 60:   return CandleInterval::H1;
        case 120:  return CandleInterval::H2;
        case 240:  return CandleInterval::H4;
        case 1440: return CandleInterval::D1;
        default:   return CandleInterval::M1;
    }
}

const char* intervalToString(CandleInterval interval) {
    switch (interval) {
        case CandleInterval::M1:  return "1m";
        case CandleInterval::M3:  return "3m";
        case CandleInterval::M5:  return "5m";
        case CandleInterval::M15: return "15m";
        case CandleInterval::M30: return "30m";
        case CandleInterval::H1:  return "1h";
        case CandleInterval::H2:  return "2h";
        case CandleInterval::H4:  return "4h";
        case CandleInterval::D1:  return "1d";
        default: return "1m";
    }
}

int intervalToMinutes(CandleInterval interval) {
    switch (interval) {
        case CandleInterval::M1:  return 1;
        case CandleInterval::M3:  return 3;
        case CandleInterval::M5:  return 5;
        case CandleInterval::M15: return 15;
        case CandleInterval::M30: return 30;
        case CandleInterval::H1:  return 60;
        case CandleInterval::H2:  return 120;
        case CandleInterval::H4:  return 240;
        case CandleInterval::D1:  return 1440;
        default: return 1;
    }
}

/// HTTP seed cooldown - extracted from hl_market_service.cpp
/// Uses a simple map + CRITICAL_SECTION for thread safety.
struct SeedCooldown {
    std::map<std::string, DWORD> seedTimes;
    CRITICAL_SECTION cs;
    bool csInit = false;
    bool httpSeedEnabled = true;
    int cooldownMs = 1000;

    void init() {
        if (!csInit) {
            InitializeCriticalSection(&cs);
            csInit = true;
        }
    }

    void cleanup() {
        if (csInit) {
            DeleteCriticalSection(&cs);
            csInit = false;
        }
    }

    bool canSeed(const char* coin) {
        if (!coin || !httpSeedEnabled) return false;

        EnterCriticalSection(&cs);
        auto it = seedTimes.find(std::string(coin));
        DWORD now = GetTickCount();
        bool canSeedResult = true;

        if (it != seedTimes.end()) {
            DWORD elapsed = now - it->second;
            canSeedResult = (elapsed >= (DWORD)cooldownMs);
        }

        LeaveCriticalSection(&cs);
        return canSeedResult;
    }

    void recordSeed(const char* coin) {
        if (!coin) return;
        EnterCriticalSection(&cs);
        seedTimes[std::string(coin)] = GetTickCount();
        LeaveCriticalSection(&cs);
    }

    void clearCooldowns() {
        EnterCriticalSection(&cs);
        seedTimes.clear();
        LeaveCriticalSection(&cs);
    }
};

} // namespace MktLogic

//=============================================================================
// TEST CASES: minutesToInterval
//=============================================================================

TEST_CASE(minutes_to_interval_1m) {
    ASSERT_EQ((int)MktLogic::minutesToInterval(1), (int)MktLogic::CandleInterval::M1);
}

TEST_CASE(minutes_to_interval_3m) {
    ASSERT_EQ((int)MktLogic::minutesToInterval(3), (int)MktLogic::CandleInterval::M3);
}

TEST_CASE(minutes_to_interval_5m) {
    ASSERT_EQ((int)MktLogic::minutesToInterval(5), (int)MktLogic::CandleInterval::M5);
}

TEST_CASE(minutes_to_interval_15m) {
    ASSERT_EQ((int)MktLogic::minutesToInterval(15), (int)MktLogic::CandleInterval::M15);
}

TEST_CASE(minutes_to_interval_30m) {
    ASSERT_EQ((int)MktLogic::minutesToInterval(30), (int)MktLogic::CandleInterval::M30);
}

TEST_CASE(minutes_to_interval_1h) {
    ASSERT_EQ((int)MktLogic::minutesToInterval(60), (int)MktLogic::CandleInterval::H1);
}

TEST_CASE(minutes_to_interval_2h) {
    ASSERT_EQ((int)MktLogic::minutesToInterval(120), (int)MktLogic::CandleInterval::H2);
}

TEST_CASE(minutes_to_interval_4h) {
    ASSERT_EQ((int)MktLogic::minutesToInterval(240), (int)MktLogic::CandleInterval::H4);
}

TEST_CASE(minutes_to_interval_1d) {
    ASSERT_EQ((int)MktLogic::minutesToInterval(1440), (int)MktLogic::CandleInterval::D1);
}

TEST_CASE(minutes_to_interval_unsupported_defaults_to_m1) {
    ASSERT_EQ((int)MktLogic::minutesToInterval(7), (int)MktLogic::CandleInterval::M1);
    ASSERT_EQ((int)MktLogic::minutesToInterval(0), (int)MktLogic::CandleInterval::M1);
    ASSERT_EQ((int)MktLogic::minutesToInterval(-1), (int)MktLogic::CandleInterval::M1);
    ASSERT_EQ((int)MktLogic::minutesToInterval(10), (int)MktLogic::CandleInterval::M1);
    ASSERT_EQ((int)MktLogic::minutesToInterval(45), (int)MktLogic::CandleInterval::M1);
    ASSERT_EQ((int)MktLogic::minutesToInterval(360), (int)MktLogic::CandleInterval::M1);
}

//=============================================================================
// TEST CASES: intervalToString
//=============================================================================

TEST_CASE(interval_to_string_all_values) {
    ASSERT_STREQ(MktLogic::intervalToString(MktLogic::CandleInterval::M1), "1m");
    ASSERT_STREQ(MktLogic::intervalToString(MktLogic::CandleInterval::M3), "3m");
    ASSERT_STREQ(MktLogic::intervalToString(MktLogic::CandleInterval::M5), "5m");
    ASSERT_STREQ(MktLogic::intervalToString(MktLogic::CandleInterval::M15), "15m");
    ASSERT_STREQ(MktLogic::intervalToString(MktLogic::CandleInterval::M30), "30m");
    ASSERT_STREQ(MktLogic::intervalToString(MktLogic::CandleInterval::H1), "1h");
    ASSERT_STREQ(MktLogic::intervalToString(MktLogic::CandleInterval::H2), "2h");
    ASSERT_STREQ(MktLogic::intervalToString(MktLogic::CandleInterval::H4), "4h");
    ASSERT_STREQ(MktLogic::intervalToString(MktLogic::CandleInterval::D1), "1d");
}

//=============================================================================
// TEST CASES: intervalToMinutes
//=============================================================================

TEST_CASE(interval_to_minutes_all_values) {
    ASSERT_EQ(MktLogic::intervalToMinutes(MktLogic::CandleInterval::M1), 1);
    ASSERT_EQ(MktLogic::intervalToMinutes(MktLogic::CandleInterval::M3), 3);
    ASSERT_EQ(MktLogic::intervalToMinutes(MktLogic::CandleInterval::M5), 5);
    ASSERT_EQ(MktLogic::intervalToMinutes(MktLogic::CandleInterval::M15), 15);
    ASSERT_EQ(MktLogic::intervalToMinutes(MktLogic::CandleInterval::M30), 30);
    ASSERT_EQ(MktLogic::intervalToMinutes(MktLogic::CandleInterval::H1), 60);
    ASSERT_EQ(MktLogic::intervalToMinutes(MktLogic::CandleInterval::H2), 120);
    ASSERT_EQ(MktLogic::intervalToMinutes(MktLogic::CandleInterval::H4), 240);
    ASSERT_EQ(MktLogic::intervalToMinutes(MktLogic::CandleInterval::D1), 1440);
}

//=============================================================================
// TEST CASES: Roundtrip consistency
//=============================================================================

TEST_CASE(roundtrip_minutes_to_interval_to_minutes) {
    // For all supported minute values, roundtrip should be identity
    int supported[] = {1, 3, 5, 15, 30, 60, 120, 240, 1440};
    for (int m : supported) {
        auto interval = MktLogic::minutesToInterval(m);
        int back = MktLogic::intervalToMinutes(interval);
        ASSERT_EQ(back, m);
    }
}

TEST_CASE(roundtrip_minutes_to_interval_to_string_to_minutes) {
    // Verify string representations are correct for all intervals
    struct TestCase { int minutes; const char* expected; };
    TestCase cases[] = {
        {1, "1m"}, {3, "3m"}, {5, "5m"}, {15, "15m"}, {30, "30m"},
        {60, "1h"}, {120, "2h"}, {240, "4h"}, {1440, "1d"}
    };
    for (const auto& tc : cases) {
        auto interval = MktLogic::minutesToInterval(tc.minutes);
        const char* str = MktLogic::intervalToString(interval);
        ASSERT_STREQ(str, tc.expected);
    }
}

//=============================================================================
// TEST CASES: HTTP Seed Cooldown
//=============================================================================

TEST_CASE(seed_cooldown_first_call_allowed) {
    MktLogic::SeedCooldown cd;
    cd.init();
    ASSERT_TRUE(cd.canSeed("BTC"));
    cd.cleanup();
}

TEST_CASE(seed_cooldown_blocked_after_record) {
    MktLogic::SeedCooldown cd;
    cd.init();
    cd.cooldownMs = 5000;  // 5 second cooldown

    cd.recordSeed("BTC");
    // Immediately after recording, should be blocked
    ASSERT_FALSE(cd.canSeed("BTC"));
    cd.cleanup();
}

TEST_CASE(seed_cooldown_different_coins_independent) {
    MktLogic::SeedCooldown cd;
    cd.init();
    cd.cooldownMs = 5000;

    cd.recordSeed("BTC");
    // ETH should still be allowed
    ASSERT_TRUE(cd.canSeed("ETH"));
    // BTC should be blocked
    ASSERT_FALSE(cd.canSeed("BTC"));
    cd.cleanup();
}

TEST_CASE(seed_cooldown_clears_all) {
    MktLogic::SeedCooldown cd;
    cd.init();
    cd.cooldownMs = 5000;

    cd.recordSeed("BTC");
    cd.recordSeed("ETH");
    cd.recordSeed("SOL");

    ASSERT_FALSE(cd.canSeed("BTC"));

    cd.clearCooldowns();

    // All should be allowed after clear
    ASSERT_TRUE(cd.canSeed("BTC"));
    ASSERT_TRUE(cd.canSeed("ETH"));
    ASSERT_TRUE(cd.canSeed("SOL"));
    cd.cleanup();
}

TEST_CASE(seed_cooldown_null_coin_returns_false) {
    MktLogic::SeedCooldown cd;
    cd.init();
    ASSERT_FALSE(cd.canSeed(nullptr));
    cd.cleanup();
}

TEST_CASE(seed_cooldown_disabled_returns_false) {
    MktLogic::SeedCooldown cd;
    cd.init();
    cd.httpSeedEnabled = false;
    ASSERT_FALSE(cd.canSeed("BTC"));
    cd.cleanup();
}

TEST_CASE(seed_cooldown_zero_ms_always_allowed) {
    MktLogic::SeedCooldown cd;
    cd.init();
    cd.cooldownMs = 0;

    cd.recordSeed("BTC");
    // With 0ms cooldown, immediately allowed again
    ASSERT_TRUE(cd.canSeed("BTC"));
    cd.cleanup();
}

TEST_CASE(seed_cooldown_expires_after_threshold) {
    MktLogic::SeedCooldown cd;
    cd.init();
    cd.cooldownMs = 50;  // Very short cooldown for testing

    cd.recordSeed("BTC");
    ASSERT_FALSE(cd.canSeed("BTC"));

    // Wait for cooldown to expire
    Sleep(60);

    ASSERT_TRUE(cd.canSeed("BTC"));
    cd.cleanup();
}

//=============================================================================
// MAIN
//=============================================================================

int main() {
    printf("=== OPM-9: Market Service Tests ===\n\n");

    // minutesToInterval
    RUN_TEST(minutes_to_interval_1m);
    RUN_TEST(minutes_to_interval_3m);
    RUN_TEST(minutes_to_interval_5m);
    RUN_TEST(minutes_to_interval_15m);
    RUN_TEST(minutes_to_interval_30m);
    RUN_TEST(minutes_to_interval_1h);
    RUN_TEST(minutes_to_interval_2h);
    RUN_TEST(minutes_to_interval_4h);
    RUN_TEST(minutes_to_interval_1d);
    RUN_TEST(minutes_to_interval_unsupported_defaults_to_m1);

    // intervalToString
    RUN_TEST(interval_to_string_all_values);

    // intervalToMinutes
    RUN_TEST(interval_to_minutes_all_values);

    // Roundtrip consistency
    RUN_TEST(roundtrip_minutes_to_interval_to_minutes);
    RUN_TEST(roundtrip_minutes_to_interval_to_string_to_minutes);

    // HTTP seed cooldown
    RUN_TEST(seed_cooldown_first_call_allowed);
    RUN_TEST(seed_cooldown_blocked_after_record);
    RUN_TEST(seed_cooldown_different_coins_independent);
    RUN_TEST(seed_cooldown_clears_all);
    RUN_TEST(seed_cooldown_null_coin_returns_false);
    RUN_TEST(seed_cooldown_disabled_returns_false);
    RUN_TEST(seed_cooldown_zero_ms_always_allowed);
    RUN_TEST(seed_cooldown_expires_after_threshold);

    return printTestSummary();
}
