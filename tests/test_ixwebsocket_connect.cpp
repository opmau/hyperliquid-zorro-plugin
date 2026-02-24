//=============================================================================
// test_ixwebsocket_connect.cpp - IXWebSocket integration test [OPM-126]
//=============================================================================
// Part of Hyperliquid Plugin for Zorro
//
// LAYER: Test
// PURPOSE: Verify IXWebSocket links and can connect to Hyperliquid testnet.
//          Step 1 acceptance test for OPM-126.
//=============================================================================

#include "test_framework.h"
#include <IXWebSocket.h>
#include <IXNetSystem.h>
#include <atomic>
#include <chrono>
#include <thread>
#include <string>

//-----------------------------------------------------------------------------
// Test: IXWebSocket library links and basic object construction works
//-----------------------------------------------------------------------------
TEST_CASE(ixwebsocket_links) {
    ix::WebSocket ws;
    ws.setUrl("wss://api.hyperliquid-testnet.xyz/ws");
    ws.disableAutomaticReconnection();
    ASSERT_TRUE(true);
}

//-----------------------------------------------------------------------------
// Test: Can connect to Hyperliquid testnet via IXWebSocket
//-----------------------------------------------------------------------------
TEST_CASE(ixwebsocket_connects_to_testnet) {
    std::atomic<bool> connected{false};
    std::atomic<bool> gotError{false};
    std::string errorMsg;

    ix::WebSocket ws;
    ws.setUrl("wss://api.hyperliquid-testnet.xyz/ws");
    ws.disableAutomaticReconnection();

    ws.setOnMessageCallback([&](const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Open) {
            connected = true;
        } else if (msg->type == ix::WebSocketMessageType::Error) {
            errorMsg = msg->errorInfo.reason;
            gotError = true;
        }
    });

    ws.start();

    auto start = std::chrono::steady_clock::now();
    while (!connected && !gotError) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        auto elapsed = std::chrono::steady_clock::now() - start;
        if (std::chrono::duration_cast<std::chrono::seconds>(elapsed).count() > 10)
            break;
    }

    ws.stop();

    if (gotError) {
        printf("\n  Connection error: %s\n", errorMsg.c_str());
    }
    ASSERT_MSG(connected.load(), "Should connect to testnet within 10 seconds");
}

//-----------------------------------------------------------------------------
// Test: Can send subscription and receive data via IXWebSocket
//-----------------------------------------------------------------------------
TEST_CASE(ixwebsocket_send_receive) {
    std::atomic<bool> connected{false};
    std::atomic<bool> gotMessage{false};

    ix::WebSocket ws;
    ws.setUrl("wss://api.hyperliquid-testnet.xyz/ws");
    ws.disableAutomaticReconnection();

    ws.setOnMessageCallback([&](const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Open)
            connected = true;
        else if (msg->type == ix::WebSocketMessageType::Message)
            gotMessage = true;
    });

    ws.start();
    std::this_thread::sleep_for(std::chrono::seconds(3));
    if (!connected) { ws.stop(); printf("\n  Skip: connect failed\n"); return; }

    ws.send(R"({"method":"subscribe","subscription":{"type":"allMids"}})");
    std::this_thread::sleep_for(std::chrono::seconds(3));
    ws.stop();
    ASSERT_MSG(gotMessage.load(), "Should receive allMids data within 5 seconds");
}

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------
int main() {
    printf("=== IXWebSocket Integration Tests [OPM-126] ===\n\n");

    ix::initNetSystem();

    RUN_TEST(ixwebsocket_links);
    RUN_TEST(ixwebsocket_connects_to_testnet);
    RUN_TEST(ixwebsocket_send_receive);

    int result = hl::test::printTestSummary();
    ix::uninitNetSystem();
    return result;
}
