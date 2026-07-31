//=============================================================================
// hl_broker_trade.cpp - Broker order execution exports
//=============================================================================
// Part of Hyperliquid Plugin for Zorro
//
// LAYER: API
// DEPENDENCIES: hl_broker_internal.h
// THREAD SAFETY: Main thread only (Zorro calls are single-threaded)
//
// This module provides the order EXECUTION exports:
// - BrokerBuy2: place new orders
// - BrokerSell2: close positions
//
// Trade status queries (BrokerTrade) live in hl_broker_trade_query.cpp.
// The two share the position-accounting helpers defined below and declared
// in hl_broker_internal.h.
//=============================================================================

#include "hl_broker_internal.h"

//-----------------------------------------------------------------------------
// [OPM-733] Shared regime predicate: does another Zorro-visible tradeID track
// part of this asset+side? BrokerBuy2's pre-extend snapshot and BrokerTrade's
// IMPORTED_ reporting must use the SAME answer — if the snapshot runs while
// already in multi-tracker mode, it absorbs a sibling tradeID's fill into the
// IMPORTED_ share and Zorro's ledger inflates by exactly that amount.
// Counts only non-IMPORTED_/non-RESUMED_, non-cancelled entries with net open
// size (filledSize - closedSize) > 0, so trades fully closed by Zorro stop
// counting and the IMPORTED_ trade returns to sole-tracker self-healing.
//-----------------------------------------------------------------------------
bool hasOtherSameSideTracker(int excludeTradeId, const char* coin,
                             hl::OrderSide side) {
    if (!hl::g_trading.tradeCsInit) return false;
    bool found = false;
    EnterCriticalSection(&hl::g_trading.tradeCs);
    for (auto it = hl::g_trading.tradeMap.begin();
         it != hl::g_trading.tradeMap.end(); ++it) {
        if (it->first == excludeTradeId) continue;
        const hl::OrderState& other = it->second;
        if (other.side != side) continue;
        if (strcmp(other.coin, coin) != 0) continue;
        // [OPM-733] Net open size, with a sub-lot epsilon so a sibling that is
        // fully closed but left a tiny float residual stops counting as a
        // tracker (otherwise the IMPORTED_ trade never returns to sole mode).
        const double lotEps = (hl::g_trading.lotSize > 0)
            ? hl::g_trading.lotSize * 0.5 : 1e-9;
        if (other.filledSize - other.closedSize < lotEps) continue;
        if (other.status == hl::OrderStatus::Cancelled) continue;
        if (strncmp(other.orderId, "RESUMED_", 8) == 0) continue;
        if (strncmp(other.orderId, "IMPORTED_", 9) == 0) continue;
        found = true;
        break;
    }
    LeaveCriticalSection(&hl::g_trading.tradeCs);
    return found;
}

//-----------------------------------------------------------------------------
// [OPM-680][OPM-733] Account for a Zorro-driven close against a trade's
// reported size. BrokerTrade's fill-poll reports the open size back to Zorro,
// and Zorro overwrites its ledger with whatever we return — so every path
// that confirms a close to Zorro must record it here.
//   - IMPORTED_ trades: filledSize IS the trade's position share; decrement
//     it directly (OPM-680).
//   - Mapped trades: filledSize is the ENTRY order's cumulative fill, and
//     WS/HTTP lookups re-derive it from the exchange — decrementing it would
//     be undone by the next lookup. Accumulate closes in closedSize instead;
//     BrokerTrade reports filledSize - closedSize (OPM-733 incident 1:
//     partial close re-reported as the gross entry fill).
//-----------------------------------------------------------------------------
void recordZorroClose(int tradeId, hl::OrderState& state, double closeSz) {
    if (closeSz <= 0) return;
    if (strncmp(state.orderId, "IMPORTED_", 9) == 0) {
        double newShare = state.filledSize - closeSz;
        if (newShare < 0) newShare = 0;
        state.filledSize = newShare;
    } else {
        state.closedSize += closeSz;
    }
    hl::trading::storeOrder(tradeId, state);
}

//=============================================================================
// BrokerBuy2 - Place order
//=============================================================================

