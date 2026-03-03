# Data Flows

Step-by-step traces of what happens when Zorro calls the plugin. Use this to debug why something isn't working.

---

## 1. Startup / Login

```
Zorro                         Plugin
  │                              │
  │──── zorro(GLOBALS*) ────────►│  Extract function pointers from Globals->Functions[]
  │                              │    nap          = Functions[13]
  │                              │    http_status  = Functions[142]
  │                              │    http_result  = Functions[143]
  │                              │    http_free    = Functions[144]
  │                              │    http_request = Functions[654]
  │                              │
  │──── BrokerOpen() ──────────►│  Store BrokerMessage callback
  │                              │  initGlobals() — zero g_config, g_assets, g_trading
  │                              │  crypto::init() — create secp256k1 context
  │◄──── return PLUGIN_TYPE ─────│
  │                              │
  │──── SET_DIAGNOSTICS ────────►│  g_config.diagLevel = N
  │──── SET_HWND ───────────────►│  g_config.zorroWindow = HWND
  │                              │
  │──── BrokerLogin(user,pwd) ──►│  1. Clear g_config, store credentials
  │                              │  2. Set baseUrl (Real→mainnet, else→testnet)
  │                              │  3. Derive address from private key
  │                              │  4. trading::init() — reset tradeMap, counters
  │                              │  5. market::refreshMeta() — HTTP GET /info
  │                              │     Populates g_assets with AssetInfo for all perps
  │                              │     Retries once on failure (2s delay)
  │                              │  6. account::checkUserRole() — detect agent wallet
  │                              │  7. Create PriceCache + WebSocketManager
  │                              │  8. wsMgr->start(wsUrl) — connect to WS
  │                              │  9. Subscribe: userFills, clearinghouseState, openOrders
  │                              │ 10. markInitialSubscriptionsQueued()
  │◄──── return 1 ───────────────│
```

**Failure modes:**
- `zorro()` not called → all function pointers are `nullptr` → every HTTP call fails silently
- Meta fetch fails twice → `BrokerLogin` returns 0, Zorro shows "Login failed"
- Agent wallet in User field → Warning, account data will be empty

---

## 2. Asset Subscription

When Zorro calls `BrokerAsset(symbol, pPrice=NULL, ...)` during initialization:

```
BrokerAsset("BTC-USDC", NULL, ...)
  │
  ├─ parsePerpDex("BTC-USDC") → perpDex="", coin="BTC"
  ├─ buildCoinForApi("", "BTC") → "BTC"
  │
  ├─ market::getAsset("BTC-USDC")  ← try display name first
  │   └─ Not found? Try getAsset("BTC")  ← try normalized coin
  │       └─ Not found? Try getAsset("BTC")  ← try API format
  │           └─ Found in g_assets → AssetInfo
  │
  ├─ wsMgr->subscribeL2Book("BTC")  ← WS subscription for real-time prices
  │
  ├─ Set outputs:
  │   pPip      = 10^(-pxDecimals)          e.g., 0.1 for szDecimals=5
  │   pPipCost  = pip * lotAmount = 0.000001  (constant for all assets)
  │   pMinAmount = 10^(-szDecimals)          e.g., 0.00001 for szDecimals=5
  │   pMargin   = -maxLeverage               e.g., -50 (negative = leverage)
  │
  └─ return 1 (asset available)
```

**PerpDex example:**
```
BrokerAsset("GOLD-USDC_xyz", NULL, ...)
  │
  ├─ parsePerpDex("GOLD-USDC_xyz") → perpDex="xyz", coin="GOLD"
  ├─ buildCoinForApi("xyz", "GOLD") → "xyz:GOLD"
  ├─ wsMgr->subscribeL2Book("xyz:GOLD")
  └─ return 1
```

---

## 3. Price Resolution

When Zorro calls `BrokerAsset(symbol, pPrice, pSpread, ...)` during bar updates:

