//=============================================================================
// hl_broker_commands.cpp - BrokerCommand handler (standard Zorro modes)
//=============================================================================
// Part of Hyperliquid Plugin for Zorro
//
// LAYER: API
// DEPENDENCIES: hl_broker_internal.h
// THREAD SAFETY: Main thread only
//
// This module handles the BrokerCommand modes Zorro itself defines:
// - GET_COMPLIANCE, GET_MAXREQUESTS, GET_MAXTICKS, GET_BROKERZONE
// - SET_DIAGNOSTICS, SET_AMOUNT, SET_HWND, SET_ORDERTYPE, SET_SYMBOL
// - GET_POSITION, GET_TRADES, GET_PRICE
// - DO_CANCEL
//
// The Hyperliquid-specific 500xx range lives in hl_broker_commands_hl.cpp and
// is reached through this module's default branch.
//=============================================================================

#include "hl_broker_internal.h"

double handleBrokerCommand(int mode, intptr_t parameter) {
    switch (mode) {

    //=========================================================================
    // STANDARD ZORRO COMMANDS
    //=========================================================================

    case GET_COMPLIANCE:
        // Return 0: let Accounts.csv NFA column control compliance [OPM-213]
        // Previously hardcoded 2+8 which overrode user settings
        return 0;

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

        // [OPM-791] Zorro auto-calls SET_ORDERTYPE at every order entry and can
        // only derive 0/1/2/3 from TradeMode — it can never send 4 (ALO). So a
        // script's brokerCommand(50012,"Alo") was reliably clobbered back to
        // "Ioc" microseconds before the order was built. In live trading this
        // meant every order went out tif:"Ioc" even when the strategy had
        // explicitly requested ALO, so no post-only order ever reached the
        // exchange.
        //
        // While the sticky override is active we keep the script's choice and
        // still consume the +8 STOP flag above. Cleared by 50012("Ioc"/"Gtc")
        // and at login.
        if (hl::g_config.orderTypeSticky) {
            if (baseType != 0 && baseType != 2 && baseType != 4) {
                return 0;  // AON (1), AON+GTC (3) still unsupported
            }
            if (hl::g_config.diagLevel >= 1) {
                hl::g_logger.logf(1, "SET_ORDERTYPE: %d ignored — sticky %s "
                    "override active (50012)%s",
                    orderType, hl::g_config.orderType, isStop ? " [STOP]" : "");
            }
            return (orderType == 0) ? 1 : orderType;
        }

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
            // Zorro never sends this; only a script calling
            // brokerCommand(157,4) directly reaches it. Treat it as the same
            // explicit intent 50012("Alo") expresses, so the next auto-call
            // does not undo it.
            strcpy_s(hl::g_config.orderType, "Alo");
            hl::trading::setOrderType("Alo");
            hl::g_config.orderTypeSticky = true;
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

        // Read priceSymbol rather than currentSymbol — but the two are currently
        // identical: SET_SYMBOL and BrokerAsset each write BOTH fields, so neither
        // is isolated from the subscription loop and priceSymbol is not the "only
        // reliable asset context" an earlier comment here claimed. The caller must
        // SET_SYMBOL immediately before asking for a price. [OPM-6, OPM-1132]
        if (!hl::g_trading.priceSymbol[0]) {
            // Always log — this is a data loss event the strategy needs to see
            hl::g_logger.logf(1, "GET_PRICE(%d): priceSymbol empty (no SET_SYMBOL), "
                "currentSymbol='%s' — returning 0",
                priceType, hl::g_trading.currentSymbol);
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

        if (bid <= 0 || ask <= 0) {
            // Always log — returning 0 means the strategy loses bid/ask data
            hl::g_logger.logf(1, "GET_PRICE(%s): returning 0 — bid=%.4f ask=%.4f "
                "(cache=%s, currentSymbol='%s')",
                lookupCoin, bid, ask, fromCache ? "hit" : "miss",
                hl::g_trading.currentSymbol);
            return 0.0;
        }
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

        // Use same symbol→coin conversion as BrokerAsset [OPM-203]
        // normalizeCoin only strips after '-', but perpDex assets need
        // parsePerpDex + buildCoinForApi to match the cache key.
        char perpDex[32], coin[64];
        parsePerpDex(symbol, perpDex, sizeof(perpDex), coin, sizeof(coin));

        // [OPM-226] If parsePerpDex didn't find a perpDex suffix (bare name like "XYZ100"),
        // resolve it from g_assets. Strategies pass the Zorro Name column (e.g. "XYZ100"),
        // not the full Symbol (e.g. "XYZ100-USDC_xyz"), so the dex suffix is missing.
        //
        // [OPM-600] Only apply the perpDex rewrite when NO main-dex (or spot) asset
        // with this coin exists. Hyperliquid cash/hyna universes contain coins that
        // also exist on main-dex (BTC, ETH, SOL, ADA, ...). Unconditional rewriting
        // caused cache lookups to miss main-dex positions, leading strategies to
        // double up on existing positions.
        if (!perpDex[0]) {
            bool hasNonPerpDexMatch = false;
            for (int i = 0; i < hl::g_assets.count; ++i) {
                const hl::AssetInfo* a = hl::g_assets.getByIndex(i);
                if (a && !a->isPerpDex && _stricmp(a->coin, coin) == 0) {
                    hasNonPerpDexMatch = true;
                    break;
                }
            }
            if (!hasNonPerpDexMatch) {
                for (int i = 0; i < hl::g_assets.count; ++i) {
                    const hl::AssetInfo* a = hl::g_assets.getByIndex(i);
                    if (a && a->isPerpDex && a->perpDex[0]
                        && _stricmp(a->coin, coin) == 0) {
                        strncpy_s(perpDex, a->perpDex, _TRUNCATE);
                        break;
                    }
                }
            }
        }

        std::string coinForApi = buildCoinForApi(perpDex, coin);

        double posSize = hl::account::getPositionSize(coinForApi.c_str());
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
            double lotAmount = asset ? asset->minSize : 1.0;  // [OPM-198] use pre-calculated

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
            // [OPM-680] state.filledSize tracks THIS tradeID's share of the
            // broker position (not the live aggregate). Initialized to the
            // import-time size; BrokerSell2 decrements it on close. See
            // hl_broker_trade.cpp:411 (BrokerTrade IMPORTED_ branch) and
            // hl_broker_trade.cpp:315 (BrokerSell2 close hook) for details.
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
            // [OPM-797] Cancel every resting order for the current SET_SYMBOL,
            // or account-wide when no symbol has been selected. Sourced from
            // the exchange, not the tradeMap, so orders left behind by a
            // crashed session are reachable after a restart.
            const char* sym = hl::g_trading.currentSymbol;
            int n = hl::trading::cancelAllOrders((sym && *sym) ? sym : nullptr);
            hl::g_logger.logf(1, "DO_CANCEL(0): cancelled %d resting order(s) for %s",
                              n, (sym && *sym) ? sym : "ALL symbols");
            return n;
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

    default:
        // Hyperliquid-specific 500xx commands.
        return handleHyperliquidCommand(mode, parameter);
    }
}
