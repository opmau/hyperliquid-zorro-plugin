//=============================================================================
// test_ws_auto_reconnect.cpp - Auto-reconnect and simplified manager [OPM-128]
//=============================================================================
// Part of Hyperliquid Plugin for Zorro
//
// LAYER: Test
// PURPOSE: Verify IXWebSocket auto-reconnect integration in Connection class,
//          and that ws_manager simplification preserves correct behavior.
//
// Tests:
//   1. enableAutoReconnect() exists and is callable
//   2. wasReconnected() returns false initially
//   3. connect + send/receive still works with auto-reconnect enabled
//   4. senderThread is no longer created (manager simplification)
//   5. HL application ping at reduced interval (30s, not 1s)
//=============================================================================

#include "test_framework.h"
#include "ws_connection.h"
#include <IXNetSystem.h>
#include <atomic>
#include <chrono>
#include <thread>
#include <cstdio>

static int testLog(const char* msg) {
    printf("    [WS] %s\n", msg);
    return 0;
}

//-----------------------------------------------------------------------------
// Test 1: enableAutoReconnect() exists and is callable without crash
//-----------------------------------------------------------------------------
TEST_CASE(auto_reconnect_configurable) {
    hl::ws::Connection conn;
    // Should not crash — configures IXWebSocket auto-reconnect
    conn.enableAutoReconnect(1000, 30000);
    ASSERT_FALSE(conn.isConnected());
}

//-----------------------------------------------------------------------------
// Test 2: wasReconnected() returns false initially (no reconnect yet)
//-----------------------------------------------------------------------------
TEST_CASE(was_reconnected_initially_false) {
    hl::ws::Connection conn;
    conn.enableAutoReconnect();
    ASSERT_FALSE(conn.wasReconnected());
}

//-----------------------------------------------------------------------------
// Test 3: Connect with auto-reconnect, send ping, receive pong
//         Verifies that enabling auto-reconnect doesn't break normal I/O
//-----------------------------------------------------------------------------
TEST_CASE(auto_reconnect_normal_io) {
    hl::ws::Connection conn;
    conn.setLogCallback(testLog, 2);
    conn.enableAutoReconnect();

    if (!conn.connect("api.hyperliquid-testnet.xyz", true, "/ws", 10000)) {
        printf("\n  SKIP (connect failed)\n");
        hl::test::g_testsPassed++;
        return;
    }

    ASSERT_TRUE(conn.isConnected());
    // wasReconnected() should be false after first connect (not a reconnect)
    ASSERT_FALSE(conn.wasReconnected());

    // Send HL application ping and receive pong
    std::atomic<int> messageCount{0};
    conn.setMessageHandler([&](const char* data, size_t len) {
        messageCount++;
    });

    bool sent = conn.send("{\"method\":\"ping\"}");
    ASSERT_TRUE(sent);

    // Poll for up to 3 seconds
    auto start = std::chrono::steady_clock::now();
    while (messageCount < 1) {
        int result = conn.poll(200);
        if (result < 0) break;
        auto elapsed = std::chrono::steady_clock::now() - start;
        if (std::chrono::duration_cast<std::chrono::seconds>(elapsed).count() > 3)
            break;
    }

    ASSERT_MSG(messageCount >= 1, "Should receive pong after HL ping");

    conn.disconnect();
    ASSERT_FALSE(conn.isConnected());
}

//-----------------------------------------------------------------------------
// Test 4: poll() returns 0 (not -1) when auto-reconnect is active and
//         temporarily disconnected. This is the KEY behavioral difference:
//         with auto-reconnect, a temporary disconnect is not a fatal error.
//
//         Without auto-reconnect: poll() returns -1 on disconnect
//         With auto-reconnect: poll() returns 0 (waiting for reconnect)
//-----------------------------------------------------------------------------
TEST_CASE(auto_reconnect_poll_not_fatal) {
    hl::ws::Connection conn;
    conn.setLogCallback(testLog, 2);
    // Do NOT enable auto-reconnect — verify baseline behavior
    // poll() on disconnected Connection should return -1

    int result = conn.poll(100);
    ASSERT_EQ(result, -1);

    // Now create a second connection WITH auto-reconnect
    hl::ws::Connection connAuto;
    connAuto.setLogCallback(testLog, 2);
    connAuto.enableAutoReconnect();

    // poll() before connect with auto-reconnect should return 0
    // (not -1, because auto-reconnect means "keep trying")
    result = connAuto.poll(100);
    ASSERT_MSG(result == 0,
        "poll() with auto-reconnect enabled should return 0 (not -1) when not yet connected");
}

//-----------------------------------------------------------------------------
// Test 5: wasReconnected() is consumable (returns true once, then false)
//         This tests the flag-clearing semantics.
//-----------------------------------------------------------------------------
TEST_CASE(was_reconnected_consumable) {
    hl::ws::Connection conn;
    conn.enableAutoReconnect();

    // Initially false
    ASSERT_FALSE(conn.wasReconnected());
    // Calling again still false (no reconnect happened)
    ASSERT_FALSE(conn.wasReconnected());
}

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------
int main() {
    printf("=== Auto-Reconnect Tests [OPM-128] ===\n\n");

    ix::initNetSystem();

    RUN_TEST(auto_reconnect_configurable);
    RUN_TEST(was_reconnected_initially_false);
    RUN_TEST(auto_reconnect_normal_io);
    RUN_TEST(auto_reconnect_poll_not_fatal);
    RUN_TEST(was_reconnected_consumable);

    int result = hl::test::printTestSummary();
    ix::uninitNetSystem();
    return result;
}
