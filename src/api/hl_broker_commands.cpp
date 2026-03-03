//=============================================================================
// hl_broker_commands.cpp - BrokerCommand handler
//=============================================================================
// Part of Hyperliquid Plugin for Zorro
//
// LAYER: API
// DEPENDENCIES: hl_broker_internal.h
// THREAD SAFETY: Main thread only
//
// This module handles all BrokerCommand modes:
// - GET_COMPLIANCE, GET_MAXREQUESTS, GET_MAXTICKS, GET_BROKERZONE
// - SET_DIAGNOSTICS, SET_AMOUNT, SET_HWND
// - GET_POSITION, GET_TRADES, GET_PRICE
// - DO_CANCEL
// - Export commands (50001-50003)
// - Custom commands (50010-50031)
//=============================================================================

#include "hl_broker_internal.h"
#include "../services/hl_trading_twap.h"
#include "../services/hl_trading_modify.h"
#include "../services/hl_trading_bracket.h"

//=============================================================================
// HANDLER IMPLEMENTATION
//=============================================================================

double handleBrokerCommand(int mode, intptr_t parameter) {
    switch (mode) {

    //=========================================================================
    // STANDARD ZORRO COMMANDS
    //=========================================================================

    case GET_COMPLIANCE:
        // 2 = no hedging (one position per asset)
        // 8 = no direct trade close (use NFA-style opposite order)
        return 2 + 8;

    case GET_MAXREQUESTS:
        return 5;  // Max concurrent HTTP requests

    case GET_MAXTICKS:
        return 5000;  // Hyperliquid supports 5000 candles per request

    case GET_BROKERZONE:
        return 0;  // Hyperliquid returns UTC timestamps

    case SET_DIAGNOSTICS:
        hl::g_config.diagLevel = (int)parameter;
        hl::g_logger.level = (int)parameter;  // Sync logger gate with config
        if (hl::g_wsManager) {
            auto* wsMgr = static_cast<hl::ws::WebSocketManager*>(hl::g_wsManager);
            wsMgr->setDiagLevel(hl::g_config.diagLevel);
        }
        return 1;

    case SET_AMOUNT: {
        double u = *(double*)parameter;
        hl::g_trading.lotSize = (u > 0) ? u : 1.0;
        return 1;
    }

    case GET_VOLUME:  // 61 - Zorro requests volume type
        // Return 1 to signal tick activity exists; Zorro may skip bar generation if 0
        return 1;

    case GET_PRICETYPE:  // 150 - Type of prices (0=traded, 2=bid/ask)
        return 2;  // We return bid/ask prices

    case SET_FUNCTIONS:  // 175 - Zorro sending extended function table; accept silently
        return 1;

    case SET_SERVER:  // 182 - Server field from Accounts.csv (after ':')
        // We derive the URL from Real/Demo flag, so we ignore Server content
        return 1;

    case SET_CCY:  // 183 - Currency from CCY field in Accounts.csv
        // We always use USD for crypto perps
        return 1;

    case SET_ORDERTYPE: {  // 157 - Order type for next trade
        // Zorro order types (from brokercommand docs):
        // 0 = Broker default (highest fill probability) -> HL "Ioc"
        // 1 = AON (all-or-nothing) -> Not supported by HL
        // 2 = GTC (good-till-cancelled) -> HL "Gtc"
        // 3 = AON+GTC -> Not supported
        // +8 = STOP flag → trigger order on HL [OPM-77]
        int orderType = (int)parameter;
        int baseType = orderType & 7;  // Strip +8 STOP flag
        bool isStop = (orderType & 8) != 0;  // [OPM-77]

        // Store STOP flag for next BrokerBuy2 call
        hl::g_config.stopOrderPending = isStop;

        switch (baseType) {
        case 0:  // Broker default → IOC (highest fill probability on HL)
            strcpy_s(hl::g_config.orderType, "Ioc");
            hl::trading::setOrderType("Ioc");
            break;
        case 2:  // GTC
            strcpy_s(hl::g_config.orderType, "Gtc");
            hl::trading::setOrderType("Gtc");
            break;
        case 4:  // Broker-specific → ALO (Post-Only / Add-Liquidity-Only)
            strcpy_s(hl::g_config.orderType, "Alo");
            hl::trading::setOrderType("Alo");
            break;
        default:
            return 0;  // AON (1), AON+GTC (3) not supported
        }
        if (hl::g_config.diagLevel >= 1) {
            hl::g_logger.logf(1, "SET_ORDERTYPE: %d -> %s%s",
                orderType, hl::g_config.orderType, isStop ? " [STOP]" : "");
        }
        // Return non-zero to confirm support. For type 0, return 1 since 0 means "not supported".
        return (orderType == 0) ? 1 : orderType;
    }

    case SET_HWND:  // 172
        hl::g_config.zorroWindow = (HWND)parameter;
        if (hl::g_wsManager) {
            auto* wsMgr = static_cast<hl::ws::WebSocketManager*>(hl::g_wsManager);
            wsMgr->setZorroWindow(hl::g_config.zorroWindow);
        }
        if (hl::g_config.diagLevel >= 1) {
            char msg[64];
            sprintf_s(msg, "SET_HWND: handle=0x%p", hl::g_config.zorroWindow);
            hl::g_logger.log(1, msg);
        }
        return 1;

    case SET_SYMBOL: {  // 132 - Set asset symbol for subsequent commands
        const char* sym = (const char*)parameter;
        if (sym && *sym) {
            // Convert Zorro display name to API format for cache lookups
            char perpDex[32], coin[64];
            parsePerpDex(sym, perpDex, sizeof(perpDex), coin, sizeof(coin));
            std::string coinForApi = buildCoinForApi(perpDex, coin);
            strncpy_s(hl::g_trading.currentSymbol, coinForApi.c_str(), _TRUNCATE);
            strncpy_s(hl::g_trading.priceSymbol, coinForApi.c_str(), _TRUNCATE);
            if (hl::g_config.diagLevel >= 2)
                hl::g_logger.logf(2, "SET_SYMBOL: %s -> %s", sym, hl::g_trading.currentSymbol);
        }
        return 1;
    }

    case GET_PRICE: {
        // Return price for current asset from PriceCache (per-asset lookup)
        // Parameter: 4=last(mid), 5=ask, 6=bid
        int priceType = (int)parameter;

        // Use priceSymbol (set by SET_SYMBOL) — the ONLY reliable asset context.
        // BrokerAsset subscription loops overwrite currentSymbol for every asset,
        // so currentSymbol is NOT safe for GET_PRICE. [OPM-6]
        if (!hl::g_trading.priceSymbol[0]) {
            if (hl::g_config.diagLevel >= 1)
                hl::g_logger.log(1, "GET_PRICE: No SET_SYMBOL received, returning 0");
            return 0.0;
        }
        const char* lookupCoin = hl::g_trading.priceSymbol;

        // Query PriceCache directly (thread-safe, per-asset)
        if (!hl::g_priceCache) return 0.0;
        auto* cache = static_cast<hl::ws::PriceCache*>(hl::g_priceCache);
        double bid = cache->getBid(lookupCoin);
        double ask = cache->getAsk(lookupCoin);
        bool fromCache = (bid > 0 && ask > 0);

        // Fallback: try market service if WS cache is empty
        if (!fromCache) {
            hl::PriceData price = hl::market::getPrice(lookupCoin);
            bid = price.bid;
            ask = price.ask;
        }

        if (hl::g_config.diagLevel >= 1) {
            hl::g_logger.logf(1, "GET_PRICE(%s): %s bid=%.4f ask=%.4f",
                lookupCoin, fromCache ? "cache" : "HTTP", bid, ask);
        }

        if (bid <= 0 || ask <= 0) return 0.0;
        if (priceType == 6) return bid;
        if (priceType == 5) return ask;
        return (bid + ask) / 2.0;  // Default: mid price
    }

    //=========================================================================
    // GET_POSITION - Query net position
    //=========================================================================

    case GET_POSITION: {
        if (parameter == 0) {
            // Rebuild all positions from broker
            if (hl::g_config.diagLevel >= 1) {
                hl::g_logger.log(1, "GET_POSITION: Rebuilding from broker");
            }

            auto positions = hl::account::getAllPositions();
            int posCount = 0;

            for (const auto& pos : positions) {
                if (pos.size == 0) continue;

                int tradeId = hl::trading::generateTradeId();
                hl::OrderState state;
                strncpy_s(state.coin, pos.coin.c_str(), _TRUNCATE);
                state.side = (pos.size > 0) ? hl::OrderSide::Buy : hl::OrderSide::Sell;
                state.requestedSize = fabs(pos.size);
                state.filledSize = fabs(pos.size);
                state.avgPrice = pos.entryPrice;
                state.status = hl::OrderStatus::Filled;
                sprintf_s(state.orderId, "RESUMED_%d", tradeId);

                hl::trading::storeOrder(tradeId, state);
                posCount++;

                if (hl::g_config.diagLevel >= 2) {
                    char msg[128];
                    sprintf_s(msg, "Resumed: %s %.4f @ %.2f (ID=%d)",
                              pos.coin.c_str(), pos.size, pos.entryPrice, tradeId);
                    hl::g_logger.log(2, msg);
                }
            }

            if (hl::g_config.diagLevel >= 1) {
                char msg[64];
                sprintf_s(msg, "Rebuilt %d positions", posCount);
                hl::g_logger.log(1, msg);
            }

            // [OPM-136] Warn when no positions found during rebuild
            if (posCount == 0 && hl::g_logger.callback) {
                hl::account::Balance bal = hl::account::getBalance();
                if (bal.dataReceived && bal.accountValue <= 0) {
                    hl::g_logger.callback(
                        "WARNING: No positions found and balance is zero. "
                        "Verify the User field contains your MASTER account address.");
                }
            }
            return 1;
        }

        // Get position for specific symbol
        const char* symbol = (const char*)parameter;
        if (!symbol || !*symbol) return 0;

        char coin[64];
        strncpy_s(coin, symbol, _TRUNCATE);
        hl::utils::normalizeCoin(coin, coin, sizeof(coin));

        double posSize = hl::account::getPositionSize(coin);
        return posSize;
    }

    //=========================================================================
    // GET_TRADES - Fill TRADE array with positions
    //=========================================================================

    case GET_TRADES: {
        TRADE* trades = (TRADE*)parameter;
        if (!trades) return 0;

        if (hl::g_config.diagLevel >= 1) {
            hl::g_logger.log(1, "GET_TRADES: Fetching positions");
        }

        auto positions = hl::account::getAllPositions();
        int count = 0;

        for (const auto& pos : positions) {
            if (pos.size == 0) continue;

            TRADE* t = &trades[count];
            memset(t, 0, sizeof(TRADE));

            t->nID = hl::trading::generateTradeId();

            const hl::AssetInfo* asset = hl::market::getAsset(pos.coin.c_str());
            double lotAmount = asset ? pow(10.0, -asset->szDecimals) : 1.0;

            t->nLots = (int)round(fabs(pos.size) / lotAmount);
            if (t->nLots < 1) t->nLots = 1;

            t->flags = TR_OPEN;
            if (pos.size < 0) t->flags |= TR_SHORT;

            t->fEntryPrice = (float)pos.entryPrice;
            t->fUnits = (float)lotAmount;

            strncpy_s((char*)t->Skill, sizeof(t->Skill), pos.coin.c_str(), _TRUNCATE);

            hl::OrderState state;
            strncpy_s(state.coin, pos.coin.c_str(), _TRUNCATE);
            state.side = (pos.size > 0) ? hl::OrderSide::Buy : hl::OrderSide::Sell;
            state.filledSize = fabs(pos.size);
            state.avgPrice = pos.entryPrice;
            state.status = hl::OrderStatus::Filled;
            sprintf_s(state.orderId, "IMPORTED_%d", t->nID);
            hl::trading::storeOrder(t->nID, state);

            count++;

            if (hl::g_config.diagLevel >= 2) {
                char msg[128];
                sprintf_s(msg, "Trade[%d]: %s %.4f @ %.2f (ID=%d)",
                          count-1, pos.coin.c_str(), pos.size, pos.entryPrice, t->nID);
                hl::g_logger.log(2, msg);
            }
        }

        if (hl::g_config.diagLevel >= 1) {
            char msg[64];
            sprintf_s(msg, "GET_TRADES: Returned %d positions", count);
            hl::g_logger.log(1, msg);
        }

        return count;
    }

    //=========================================================================
    // DO_CANCEL - Cancel order
    //=========================================================================

    case DO_CANCEL: {
        int tradeId = (int)parameter;
        if (tradeId == 0) {
            hl::g_logger.log(1, "DO_CANCEL: Cancel all not implemented");
            return 0;
        }

        bool success = hl::trading::cancelOrderByTradeId(tradeId);
        if (hl::g_config.diagLevel >= 1) {
            char msg[64];
            sprintf_s(msg, "DO_CANCEL: TradeID=%d %s",
                      tradeId, success ? "OK" : "FAILED");
            hl::g_logger.log(1, msg);
        }
        return success ? 1 : 0;
    }

    //=========================================================================
    // CUSTOM HYPERLIQUID COMMANDS
    //=========================================================================

    case HL_ENABLE_WEBSOCKET: {
        int enable = (int)parameter;

        if (enable && !hl::g_config.enableWebSocket) {
            hl::g_config.enableWebSocket = true;

            if (!hl::g_priceCache) {
                hl::g_priceCache = new hl::ws::PriceCache();
            }
            if (!hl::g_wsManager) {
                auto* cache = static_cast<hl::ws::PriceCache*>(hl::g_priceCache);
                auto* wsMgr = new hl::ws::WebSocketManager(*cache);
                wsMgr->setDiagLevel(hl::g_config.diagLevel);
                wsMgr->setUserAddress(hl::g_config.walletAddress);
                hl::g_wsManager = wsMgr;
            }

            std::string wsUrl = hl::g_config.baseUrl;
            size_t pos = wsUrl.find("https://");
            if (pos != std::string::npos) {
                wsUrl.replace(pos, 8, "wss://");
            }
            wsUrl += "/ws";

            auto* wsMgr = static_cast<hl::ws::WebSocketManager*>(hl::g_wsManager);
            wsMgr->start(wsUrl, hl::g_config.isTestnet);

            hl::g_logger.log(1, "WebSocket enabled");
            return 1;
        }

        if (!enable && hl::g_config.enableWebSocket) {
            hl::g_config.enableWebSocket = false;

            if (hl::g_wsManager) {
                auto* wsMgr = static_cast<hl::ws::WebSocketManager*>(hl::g_wsManager);
                wsMgr->stop();
                delete wsMgr;
                hl::g_wsManager = nullptr;
            }
            if (hl::g_priceCache) {
                auto* cache = static_cast<hl::ws::PriceCache*>(hl::g_priceCache);
                delete cache;
                hl::g_priceCache = nullptr;
            }

            hl::g_logger.log(1, "WebSocket disabled");
            return 1;
        }

        return hl::g_config.enableWebSocket ? 1 : 0;
    }

    case HL_SET_ORDER_TYPE: {
        const char* orderType = (const char*)parameter;
        if (!orderType || !*orderType) return 0;

        if (_stricmp(orderType, "Ioc") == 0 ||
            _stricmp(orderType, "Gtc") == 0 ||
            _stricmp(orderType, "Alo") == 0) {
            strcpy_s(hl::g_config.orderType, orderType);
            hl::trading::setOrderType(orderType);
            if (hl::g_config.diagLevel >= 1) {
                hl::g_logger.logf(1, "Order type set: %s", orderType);
            }
            return 1;
        }
        hl::g_logger.logf(1, "Invalid order type: %s", orderType);
        return 0;
    }

    case HL_SET_ACCOUNT_MODE: {
        int mode = (int)parameter;
        if (mode < 0 || mode > 1) return 0;
        hl::g_config.accountMode = mode;
        if (hl::g_config.diagLevel >= 1) {
            hl::g_logger.logf(1, "Account mode: %d (%s)",
                              mode, mode == 0 ? "API wallet" : "vault");
        }
        return 1;
    }

    case HL_GET_OPEN_ORDERS: {
        const char* symbol = (parameter != 0) ? (const char*)parameter : nullptr;
        return 0;
    }

    case HL_VALIDATE_PRICES: {
        auto btcPrice = hl::market::getPrice("BTC");
        auto ethPrice = hl::market::getPrice("ETH");
        auto solPrice = hl::market::getPrice("SOL");

        bool ok = (btcPrice.mid > 0 && ethPrice.mid > 0 && solPrice.mid > 0);

        if (hl::g_config.diagLevel >= 1) {
            char msg[128];
            sprintf_s(msg, "Prices: BTC=%.2f ETH=%.2f SOL=%.2f",
                      btcPrice.mid, ethPrice.mid, solPrice.mid);
            hl::g_logger.log(1, msg);
        }
        return ok ? 1 : 0;
    }

    case HL_GET_FUNDING_RATE: {
        // Return current hourly funding rate for a coin [OPM-172]
        // Parameter: string coin name, or 0 to use current symbol
        const char* coin = nullptr;
        if (parameter != 0) {
            coin = (const char*)parameter;
        } else {
            coin = hl::g_trading.currentSymbol;
        }
        if (!coin || !*coin) return 0.0;
        return hl::market::getFundingRate(coin);
    }

    case HL_FORCE_WS_DISCONNECT: {
        // Debug: force WebSocket disconnect to test auto-reconnect [OPM-170]
        if (!hl::g_wsManager) return 0;
        auto* wsMgr = static_cast<hl::ws::WebSocketManager*>(hl::g_wsManager);
        hl::g_logger.log(1, "DEBUG: Forcing WS disconnect (OPM-170 test)");
        wsMgr->forceDisconnectForTest();
        return 1;
    }

    case HL_SCHEDULE_CANCEL: {
        // Dead man's switch [OPM-83]
        // param = seconds from now (0 = clear). Plugin converts to absolute ms.
        int seconds = (int)parameter;
        if (seconds > 0) {
            uint64_t timeMs = ((uint64_t)time(nullptr) + (uint64_t)seconds) * 1000;
            return hl::trading::scheduleCancel(timeMs) ? 1 : 0;
        }
        return hl::trading::clearScheduleCancel() ? 1 : 0;
    }

    //=========================================================================
    // EXPORT COMMANDS (50001-50003) [OPM-13]
    //=========================================================================

    case HL_EXPORT_ASSETS: {
        // Export basic asset CSV for top coins (BTC, ETH, SOL)
        const char* path = (const char*)parameter;
        if (!path || !*path) return 0;

        FILE* f = nullptr;
        if (0 != fopen_s(&f, path, "w") || !f) return 0;

        fprintf(f, "Name,Price,Spread,RollLong,RollShort,PIP,PIPCost,MarginCost,Leverage,LotAmount,Commission\n");

        const char* coins[] = {"BTC", "ETH", "SOL"};
        for (int i = 0; i < 3; i++) {
            hl::PriceData px = hl::market::getPrice(coins[i]);
            if (px.mid <= 0) continue;

            const hl::AssetInfo* asset = hl::market::getAsset(coins[i]);
            double pip, lotAmt;
            int lev;
            if (asset) {
                pip = pow(10.0, -asset->pxDecimals);
                lotAmt = pow(10.0, -asset->szDecimals);
                lev = asset->maxLeverage;
            } else {
                pip = (px.mid >= 1000.0) ? 0.5 : 0.01;
                lotAmt = 1.0;
                lev = 1;
            }
            double pipCost = pip * lotAmt;  // 10^(-6) for perps, 10^(-8) for spot [OPM-141]
            double spread = (px.ask > 0 && px.bid > 0) ? (px.ask - px.bid) : 0.0;

            fprintf(f, "%s,%.8f,%.8f,0,0,%.8f,%.8f,0,%d,%.8f,-0.035\n",
                    coins[i], px.mid, spread, pip, pipCost, lev, lotAmt);
        }

        fclose(f);
        if (hl::g_config.diagLevel >= 1)
            hl::g_logger.logf(1, "HL_EXPORT_ASSETS: Wrote %s", path);
        return 1;
    }

    case HL_EXPORT_META: {
        // Export all assets from registry into Zorro Assets CSV
        const char* path = (const char*)parameter;
        if (!path || !*path) return 0;
        if (hl::g_assets.count <= 0) return 0;

        FILE* f = nullptr;
        if (0 != fopen_s(&f, path, "w") || !f) return 0;

        fprintf(f, "Name,Price,Spread,RollLong,RollShort,PIP,PIPCost,MarginCost,Leverage,LotAmount,Commission\n");

        // Use PriceCache for fast bulk lookups (no HTTP fallback)
        hl::ws::PriceCache* cache = hl::g_priceCache
            ? static_cast<hl::ws::PriceCache*>(hl::g_priceCache) : nullptr;

        int written = 0;
        for (int i = 0; i < hl::g_assets.count; i++) {
            const hl::AssetInfo* asset = hl::g_assets.getByIndex(i);
            if (!asset || !asset->coin[0]) continue;

            double mid = 0.0, spread = 0.0;
            if (cache) {
                double bid = cache->getBid(asset->coin);
                double ask = cache->getAsk(asset->coin);
                if (bid > 0 && ask > 0) {
                    mid = (bid + ask) / 2.0;
                    spread = ask - bid;
                }
            }

            double pip = pow(10.0, -asset->pxDecimals);
            double lotAmt = pow(10.0, -asset->szDecimals);
            double pipCost = pip * lotAmt;  // 10^(-6) for perps, 10^(-8) for spot [OPM-141]

            fprintf(f, "%s,%.8f,%.8f,0,0,%.8f,%.8f,0,%d,%.8f,-0.035\n",
                    asset->coin, mid, spread, pip, pipCost, asset->maxLeverage, lotAmt);
            written++;
        }

        fclose(f);
        if (hl::g_config.diagLevel >= 1)
            hl::g_logger.logf(1, "HL_EXPORT_META: Wrote %d assets to %s", written, path);
        return written;
    }

    case HL_EXPORT_ACCOUNT: {
        // Export Accounts.csv row with current connection info
        const char* path = (const char*)parameter;
        if (!path || !*path) return 0;

        FILE* f = nullptr;
        if (0 != fopen_s(&f, path, "w") || !f) return 0;

        const char* dllName =
#ifdef DEV_BUILD
            "Hyperliquid_Dev.dll";
#else
            "Hyperliquid.dll";
#endif

        fprintf(f, "Name,Server,AccountId,User,Pass,Assets,CCY,Real,NFA,Plugin\n");
        fprintf(f, "%s,%s,%s,%s,%s,AssetsHyperliquid,USD,%d,0,%s\n",
                PLUGIN_NAME,
                hl::g_config.baseUrl[0] ? hl::g_config.baseUrl : "https://api.hyperliquid.xyz",
                hl::g_config.walletAddress[0] ? hl::g_config.walletAddress : "0",
                hl::g_config.walletAddress[0] ? hl::g_config.walletAddress : "0",
                hl::g_config.privateKey[0] ? hl::g_config.privateKey : "0",
                hl::g_config.isTestnet ? 0 : 1,
                dllName);

        fclose(f);
        if (hl::g_config.diagLevel >= 1)
            hl::g_logger.logf(1, "HL_EXPORT_ACCOUNT: Wrote %s", path);
        return 1;
    }

    //=========================================================================
    // TWAP COMMANDS (50040-50041) [OPM-81]
    //=========================================================================

    case HL_PLACE_TWAP: {
        if (parameter == 0) return 0;
        const hl::TwapRequest* req = (const hl::TwapRequest*)parameter;
        hl::TwapResult res = hl::trading::placeTwapOrder(*req);
        if (!res.success) {
            hl::g_logger.logf(1, "HL_PLACE_TWAP failed: %s", res.error.c_str());
            return 0;
        }
        return (double)res.twapId;
    }

    case HL_CANCEL_TWAP: {
        uint64_t twapId = (uint64_t)parameter;
        if (twapId == 0) return 0;
        const char* coin = hl::g_trading.currentSymbol;
        if (!coin || !*coin) return 0;
        bool ok = hl::trading::cancelTwapOrder(coin, twapId);
        return ok ? 1 : 0;
    }

    //=========================================================================
    // MODIFY ORDER (50042) [OPM-80]
    //=========================================================================

    case HL_MODIFY_ORDER: {
        if (parameter == 0) return 0;
        const hl::ModifyRequest* req = (const hl::ModifyRequest*)parameter;
        hl::ModifyResult res = hl::trading::modifyOrder(*req);
        if (!res.success) {
            hl::g_logger.logf(1, "HL_MODIFY_ORDER failed: %s", res.error.c_str());
            return 0;
        }
        return 1;
    }

    //=========================================================================
    // BRACKET ORDER (50043) [OPM-79]
    //=========================================================================

    case HL_PLACE_BRACKET: {
        if (parameter == 0) return 0;
        const hl::BracketRequest* req = (const hl::BracketRequest*)parameter;
        hl::BracketResult res = hl::trading::placeBracketOrder(*req);
        if (!res.success) {
            hl::g_logger.logf(1, "HL_PLACE_BRACKET failed: %s", res.error.c_str());
            return 0;
        }
        return (double)res.entryTradeId;
    }

    default:
        if (hl::g_config.diagLevel >= 3) {
            char msg[64];
            sprintf_s(msg, "BrokerCommand: Unknown mode %d", mode);
            hl::g_logger.log(3, msg);
        }
        return 0;
    }
}
