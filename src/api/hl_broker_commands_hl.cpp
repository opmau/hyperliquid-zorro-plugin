//=============================================================================
// hl_broker_commands_hl.cpp - Hyperliquid-specific BrokerCommand modes (500xx)
//=============================================================================
// Part of Hyperliquid Plugin for Zorro
//
// LAYER: API
// DEPENDENCIES: hl_broker_internal.h
// THREAD SAFETY: Main thread only
//
// Commands outside Zorro's standard set, callable from Lite-C via
// brokerCommand(). Reached from handleBrokerCommand's default branch.
//
//   50001-50003  Export assets / meta / account CSV
//   50010-50012  Validate prices, enable WebSocket, set order type
//   50020-50021  Open-order count, account mode
//   50023        Last order-reject class + text          [OPM-795]
//   50030-50032  Force WS disconnect, funding rate, scheduleCancel
//   50040-50041  TWAP place / cancel
//   50042-50044  Modify (struct), bracket, modify-by-tradeID  [OPM-793]
//   50045        Exchange order ID for a trade ID         [OPM-1085]
//=============================================================================

#include "hl_broker_internal.h"
#include "../services/hl_trading_twap.h"
#include "../services/hl_trading_modify.h"
#include "../services/hl_trading_bracket.h"

double handleHyperliquidCommand(int mode, intptr_t parameter) {
    switch (mode) {

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

        // [OPM-794] Canonicalize on ingestion. Storing the caller's casing
        // verbatim put "ioc" in the JSON while the msgpack signing path
        // normalized it to "Ioc" — the hashes disagree, HL recovers a phantom
        // signer and rejects with "User or API Wallet does not exist".
        const char* canonical = hl::utils::canonicalTif(orderType);
        if (!canonical) {
            hl::g_logger.logf(1, "Invalid order type: %s", orderType);
            return 0;
        }

        strcpy_s(hl::g_config.orderType, canonical);
        hl::trading::setOrderType(canonical);

        // [OPM-791] "Alo" is an explicit script decision that must survive
        // Zorro's auto SET_ORDERTYPE at order entry; "Ioc"/"Gtc" release it.
        hl::g_config.orderTypeSticky = (strcmp(canonical, "Alo") == 0);

        if (hl::g_config.diagLevel >= 1) {
            hl::g_logger.logf(1, "Order type set: %s%s", canonical,
                hl::g_config.orderTypeSticky ? " (sticky — survives SET_ORDERTYPE)" : "");
        }
        return 1;
    }

    case HL_GET_LAST_ORDER_ERROR: {
        // [OPM-795] Distinguish "post-only would cross -> reprice one tick
        // back" from "margin/signing failure -> stop trading this asset".
        // Both used to surface identically as BrokerBuy2 == 0.
        //
        // Returns the error class (0=none, 1=post-only-would-match, 2=margin,
        // 3=other). Pass a char buffer of at least 256 bytes as the parameter
        // to also receive the exchange's verbatim text.
        const char* text = hl::trading::getLastOrderErrorText();
        if (parameter != 0) {
            char* outBuf = (char*)parameter;
            strncpy_s(outBuf, 256, text ? text : "", _TRUNCATE);
        }
        return hl::trading::getLastOrderErrorClass();
    }

    case HL_MODIFY_BY_TRADEID: {
        // [OPM-793] Scalar C-ABI reprice, reachable from Lite-C:
        //   var p[3]; p[0] = tradeID; p[1] = newPrice; p[2] = newSize;
        //   brokerCommand(50044, p);
        // p[2] <= 0 keeps the current size.
        if (parameter == 0) return hl::trading::MODIFY_FAILED;
        const double* p = (const double*)parameter;
        return (double)hl::trading::modifyByTradeId(
            (int)p[0], p[1], p[2]);
    }

    case HL_GET_ORDER_OID: {
        // [OPM-1085] The exchange's own order ID for one of the strategy's
        // trades, so the strategy can match its records against the fills the
        // exchange reports. Without that join key every exchange fill reads as
        // one nobody placed, which is also how a liquidation, an
        // auto-deleverage and a manual trade on the same address read.
        //
        //   ThisTrade = enterLong();
        //   var oid = brokerCommand(50045, TradeID);
        //
        // Keyed by trade ID rather than "the most recent order": an order can
        // fill in slices across bars, and several can be working at once.
        int tradeId = (int)parameter;

        hl::OrderState state;
        if (tradeId <= 0 || !hl::trading::getOrder(tradeId, state)) {
            if (hl::g_config.diagLevel >= 2) {
                char msg[80];
                sprintf_s(msg, "HL_GET_ORDER_OID: trade %d not tracked", tradeId);
                hl::g_logger.log(2, msg);
            }
            return 0;
        }

        // A tracked trade need not carry an exchange ID: the map also holds the
        // synthetic forms (PENDING_/RESUMED_/IMPORTED_/DRY_RUN) [OPM-797].
        // Those report "no ID" rather than a parsed prefix.
        return hl::utils::exchangeOrderIdToDouble(state.orderId);
    }

    case HL_SET_ACCOUNT_MODE: {
        int mode = (int)parameter;
        if (mode < 0 || mode > 1) return 0;
        hl::g_config.accountMode = mode;
        // [OPM-202] When vault mode enabled, use walletAddress as vaultAddress
        if (mode == 1 && !hl::g_config.vaultAddress[0]) {
            strncpy_s(hl::g_config.vaultAddress, hl::g_config.walletAddress, _TRUNCATE);
        } else if (mode == 0) {
            hl::g_config.vaultAddress[0] = '\0';
        }
        if (hl::g_config.diagLevel >= 1) {
            hl::g_logger.logf(1, "Account mode: %d (%s) vaultAddress=%s",
                              mode, mode == 0 ? "API wallet" : "vault",
                              hl::g_config.vaultAddress[0] ? hl::g_config.vaultAddress : "(none)");
        }
        return 1;
    }

    case HL_GET_OPEN_ORDERS: {
        // [OPM-798] Two defects fixed here:
        //   1. The symbol was matched UNNORMALIZED, so a Zorro display name
        //      ("TSLA-USDC_xyz") never matched the cache key ("xyz:TSLA") and
        //      perpDex symbols always reported 0.
        //   2. There was no HTTP fallback. The WS openOrders cache is empty
        //      until the subscription delivers its first snapshot (and stays
        //      empty if the subscription is not live), so a genuinely resting
        //      order reported 0.
        const char* rawSymbol = (parameter != 0) ? (const char*)parameter : nullptr;

        std::string coinForApi;
        char perpDex[32] = {0}, coin[64] = {0};
        if (rawSymbol && *rawSymbol) {
            parsePerpDex(rawSymbol, perpDex, sizeof(perpDex), coin, sizeof(coin));
            coinForApi = buildCoinForApi(perpDex, coin);
        }

        // Fast path: WS cache.
        if (hl::g_config.enableWebSocket && hl::g_priceCache) {
            auto* cache = static_cast<hl::ws::PriceCache*>(hl::g_priceCache);
            auto all = cache->getAllOpenOrders();
            if (!all.empty()) {
                int count = 0;
                for (const auto& o : all) {
                    if (coinForApi.empty()
                        || hl::utils::coinMatches(o.coin.c_str(), coinForApi.c_str()))
                        count++;
                }
                if (count > 0) {
                    if (hl::g_config.diagLevel >= 2)
                        hl::g_logger.logf(2, "GET_OPEN_ORDERS(%s): %d (WS cache)",
                                          coinForApi.empty() ? "ALL" : coinForApi.c_str(),
                                          count);
                    return count;
                }
            }
        }

        // Authoritative fallback: ask the exchange.
        auto orders = hl::trading::fetchOpenOrders(perpDex[0] ? perpDex : nullptr);
        int count = 0;
        for (const auto& o : orders) {
            if (coinForApi.empty()
                || hl::utils::coinMatches(o.coin.c_str(), coinForApi.c_str()))
                count++;
        }
        if (hl::g_config.diagLevel >= 1)
            hl::g_logger.logf(1, "GET_OPEN_ORDERS(%s): %d (HTTP)",
                              coinForApi.empty() ? "ALL" : coinForApi.c_str(), count);
        return count;
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
        // Parameter: string coin name (recommended), or 0 to use currentSymbol
        // currentSymbol is set by BrokerAsset price queries (from asset() calls)
        const char* coin = nullptr;
        if (parameter != 0) {
            coin = (const char*)parameter;
        } else {
            coin = hl::g_trading.currentSymbol;
        }
        if (!coin || !*coin) {
            hl::g_logger.log(1, "HL_GET_FUNDING_RATE: no coin — pass coin name as parameter "
                                "or call asset() first");
            return 0.0;
        }
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
            if (!asset) continue;  // [OPM-198] skip assets without metadata
            double pip = asset->tickSize;
            double lotAmt = asset->minSize;
            int lev = asset->maxLeverage;
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

            double pip = asset->tickSize;       // [OPM-198] use pre-calculated
            double lotAmt = asset->minSize;
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

    case HL_SET_EXPORT_NFA: {
        // [OPM-801] Compliance bitfield for the NFA column of 50003's template.
        // Zorro never passes the account's NFA setting to the plugin, so the
        // caller supplies it: 1 = no partial closing, 2 = no hedging, 4 = FIFO,
        // 8 = no closing of trades (14/15 = full NFA compliance).
        int nfa = (int)parameter;
        if (nfa < 0 || nfa > 15) {
            hl::g_logger.logf(1, "HL_SET_EXPORT_NFA: %d out of range (0-15)", nfa);
            return 0;
        }
        hl::g_config.exportNfa = nfa;
        if (hl::g_config.diagLevel >= 1)
            hl::g_logger.logf(1, "HL_SET_EXPORT_NFA: export NFA column = %d", nfa);
        return 1;
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

        // NFA column comes from the caller, never from the plugin: it is the
        // user's compliance setting, made in Accounts.csv or via
        // set(NFA)/Hedge, and defaults to neutral. [OPM-801]
        fprintf(f, "Name,Server,AccountId,User,Pass,Assets,CCY,Real,NFA,Plugin\n");
        fprintf(f, "%s,%s,%s,%s,%s,AssetsHyperliquid,USD,%d,%d,%s\n",
                PLUGIN_NAME,
                hl::g_config.baseUrl[0] ? hl::g_config.baseUrl : "https://api.hyperliquid.xyz",
                hl::g_config.walletAddress[0] ? hl::g_config.walletAddress : "0",
                hl::g_config.walletAddress[0] ? hl::g_config.walletAddress : "0",
                hl::g_config.privateKey[0] ? hl::g_config.privateKey : "0",
                hl::g_config.isTestnet ? 0 : 1,
                hl::g_config.exportNfa,
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
