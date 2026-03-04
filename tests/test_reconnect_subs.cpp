//=============================================================================
// test_reconnect_subs.cpp - Reconnect subscription burst test [OPM-170]
//=============================================================================
// Part of Hyperliquid Plugin for Zorro
//
// LAYER: Test
// PURPOSE: Verify whether sending subscriptions immediately after reconnect
//          causes connection drops (code 1006). Tests the hypothesis that
//          a burst of subscriptions on a fresh connection triggers the
//          reconnect loop described in OPM-170.
//
// Tests:
//   1. Fresh connection + 5 subs burst → connection stays alive?
//   2. Disconnect + reconnect + 5 subs burst → connection stays alive?
//   3. Disconnect + reconnect + delayed subs → connection stays alive?
//=============================================================================

#include "test_framework.h"
#include "ws_connection.h"
#include <IXNetSystem.h>
#include <atomic>
#include <chrono>
#include <thread>
#include <cstdio>
#include <vector>
#include <string>

static int testLog(const char* msg) {
    printf("    [WS] %s\n", msg);
    return 0;
}

// Track close codes received
static std::atomic<int> g_closeCode{0};
static std::atomic<int> g_messageCount{0};

static const char* TESTNET_HOST = "api.hyperliquid-testnet.xyz";
// Dummy address for subscription tests (server should accept but send empty data)
static const char* DUMMY_ADDR = "0x0000000000000000000000000000000000000000";

// Helper: send the 5 subscriptions that ws_manager sends on reconnect
static int sendReconnectBurst(hl::ws::Connection& conn) {
    int sent = 0;
    char buf[512];

    // 1. orderUpdates (from subscribeInitialChannels)
    sprintf_s(buf, "{\"method\":\"subscribe\",\"subscription\":"
             "{\"type\":\"orderUpdates\",\"user\":\"%s\"}}", DUMMY_ADDR);
    if (conn.send(buf)) sent++;

    // 2. l2Book for BTC (from sendPendingL2Subscriptions)
    if (conn.send("{\"method\":\"subscribe\",\"subscription\":"
                  "{\"type\":\"l2Book\",\"coin\":\"BTC\"}}")) sent++;

    // 3. userFills (from sendPendingAccountSubscriptions)
    sprintf_s(buf, "{\"method\":\"subscribe\",\"subscription\":"
             "{\"type\":\"userFills\",\"user\":\"%s\"}}", DUMMY_ADDR);
    if (conn.send(buf)) sent++;

    // 4. clearinghouseState
    sprintf_s(buf, "{\"method\":\"subscribe\",\"subscription\":"
             "{\"type\":\"clearinghouseState\",\"user\":\"%s\"}}", DUMMY_ADDR);
    if (conn.send(buf)) sent++;

    // 5. openOrders
    sprintf_s(buf, "{\"method\":\"subscribe\",\"subscription\":"
             "{\"type\":\"openOrders\",\"user\":\"%s\"}}", DUMMY_ADDR);
    if (conn.send(buf)) sent++;

    return sent;
}

// Helper: poll for N seconds, return true if connection stayed alive
static bool pollAndCheck(hl::ws::Connection& conn, int seconds) {
    auto start = std::chrono::steady_clock::now();
    while (true) {
        int result = conn.poll(200);
        if (result < 0) {
            printf("    poll() returned -1 (disconnect)\n");
            return false;
        }
        auto elapsed = std::chrono::steady_clock::now() - start;
        if (std::chrono::duration_cast<std::chrono::seconds>(elapsed).count() >= seconds)
            break;
    }
    return conn.isConnected();
}

