//=============================================================================
// hl_trading_service.h - Order management service
//=============================================================================
// Part of Hyperliquid Plugin for Zorro
//
// LAYER: Services
// DEPENDENCIES: hl_types.h, hl_globals.h, hl_crypto.h, hl_http.h, ws_manager.h
// THREAD SAFETY: All public functions are thread-safe (use g_trading.tradeCs)
//
// This module provides:
// - Order placement (build, sign, submit via HTTP or WebSocket)
// - Order cancellation
// - Trade ID management (Zorro <-> Hyperliquid mapping)
// - Order status tracking
//=============================================================================

#pragma once

#include "../foundation/hl_types.h"
#include <string>
#include <functional>
#include <cstdint>

namespace hl {
namespace trading {

// =============================================================================
// ORDER PLACEMENT
// =============================================================================

/// Place an order
/// @param request Order parameters
/// @return OrderResult with success status, order ID, and fill info
///
/// This function:
/// 1. Validates the request (price, size)
/// 2. Generates a CLOID with embedded trade ID
/// 3. Builds and signs the order (EIP-712)
/// 4. Submits via WebSocket (if available) or HTTP
/// 5. Stores the trade mapping
/// 6. Returns immediately with the trade ID (actual fill is async)
OrderResult placeOrder(const OrderRequest& request);

/// Place an order with explicit trade ID (for pre-generated IDs)
/// @param request Order parameters
/// @param tradeId Pre-generated Zorro trade ID to use
/// @return OrderResult with success status
OrderResult placeOrderWithId(const OrderRequest& request, int tradeId);

/// Cancel an order by exchange order ID
/// @param coin Asset name (e.g., "BTC")
/// @param oid Exchange order ID
/// @return true if cancel was submitted successfully
bool cancelOrder(const char* coin, const char* oid);

/// Cancel an order by Zorro trade ID
/// @param tradeId Zorro trade ID
/// @return true if cancel was submitted successfully
bool cancelOrderByTradeId(int tradeId);

/// Cancel all open orders for a coin (or all coins if nullptr)
/// @param coin Asset name, or nullptr for all
/// @return Number of cancellation requests sent
int cancelAllOrders(const char* coin = nullptr);

// =============================================================================
// TRADE ID MANAGEMENT
// =============================================================================

/// Generate a new unique trade ID
/// Thread-safe (uses atomic increment)
int generateTradeId();

/// Generate a CLOID (client order ID) with embedded trade ID
/// Format: HLZorro_TIMESTAMP_TRADEID_RANDOM
/// @param tradeId Trade ID to embed
/// @param outCloid Output buffer (minimum 64 chars)
/// @param outSize Size of output buffer
void generateCloid(int tradeId, char* outCloid, size_t outSize);

/// Extract trade ID from CLOID (if it was created by this plugin)
/// @param cloid Client order ID
/// @return Extracted trade ID, or 0 if not parseable
int extractTradeIdFromCloid(const char* cloid);

/// Generate a monotonic nonce (timestamp + counter to avoid collisions)
uint64_t generateNonce();

// =============================================================================
// ORDER TRACKING
// =============================================================================

/// Store a trade mapping (called after order placement)
/// @param tradeId Zorro trade ID
/// @param state Order state to store
void storeOrder(int tradeId, const OrderState& state);

/// Get order state by Zorro trade ID
/// @param tradeId Zorro trade ID
/// @param outState Output for order state
/// @return true if found
bool getOrder(int tradeId, OrderState& outState);

/// Get order state by CLOID
/// @param cloid Client order ID
/// @param outState Output for order state
/// @return true if found
bool getOrderByCloid(const char* cloid, OrderState& outState);

/// Get order state by exchange order ID
/// @param oid Exchange order ID
/// @param outState Output for order state
/// @return true if found
bool getOrderByOid(const char* oid, OrderState& outState);

/// Update order state (e.g., from fill notification)
/// @param tradeId Zorro trade ID
/// @param filledSize New filled size
/// @param avgPrice New average price
/// @param status New status
/// @return true if order was found and updated
bool updateOrder(int tradeId, double filledSize, double avgPrice, OrderStatus status);

/// Update order by CLOID
/// @param cloid Client order ID
/// @param filledSize New filled size
/// @param avgPrice New average price
/// @param status New status
/// @return true if order was found and updated
bool updateOrderByCloid(const char* cloid, double filledSize, double avgPrice, OrderStatus status);

/// Remove order from tracking (e.g., after full fill or cancel)
/// @param tradeId Zorro trade ID
void removeOrder(int tradeId);

/// Find trade ID by CLOID
/// @param cloid Client order ID
/// @return Trade ID, or 0 if not found
int findTradeIdByCloid(const char* cloid);

/// Find trade ID by exchange order ID
/// @param oid Exchange order ID
/// @return Trade ID, or 0 if not found
int findTradeIdByOid(const char* oid);

// =============================================================================
// ORDER STATUS QUERY (three-state reconciliation) [OPM-89]
// =============================================================================

/// Query exchange for order status by CLOID.
/// Uses POST /info {"type":"orderStatus","user":"...","oid":"<cloid>"}
/// Returns three-state result: Found (order on exchange), NotFound (safe to retry),
/// Failed (uncertain — caller should assume order may exist).
CloidQueryResult queryOrderByCloid(const char* cloid);

// =============================================================================
// CONFIGURATION
// =============================================================================

/// Set the order type for new orders
/// @param orderType "Ioc", "Gtc", or "Alo"
void setOrderType(const char* orderType);

/// Get current order type
const char* getOrderType();

/// Set dry run mode (build orders but don't send)
void setDryRun(bool enabled);

/// Check if dry run mode is enabled
bool isDryRun();

// =============================================================================
// CALLBACKS (for async fill notifications)
// =============================================================================

/// Fill callback type (called when order is filled or partially filled)
using FillCallback = std::function<void(int tradeId, double filledSize, double avgPrice, OrderStatus status)>;

/// Set fill callback (called from WebSocket message handler)
void setFillCallback(FillCallback callback);

/// Called internally when a fill is received
void notifyFill(const char* cloid, double filledSize, double avgPrice, const char* status);

// =============================================================================
// INITIALIZATION
// =============================================================================

/// Initialize trading service (call once at login)
void init();

/// Cleanup trading service (call at logout)
void cleanup();

} // namespace trading
} // namespace hl