```
BrokerAsset("BTC-USDC", &price, &spread, ...)
  │
  ├─ parsePerpDex → coin="BTC", coinForApi="BTC"
  │
  ├─ market::getPrice("BTC")
  │   │
  │   ├─ [1] Check WS PriceCache
  │   │   cache->getPriceData("BTC")
  │   │   If fresh (age < PRICE_MAX_AGE_HTTP_MS = 1500ms):
  │   │     └─ Return {bid, ask, mid, timestamp}  ← FAST PATH
  │   │
  │   ├─ [2] WS stale? Try HTTP seed
  │   │   If canSeedHttp("BTC") (cooldown expired, 1s per symbol):
  │   │     └─ HTTP POST /info {"type":"l2Book","coin":"BTC"}
  │   │        Parse response → update PriceCache → recordSeed("BTC")
  │   │
  │   ├─ [3] First query? Wait briefly for WS
  │   │   If no data yet and WS running:
  │   │     Sleep(WS_FIRST_DATA_WAIT_MS = 300ms), recheck cache
  │   │
  │   └─ [4] Return whatever is cached (even if stale)
  │       If nothing: return {0, 0, 0, 0}
  │
  ├─ If bid <= 0 or ask <= 0: return 0 (Zorro retries next bar)
  │
  ├─ Set outputs:
  │   pPrice  = ask          (Zorro convention: pPrice = ask price)
  │   pSpread = ask - bid
  │
  └─ return 1
```

**WS data flow (background):**
```
Exchange WS → IXWebSocket thread → messageQueue_ → WS Manager poll()
  │
  ├─ allMids channel → PriceCache::setPrice(coin, midPrice)
  ├─ l2Book channel  → PriceCache::setBidAsk(coin, bid, ask)
  └─ Both update PriceCache timestamp
```

---

## 4. Order Placement

When Zorro calls `BrokerBuy2(symbol, volume, stopDist, limit, ...)`:

### Normal Order (Entry or Market)

```
BrokerBuy2("BTC-USDC", 100, 0, 0, &price, &fill)
  │
  ├─ volume=100, limit=0, stopDist=0 → Market order
  ├─ isCloseOrder = (stopDist == -1) → false
  ├─ isTriggerOrder = g_config.stopOrderPending → false
  ├─ Reset stopOrderPending = false
  │
  ├─ parsePerpDex → coin="BTC", coinForApi="BTC"
  │
  ├─ Build OrderRequest:
  │   side = Buy (volume > 0)
  │   size = |volume| * lotSize = 100 * 0.0001 = 0.01 BTC
  │   limitPrice = 0 → market order path
  │
  ├─ Market order: get current price
  │   market::getPrice("BTC") → ask = 67000.0
  │   Apply 5% slippage: limitPrice = 67000 * 1.05 = 70350
  │   Force orderType = IOC
  │
  ├─ Generate tradeId = trading::generateTradeId() (atomic increment)
  │
  ├─ trading::placeOrderWithId(request, tradeId)
  │   │
  │   ├─ Generate CLOID: "0x{tradeId:08x}{timestamp:016llx}{counter:08x}"
  │   ├─ Generate nonce (monotonic timestamp-based)
  │   │
  │   ├─ Build OrderAction for EIP-712:
  │   │   asset=0 (BTC index), isBuy=true, price="70350"
  │   │   size="0.01", reduceOnly=false, orderType="Ioc"
  │   │
  │   ├─ Signing pipeline:
  │   │   OrderAction → msgpack::packOrderAction()
  │   │     → append nonce (8 bytes BE) + vault flag (1 byte)
  │   │     → keccak256() → connectionId
  │   │     → eip712::encodeAgentType(source, connectionId)
  │   │     → eip712::generateMessageHash(domainSep, structHash)
  │   │     → crypto::signHash() → {r, s, v}
  │   │
  │   ├─ Build JSON payload:
  │   │   {"action":{"type":"order","orders":[...],"grouping":"na"},
  │   │    "nonce":N,"signature":{"r":"0x...","s":"0x...","v":V}}
  │   │
  │   ├─ Submit:
  │   │   WS connected? → wsMgr->sendOrderSync(json, 5000ms)
  │   │   WS not connected? → http::exchangePost(json)
  │   │
  │   ├─ Parse response → OrderResult
  │   │   success=true, oid="0x...", filledSize=0.01, avgPrice=67001.5
  │   │
  │   └─ Store in tradeMap[tradeId] = OrderState{...}
  │
  ├─ account::applyFill("BTC", 0.01, 67001.5, true)  ← immediate cache update
  │
  ├─ Set outputs:
  │   *pFill = round(0.01 / 0.0001) = 100 lots
  │   *pPrice = 67001.5
  │
  └─ return tradeId
```

