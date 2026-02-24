// Compilation test for hl_config.h
// This file just verifies the header is syntactically correct

#include "../src/foundation/hl_config.h"
#include <cstdio>

int main() {
    // Test that constants are accessible
    printf("Plugin: %s v%s\n", hl::config::PLUGIN_NAME, hl::config::PLUGIN_VERSION);
    printf("Mainnet REST: %s\n", hl::config::MAINNET_REST);
    printf("Testnet WS: %s\n", hl::config::TESTNET_WS);
    printf("HTTP Timeout: %d ms\n", hl::config::HTTP_TIMEOUT_MS);
    printf("Max Assets: %d\n", hl::config::MAX_ASSETS);
    printf("\nhl_config.h compiles OK!\n");
    return 0;
}