DLLFUNC int BrokerBuy2(char* symbol, int volume, double stopDist,
                       double limit, double* pPrice, int* pFill) {
    hl::g_logger.logf(1, ">>> BrokerBuy2 CALLED: sym=%s vol=%d stop=%.2f limit=%.2f",
                       symbol ? symbol : "(null)", volume, stopDist, limit);

    // Initialize outputs
    if (pFill) *pFill = 0;
    if (pPrice) *pPrice = 0;

    if (!symbol || !*symbol || !volume) return 0;

    // Fatal error: halt strategy [OPM-170]
    if (hl::g_fatalError.load()) {
        return 0;  // quit already called from BrokerAsset
    }

    bool isCloseOrder = (stopDist == -1);

    // Detect trigger (stop-loss) order [OPM-77]
    bool isTriggerOrder = hl::g_config.stopOrderPending && !isCloseOrder && (limit > 0);
    hl::g_config.stopOrderPending = false;  // Always reset after consumption

    if (hl::g_config.diagLevel >= 1) {
        char msg[256];
        sprintf_s(msg, "%s%s: Symbol=%s Volume=%d Limit=%.2f",
                  isCloseOrder ? "CLOSE" : "OPEN",
                  isTriggerOrder ? " [TRIGGER-SL]" : "",
                  symbol, volume, limit);
        hl::g_logger.log(1, msg);
    }

    // Parse symbol
    char perpDex[32], coin[64];
    parsePerpDex(symbol, perpDex, sizeof(perpDex), coin, sizeof(coin));
    std::string coinForApi = buildCoinForApi(perpDex, coin);

    // Build order request
    hl::OrderRequest request;
    request.coin = coinForApi;
    request.side = (volume > 0) ? hl::OrderSide::Buy : hl::OrderSide::Sell;
    request.size = fabs((double)volume) * hl::g_trading.lotSize;

    if (isTriggerOrder) {
        // --- Trigger (stop-loss) order [OPM-77] ---
        request.triggerType = hl::TriggerType::SL;
        request.triggerPx = limit;   // Zorro passes trigger price as Limit
        request.triggerIsMarket = true;
        request.reduceOnly = true;   // HL requires reduceOnly for tpsl orders

        // Price field: slippage-protected worst-case execution price
        double slippage = hl::config::MARKET_ORDER_SLIPPAGE;
        request.limitPrice = (volume > 0)
            ? limit * (1.0 + slippage)   // Buy stop: accept above trigger
            : limit * (1.0 - slippage);  // Sell stop: accept below trigger

        hl::g_logger.logf(1, "BrokerBuy2: TRIGGER SL - triggerPx=%.4f slippagePrice=%.4f",
                          request.triggerPx, request.limitPrice);
    } else {
        // --- Existing limit/market order path ---
        request.limitPrice = (limit > 0) ? limit : 0;
        request.reduceOnly = isCloseOrder;

        // Convert config orderType string to enum
        if (_stricmp(hl::g_config.orderType, "Gtc") == 0) {
            request.orderType = hl::OrderType::Gtc;
        } else if (_stricmp(hl::g_config.orderType, "Alo") == 0) {
            request.orderType = hl::OrderType::Alo;
        } else {
            request.orderType = hl::OrderType::Ioc;  // Default
        }

        // If no limit price, this is a market order: use IOC + slippage buffer
        if (request.limitPrice <= 0) {
            hl::PriceData price = hl::market::getPrice(coinForApi.c_str());
            double basePrice = (volume > 0) ? price.ask : price.bid;

            if (basePrice <= 0) {
                // [OPM-227] Close with no price: check if position is already flat
                if (isCloseOrder) {
                    hl::account::PositionInfo pos = hl::account::getPosition(coinForApi.c_str());
                    DWORD posAge = hl::account::getPositionsAge();
                    if (!pos.isOpen() && posAge != MAXDWORD) {
                        hl::g_logger.logf(1, "BrokerBuy2: CLOSE - no price but %s is flat (age %ums)",
                                          coinForApi.c_str(), posAge);
                        int earlyTradeId = hl::trading::generateTradeId();
                        if (pFill) *pFill = abs(volume);
                        if (pPrice) *pPrice = 0;
                        return earlyTradeId;
                    }
                }
                hl::g_logger.log(1, "BrokerBuy2: No price available");
                return 0;
            }

            // Apply slippage buffer so IOC fills even if price moves slightly
            double slippage = hl::config::MARKET_ORDER_SLIPPAGE;
            request.limitPrice = (volume > 0)
                ? basePrice * (1.0 + slippage)   // Buy: above ask
                : basePrice * (1.0 - slippage);  // Sell: below bid
            request.orderType = hl::OrderType::Ioc;

            hl::g_logger.logf(1, "BrokerBuy2: Market order - IOC @ %.2f (base %.2f, slippage %.0f%%)",
                              request.limitPrice, basePrice, slippage * 100);
        }
    }

    // Generate trade ID before placing order
    int tradeId = hl::trading::generateTradeId();

    // [OPM-227] Close order: verify exchange has a position before reduce-only order.
    // With NFA mode, Zorro closes via BrokerBuy2(StopDist=-1). If the position was
    // already closed (externally, liquidation, or entry never filled), the reduce-only
    // order would be rejected → Error 075. Pre-check avoids this.
    if (isCloseOrder) {
        hl::account::PositionInfo pos = hl::account::getPosition(coinForApi.c_str());
        DWORD posAge = hl::account::getPositionsAge();
        if (!pos.isOpen() && posAge != MAXDWORD) {
            hl::g_logger.logf(1, "BrokerBuy2: CLOSE - %s has no position (data age %ums), "
                              "already flat", coinForApi.c_str(), posAge);
            if (pFill) *pFill = abs(volume);
            if (pPrice) *pPrice = 0;
            return tradeId;
        }
    }

    // [OPM-680] Same-side EXTEND snapshot.
    // If this BrokerBuy2 will add a new tradeID alongside an existing IMPORTED_
    // trade on the same asset+side, we must snapshot the IMPORTED_'s share to
    // the current live position size BEFORE applyFill inflates the cache.
    // Otherwise BrokerTrade's multi-tracker branch would read a stale share
    // (the one set at GET_TRADES time, possibly missing intervening external
    // changes). See BrokerTrade IMPORTED_ branch and tests/test_imported_trades.
    // Skipped for close orders (opposite direction) and trigger orders (not
    // filled yet) — neither establishes multi-tracker mode.
    // [OPM-733] Only valid on the sole->multi transition: once another tradeID
    // already tracks part of the position, the live aggregate includes that
    // sibling's fill, and snapshotting would absorb it into the IMPORTED_
    // share (double-count). In multi-tracker mode the share is already
    // maintained by BrokerSell2's decrement hook — leave it untouched.
    if (!isCloseOrder && !isTriggerOrder && hl::g_trading.tradeCsInit
        && !hasOtherSameSideTracker(tradeId, coinForApi.c_str(), request.side)) {
        double currentLive = fabs(hl::account::getPosition(coinForApi.c_str()).size);
        EnterCriticalSection(&hl::g_trading.tradeCs);
        for (auto it = hl::g_trading.tradeMap.begin();
             it != hl::g_trading.tradeMap.end(); ++it) {
            hl::OrderState& other = it->second;
            if (strncmp(other.orderId, "IMPORTED_", 9) != 0) continue;
            if (other.side != request.side) continue;
            if (strcmp(other.coin, coinForApi.c_str()) != 0) continue;
            if (other.filledSize != currentLive) {
                other.filledSize = currentLive;  // snapshot share at extend boundary
            }
            break;  // at most one IMPORTED_ per asset+side
        }
        LeaveCriticalSection(&hl::g_trading.tradeCs);
    }

    // Place order via trading service with explicit trade ID
    hl::OrderResult result = hl::trading::placeOrderWithId(request, tradeId);

    if (!result.success) {
        // [OPM-227] Close rejected — position may have been closed externally
        // between pre-check and order submission (race), or pre-check was skipped
        // because position data was unavailable at that time.
        if (isCloseOrder) {
            hl::account::PositionInfo pos = hl::account::getPosition(coinForApi.c_str());
            DWORD posAge = hl::account::getPositionsAge();
            if (!pos.isOpen() && posAge != MAXDWORD) {
                hl::g_logger.logf(1, "BrokerBuy2: CLOSE rejected but %s is flat (age %ums) "
                                  "— reporting success", coinForApi.c_str(), posAge);
                if (pFill) *pFill = abs(volume);
                if (pPrice) *pPrice = 0;
                return tradeId;
            }
        }
        if (hl::g_config.diagLevel >= 1) {
            hl::g_logger.logf(1, "BrokerBuy2: Order failed - %s", result.error.c_str());
        }
        return 0;
    }

    // Return trade ID and fill info
    if (pFill) *pFill = (hl::g_trading.lotSize > 0)
        ? (int)round(result.filledSize / hl::g_trading.lotSize)
        : (int)round(result.filledSize);
    if (pPrice) *pPrice = result.avgPrice > 0 ? result.avgPrice : request.limitPrice;

    // Bridge fill → position cache so GET_POSITION sees it immediately [OPM-85]
    if (result.filledSize > 0 && result.avgPrice > 0) {
        hl::account::applyFill(coinForApi.c_str(), result.filledSize, result.avgPrice,
                               request.side == hl::OrderSide::Buy);
    }

    if (hl::g_config.diagLevel >= 1) {
        char msg[256];
        sprintf_s(msg, "Order placed: tradeID=%d filled=%.6f @ %.2f",
                  tradeId, result.filledSize, result.avgPrice);
        hl::g_logger.log(1, msg);
    }

    return tradeId;
}