### Trigger (Stop-Loss) Order

Zorro calls `SET_ORDERTYPE` with bit 8 set, then calls `BrokerBuy2` twice:

```
SET_ORDERTYPE(ordertype | 8)
  └─ g_config.stopOrderPending = true

BrokerBuy2("BTC-USDC", 100, 0, 67000, ...)   ← Entry order
  ├─ stopOrderPending consumed → isTriggerOrder=false (limit > 0 but not trigger)
  └─ Normal limit order placed

BrokerBuy2("BTC-USDC", -100, 0, 65000, ...)  ← Stop-loss order
  ├─ stopOrderPending=true, limit=65000 → isTriggerOrder=true
  ├─ Build trigger request:
  │   triggerType = SL
  │   triggerPx = 65000 (trigger price from Limit parameter)
  │   triggerIsMarket = true
  │   reduceOnly = true
  │   limitPrice = 65000 * 0.95 = 61750 (slippage-protected)
  └─ Place via packTriggerOrderAction()
```

---

## 4b. Bracket Order (Entry + TP + SL)

When a Lite-C strategy calls `brokerCommand(HL_PLACE_BRACKET, &request)`:

```
HL_PLACE_BRACKET (50043)
  |
  +-- Validate BracketRequest:
  |   coin, side, size, limitPrice, tpTriggerPx, slTriggerPx
  |
  +-- Resolve assetIndex from coin (via g_assets)
  |
  +-- Build entry order (OrderAction):
  |   packOrderAction(asset, isBuy, limitPrice, size, orderType)
  |
  +-- If hasTP(): Build TP trigger order:
  |   packTriggerOrderAction(asset, !isBuy, tpTriggerPx, size,
  |                          reduceOnly=true, tpsl="tp")
  |
  +-- If hasSL(): Build SL trigger order:
  |   packTriggerOrderAction(asset, !isBuy, slTriggerPx, size,
  |                          reduceOnly=true, tpsl="sl")
  |
  +-- Combine into single JSON with grouping="normalTpsl"
  |   {"action":{"type":"order","orders":[entry,tp,sl],
  |              "grouping":"normalTpsl"},
  |    "nonce":N,"signature":{r,s,v}}
  |
  +-- Sign and POST to /exchange
  |
  +-- Parse response:
  |   Extract OIDs for each order
  |   Store all in tradeMap with separate trade IDs
  |
  +-- Return entryTradeId
```

**Key behavior:** With `normalTpsl` grouping, when the TP fills, the SL is automatically cancelled (status: `siblingFilledCanceled`), and vice versa. `BrokerTrade` recognizes this status and treats it as a closed trade.

---

## 4c. TWAP Order

When a Lite-C strategy calls `brokerCommand(HL_PLACE_TWAP, &request)`:

```
HL_PLACE_TWAP (50040)
  |
  +-- Validate TwapRequest:
  |   coin, side, size, durationMinutes, randomize, reduceOnly
  |
  +-- Resolve assetIndex from coin
  |
  +-- Build TwapOrderAction:
  |   packTwapOrderAction(asset, isBuy, size, duration, randomize, reduceOnly)
  |
  +-- Sign and POST to /exchange
  |
  +-- Parse response: extract twapId
  |
  +-- Return twapId (double)

HL_CANCEL_TWAP (50041)
  |
  +-- packTwapCancelAction(asset, twapId)
  +-- Sign and POST to /exchange
  +-- Return 1/0
```

**Note:** The exchange splits the TWAP into suborders executed every ~30 seconds. The `twapId` is used to cancel the entire TWAP sequence.

---

## 4d. Atomic Order Modify

When a Lite-C strategy calls `brokerCommand(HL_MODIFY_ORDER, &request)`:

