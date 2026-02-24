//=============================================================================
// compile_test_transport.cpp - Compile-only test for transport layer
//=============================================================================
// This file just instantiates the transport classes to verify they compile.
// It's not meant to run successfully - just to catch compile errors.
//=============================================================================

#include "ws_types.h"
#include "ws_price_cache.h"
#include "ws_connection.h"
#include "ws_manager.h"

int main() {
    // Instantiate types
    hl::ws::PriceData priceData;
    hl::ws::AccountData accountData;
    hl::ws::PositionData positionData;
    hl::ws::OpenOrderData openOrder;
    hl::ws::FillData fill;
    hl::ws::OrderRequest orderReq;
    hl::ws::OrderResponse orderResp;
    hl::ws::CancelRequest cancelReq;

    // Instantiate cache
    hl::ws::PriceCache cache;
    cache.setBidAsk("TEST", 100.0, 101.0);

    // Instantiate connection (but don't connect)
    hl::ws::Connection conn;
    (void)conn.isConnected();

    // Instantiate manager (but don't start)
    hl::ws::WebSocketManager mgr(cache);
    (void)mgr.isRunning();

    // Verify enum
    hl::ws::ConnectionState state = hl::ws::ConnectionState::Disconnected;
    (void)state;

    return 0;
}
