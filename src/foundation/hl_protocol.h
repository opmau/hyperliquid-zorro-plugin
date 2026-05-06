//=============================================================================
// hl_protocol.h - Canonical Hyperliquid protocol identifier strings
//=============================================================================
// Part of Hyperliquid Plugin for Zorro
//
// LAYER: Foundation
// DEPENDENCIES: None
// THREAD SAFETY: All constants — read-only, no synchronization required
//
// Centralizes the wire-format string identifiers used across HL's protocols:
//   - WebSocket channel names           (incoming message routing)
//   - WebSocket subscription type names (outgoing subscribe payloads)
//   - HTTP /info request type names     (POST body "type" field)
//   - Exchange action type names        (POST /exchange "action.type" field)
//
// Why this exists:
//   Before this header, channel/type strings were repeated as raw literals
//   across ws_manager.cpp dispatch, hl_account_service.cpp HTTP requests,
//   and hl_meta_*.cpp metadata fetches. A typo would silently subscribe to
//   a non-existent channel or send an unrecognized request.
//
// Migration policy:
//   Use the constants in NEW code unconditionally. Existing string-literal
//   call sites are migrated opportunistically — see OPM-446 for the
//   incremental migration plan.
//
// Protocol references (verified 2026-05-06):
//   WS:        https://hyperliquid.gitbook.io/hyperliquid-docs/for-developers/api/websocket
//   /info:     https://hyperliquid.gitbook.io/hyperliquid-docs/for-developers/api/info-endpoint
//   /exchange: https://hyperliquid.gitbook.io/hyperliquid-docs/for-developers/api/exchange-endpoint
//=============================================================================

#pragma once

namespace hl {
namespace protocol {

// =====================================================================
// WebSocket channel names — appear in the "channel" field of inbound
// messages. Used by WebSocketManager::handleMessage() for routing.
// =====================================================================
namespace ws_channel {

constexpr const char* L2_BOOK              = "l2Book";
constexpr const char* CLEARINGHOUSE_STATE  = "clearinghouseState";
constexpr const char* OPEN_ORDERS          = "openOrders";
constexpr const char* USER_FILLS           = "userFills";
constexpr const char* ORDER_UPDATES        = "orderUpdates";
constexpr const char* ALL_MIDS             = "allMids";
constexpr const char* POST                 = "post";
constexpr const char* PONG                 = "pong";
constexpr const char* SUBSCRIPTION_RESPONSE = "subscriptionResponse";
// NOTE: spelled ERR (not ERROR) because <wingdi.h> defines ERROR as a macro.
constexpr const char* ERR                  = "error";

} // namespace ws_channel

// =====================================================================
// WebSocket subscription "type" values — appear inside the
// "subscription":{"type":"..."} field of outbound subscribe payloads.
//
// Note: Most subscription types share the wire string with their
// inbound channel name (e.g. "l2Book"). They are duplicated here under
// different names so callers can express intent ("I'm subscribing")
// vs. ("I'm dispatching an inbound message") without coupling.
// =====================================================================
namespace ws_subscription {

constexpr const char* L2_BOOK              = "l2Book";
constexpr const char* CLEARINGHOUSE_STATE  = "clearinghouseState";
constexpr const char* OPEN_ORDERS          = "orders";   // subscribe type "orders" yields channel "openOrders"
constexpr const char* USER_FILLS           = "userFills";
constexpr const char* ORDER_UPDATES        = "orderUpdates";
constexpr const char* ALL_MIDS             = "allMids";

} // namespace ws_subscription

// =====================================================================
// HTTP POST /info request "type" values — appear in the body of
// /info requests as {"type":"X","user":"...",...}.
// =====================================================================
namespace info_type {

constexpr const char* META                       = "meta";
constexpr const char* SPOT_META                  = "spotMeta";
constexpr const char* PERP_DEXS                  = "perpDexs";
constexpr const char* L2_BOOK                    = "l2Book";
constexpr const char* ALL_MIDS                   = "allMids";
constexpr const char* CANDLE_SNAPSHOT            = "candleSnapshot";
constexpr const char* CLEARINGHOUSE_STATE        = "clearinghouseState";
constexpr const char* SPOT_CLEARINGHOUSE_STATE   = "spotClearinghouseState";
constexpr const char* OPEN_ORDERS                = "openOrders";
constexpr const char* USER_FILLS                 = "userFills";
constexpr const char* ORDER_STATUS               = "orderStatus";
constexpr const char* USER_ROLE                  = "userRole";
constexpr const char* USER_ABSTRACTION           = "userAbstraction";
constexpr const char* FUNDING_HISTORY            = "fundingHistory";

} // namespace info_type

// =====================================================================
// POST /exchange action "type" values — appear in the
// signed action payload as {"type":"X",...}. Order-of-fields and
// exact spelling are SIGNATURE-CRITICAL: a typo will cause the
// server to reject with "L1 error" because the recovered address
// will not match. Do not change these values.
// =====================================================================
namespace action_type {

constexpr const char* ORDER            = "order";
constexpr const char* CANCEL           = "cancel";
constexpr const char* CANCEL_BY_CLOID  = "cancelByCloid";
constexpr const char* MODIFY           = "modify";
constexpr const char* BATCH_MODIFY     = "batchModify";
constexpr const char* SCHEDULE_CANCEL  = "scheduleCancel";
constexpr const char* TWAP_ORDER       = "twapOrder";
constexpr const char* TWAP_CANCEL      = "twapCancel";

} // namespace action_type

} // namespace protocol
} // namespace hl