```
HL_MODIFY_ORDER (50042)
  |
  +-- Validate ModifyRequest:
  |   Target: oid or oidCloid (one must be set)
  |   New params: coin, side, size, limitPrice, orderType
  |
  +-- Resolve assetIndex from coin
  |
  +-- Build modify action:
  |   packBatchModifyAction(oid/cloid, asset, isBuy, price, size, orderType)
  |
  +-- Sign and POST to /exchange
  |
  +-- Return 1 on success, 0 on failure
```

**Key benefit:** batchModify preserves queue priority -- the order keeps its position in the order book, unlike cancel+replace.

---

## 4e. Dead Man's Switch (scheduleCancel)

When a Lite-C strategy calls `brokerCommand(HL_SCHEDULE_CANCEL, seconds)`:

```
HL_SCHEDULE_CANCEL (50032)
  |
  +-- seconds > 0: SET mode
  |   +-- Convert: timeMs = (now + seconds) * 1000
  |   +-- packScheduleCancelAction(timeMs)
  |   +-- Sign and POST to /exchange
  |   +-- Return 1 on success
  |
  +-- seconds == 0: CLEAR mode
      +-- packScheduleCancelAction(0)   // time=null clears
      +-- Sign and POST to /exchange
      +-- Return 1 on success
```

**Usage:** Strategy calls `brokerCommand(HL_SCHEDULE_CANCEL, 300)` to cancel all open orders if no heartbeat within 5 minutes. Re-call periodically to push the deadline forward. Call with 0 to clear.

---

## 5. Trade Status Polling

Zorro calls `BrokerTrade(tradeId, ...)` to check order status and P&L:

```
BrokerTrade(42, &open, &close, &roll, &profit)
  │
  ├─ Get OrderState from tradeMap[42]
  │   Not found? → return NAY (unknown trade)
  │
  ├─ Check orderId prefix for special handling:
  │
  │   ┌─ "PENDING_..." (order submitted, no exchange OID yet)
  │   │  Query exchange: trading::queryOrderByCloid(cloid)
  │   │  Three outcomes:
  │   │    Found → update tradeMap with real OID + status
  │   │    NotFound → mark Cancelled, return NAY-1
  │   │    Failed → return NAY (retry next call)
  │   │
  │   ├─ "RESUMED_..." (historical position synced at startup)
  │   │  Use stored avgPrice as pOpen
  │   │  Calculate PnL: (currentPrice - avgPrice) * filledSize
  │   │  Return fillLots (no exchange query needed)
  │   │
  │   ├─ "IMPORTED_..." (position synced via GET_TRADES)
  │   │  Query LIVE position: account::getPosition(coin)
  │   │  If position closed or reversed → return 0
  │   │  If still open → return current size as lots
  │   │
  │   └─ Normal OID (exchange-assigned order ID)
  │      ├─ Check WS PriceCache for fills
  │      │  cache->getFillsForOrder(oid)
  │      │  Aggregate: totalFilled, avgPrice
  │      │  Update tradeMap if new fill data
  │      │
  │      ├─ HTTP stale check (if Open/PartialFill):
  │      │  If order age > 5s (unfilled) or 10s (partial):
  │      │    queryOrderByCloid() → update if filled/canceled
  │      │
  │      └─ Return:
  │         pOpen = entry price
  │         pProfit = (currentPrice - entryPrice) * size * direction
  │         return fillLots (or 0 if still pending, NAY-1 if cancelled)
```

**Partial fill detection:**

`filledSize >= requestedSize * 0.999` counts as `Filled` (not `PartialFill`). This handles floating-point rounding: e.g., 0.9999 BTC filled on a 1.0 BTC order is treated as fully filled.

---

## 6. Account Query

When Zorro calls `BrokerAccount(...)`:

