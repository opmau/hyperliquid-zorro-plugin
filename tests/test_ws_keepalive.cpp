//=============================================================================
// test_ws_keepalive.cpp - WebSocket protocol keepalive test [OPM-868]
//=============================================================================
// Part of Hyperliquid Plugin for Zorro
//
// LAYER: Test
// NETWORK: Connects to api.hyperliquid-testnet.xyz (requires internet)
// PURPOSE: Verify the connection configures a protocol-level ping and that the
//          exchange answers it, so an idle connection is neither dropped nor
//          left undetectably dead.
//
// Standalone target, deliberately not part of the default unit run: it needs
// network access and holds a connection open for over a minute of real time.
// Build and run it on its own:
//
//     cmake --build <build-dir> --config Release --target test_ws_keepalive
//     <build-dir>/Release/test_ws_keepalive.exe
//
// A run that cannot reach the exchange is skipped rather than failed, so a
// green result on a machine with no network says nothing.
//
// Two failures this guards against, in opposite directions:
//
//   1. No ping configured. IXWebSocket sends no ping frames unless an interval
//      is set, and without them it never tracks pongs. A connection whose peer
//      has gone away then looks alive for as long as the process runs, because
//      a send into a half-open socket completes locally and reports success.
//      Prices stop arriving while every price read falls through to its slower
//      fallback path.
//
//   2. Ping configured but unanswered. The transport closes a socket when the
//      ping interval elapses with no pong, so an exchange that ignored ping
//      frames would have every connection torn down and rebuilt a few times a
//      minute — worse than the problem the ping solves.
//
// The two keepalives are not interchangeable and this test needs both. The
// exchange closes a connection carrying no *application* traffic as inactive
// after about a minute, and protocol pongs do not reset that timer; the
// application ping below is what holds the session open, mirroring what the
// manager sends in production. Only the protocol ping, in turn, detects a peer
// that has stopped answering. A run that stays connected while pongs come back
// and no ping-timeout close appears rules out both failures above.
//=============================================================================

#include "test_framework.h"
#include "ws_connection.h"
#include "hl_config.h"
#include <IXNetSystem.h>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <string>

// The transport closes an unanswered connection with this reason, which the
// Connection log callback reports on its "Close frame" line.
static const char* PING_TIMEOUT_REASON = "Ping timeout";

static bool g_sawPingTimeout = false;
static bool g_sawPong = false;

static int keepaliveLog(const char* msg) {
    printf("    [WS] %s\n", msg);
    if (strstr(msg, PING_TIMEOUT_REASON)) g_sawPingTimeout = true;
    if (strstr(msg, "Pong received")) g_sawPong = true;
    return 0;
}

//-----------------------------------------------------------------------------
// Test: an idle connection survives several ping intervals
//-----------------------------------------------------------------------------
TEST_CASE(keepalive_idle_connection_survives_ping_intervals) {
    const int pingSecs = hl::config::WS_PING_INTERVAL_MS / 1000;
    ASSERT_MSG(pingSecs > 0, "ping interval must be configured");

    // Three intervals: one to send the first ping, the rest to catch a close
    // that only happens when a pong fails to come back.
    const int holdSecs = pingSecs * 3 + 5;

    g_sawPingTimeout = false;
    g_sawPong = false;

    hl::ws::Connection conn;
    conn.setLogCallback(keepaliveLog, 2);

    if (!conn.connect("api.hyperliquid-testnet.xyz", true, "/ws", 10000)) {
        printf("\n  SKIP (connect failed)\n");
        hl::test::g_testsPassed++;  // Don't fail on network issues
        return;
    }

    printf("    holding a quiet connection for %ds (protocol ping every %ds)...\n",
           holdSecs, pingSecs);

    // Deliberately subscribe to nothing: market data would keep the socket busy
    // and hide whether the pings alone sustain it. The application ping goes out
    // on the same cadence the manager uses, which is what keeps the exchange
    // from closing the session as inactive.
    const long APP_PING_SECS = 30;
    long lastAppPing = -APP_PING_SECS;  // send one immediately

    auto start = std::chrono::steady_clock::now();
    for (;;) {
        long elapsed = static_cast<long>(
            std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::steady_clock::now() - start).count());
        if (elapsed >= holdSecs) break;

        if (elapsed - lastAppPing >= APP_PING_SECS) {
            conn.send("{\"method\":\"ping\"}");
            lastAppPing = elapsed;
        }
        conn.poll(500);
    }

    bool stillConnected = conn.isConnected();
    conn.disconnect();

    ASSERT_MSG(!g_sawPingTimeout,
               "exchange did not answer a protocol ping — enabling the ping "
               "interval would churn connections instead of sustaining them");
    ASSERT_MSG(stillConnected,
               "idle connection did not survive the ping intervals");
    ASSERT_MSG(g_sawPong, "no pong observed — protocol ping is not running");
}

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------
int main() {
    printf("=== WebSocket Keepalive Tests [OPM-868] ===\n\n");

    ix::initNetSystem();

    RUN_TEST(keepalive_idle_connection_survives_ping_intervals);

    int result = hl::test::printTestSummary();
    ix::uninitNetSystem();
    return result;
}
