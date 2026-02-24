//=============================================================================
// test_poll_timeout.cpp - Verify poll() respects its timeoutMs parameter
//=============================================================================
#include "test_framework.h"
#include "ws_connection.h"
#include <IXNetSystem.h>
#include <cstdio>

static int testLog(const char* msg) {
    printf("    [WS] %s\n", msg);
    return 0;
}

void test_poll_returns_within_timeout() {
    hl::ws::Connection conn;
    conn.setLogCallback(testLog, 1);

    if (!conn.connect("api.hyperliquid-testnet.xyz", true, "/ws", 10000)) {
        printf("  SKIP (connect failed, error %d)\n", conn.getLastError());
        return;
    }

    // Do NOT send subscriptions — no data will arrive.
    const DWORD POLL_TIMEOUT_MS = 500;
    const DWORD MAX_ALLOWED_MS  = POLL_TIMEOUT_MS + 1000;

    printf("  Calling poll(%d)...\n", POLL_TIMEOUT_MS);
    DWORD start = GetTickCount();
    int result = conn.poll(POLL_TIMEOUT_MS);
    DWORD elapsed = GetTickCount() - start;

    printf("  poll(%d) returned %d in %dms (max allowed: %dms)\n",
           POLL_TIMEOUT_MS, result, elapsed, MAX_ALLOWED_MS);

    if (result < 0) {
        printf("  disconnect reason: %d, error: %d\n",
               (int)conn.disconnectReason(), conn.getLastError());
    }

    conn.disconnect();

    // poll() must return within bounded time
    ASSERT_MSG(elapsed <= MAX_ALLOWED_MS,
        "poll(500) must return within 1500ms — timeoutMs is being IGNORED");

    // poll() must not return an error (no disconnect)
    ASSERT_MSG(result >= 0,
        "poll() returned error — connection died unexpectedly");
}

int main() {
    printf("\n===================================================\n");
    printf(" Test: poll() timeout contract [OPM-106]\n");
    printf("===================================================\n\n");

    ix::initNetSystem();
    RUN_TEST(poll_returns_within_timeout);
    int result = hl::test::printTestSummary();
    ix::uninitNetSystem();
    return result;
}
