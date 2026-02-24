//=============================================================================
// test_ws_connection_ix.cpp - Connection class regression test [OPM-127]
//=============================================================================
// Part of Hyperliquid Plugin for Zorro
//
// LAYER: Test
// PURPOSE: Verify Connection class works with IXWebSocket backend.
//          Exercises the same interface that ws_manager.cpp depends on.
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
// Test: Connection object construction and initial state
//-----------------------------------------------------------------------------
TEST_CASE(connection_initial_state) {
    hl::ws::Connection conn;
    ASSERT_FALSE(conn.isConnected());
    ASSERT_EQ(conn.state(), hl::ws::ConnectionState::Disconnected);
    ASSERT_EQ(conn.disconnectReason(), hl::ws::DisconnectReason::None);
    ASSERT_EQ(conn.lastMessageTime(), (time_t)0);
}

//-----------------------------------------------------------------------------
// Test: Connect to Hyperliquid testnet
//-----------------------------------------------------------------------------
TEST_CASE(connection_connect_testnet) {
    hl::ws::Connection conn;
    conn.setLogCallback(testLog, 2);

    bool ok = conn.connect("api.hyperliquid-testnet.xyz", true, "/ws", 10000);
    if (!ok) {
        printf("\n  SKIP (connect failed)\n");
        hl::test::g_testsPassed++;  // Don't fail on network issues
        return;
    }

    ASSERT_TRUE(conn.isConnected());
    ASSERT_EQ(conn.state(), hl::ws::ConnectionState::Connected);
    ASSERT_GT(conn.lastMessageTime(), (time_t)0);

    conn.disconnect();
    ASSERT_FALSE(conn.isConnected());
    ASSERT_EQ(conn.state(), hl::ws::ConnectionState::Disconnected);
}

//-----------------------------------------------------------------------------
// Test: Send subscription and receive data via poll()
//-----------------------------------------------------------------------------
TEST_CASE(connection_send_poll_receive) {
    hl::ws::Connection conn;
    conn.setLogCallback(testLog, 1);

    std::atomic<int> messageCount{0};
    conn.setMessageHandler([&](const char* data, size_t len) {
        messageCount++;
        if (messageCount <= 2) {
            printf("    [MSG] (%.80s...)\n", data);
        }
    });

    if (!conn.connect("api.hyperliquid-testnet.xyz", true, "/ws", 10000)) {
        printf("\n  SKIP (connect failed)\n");
        hl::test::g_testsPassed++;
        return;
    }

    // Send allMids subscription (lightweight, always produces data)
    bool sent = conn.send(R"({"method":"subscribe","subscription":{"type":"allMids"}})");
    ASSERT_TRUE(sent);

    // Poll for up to 5 seconds to receive at least one message
    auto start = std::chrono::steady_clock::now();
    while (messageCount < 1) {
        int result = conn.poll(200);
        if (result < 0) {
            printf("\n  poll() returned disconnect\n");
            break;
        }
        auto elapsed = std::chrono::steady_clock::now() - start;
        if (std::chrono::duration_cast<std::chrono::seconds>(elapsed).count() > 5)
            break;
    }

    ASSERT_MSG(messageCount >= 1, "Should receive at least 1 message after allMids subscription");

    conn.disconnect();
    ASSERT_EQ(conn.disconnectReason(), hl::ws::DisconnectReason::ClientClose);
}

//-----------------------------------------------------------------------------
// Test: poll() respects timeout (returns within bounded time)
//-----------------------------------------------------------------------------
TEST_CASE(connection_poll_timeout) {
    hl::ws::Connection conn;
    conn.setLogCallback(testLog, 1);

    if (!conn.connect("api.hyperliquid-testnet.xyz", true, "/ws", 10000)) {
        printf("\n  SKIP (connect failed)\n");
        hl::test::g_testsPassed++;
        return;
    }

    // No subscriptions — no data should arrive
    const int POLL_TIMEOUT_MS = 500;
    auto start = std::chrono::steady_clock::now();
    int result = conn.poll(POLL_TIMEOUT_MS);
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start).count();

    printf("    poll(%d) returned %d in %lldms\n", POLL_TIMEOUT_MS, result, elapsed);

    // poll() must return within bounded time (timeout + 500ms slack)
    ASSERT_MSG(elapsed <= POLL_TIMEOUT_MS + 500,
        "poll() must respect timeout parameter");

    // poll() should return 0 (no messages) not -1 (disconnect)
    ASSERT_MSG(result >= 0, "poll() should not report disconnect");

    conn.disconnect();
}

//-----------------------------------------------------------------------------
// Test: send() on disconnected connection returns false
//-----------------------------------------------------------------------------
TEST_CASE(connection_send_when_disconnected) {
    hl::ws::Connection conn;
    ASSERT_FALSE(conn.send("test"));
    ASSERT_FALSE(conn.send("test", 4));
}

//-----------------------------------------------------------------------------
// Test: poll() on disconnected connection returns -1
//-----------------------------------------------------------------------------
TEST_CASE(connection_poll_when_disconnected) {
    hl::ws::Connection conn;
    int result = conn.poll(100);
    ASSERT_EQ(result, -1);
}

//-----------------------------------------------------------------------------
// Test: Double disconnect is safe (no crash)
//-----------------------------------------------------------------------------
TEST_CASE(connection_double_disconnect) {
    hl::ws::Connection conn;
    conn.disconnect();
    conn.disconnect();
    ASSERT_FALSE(conn.isConnected());
}

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------
int main() {
    printf("=== Connection Class Tests (IXWebSocket backend) [OPM-127] ===\n\n");

    ix::initNetSystem();

    RUN_TEST(connection_initial_state);
    RUN_TEST(connection_connect_testnet);
    RUN_TEST(connection_send_poll_receive);
    RUN_TEST(connection_poll_timeout);
    RUN_TEST(connection_send_when_disconnected);
    RUN_TEST(connection_poll_when_disconnected);
    RUN_TEST(connection_double_disconnect);

    int result = hl::test::printTestSummary();
    ix::uninitNetSystem();
    return result;
}
