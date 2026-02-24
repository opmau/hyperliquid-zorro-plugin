//=============================================================================
// ws_types.h - WebSocket data structures and callback types
//=============================================================================
// Part of Hyperliquid Plugin for Zorro
//
// LAYER: Transport
// DEPENDENCIES: <windows.h>, <string>
// THREAD SAFETY: All structures are POD or simple types (thread-safety
//                depends on external synchronization)
//=============================================================================

#pragma once

#include <windows.h>
#include <string>

namespace hl {
namespace ws {

//=============================================================================
// PRICE AND MARKET DATA
//=============================================================================

// Price data with bid/ask/mid and timestamp
struct PriceData {
    double bid;
    double ask;
    double mid;
    DWORD timestamp;  // GetTickCount() when updated

    PriceData() : bid(0), ask(0), mid(0), timestamp(0) {}
};

//=============================================================================
// ACCOUNT DATA
//=============================================================================

// Account data from webData3/clearinghouseState subscription
struct AccountData {
    double accountValue;      // Perps equity (from marginSummary/crossMarginSummary)
    double totalMarginUsed;   // Margin currently used
    double withdrawable;      // Available to withdraw
    double totalNtlPos;       // Total notional position value
    double spotUSDC;          // Spot USDC balance (from spotClearinghouseState)
    DWORD timestamp;
    DWORD spotTimestamp;      // When spot data was last fetched

    AccountData() : accountValue(0), totalMarginUsed(0), withdrawable(0),
                   totalNtlPos(0), spotUSDC(0), timestamp(0), spotTimestamp(0) {}
};

// Position data from clearinghouseState subscription
struct PositionData {
    std::string coin;
    double size;           // Positive = long, negative = short
    double entryPx;        // Entry price
    double unrealizedPnl;  // Unrealized P&L
    double leverage;       // Current leverage
    double liquidationPx;  // Liquidation price
    double marginUsed;     // Margin used for this position
    DWORD timestamp;

    PositionData() : size(0), entryPx(0), unrealizedPnl(0), leverage(0),
                    liquidationPx(0), marginUsed(0), timestamp(0) {}
};

//=============================================================================
// ORDER DATA
//=============================================================================

// Open order data from openOrders subscription
struct OpenOrderData {
    std::string oid;       // Order ID
    std::string coin;      // Asset
    bool isBuy;            // Buy or sell
    double limitPx;        // Limit price
    double sz;             // Current size (may be partially filled)
    double origSz;         // Original size
    std::string orderType; // "Limit", "Stop", etc.
    DWORD timestamp;

    OpenOrderData() : isBuy(false), limitPx(0), sz(0), origSz(0), timestamp(0) {}
};

// Fill data from userFills subscription
struct FillData {
    std::string oid;       // Order ID
    std::string coin;      // Asset
    bool isBuy;            // Buy or sell
    double px;             // Fill price
    double sz;             // Fill size
    double fee;            // Fee paid
    long long time;        // Fill timestamp (ms)
    std::string tid;       // Trade ID

    FillData() : isBuy(false), px(0), sz(0), fee(0), time(0) {}
};

//=============================================================================
// ORDER REQUEST/RESPONSE FOR WEBSOCKET POST
//=============================================================================

// Order request for WebSocket post
struct OrderRequest {
    int requestId;          // Unique ID for correlation
    std::string coin;
    bool isBuy;
    double limitPx;
    double sz;
    std::string orderType;  // "Ioc", "Gtc", "Alo"
    bool reduceOnly;
    int assetIndex;         // Asset index for API (0 = BTC, etc.)
    std::string cloid;      // Client order ID (optional)

    OrderRequest() : requestId(0), isBuy(false), limitPx(0), sz(0),
                    reduceOnly(false), assetIndex(0) {}
};

// Order response from WebSocket post
struct OrderResponse {
    int requestId;          // Matches request
    bool success;
    std::string oid;        // Order ID if successful
    std::string error;      // Error message if failed
    double filledSz;        // Amount filled
    double avgPx;           // Average fill price
    std::string status;     // "open", "filled", "canceled", etc.

    OrderResponse() : requestId(0), success(false), filledSz(0), avgPx(0) {}
};

// Cancel request for WebSocket post
struct CancelRequest {
    int requestId;
    std::string coin;
    std::string oid;        // Order ID to cancel
    int assetIndex;

    CancelRequest() : requestId(0), assetIndex(0) {}
};

//=============================================================================
// CALLBACK TYPES
//=============================================================================

// Order update callback (from orderUpdates subscription)
// Called on WS connection thread — implementation must be thread-safe
typedef void (*OrderUpdateCallback)(const char* oid, const char* cloid,
                                    const char* status, double filledSz,
                                    double avgPx);

// Order response callback (from post requests)
typedef void (*OrderResponseCallback)(const OrderResponse& response);

// Signature callback for signing order requests
typedef bool (*SignatureCallback)(const char* action, const char* nonce,
                                  char* sigR, char* sigS, int* sigV);

// Log callback (matches Zorro's BrokerMessage signature)
typedef int (*LogCallback)(const char*);

//=============================================================================
// CONNECTION STATE
//=============================================================================

enum class ConnectionState {
    Disconnected,
    Connecting,
    Connected,
    Reconnecting
};

/// Reason for the most recent WebSocket disconnect.
/// Used by the manager to produce descriptive reconnect log messages.
enum class DisconnectReason {
    None,           // Not disconnected (or reason unknown)
    ServerClose,    // Server sent a clean close frame
    NetworkError,   // Network/connection error
    ClientClose     // We initiated disconnect()
};

} // namespace ws
} // namespace hl