//=============================================================================
// BrokerSell2 - Close position
//=============================================================================

DLLFUNC int BrokerSell2(int tradeId, int amount, double limit,
                        double* pClose, double* pCost, double* pProfit, int* pFill) {
    // Initialize outputs
    if (pFill) *pFill = 0;
    if (pClose) *pClose = 0;
    if (pCost) *pCost = 0;
    if (pProfit) *pProfit = 0;

    if (hl::g_config.diagLevel >= 1) {
        char msg[128];
        sprintf_s(msg, "BrokerSell2: TradeID=%d Amount=%d Limit=%.2f",
                  tradeId, amount, limit);
        hl::g_logger.log(1, msg);
    }

    // Get original trade
    hl::OrderState state;
    if (!hl::trading::getOrder(tradeId, state)) {
        hl::g_logger.log(1, "BrokerSell2: Trade not found");
        return 0;
    }

    // [OPM-227] Verify exchange has position before submitting reduce-only close.
    // If position was already closed (externally, liquidation, entry never filled),
    // report success so Zorro removes the phantom trade instead of retrying Error 075.
    {
        hl::account::PositionInfo pos = hl::account::getPosition(state.coin);
        DWORD posAge = hl::account::getPositionsAge();
        if (!pos.isOpen() && posAge != MAXDWORD) {
            hl::g_logger.logf(1, "BrokerSell2: No position for %s (data age %ums), "
                              "already flat", state.coin, posAge);
            if (pClose) {
                hl::PriceData price = hl::market::getPrice(state.coin);
                *pClose = price.mid > 0 ? price.mid : state.avgPrice;
            }
            if (pProfit) *pProfit = 0;
            if (pFill) *pFill = abs(amount);
            // [OPM-733] We confirmed a close to Zorro — record it so the
            // fill-poll doesn't resurrect the pre-close size.
            recordZorroClose(tradeId, state,
                             fabs((double)amount) * hl::g_trading.lotSize);
            return tradeId;
        }
    }

    // Place opposite order to close
    double closeSize = fabs((double)amount) * hl::g_trading.lotSize;
    // Opposite direction: if original was Buy, close with Sell
    bool closingBuy = (state.side == hl::OrderSide::Sell);

    hl::OrderRequest request;
    request.coin = state.coin;  // char[32] -> std::string conversion
    request.side = closingBuy ? hl::OrderSide::Buy : hl::OrderSide::Sell;
    request.size = closeSize;
    request.limitPrice = limit > 0 ? limit : 0;
    request.reduceOnly = true;

    // Convert config orderType string to enum
    if (_stricmp(hl::g_config.orderType, "Gtc") == 0) {
        request.orderType = hl::OrderType::Gtc;
    } else if (_stricmp(hl::g_config.orderType, "Alo") == 0) {
        request.orderType = hl::OrderType::Alo;
    } else {
        request.orderType = hl::OrderType::Ioc;
    }

    // If no limit price, this is a market close: use IOC + slippage buffer
    if (request.limitPrice <= 0) {
        hl::PriceData price = hl::market::getPrice(state.coin);
        double basePrice = closingBuy ? price.ask : price.bid;

        if (basePrice > 0) {
            double slippage = hl::config::MARKET_ORDER_SLIPPAGE;
            request.limitPrice = closingBuy
                ? basePrice * (1.0 + slippage)   // Closing buy: above ask
                : basePrice * (1.0 - slippage);  // Closing sell: below bid
            request.orderType = hl::OrderType::Ioc;

            hl::g_logger.logf(1, "BrokerSell2: Market close - IOC @ %.2f (base %.2f, slippage %.0f%%)",
                              request.limitPrice, basePrice, slippage * 100);
        }
    }

    // [OPM-792] Place under an explicit internal tradeId so the resting close
    // order stays addressable. placeOrder() allocates one internally and drops
    // it on the floor, which left the close order uncancellable from script:
    // BrokerSell2 returns the ORIGINAL id, so DO_CANCEL targeted the already
    // filled entry oid.
    int closeTradeId = hl::trading::generateTradeId();
    hl::OrderResult result = hl::trading::placeOrderWithId(request, closeTradeId);

    if (!result.success) {
        // [OPM-227] Close rejected — re-check position (may have been closed
        // between pre-check and order submission)
        hl::account::PositionInfo pos = hl::account::getPosition(state.coin);
        DWORD posAge = hl::account::getPositionsAge();
        if (!pos.isOpen() && posAge != MAXDWORD) {
            hl::g_logger.logf(1, "BrokerSell2: Close rejected but %s is flat (age %ums) "
                              "— reporting success", state.coin, posAge);
            if (pClose) {
                hl::PriceData price = hl::market::getPrice(state.coin);
                *pClose = price.mid > 0 ? price.mid : state.avgPrice;
            }
            if (pProfit) *pProfit = 0;
            if (pFill) *pFill = abs(amount);
            // [OPM-733] We confirmed a close to Zorro — record it so the
            // fill-poll doesn't resurrect the pre-close size.
            recordZorroClose(tradeId, state, closeSize);
            return tradeId;
        }
        hl::g_logger.logf(1, "BrokerSell2: Close failed - %s", result.error.c_str());
        return 0;
    }

    // [OPM-792] Report ONLY what the exchange actually filled.
    //
    // This used to report abs(amount) — a full close — whenever filledSize was
    // 0, which is exactly what a resting (ALO/GTC) close order returns. Zorro
    // then wrote the position off its books while the contracts were still on
    // the exchange, and applyFill/recordZorroClose corrupted the plugin's own
    // caches to match. A resting close must move nothing until it fills; the
    // WS userFills / BrokerTrade poll picks the fill up when it happens.
    double closeFillSize = (result.filledSize > 0) ? result.filledSize : 0.0;
    bool restingClose = (closeFillSize <= 0);

    if (pFill) *pFill = (closeFillSize > 0 && hl::g_trading.lotSize > 0)
        ? (int)round(closeFillSize / hl::g_trading.lotSize) : 0;

    // pClose/pProfit describe a realised close — leave them at 0 while resting.
    double closePx = (closeFillSize > 0 && result.avgPrice > 0) ? result.avgPrice : 0.0;
    if (pClose) *pClose = closePx;

    // pCost: Hyperliquid perps have no rollover/swap fees, so always 0 [OPM-215]
    // (pCost was initialized to 0 above — intentionally unchanged)

    // pProfit: P&L from entry price vs close fill price [OPM-215]
    if (pProfit && state.avgPrice > 0 && closePx > 0) {
        double pnl = (closePx - state.avgPrice) * closeFillSize;
        if (state.side == hl::OrderSide::Sell) pnl = -pnl;  // Short: invert
        *pProfit = pnl;
    }

    if (closeFillSize > 0 && closePx > 0) {
        // Bridge fill → position cache so GET_POSITION sees it immediately [OPM-85]
        hl::account::applyFill(state.coin, closeFillSize, closePx, closingBuy);
        // [OPM-680][OPM-733] Record the Zorro-driven close so BrokerTrade's
        // fill-poll reports the reduced open size (see recordZorroClose).
        recordZorroClose(tradeId, state, closeFillSize);
    }

    // [OPM-792] Link the close order to the position while any of it is still
    // working, so DO_CANCEL(tradeId) reaches the close order rather than the
    // filled entry oid. Cleared once the close is fully filled.
    bool fullyClosed = (closeFillSize >= closeSize - 1e-12);
    hl::OrderState linkState;
    if (hl::trading::getOrder(tradeId, linkState)) {
        linkState.closeTradeId = fullyClosed ? 0 : closeTradeId;
        hl::trading::storeOrder(tradeId, linkState);
    }

    if (hl::g_config.diagLevel >= 1) {
        hl::g_logger.logf(1, "BrokerSell2: %s — requested %.6f, filled %.6f "
                          "(closeTradeId=%d oid=%s)",
                          restingClose ? "RESTING (nothing closed yet)"
                                       : (fullyClosed ? "FILLED" : "PARTIAL"),
                          closeSize, closeFillSize, closeTradeId,
                          result.oid.c_str());
    }

    return tradeId;
}

