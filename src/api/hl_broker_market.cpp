//=============================================================================
// hl_broker_market.cpp - Broker data query exports
//=============================================================================
// Part of Hyperliquid Plugin for Zorro
//
// LAYER: API
// DEPENDENCIES: hl_broker_internal.h
// THREAD SAFETY: Main thread only (Zorro calls are single-threaded)
//
// This module provides read-only data exports:
// - BrokerAsset: price and asset parameter queries
// - BrokerAccount: balance queries
// - BrokerHistory2: historical candle data
//=============================================================================

#include "hl_broker_internal.h"

//=============================================================================
// BrokerAsset - Get asset price and info
//=============================================================================

DLLFUNC int BrokerAsset(char* symbol, double* pPrice, double* pSpread,
                        double* pVolume, double* pPip, double* pPipCost,
                        double* pMinAmount, double* pMargin,
                        double* pRollLong, double* pRollShort) {
    if (!symbol || !*symbol) return 0;

    if (hl::g_config.diagLevel >= 3) {
        hl::g_logger.logf(3, "BrokerAsset: %s", symbol);
    }

    // Parse perpDex and coin
    char perpDex[32], coin[64];
    parsePerpDex(symbol, perpDex, sizeof(perpDex), coin, sizeof(coin));

    std::string coinForApi = buildCoinForApi(perpDex, coin);

    // Subscription mode: pPrice=NULL means "subscribe this asset"
    // Per Zorro docs (brokerplugin.md): return 1 if asset is available, no price needed
    if (!pPrice) {
        // Try display name first (matches info.name), then API coin, then API format
        const hl::AssetInfo* asset = hl::market::getAsset(symbol);
        if (!asset) asset = hl::market::getAsset(coin);
        if (!asset) asset = hl::market::getAsset(coinForApi.c_str());
        if (asset) {
            // Subscribe to L2 book ONLY for metadata-confirmed assets.
            // Subscribing to non-existent coins (e.g., LINK/TRX/XRP on testnet)
            // causes the HL WS server to error/disconnect. [OPM-109]
            if (hl::g_wsManager) {
                auto* wsMgr = static_cast<hl::ws::WebSocketManager*>(hl::g_wsManager);
                wsMgr->subscribeL2Book(coinForApi);
            }
            // Set static parameters (don't require price data)
            double pip = pow(10.0, -asset->pxDecimals);
            double lotAmount = pow(10.0, -asset->szDecimals);
            if (pPip) *pPip = pip;
            if (pPipCost) *pPipCost = pip * lotAmount;
            if (pMinAmount) *pMinAmount = lotAmount;
            if (pMargin) *pMargin = -asset->maxLeverage;
            if (pRollLong) *pRollLong = 0;
            if (pRollShort) *pRollShort = 0;
            if (pVolume) *pVolume = 0;
            if (hl::g_config.diagLevel >= 2)
                hl::g_logger.logf(2, "BrokerAsset: %s subscription OK", coinForApi.c_str());
            return 1;
        }
        hl::g_logger.logf(1, "BrokerAsset: %s not found in metadata (skipping WS subscription)",
                          coinForApi.c_str());
        return 0;
    }

    // Price query mode: get current price via market service
    hl::PriceData price = hl::market::getPrice(coinForApi.c_str());

    if (price.bid <= 0 || price.ask <= 0) {
        if (hl::g_config.diagLevel >= 1) {
            hl::g_logger.logf(1, "BrokerAsset: No price for %s", coinForApi.c_str());
        }
        return 0;
    }

    // Get asset metadata (try display name first, then API coin, then API format)
    const hl::AssetInfo* asset = hl::market::getAsset(symbol);
    if (!asset) asset = hl::market::getAsset(coin);
    if (!asset) asset = hl::market::getAsset(coinForApi.c_str());

    // [OPM-6] DO NOT set currentSymbol here.
    // BrokerAsset is called for ALL subscribed assets during each bar update.
    // Setting currentSymbol here corrupts GET_PRICE context (returns wrong asset).
    // GET_PRICE now exclusively uses priceSymbol (set by SET_SYMBOL).

    // Zorro best practice: pPrice = ASK, pSpread = ASK - BID
    if (pPrice) *pPrice = price.ask;
    if (pSpread) *pSpread = price.ask - price.bid;

    // Asset parameters
    double pip = asset ? pow(10.0, -asset->pxDecimals) : 1.0;
    double lotAmount = asset ? pow(10.0, -asset->szDecimals) : 0.0001;

    if (pPip) *pPip = pip;
    if (pPipCost) *pPipCost = pip * lotAmount;
    if (pMinAmount) *pMinAmount = lotAmount;
    if (pMargin) *pMargin = asset ? -asset->maxLeverage : -50;
    if (pRollLong) *pRollLong = 0;
    if (pRollShort) *pRollShort = 0;
    if (pVolume) *pVolume = 0;

    return 1;
}

//=============================================================================
// BrokerAccount - Get account balance
//=============================================================================

