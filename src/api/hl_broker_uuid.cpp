//=============================================================================
// hl_broker_uuid.cpp - Zorro UUID trade identifiers (GET_UUID / SET_UUID)
//=============================================================================
// Part of Hyperliquid Plugin for Zorro
//
// LAYER: API
// DEPENDENCIES: hl_broker_internal.h
// THREAD SAFETY: Main thread only (Zorro calls are single-threaded)
//
// An int trade ID only means something to the plugin process that issued it:
// the plugin keeps the ID -> order mapping in memory. A new Zorro process
// leaves Zorro holding IDs the plugin can no longer resolve, and every later
// close of those trades fails.
//
// With brokerCommand(50046, 1) BrokerBuy2 returns -1 instead, and Zorro reads
// the trade's identifier with GET_UUID and stores it with the trade. Before
// every later BrokerSell2, BrokerTrade or DO_CANCEL on that trade it sends the
// identifier back with SET_UUID and passes trade ID -1. The identifier names
// the coin, the side and the opening order's cloid (see hl_trade_uuid.h), so
// a trade the plugin no longer tracks can be rebuilt from the exchange.
//=============================================================================

#include "hl_broker_internal.h"

// Identifier of the trade BrokerBuy2 last opened, read by GET_UUID.
static char s_lastUuid[UUIDSIZE] = {0};
// Identifier sent by SET_UUID, consumed by the next call made with ID -1.
static char s_pendingUuid[UUIDSIZE] = {0};

static double nowDate() {
    return (double)time(NULL) / 86400.0 + 25569.0;
}

bool uuidRecordOpened(const char* coin, hl::OrderSide side, const char* cloid) {
    if (!hl::uuid::format(coin, side, cloid, s_lastUuid, sizeof(s_lastUuid))) {
        hl::g_logger.logf(1, "UUID: cannot build an identifier for %s (cloid=%s) "
                             "- reporting an int trade ID",
                          coin ? coin : "", cloid ? cloid : "");
        return false;
    }
    hl::g_logger.logf(2, "UUID: opened %s", s_lastUuid);
    return true;
}

bool uuidGet(char* out) {
    if (!out) return false;
    strncpy_s(out, UUIDSIZE, s_lastUuid, _TRUNCATE);
    return s_lastUuid[0] != 0;
}

void uuidSet(const char* uuid) {
    strncpy_s(s_pendingUuid, uuid ? uuid : "", _TRUNCATE);
    hl::g_logger.logf(2, "UUID: set %s", s_pendingUuid);
}

void uuidResetSession() {
    s_lastUuid[0] = 0;
    s_pendingUuid[0] = 0;
}

// Parse and consume the pending identifier. It is cleared even when it does
// not parse, so a stale identifier can never be applied to a later call.
static bool uuidTakePending(hl::uuid::TradeUuid& out) {
    bool ok = hl::uuid::parse(s_pendingUuid, out);
    if (!ok) {
        hl::g_logger.logf(1, "UUID: trade ID -1 without a valid identifier ('%s')",
                          s_pendingUuid);
    }
    s_pendingUuid[0] = 0;
    return ok;
}

// The tracked trade opened with this cloid, or 0.
static int uuidResolve(const hl::uuid::TradeUuid& u) {
    if (!hl::g_trading.tradeCsInit) return 0;
    int found = 0;
    EnterCriticalSection(&hl::g_trading.tradeCs);
    for (const auto& pair : hl::g_trading.tradeMap) {
        if (_stricmp(pair.second.cloid, u.cloid) == 0) {
            found = pair.first;
            break;
        }
    }
    LeaveCriticalSection(&hl::g_trading.tradeCs);
    return found;
}

// Rebuild an untracked trade from the exchange's record of its opening order.
// Returns the new trade ID, or 0 when the exchange does not confirm the order.
static int uuidAdopt(const hl::uuid::TradeUuid& u) {
    hl::CloidQueryResult qr = hl::trading::queryOrderByCloid(u.cloid);
    if (qr.outcome != hl::QueryOutcome::Found) {
        hl::g_logger.logf(1, "UUID: %s %s not confirmed by the exchange (%s)",
                          u.coin, u.cloid, qr.status);
        return 0;
    }

    hl::OrderState st;
    if (qr.oid[0]) strncpy_s(st.orderId, qr.oid, _TRUNCATE);
    else sprintf_s(st.orderId, "PENDING_%s", u.cloid);
    strncpy_s(st.cloid, u.cloid, _TRUNCATE);
    strncpy_s(st.coin, u.coin, _TRUNCATE);
    st.side = u.side;
    st.filledSize = qr.filledSize;
    st.requestedSize = qr.origSize > 0 ? qr.origSize : qr.filledSize;
    if (strcmp(qr.status, "filled") == 0)
        st.status = hl::determineFilledStatus(qr.filledSize, st.requestedSize);
    else if (strcmp(qr.status, "canceled") == 0)
        st.status = hl::OrderStatus::Cancelled;
    else
        st.status = hl::OrderStatus::Open;
    st.lastUpdate = nowDate();
    st.uuidAdopted = true;

    int id = hl::trading::generateTradeId();
    st.zorroTradeId = id;
    hl::trading::storeOrder(id, st);
    hl::g_logger.logf(1, "UUID: adopted %s %s %s as trade %d "
                         "(exchange status=%s, entry filled=%.6f)",
                      u.coin, u.side == hl::OrderSide::Buy ? "long" : "short",
                      u.cloid, id, qr.status, qr.filledSize);
    if (st.status == hl::OrderStatus::Open) {
        hl::g_logger.logf(1, "UUID: trade %d's opening order is still working "
                             "on the exchange", id);
    }
    return id;
}

int uuidTradeForClose() {
    hl::uuid::TradeUuid u;
    if (!uuidTakePending(u)) return 0;
    int id = uuidResolve(u);
    return id ? id : uuidAdopt(u);
}

int uuidResolvePending() {
    hl::uuid::TradeUuid u;
    if (!uuidTakePending(u)) return 0;
    return uuidResolve(u);
}

int uuidCancelPending() {
    hl::uuid::TradeUuid u;
    if (!uuidTakePending(u)) return 0;

    int id = uuidResolve(u);
    if (id) return hl::trading::cancelOrderByTradeId(id) ? 1 : 0;

    // Untracked: cancel the opening order directly if it is still working.
    hl::CloidQueryResult qr = hl::trading::queryOrderByCloid(u.cloid);
    if (qr.outcome != hl::QueryOutcome::Found
        || !hl::utils::isExchangeOrderId(qr.oid)
        || strcmp(qr.status, "filled") == 0
        || strcmp(qr.status, "canceled") == 0) {
        return 0;
    }
    return hl::trading::cancelOrder(u.coin, qr.oid) ? 1 : 0;
}
