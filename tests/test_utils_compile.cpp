// Compilation and unit test for hl_utils
#include "../src/foundation/hl_utils.h"
#include <cstdio>
#include <cassert>
#include <cstring>
#include <cmath>

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

int main() {
    printf("=== hl_utils.h/cpp tests ===\n\n");

    test_string_helpers();
    test_time_helpers();
    test_numeric_helpers();

    printf("=== All hl_utils tests passed! ===\n");
    return 0;
}
