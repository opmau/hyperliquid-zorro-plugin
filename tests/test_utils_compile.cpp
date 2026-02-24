// Compilation and unit test for hl_utils
#include "../src/foundation/hl_utils.h"
#include <cstdio>
#include <cassert>
#include <cstring>
#include <cmath>
#include <limits>

using namespace hl::utils;

// Helper for floating-point comparison with tolerance
bool approxEqual(double a, double b, double epsilon = 1e-9) {
    return fabs(a - b) < epsilon;
}

void test_string_helpers() {
    printf("[1] Testing string helpers...\n");

    // skipWhitespace
    assert(strcmp(skipWhitespace("  hello"), "hello") == 0);
    assert(strcmp(skipWhitespace("\n\t test"), "test") == 0);
    assert(strcmp(skipWhitespace("nowhitespace"), "nowhitespace") == 0);
    printf("    skipWhitespace OK\n");

    // normalizeCoin
    char coin[32];
    normalizeCoin("BTC-USDC", coin, sizeof(coin));
    assert(strcmp(coin, "BTC") == 0);    // strips at dash
    normalizeCoin("eth-usd", coin, sizeof(coin));
    assert(strcmp(coin, "ETH") == 0);    // strips at dash, uppercases
    normalizeCoin("SOL", coin, sizeof(coin));
    assert(strcmp(coin, "SOL") == 0);    // bare coin unchanged
    normalizeCoin("BTC/USDC", coin, sizeof(coin));
    assert(strcmp(coin, "BTC/USDC") == 0);  // slash preserved for spot
    normalizeCoin("@107", coin, sizeof(coin));
    assert(strcmp(coin, "@107") == 0);   // @-format preserved for spot
    printf("    normalizeCoin OK\n");

    // parsePerpDex (dot-suffix format, OPM-132)
    char perpDex[32], coinPart[64];
    assert(parsePerpDex("GOLD-USDC.xyz", perpDex, sizeof(perpDex), coinPart, sizeof(coinPart)));
    assert(strcmp(perpDex, "xyz") == 0);
    assert(strcmp(coinPart, "GOLD-USDC") == 0);
    assert(!parsePerpDex("BTC-USDC", perpDex, sizeof(perpDex), coinPart, sizeof(coinPart)));
    assert(strcmp(perpDex, "") == 0);
    assert(strcmp(coinPart, "BTC-USDC") == 0);
    assert(!parsePerpDex("BTC", perpDex, sizeof(perpDex), coinPart, sizeof(coinPart)));
    assert(strcmp(perpDex, "") == 0);
    assert(strcmp(coinPart, "BTC") == 0);
    printf("    parsePerpDex OK\n");

    // buildCoinName (3-arg with collateral, OPM-132)
    assert(buildCoinName("xyz", "GOLD", "USDC") == "GOLD-USDC.xyz");
    assert(buildCoinName("", "BTC", "USDC") == "BTC-USDC");
    assert(buildCoinName("", "BTC", "") == "BTC");
    assert(buildCoinName(nullptr, "ETH") == "ETH");
    printf("    buildCoinName OK\n");

    printf("    OK\n\n");
}

void test_time_helpers() {
    printf("[2] Testing time helpers...\n");

    // nowMs (just verify it returns something > 0)
    uint32_t t1 = nowMs();
    assert(t1 > 0);
    printf("    nowMs=%u\n", t1);

    // currentTimestampMs
    int64_t ts = currentTimestampMs();
    assert(ts > 1700000000000LL); // After 2023
    printf("    currentTimestampMs=%lld\n", ts);

    // OLE date conversion (Jan 1, 2020 00:00:00 UTC = Unix 1577836800)
    double ole = unixToOleDate(1577836800);
    printf("    unixToOleDate(1577836800)=%.6f\n", ole);
    int64_t back = oleToUnix(ole);
    assert(back == 1577836800);
    printf("    oleToUnix round-trip OK\n");

    // MS conversion
    double oleMs = unixMsToOleDate(1577836800000LL);
    int64_t backMs = oleToUnixMs(oleMs);
    assert(backMs == 1577836800000LL);
    printf("    unixMsToOleDate round-trip OK\n");

    printf("    OK\n\n");
}

