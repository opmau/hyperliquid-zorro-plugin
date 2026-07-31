//=============================================================================
// hl_broker_trade_query.cpp - Broker trade status query export
//=============================================================================
// Part of Hyperliquid Plugin for Zorro
//
// LAYER: API
// DEPENDENCIES: hl_broker_internal.h
// THREAD SAFETY: Main thread only (Zorro calls are single-threaded)
//
// This module provides BrokerTrade, which Zorro calls on a timer for every
// open trade and whose return value it writes straight into its own ledger
// (the "filled X of Y" log line). That makes this the most safety-critical
// read path in the plugin: over-reporting resurrects closed size, and
// under-reporting discards contracts the exchange still holds. See OPM-733
// and OPM-798.
//
// Order placement (BrokerBuy2/BrokerSell2) lives in hl_broker_trade.cpp.
//=============================================================================

#include "hl_broker_internal.h"

//=============================================================================
// BrokerTrade - Query trade status
//=============================================================================

DLLFUNC int BrokerTrade(int tradeId, double* pOpen, double* pClose,
                        double* pRoll, double* pProfit) {
    if (!hl::g_config.walletAddress[0]) return 0;

    if (hl::g_config.diagLevel >= 2) {
        char msg[64];
        sprintf_s(msg, "BrokerTrade: TradeID=%d", tradeId);
        hl::g_logger.log(2, msg);
    }

    // Get trade state
    hl::OrderState state;
    if (!hl::trading::getOrder(tradeId, state)) {
        if (hl::g_config.diagLevel >= 2)
            hl::g_logger.log(2, "BrokerTrade: Trade not found - returning NAY");
        return NAY;
    }

    // --- PENDING_ reconciliation [OPM-89] ---
    // Orders with synthetic "PENDING_<cloid>" orderId need exchange query to
    // determine real status.
    if (strncmp(state.orderId, "PENDING_", 8) == 0) {
        hl::g_logger.logf(1, "BrokerTrade: PENDING order %d (cloid=%s) — querying exchange",
                          tradeId, state.cloid);

        hl::CloidQueryResult qr = hl::trading::queryOrderByCloid(state.cloid);

        if (qr.outcome == hl::QueryOutcome::Found) {
            if (qr.oid[0]) {
                hl::OrderState updated = state;
                strncpy_s(updated.orderId, qr.oid, _TRUNCATE);
                if (strcmp(qr.status, "filled") == 0) {
                    updated.status = hl::determineFilledStatus(qr.filledSize, state.requestedSize);
                    updated.filledSize = qr.filledSize;
                    updated.avgPrice = qr.avgPrice;
                } else if (strcmp(qr.status, "canceled") == 0 ||
                           strcmp(qr.status, "siblingFilledCanceled") == 0) {  // [OPM-79]
                    updated.status = hl::OrderStatus::Cancelled;
                } else {
                    updated.status = hl::OrderStatus::Open;
                }
                updated.lastUpdate = (double)time(NULL) / 86400.0 + 25569.0;
                hl::trading::storeOrder(tradeId, updated);
                state = updated;
                hl::g_logger.logf(1, "BrokerTrade: PENDING resolved -> oid=%s status=%s",
                                  qr.oid, qr.status);
            }
        } else if (qr.outcome == hl::QueryOutcome::NotFound) {
            hl::trading::updateOrder(tradeId, 0, 0, hl::OrderStatus::Cancelled);
            hl::g_logger.logf(1, "BrokerTrade: PENDING %d NOT_FOUND — returning NAY-1", tradeId);
            return NAY - 1;
        } else {
            hl::g_logger.logf(1, "BrokerTrade: PENDING %d query FAILED — returning NAY", tradeId);
            return NAY;
        }
    }

    // --- RESUMED_ early return [OPM-90] ---
    // Historical positions synced from broker on startup. No real order ID,
    // so skip WS/HTTP lookups. Use cached entry price + live price for P&L.
    if (strncmp(state.orderId, "RESUMED_", 8) == 0) {
        if (pOpen) *pOpen = state.avgPrice;
        if (pRoll) *pRoll = 0;
        if (pProfit && state.avgPrice > 0) {
            hl::PriceData price = hl::market::getPrice(state.coin);
            double currentPx = price.mid > 0 ? price.mid : price.ask;
            if (currentPx > 0) {
                double pnl = (currentPx - state.avgPrice) * state.filledSize;
                if (state.side == hl::OrderSide::Sell) pnl = -pnl;
                *pProfit = pnl;
            }
        }
        int fillLots = (hl::g_trading.lotSize > 0)
            ? (int)round(state.filledSize / hl::g_trading.lotSize)
            : (int)round(state.filledSize);
        if (fillLots < 1 && state.filledSize > 0) fillLots = 1;
        if (hl::g_config.diagLevel >= 2)
            hl::g_logger.logf(2, "BrokerTrade: RESUMED %d -> %d lots (%.6f)",
                              tradeId, fillLots, state.filledSize);
        return fillLots;
    }

    // --- IMPORTED_ share-of-position reporting [OPM-90, OPM-680] ---
    // Positions synced via GET_TRADES need to map a single broker aggregate
    // position onto Zorro's per-tradeID fill model. Two regimes:
    //
    //   1. SOLE TRACKER — this IMPORTED_ is the only Zorro tradeID on the
    //      asset+side. The broker aggregate IS this trade's share. We report
    //      fabs(livePos.size) and keep state.filledSize synced to it. This
    //      preserves the OPM-90/18c287c behavior: external partial closes
    //      (manual operator, partial liquidation) are auto-detected because
    //      the live aggregate shrinks.
    //
    //   2. MULTI TRACKER — another Zorro tradeID exists on the same asset+side
    //      (e.g., BrokerBuy2 extended the position). Reporting the live
    //      aggregate would double-count, because the other tradeID is also
    //      tracking its own fill. We report state.filledSize (this trade's
    //      share), kept accurate by:
    //        - BrokerBuy2's pre-extend snapshot (captures the IMPORTED_'s
    //          share at the moment a new same-side tradeID is created)
    //        - BrokerSell2's decrement hook (subtracts Zorro-driven closes)
    //
    // Close/reverse detection always uses the live position: if the broker
    // reports zero or opposite direction, this trade is done regardless of
    // state.filledSize. External partial closes after entering multi-tracker
    // mode are NOT auto-detected — that ambiguity cannot be resolved at the
    // plugin level (we don't know which tradeID's share to debit). Strategies
    // that need this should reconcile at the strategy layer.
    if (strncmp(state.orderId, "IMPORTED_", 9) == 0) {
        hl::account::PositionInfo livePos = hl::account::getPosition(state.coin);
        bool importWasLong = (state.side == hl::OrderSide::Buy);
        bool currentIsLong = (livePos.size > 0);

        // Position closed or reversed direction -> trade is done
        if (!livePos.isOpen() || (importWasLong != currentIsLong)) {
            if (hl::g_config.diagLevel >= 1)
                hl::g_logger.logf(1, "BrokerTrade: IMPORTED %d %s — position %s",
                    tradeId, state.coin, !livePos.isOpen() ? "CLOSED" : "REVERSED");
            if (pOpen) *pOpen = state.avgPrice;
            if (pClose) *pClose = livePos.entryPrice > 0 ? livePos.entryPrice : state.avgPrice;
            if (pRoll) *pRoll = 0;
            if (pProfit) *pProfit = 0;
            return 0;  // Trade closed/reversed
        }

        // Detect multi-tracker regime via the shared predicate ([OPM-733]:
        // must match BrokerBuy2's snapshot gate, and a sibling fully closed
        // by Zorro no longer counts — sole-tracker self-healing resumes).
        bool multiTracker = hasOtherSameSideTracker(tradeId, state.coin, state.side);

        double actualSize;
        bool soleMode = !multiTracker;
        if (soleMode) {
            // Sole tracker: live aggregate IS this trade's share
            actualSize = fabs(livePos.size);
            if (state.filledSize != actualSize) {
                state.filledSize = actualSize;
                hl::trading::storeOrder(tradeId, state);
            }
        } else {
            // Multi-tracker: this trade's recorded share (set by BrokerBuy2
            // snapshot + decremented by BrokerSell2 hook)
            actualSize = state.filledSize;
        }

        double entryPx = state.avgPrice > 0 ? state.avgPrice : livePos.entryPrice;
        if (pOpen) *pOpen = entryPx;
        if (pRoll) *pRoll = 0;
        if (pProfit && entryPx > 0 && actualSize > 0) {
            hl::PriceData price = hl::market::getPrice(state.coin);
            double currentPx = price.mid > 0 ? price.mid : price.ask;
            if (currentPx > 0) {
                double pnl = (currentPx - entryPx) * actualSize;
                if (!currentIsLong) pnl = -pnl;
                *pProfit = pnl;
            }
        }
        int fillLots = (hl::g_trading.lotSize > 0)
            ? (int)round(actualSize / hl::g_trading.lotSize)
            : (int)round(actualSize);
        if (fillLots < 1 && actualSize > 0) fillLots = 1;
        if (hl::g_config.diagLevel >= 2)
            hl::g_logger.logf(2, "BrokerTrade: IMPORTED %d -> %d lots (%s, share=%.6f, live=%.6f)",
                              tradeId, fillLots, soleMode ? "sole" : "multi",
                              actualSize, fabs(livePos.size));
        return fillLots;
    }

    // --- WS PriceCache check for normal orders [OPM-90] ---
    // Check for fills/open orders in WS cache. This catches updates that
    // the onOrderUpdate/onFillNotify callbacks may have missed.
    if (hl::g_config.enableWebSocket && hl::g_priceCache && state.orderId[0]) {
        auto* cache = static_cast<hl::ws::PriceCache*>(hl::g_priceCache);
        auto wsFills = cache->getFillsForOrder(state.orderId);

        if (!wsFills.empty()) {
            double totalFilled = 0, totalValue = 0;
            for (const auto& fill : wsFills) {
                totalFilled += fill.sz;
                totalValue += fill.px * fill.sz;
            }
            double avgPx = totalValue / totalFilled;

            if (totalFilled >= state.filledSize) {
                hl::OrderStatus newSt = hl::determineFilledStatus(totalFilled, state.requestedSize);
                hl::trading::updateOrder(tradeId, totalFilled, avgPx, newSt);
                state.filledSize = totalFilled;
                state.avgPrice = avgPx;
            }
        } else if (state.filledSize <= 0) {
            auto wsOrder = cache->getOpenOrder(state.orderId);
            if (!wsOrder.oid.empty()) {
                if (hl::g_config.diagLevel >= 2)
                    hl::g_logger.logf(2, "BrokerTrade: WS shows order %d still open", tradeId);
            }
        }
    }

    // --- HTTP stale check for non-terminal orders [OPM-90, OPM-91] ---
    // Query exchange for orders that are Open or PartialFill after staleness window.
    // Uses 5s for unfilled, 10s for partially-filled (reduces HTTP load for GTC orders).
    if ((state.status == hl::OrderStatus::Open || state.status == hl::OrderStatus::PartialFill)
        && state.cloid[0] && state.lastUpdate > 0) {
        double now = (double)time(NULL) / 86400.0 + 25569.0;
        double ageSec = (now - state.lastUpdate) * 86400.0;
        double staleThreshold = (state.filledSize > 0) ? 10.0 : 5.0;
        if (ageSec > staleThreshold) {
            hl::CloidQueryResult qr = hl::trading::queryOrderByCloid(state.cloid);
            if (qr.outcome == hl::QueryOutcome::Found) {
                if (strcmp(qr.status, "filled") == 0 && qr.filledSize > 0) {
                    hl::OrderStatus newSt = hl::determineFilledStatus(qr.filledSize, state.requestedSize);
                    hl::trading::updateOrder(tradeId, qr.filledSize, qr.avgPrice, newSt);
                    state.filledSize = qr.filledSize;
                    state.avgPrice = qr.avgPrice;
                    state.status = newSt;
                    if (qr.oid[0]) strncpy_s(state.orderId, qr.oid, _TRUNCATE);
                    hl::g_logger.logf(1, "BrokerTrade: HTTP fallback found fill for %d", tradeId);
                } else if (strcmp(qr.status, "canceled") == 0 ||
                           strcmp(qr.status, "siblingFilledCanceled") == 0) {  // [OPM-79]
                    // [OPM-798] Preserve any partial fill. Zeroing filledSize
                    // here discarded contracts that were genuinely filled
                    // before the cancel — the exchange still holds them, and a
                    // reprice loop (cancel/replace with partials in flight)
                    // hits this on every iteration.
                    hl::trading::updateOrder(tradeId, state.filledSize, state.avgPrice,
                                             hl::OrderStatus::Cancelled);
                    state.status = hl::OrderStatus::Cancelled;
                } else if (qr.oid[0] && strcmp(state.orderId, qr.oid) != 0) {
                    hl::OrderState updated = state;
                    strncpy_s(updated.orderId, qr.oid, _TRUNCATE);
                    updated.lastUpdate = now;
                    hl::trading::storeOrder(tradeId, updated);
                    state = updated;
                }
            }
        }
    }

    // --- Generic return path ---
    // [OPM-733] Report the NET open amount: entry fill minus Zorro-driven
    // closes. Zorro's automatic fill-poll overwrites the trade's lot count
    // with this return value, so echoing the gross entry fill would undo
    // partial closes in Zorro's ledger, undoing a partial reduce.
    // [OPM-733] Sub-lot epsilon: a full close can leave a tiny positive float
    // residual (filledSize re-derived from the exchange as a sum of fills vs
    // separately-accumulated closedSize). Without this, net rounds to 0 lots
    // but the `:1` floor below would return a phantom 1-lot trade -> reconcile
    // mismatch, the exact symptom OPM-733 prevents.
    double openSize = state.filledSize - state.closedSize;
    const double lotEps = (hl::g_trading.lotSize > 0)
        ? hl::g_trading.lotSize * 0.5 : 1e-9;
    if (openSize < lotEps) openSize = 0;

    if (pOpen) *pOpen = state.avgPrice;
    if (pRoll) *pRoll = 0;

    if (pProfit && state.avgPrice > 0 && openSize > 0) {
        hl::PriceData price = hl::market::getPrice(state.coin);
        double currentPx = price.mid > 0 ? price.mid : price.ask;

        if (currentPx > 0) {
            double pnl = (currentPx - state.avgPrice) * openSize;
            if (state.side == hl::OrderSide::Sell) pnl = -pnl;
            *pProfit = pnl;
        }
    }

    // [OPM-798] A cancelled order that partially filled still left contracts on
    // the exchange. Reporting NAY-1 told Zorro the order never existed and it
    // discarded the partial from its ledger, so the position and the ledger
    // disagreed from that point on. Report the partial; only a cancel with
    // nothing filled is NAY-1.
    if (state.status == hl::OrderStatus::Cancelled && openSize <= 0) {
        return NAY - 1;
    }

    if (openSize > 0) {
        int fillLots = (hl::g_trading.lotSize > 0)
            ? (int)round(openSize / hl::g_trading.lotSize)
            : (int)round(openSize);
        return (fillLots > 0) ? fillLots : 1;
    }

    return 0;  // Pending order, or entry fully closed by Zorro
}
