// Compilation and basic functionality test for hl_globals
#include "../src/foundation/hl_globals.h"
#include <cstdio>
#include <cassert>
#include <ctime>
#ifdef _WIN32
#include <windows.h>
#endif

// Mock log callback
static int testLogCallback(const char* msg) {
    printf("  LOG: %s\n", msg);
    return 1;
}

int main() {
    printf("=== hl_globals.h/cpp compilation test ===\n\n");

    // Initialize globals
    printf("[1] Initializing globals...\n");
    hl::initGlobals();
    printf("    OK\n\n");

    // Test RuntimeConfig defaults
    printf("[2] Testing RuntimeConfig defaults...\n");
    assert(hl::g_config.isTestnet == true);
    assert(hl::g_config.diagLevel == 0);
    assert(hl::g_config.enableWebSocket == true);
    printf("    isTestnet=%d, diagLevel=%d, enableWS=%d\n",
           hl::g_config.isTestnet, hl::g_config.diagLevel,
           hl::g_config.enableWebSocket);
    printf("    baseUrl=%s\n", hl::g_config.baseUrl);
    printf("    OK\n\n");

    // Test Logger
    printf("[3] Testing Logger...\n");
    hl::g_logger.callback = testLogCallback;
    hl::g_logger.level = 2;
    hl::g_logger.info("This should appear");
    hl::g_logger.debug("This should NOT appear (level 3 > 2)");
    hl::g_logger.logf(1, "Formatted: %s=%d", "value", 42);
    printf("    OK\n\n");

    // Test AssetRegistry
    printf("[4] Testing AssetRegistry...\n");
    hl::AssetInfo btc = {};
    strcpy_s(btc.name, "BTC-USD");
    strcpy_s(btc.coin, "BTC");
    btc.index = 0;
    btc.szDecimals = 5;
    btc.tickSize = 0.1;
    assert(hl::g_assets.add(btc));
    assert(hl::g_assets.count == 1);
    assert(hl::g_assets.findByName("BTC-USD") == 0);
    assert(hl::g_assets.findByCoin("BTC") == 0);
    assert(hl::g_assets.findByName("ETH-USD") == -1);
    printf("    Added BTC, count=%d, findByName=%d\n",
           hl::g_assets.count, hl::g_assets.findByName("BTC-USD"));
    printf("    OK\n\n");

    // Test TradingState
    printf("[5] Testing TradingState...\n");
    int id1 = hl::g_trading.generateTradeId();
    int id2 = hl::g_trading.generateTradeId();
    assert(id1 == 2);  // init() sets nextTradeId=2 (Zorro treats 0/1 as failure)
    assert(id2 == 3);
    printf("    Generated IDs: %d, %d\n", id1, id2);

    uint64_t nonce1 = hl::g_trading.generateNonce();
    uint64_t nonce2 = hl::g_trading.generateNonce();
    assert(nonce2 > nonce1);
    printf("    Nonces: %llu, %llu (monotonic: %s)\n",
           nonce1, nonce2, nonce2 > nonce1 ? "yes" : "NO!");

    // Precision test: nonce should reflect actual wall-clock ms, not coarse seconds.
    // Generate a nonce, sleep 50ms, generate another. The gap should be >= 20ms
    // (allowing generous tolerance). With old time()*1000 (1-second granularity),
    // two nonces within the same second would differ by only 1-2, not ~50.
    printf("[5b] Testing nonce precision (ms-level)...\n");
    hl::g_trading.lastNonce = 0;  // Reset to avoid stale monotonic bump
    uint64_t nonceA = hl::g_trading.generateNonce();
#ifdef _WIN32
    Sleep(50);
#else
    usleep(50000);
#endif
    uint64_t nonceB = hl::g_trading.generateNonce();
    uint64_t gap = nonceB - nonceA;
    printf("    nonceA=%llu, nonceB=%llu, gap=%llu ms\n", nonceA, nonceB, gap);
    // With ms precision, gap should be ~50. With second precision, gap would be 1.
    assert(gap >= 20 && "Nonce precision too coarse — expected ms-level resolution");
    // Verify nonces are in a plausible epoch range (after 2024-01-01 = 1704067200000)
    assert(nonceA > 1704067200000ULL && "Nonce should be a Unix epoch ms timestamp");
    printf("    OK (ms-level precision confirmed)\n\n");

    // Test trade map
    hl::OrderState order = {};
    strcpy_s(order.cloid, "0x123");
    strcpy_s(order.coin, "BTC");
    order.side = hl::OrderSide::Buy;
    order.requestedSize = 0.001;
    hl::g_trading.setOrder(id1, order);

    hl::OrderState retrieved;
    assert(hl::g_trading.getOrder(id1, retrieved));
    assert(strcmp(retrieved.cloid, "0x123") == 0);
    printf("    TradeMap set/get OK\n");
    printf("    OK\n\n");

    // Cleanup
    printf("[6] Cleaning up...\n");
    hl::cleanupGlobals();
    printf("    OK\n\n");

    printf("=== All hl_globals tests passed! ===\n");
    return 0;
}