void test_numeric_helpers() {
    printf("[3] Testing numeric helpers...\n");

    // roundToDecimals (use tolerance for floating-point comparison)
    assert(approxEqual(roundToDecimals(1.23456, 2), 1.23));
    assert(approxEqual(roundToDecimals(1.235, 2), 1.24)); // round up
    assert(approxEqual(roundToDecimals(1.23456, 4), 1.2346));
    printf("    roundToDecimals OK\n");

    // roundToTickSize (use tolerance for floating-point comparison)
    assert(approxEqual(roundToTickSize(50001.23, 0.1), 50001.2));
    assert(approxEqual(roundToTickSize(50001.27, 0.1), 50001.3));
    assert(approxEqual(roundToTickSize(100.0, 5.0), 100.0));
    assert(approxEqual(roundToTickSize(102.0, 5.0), 100.0));
    assert(approxEqual(roundToTickSize(103.0, 5.0), 105.0));
    printf("    roundToTickSize OK\n");

    // formatSize
    assert(formatSize(0.00123, 5) == "0.00123");
    assert(formatSize(1.5, 2) == "1.50");
    printf("    formatSize OK\n");

    // formatPrice
    assert(formatPrice(50000.5, 1) == "50000.5");
    assert(formatPrice(50000.0, 2) == "50000.00");
    printf("    formatPrice OK\n");

    printf("    OK\n\n");
}

void test_roundPriceForExchange() {
    printf("[4] Testing roundPriceForExchange edge cases...\n");

    // --- Normal cases (verify baseline behavior) ---
    // BTC ~100000, szDecimals=5 -> pxDecimals=1 -> max 1 decimal place
    // 5 sig figs of 100123.456 = 100120
    assert(approxEqual(roundPriceForExchange(100123.456, 5), 100120.0));
    printf("    BTC-range price OK\n");

    // Small token ~0.012345, szDecimals=0 -> max 6 decimal places
    // 5 sig figs of 0.012345 = 0.012345
    assert(approxEqual(roundPriceForExchange(0.012345, 0), 0.012345));
    printf("    Small token price OK\n");

    // Integer price should pass through
    assert(approxEqual(roundPriceForExchange(50000.0, 4), 50000.0));
    printf("    Integer price OK\n");

    // --- Edge case: zero (already handled) ---
    assert(roundPriceForExchange(0.0, 4) == 0.0);
    printf("    Zero price OK\n");

    // --- Edge case: negative price (should return 0.0) ---
    assert(roundPriceForExchange(-100.0, 4) == 0.0);
    assert(roundPriceForExchange(-0.001, 2) == 0.0);
    printf("    Negative price -> 0.0 OK\n");

    // --- Edge case: NaN (should return 0.0) ---
    double nan_val = std::numeric_limits<double>::quiet_NaN();
    assert(roundPriceForExchange(nan_val, 4) == 0.0);
    printf("    NaN price -> 0.0 OK\n");

    // --- Edge case: infinity (should return 0.0) ---
    double inf_val = std::numeric_limits<double>::infinity();
    assert(roundPriceForExchange(inf_val, 4) == 0.0);
    assert(roundPriceForExchange(-inf_val, 4) == 0.0);
    printf("    Infinity price -> 0.0 OK\n");

    // --- Edge case: extremely large price (>1e15, unsafe for 5 sig figs) ---
    assert(roundPriceForExchange(1e16, 0) == 0.0);
    assert(roundPriceForExchange(9.99e15, 0) == 0.0);
    printf("    Extreme large price -> 0.0 OK\n");

    // --- Edge case: very small positive price ---
    // 0.00000123, szDecimals=0 -> maxDecPlaces=6, so rounds to 0.000001
    // (5 sig figs gives 0.0000012300, then 6-decimal rounding truncates to 0.000001)
    double tiny = roundPriceForExchange(0.00000123, 0);
    assert(approxEqual(tiny, 0.000001, 1e-10));
    printf("    Very small price OK (got %.10f)\n", tiny);

    // With szDecimals=0 and higher maxDecimals, more precision preserved
    double tiny2 = roundPriceForExchange(0.00000123, 0, 8);
    assert(approxEqual(tiny2, 0.0000012300, 1e-12));
    printf("    Very small price (maxDec=8) OK (got %.12f)\n", tiny2);

    printf("    OK\n\n");
}

int main() {
    printf("=== hl_utils.h/cpp tests ===\n\n");

    test_string_helpers();
    test_time_helpers();
    test_numeric_helpers();
    test_roundPriceForExchange();

    printf("=== All hl_utils tests passed! ===\n");
    return 0;
}