DLLFUNC int BrokerAccount(char* accountId, double* pBalance,
                          double* pTradeVal, double* pMarginVal) {
    if (!hl::g_config.walletAddress[0]) return 0;

    // Get balance from WS cache
    hl::account::Balance balance = hl::account::getBalance();

    // On first call after login, WS may still be connecting (async thread).
    // Typical time to first clearinghouseState delivery: 1-3 seconds.
    // Poll-wait once during this initial window so Zorro's INITRUN gets
    // valid data. After first successful delivery, never block again.
    if (!balance.dataReceived && !g_everReceivedAccountData
        && hl::g_config.enableWebSocket && hl::g_wsManager) {
        auto* wsMgr = static_cast<hl::ws::WebSocketManager*>(hl::g_wsManager);
        if (wsMgr->isRunning()) {
            if (hl::g_config.diagLevel >= 1)
                hl::g_logger.log(1, "BrokerAccount: Waiting for initial WS data...");
            for (int i = 0; i < 50; i++) {  // Up to 5 seconds (50 x 100ms)
                if (nap) nap(100);
                else Sleep(100);
                balance = hl::account::getBalance();
                if (balance.dataReceived) break;
            }
        }
    }
    if (balance.dataReceived && !g_everReceivedAccountData) {
        g_everReceivedAccountData = true;
        // [OPM-19] First WS delivery: fetch spot USDC (not available via WS).
        // This runs once per login — spot balance rarely changes during a session.
        hl::account::refreshSpotBalance();
        balance = hl::account::getBalance();  // Re-read with spot included
    }

    if (!balance.dataReceived) {
        // [OPM-74] WS subscription may have failed (error channel response).
        // Fall back to HTTP so BrokerAccount doesn't return 0 to Zorro.
        if (hl::g_config.diagLevel >= 1) {
            hl::g_logger.log(1, "BrokerAccount: WS not delivered, trying HTTP fallback");
        }
        if (hl::account::refreshBalance()) {
            hl::account::refreshSpotBalance();
            balance = hl::account::getBalance();
            if (balance.dataReceived) {
                g_everReceivedAccountData = true;
            }
        }
        if (!balance.dataReceived) {
            hl::g_logger.log(1, "BrokerAccount: HTTP fallback also failed");
            return 0;
        }
    }

    // accountValue=0 is valid (testnet accounts often report 0 balance).
    // One-time HTTP check to confirm, then accept the result.
    if (balance.accountValue <= 0 && !g_lastHttpFallbackTime) {
        g_lastHttpFallbackTime = GetTickCount();
        if (hl::g_config.diagLevel >= 1) {
            hl::g_logger.log(1, "BrokerAccount: accountValue=0, one-time HTTP check");
        }
        hl::account::refreshBalance();
        hl::account::refreshSpotBalance();
        balance = hl::account::getBalance();
    }

    // Zorro expects: Balance + TradeVal = Equity
    // For crypto perps: accountValue IS the total equity (includes unrealized PnL).
    if (pBalance) *pBalance = balance.accountValue;
    if (pTradeVal) *pTradeVal = 0;
    if (pMarginVal) *pMarginVal = balance.marginUsed;

    if (hl::g_config.diagLevel >= 2) {
        char msg[256];
        sprintf_s(msg, "BrokerAccount: balance=%.2f margin=%.2f withdraw=%.2f",
                  balance.accountValue, balance.marginUsed, balance.withdrawable);
        hl::g_logger.log(2, msg);
    }

    return 1;
}

//=============================================================================
// BrokerHistory2 - Get historical candles
//=============================================================================

DLLFUNC int BrokerHistory2(char* symbol, DATE start, DATE end,
                           int tickMinutes, int nTicks, T6* ticks) {
    if (!symbol || !ticks || nTicks <= 0) return 0;

    if (hl::g_config.diagLevel >= 1) {
        hl::g_logger.logf(1, "BrokerHistory2: %s, Minutes=%d, NTicks=%d",
                          symbol, tickMinutes, nTicks);
    }

    // Parse symbol
    char perpDex[32], coin[64];
    parsePerpDex(symbol, perpDex, sizeof(perpDex), coin, sizeof(coin));
    std::string coinForApi = buildCoinForApi(perpDex, coin);

    // tickMinutes=0 means tick data — not supported, let Zorro handle fallback
    if (tickMinutes <= 0) return 0;

    // Convert Zorro DATE to milliseconds
    int64_t endMs = (int64_t)((end - 25569.0) * 86400.0 * 1000.0);
    int64_t intervalMs = (int64_t)tickMinutes * 60 * 1000;

    // Guard: if endMs is before 2020-01-01 (epoch/invalid date from Zorro), use current time
    static const int64_t MIN_VALID_MS = 1577836800000LL;
    if (endMs < MIN_VALID_MS) {
        endMs = hl::utils::currentTimestampMs();
        if (hl::g_config.diagLevel >= 1) {
            hl::g_logger.logf(1, "BrokerHistory2: Invalid end date (epoch), using current time");
        }
    }

    int64_t startMs = endMs - ((int64_t)nTicks * intervalMs);

    // Get candles via market service
    auto candles = hl::market::getCandlesByMinutes(
        coinForApi.c_str(), tickMinutes, startMs, endMs, nTicks);

    if (candles.empty()) {
        if (hl::g_config.diagLevel >= 1) {
            hl::g_logger.log(1, "BrokerHistory2: No candles returned");
        }
        return 0;
    }

    // Fill T6 array in REVERSE order (Zorro wants newest first)
    int count = (int)candles.size();
    if (count > nTicks) count = nTicks;

    for (int i = 0; i < count; i++) {
        const auto& c = candles[count - 1 - i];  // Reverse order

        // Convert timestamp (already bar END time from service layer) to OLE DATE
        ticks[i].time = 25569.0 + (c.timestamp / 1000.0) / 86400.0;

        ticks[i].fOpen = (float)c.open;
        ticks[i].fHigh = (float)c.high;
        ticks[i].fLow = (float)c.low;
        ticks[i].fClose = (float)c.close;
        ticks[i].fVol = (float)c.volume;
        ticks[i].fVal = 0;
    }

    if (hl::g_config.diagLevel >= 2) {
        char msg[64];
        sprintf_s(msg, "BrokerHistory2: Returned %d candles", count);
        hl::g_logger.log(2, msg);
    }

    return count;
}