```
BrokerAccount(NULL, &balance, &tradeVal, &marginVal)
  │
  ├─ account::getBalance() — read from WS cache
  │
  ├─ First call after login? (g_everReceivedAccountData == false)
  │   ├─ WS running? Poll up to 5 seconds (50 x 100ms nap)
  │   │   Each iteration: re-read balance from cache
  │   │   Break when dataReceived == true
  │   │
  │   ├─ On first successful delivery:
  │   │   g_everReceivedAccountData = true
  │   │   account::refreshSpotBalance() ← one-time HTTP call
  │   │   Re-read balance (now includes spot USDC)
  │   │
  │   └─ Still no data? HTTP fallback:
  │       account::refreshBalance() → POST /info clearinghouseState
  │       account::refreshSpotBalance() → POST /info spotClearinghouseState
  │
  ├─ accountValue == 0? One-time HTTP verify (testnet often has 0)
  │
  ├─ Set outputs:
  │   pBalance   = accountValue  (total equity including unrealized PnL)
  │   pTradeVal  = 0             (Zorro: Balance + TradeVal = Equity)
  │   pMarginVal = marginUsed
  │
  └─ return 1
```

**WS account data flow (background):**
```
Exchange WS → clearinghouseState channel
  │
  ├─ ws_parsers: parse crossMarginSummary
  │   accountValue, totalMarginUsed, withdrawable
  │
  ├─ ws_parsers: parse assetPositions[]
  │   Per position: coin, size, entryPx, unrealizedPnl, leverage
  │
  └─ Write to PriceCache:
      cache->setAccountData(accountValue, margin, withdrawable)
      cache->setPosition(coin, size, entryPx, ...)
```

**Key detail:** `clearinghouseState` is event-driven -- it fires only when balance or positions change, not periodically. Stale threshold is 60s (same as WS health timeout).

---

## 7. Historical Candles

When Zorro calls `BrokerHistory2(symbol, start, end, tickMinutes, nTicks, ticks)`:

```
BrokerHistory2("BTC-USDC", start, end, 60, 500, ticks)
  │
  ├─ parsePerpDex → coinForApi="BTC"
  ├─ tickMinutes=0? → return 0 (tick data not supported)
  │
  ├─ Convert dates:
  │   endMs = (end - 25569.0) * 86400 * 1000
  │   startMs = endMs - (500 * 60 * 60000)
  │
  ├─ market::getCandlesByMinutes("BTC", 60, startMs, endMs, 500)
  │   │
  │   ├─ Map 60 min → CandleInterval::H1
  │   ├─ HTTP POST /info {"type":"candleSnapshot","coin":"BTC",
  │   │                    "interval":"1h","startTime":startMs,"endTime":endMs}
  │   ├─ Parse response → vector<Candle>
  │   └─ Return candles (oldest first, timestamp = bar END time)
  │
  ├─ Fill T6[] array in REVERSE order (Zorro wants newest first)
  │   ticks[0] = most recent candle
  │   ticks[N-1] = oldest candle
  │
  └─ return count
```

**Candle intervals supported:**

| Minutes | Interval | API string |
|---------|----------|------------|
| 1 | M1 | "1m" |
| 3 | M3 | "3m" |
| 5 | M5 | "5m" |
| 15 | M15 | "15m" |
| 30 | M30 | "30m" |
| 60 | H1 | "1h" |
| 120 | H2 | "2h" |
| 240 | H4 | "4h" |
| 1440 | D1 | "1d" |

Unsupported intervals are mapped to the nearest available one. Maximum 5000 candles per request (Hyperliquid API limit).

---

## 8. WebSocket Reconnection

```
Connection lost
  │
  ├─ IXWebSocket detects disconnect
  │   Fires onClose callback → Connection queues "close" message
  │
  ├─ IXWebSocket auto-reconnect kicks in
  │   Exponential backoff: 1s → 2s → 4s → ... → 30s max
  │
  ├─ On successful reconnect:
  │   Connection::wasReconnected() returns true
  │
  ├─ WS Manager detects reconnection in connectionLoop():
  │   clearReconnectedFlag()
  │   requeueSubscriptionsAfterReconnect()
  │   Re-subscribe: l2Book (all coins), clearinghouseState, userFills, openOrders
  │
  ├─ Circuit breaker:
  │   consecutiveReconnects_++ on each reconnect
  │   If >= 15: open circuit breaker → 5 min cooldown
  │   On successful message: reset counter
  │
  └─ Health check:
      isHealthy() = (now - lastMessageTime) < 60s
      Unhealthy → services fall back to HTTP for price/account data
```
