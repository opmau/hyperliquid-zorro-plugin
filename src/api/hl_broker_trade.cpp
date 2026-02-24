//=============================================================================
// hl_broker_trade.cpp - Broker order execution exports
//=============================================================================
// Part of Hyperliquid Plugin for Zorro
//
// LAYER: API
// DEPENDENCIES: hl_broker_internal.h
// THREAD SAFETY: Main thread only (Zorro calls are single-threaded)
//
// This module provides order execution exports:
// - BrokerBuy2: place new orders
// - BrokerSell2: close positions
// - BrokerTrade: query trade status and P&L
//=============================================================================

#include "hl_broker_internal.h"

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

    // Place order via trading service with explicit trade ID
    hl::OrderResult result = hl::trading::placeOrderWithId(request, tradeId);

    if (!result.success) {
        if (hl::g_config.diagLevel >= 1) {
            hl::g_logger.logf(1, "BrokerBuy2: Order failed - %s", result.error.c_str());
        }
        return 0;
    }

    // Return trade ID and fill info
    if (pFill) *pFill = (int)round(result.filledSize / hl::g_trading.lotSize);
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

    hl::OrderResult result = hl::trading::placeOrder(request);

    if (!result.success) {
        hl::g_logger.logf(1, "BrokerSell2: Close failed - %s", result.error.c_str());
        return 0;
    }

    if (pFill) *pFill = (result.filledSize > 0) ?
                        (int)round(result.filledSize / hl::g_trading.lotSize) : abs(amount);
    if (pClose) *pClose = result.avgPrice > 0 ? result.avgPrice : request.limitPrice;

    // Bridge fill → position cache so GET_POSITION sees it immediately [OPM-85]
    double closeFillSize = (result.filledSize > 0) ? result.filledSize : closeSize;
    double closeFillPx = result.avgPrice > 0 ? result.avgPrice : request.limitPrice;
    if (closeFillSize > 0 && closeFillPx > 0) {
        hl::account::applyFill(state.coin, closeFillSize, closeFillPx, closingBuy);
    }

    return tradeId;
}

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
                } else if (strcmp(qr.status, "canceled") == 0) {
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

    // --- IMPORTED_ live position lookup [OPM-90] ---
    // Positions synced via GET_TRADES. Must query CURRENT position from WS
    // cache — position may have changed due to fills from other orders.
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

        // Position still exists in same direction — return CURRENT size
        double actualSize = fabs(livePos.size);
        double entryPx = livePos.entryPrice > 0 ? livePos.entryPrice : state.avgPrice;
        if (pOpen) *pOpen = entryPx;
        if (pRoll) *pRoll = 0;
        if (pProfit && entryPx > 0) {
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
            hl::g_logger.logf(2, "BrokerTrade: IMPORTED %d -> %d lots (live=%.6f, orig=%.6f)",
                              tradeId, fillLots, actualSize, state.filledSize);
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
                hl::OrderStatus newSt = (totalFilled >= state.requestedSize * 0.999)
                    ? hl::OrderStatus::Filled : hl::OrderStatus::PartialFill;
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
                } else if (strcmp(qr.status, "canceled") == 0) {
                    hl::trading::updateOrder(tradeId, 0, 0, hl::OrderStatus::Cancelled);
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
    if (pOpen) *pOpen = state.avgPrice;
    if (pRoll) *pRoll = 0;

    if (pProfit && state.avgPrice > 0 && state.filledSize > 0) {
        hl::PriceData price = hl::market::getPrice(state.coin);
        double currentPx = price.mid > 0 ? price.mid : price.ask;

        if (currentPx > 0) {
            double pnl = (currentPx - state.avgPrice) * state.filledSize;
            if (state.side == hl::OrderSide::Sell) pnl = -pnl;
            *pProfit = pnl;
        }
    }

    if (state.status == hl::OrderStatus::Cancelled) {
        return NAY - 1;
    }

    if (state.filledSize > 0) {
        int fillLots = (int)round(state.filledSize / hl::g_trading.lotSize);
        return (fillLots > 0) ? fillLots : 1;
    }

    return 0;  // Pending order
}