//-----------------------------------------------------------------------------
// Test 1: Fresh connection + immediate subscription burst
//         Baseline: does a burst of 5 subs on a fresh connection cause 1006?
//-----------------------------------------------------------------------------
TEST_CASE(fresh_connection_sub_burst) {
    hl::ws::Connection conn;
    conn.setLogCallback(testLog, 2);

    if (!conn.connect(TESTNET_HOST, true, "/ws", 10000)) {
        printf("\n  SKIP (connect failed)\n");
        hl::test::g_testsPassed++;
        return;
    }

    g_messageCount = 0;
    conn.setMessageHandler([&](const char* data, size_t len) {
        g_messageCount++;
    });

    // Send burst immediately (no delay after connect)
    int sent = sendReconnectBurst(conn);
    printf("    Sent %d/5 subscriptions (burst, no delay)\n", sent);
    ASSERT_EQ(sent, 5);

    // Poll for 5 seconds — connection should stay alive
    bool alive = pollAndCheck(conn, 5);
    printf("    Connection alive: %s (messages received: %d)\n",
           alive ? "YES" : "NO", g_messageCount.load());

    if (!alive) {
        printf("    Disconnect reason: %d, error code: %d\n",
               (int)conn.disconnectReason(), conn.disconnectError());
    }

    ASSERT_TRUE(alive);

    conn.disconnect();
}

//-----------------------------------------------------------------------------
// Test 2: Disconnect + reconnect + immediate subscription burst
//         The reconnect scenario from OPM-170: reconnect then burst 5 subs
//-----------------------------------------------------------------------------
TEST_CASE(reconnect_immediate_sub_burst) {
    hl::ws::Connection conn;
    conn.setLogCallback(testLog, 2);
    conn.enableAutoReconnect(1000, 30000);

    if (!conn.connect(TESTNET_HOST, true, "/ws", 10000)) {
        printf("\n  SKIP (connect failed)\n");
        hl::test::g_testsPassed++;
        return;
    }

    g_messageCount = 0;
    conn.setMessageHandler([&](const char* data, size_t len) {
        g_messageCount++;
    });

    // Verify initial connection works
    ASSERT_TRUE(conn.isConnected());
    ASSERT_FALSE(conn.wasReconnected());

    // Force disconnect by stopping auto-reconnect (which calls ws_.stop())
    printf("    Forcing disconnect...\n");
    conn.stopAutoReconnect();
    ASSERT_FALSE(conn.isConnected());

    // Re-enable auto-reconnect and reconnect (simulating the real flow)
    printf("    Reconnecting...\n");
    conn.enableAutoReconnect(1000, 30000);
    if (!conn.connect(TESTNET_HOST, true, "/ws", 10000)) {
        printf("\n  SKIP (reconnect failed)\n");
        hl::test::g_testsPassed++;
        return;
    }

    ASSERT_TRUE(conn.isConnected());

    // NOW: send burst immediately after reconnect (like the manager does)
    g_messageCount = 0;
    int sent = sendReconnectBurst(conn);
    printf("    Sent %d/5 subscriptions after reconnect (burst, no delay)\n", sent);
    ASSERT_EQ(sent, 5);

    // Poll for 5 seconds — does the connection survive?
    bool alive = pollAndCheck(conn, 5);
    printf("    Connection alive after reconnect+burst: %s (messages: %d)\n",
           alive ? "YES" : "NO", g_messageCount.load());

    if (!alive) {
        printf("    *** REPRODUCED OPM-170: Connection dropped after reconnect + sub burst ***\n");
        printf("    Disconnect reason: %d, error code: %d\n",
               (int)conn.disconnectReason(), conn.disconnectError());
    }

    ASSERT_TRUE(alive);

    conn.disconnect();
}

//-----------------------------------------------------------------------------
// Test 3: Disconnect + reconnect + DELAYED subscription sends
//         If the burst causes 1006, does adding a delay fix it?
//-----------------------------------------------------------------------------
TEST_CASE(reconnect_delayed_subs) {
    hl::ws::Connection conn;
    conn.setLogCallback(testLog, 2);
    conn.enableAutoReconnect(1000, 30000);

    if (!conn.connect(TESTNET_HOST, true, "/ws", 10000)) {
        printf("\n  SKIP (connect failed)\n");
        hl::test::g_testsPassed++;
        return;
    }

    ASSERT_TRUE(conn.isConnected());

    // Force disconnect and reconnect
    printf("    Forcing disconnect...\n");
    conn.stopAutoReconnect();
    conn.enableAutoReconnect(1000, 30000);
    if (!conn.connect(TESTNET_HOST, true, "/ws", 10000)) {
        printf("\n  SKIP (reconnect failed)\n");
        hl::test::g_testsPassed++;
        return;
    }

    ASSERT_TRUE(conn.isConnected());

    // Wait 500ms grace period before subscribing
    printf("    Waiting 500ms grace period...\n");
    Sleep(500);

    g_messageCount = 0;
    conn.setMessageHandler([&](const char* data, size_t len) {
        g_messageCount++;
    });

    // Send subscriptions with 100ms delay between each
    char buf[512];
    int sent = 0;

    sprintf_s(buf, "{\"method\":\"subscribe\",\"subscription\":"
             "{\"type\":\"orderUpdates\",\"user\":\"%s\"}}", DUMMY_ADDR);
    if (conn.send(buf)) sent++;
    Sleep(100);

    if (conn.send("{\"method\":\"subscribe\",\"subscription\":"
                  "{\"type\":\"l2Book\",\"coin\":\"BTC\"}}")) sent++;
    Sleep(100);

    sprintf_s(buf, "{\"method\":\"subscribe\",\"subscription\":"
             "{\"type\":\"userFills\",\"user\":\"%s\"}}", DUMMY_ADDR);
    if (conn.send(buf)) sent++;
    Sleep(100);

    sprintf_s(buf, "{\"method\":\"subscribe\",\"subscription\":"
             "{\"type\":\"clearinghouseState\",\"user\":\"%s\"}}", DUMMY_ADDR);
    if (conn.send(buf)) sent++;
    Sleep(100);

    sprintf_s(buf, "{\"method\":\"subscribe\",\"subscription\":"
             "{\"type\":\"openOrders\",\"user\":\"%s\"}}", DUMMY_ADDR);
    if (conn.send(buf)) sent++;

    printf("    Sent %d/5 subscriptions after reconnect (delayed)\n", sent);
    ASSERT_EQ(sent, 5);

    // Poll for 5 seconds
    bool alive = pollAndCheck(conn, 5);
    printf("    Connection alive after reconnect+delayed subs: %s (messages: %d)\n",
           alive ? "YES" : "NO", g_messageCount.load());

    ASSERT_TRUE(alive);

    conn.disconnect();
}

//-----------------------------------------------------------------------------
// Test 4: Rapid reconnect cycles (3 times) with immediate burst
//         Tests whether repeated reconnect+burst triggers the loop
//-----------------------------------------------------------------------------
TEST_CASE(rapid_reconnect_cycles) {
    hl::ws::Connection conn;
    conn.setLogCallback(testLog, 2);
    conn.enableAutoReconnect(1000, 30000);

    int cyclesFailed = 0;

    for (int cycle = 0; cycle < 3; ++cycle) {
        printf("    --- Cycle %d ---\n", cycle + 1);

        if (!conn.connect(TESTNET_HOST, true, "/ws", 10000)) {
            printf("    SKIP cycle %d (connect failed)\n", cycle + 1);
            continue;
        }

        ASSERT_TRUE(conn.isConnected());

        g_messageCount = 0;
        conn.setMessageHandler([&](const char* data, size_t len) {
            g_messageCount++;
        });

        // Send burst immediately
        int sent = sendReconnectBurst(conn);
        printf("    Sent %d subs\n", sent);

        // Poll for 3 seconds
        bool alive = pollAndCheck(conn, 3);
        printf("    Alive: %s (msgs: %d)\n",
               alive ? "YES" : "NO", g_messageCount.load());

        if (!alive) {
            cyclesFailed++;
            printf("    *** Cycle %d FAILED: disconnect reason=%d error=%d ***\n",
                   cycle + 1, (int)conn.disconnectReason(), conn.disconnectError());
        }

        // Disconnect for next cycle
        conn.stopAutoReconnect();
        conn.enableAutoReconnect(1000, 30000);
    }

    printf("    Cycles failed: %d / 3\n", cyclesFailed);
    ASSERT_EQ(cyclesFailed, 0);
}

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------
int main() {
    printf("=== Reconnect Subscription Tests [OPM-170] ===\n\n");

    ix::initNetSystem();

    RUN_TEST(fresh_connection_sub_burst);
    RUN_TEST(reconnect_immediate_sub_burst);
    RUN_TEST(reconnect_delayed_subs);
    RUN_TEST(rapid_reconnect_cycles);

    int result = hl::test::printTestSummary();
    ix::uninitNetSystem();
    return result;
}
