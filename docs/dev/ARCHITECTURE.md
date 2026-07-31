# Architecture

The plugin is a Windows 32-bit DLL that implements the Zorro broker plugin interface as a thin adapter over the Hyperliquid perpetual futures exchange API.

---

## Layer Diagram

```
┌─────────────────────────────────────────────────────────┐
│  API LAYER: src/api/                                    │
│  Zorro broker exports (BrokerOpen, BrokerBuy2, etc.)    │
│  Thin adapter — no business logic                       │
└────────────────────────┬────────────────────────────────┘
                         │ calls
                         ▼
┌─────────────────────────────────────────────────────────┐
│  SERVICE LAYER: src/services/                           │
│  Business logic: market data, order management,         │
│  account queries, asset metadata                        │
└────────────────────────┬────────────────────────────────┘
                         │ calls
                         ▼
┌─────────────────────────────────────────────────────────┐
│  TRANSPORT LAYER: src/transport/                        │
│  HTTP client, WebSocket manager, price cache, parsers   │
└────────────────────────┬────────────────────────────────┘
                         │ calls
                         ▼
┌─────────────────────────────────────────────────────────┐
│  FOUNDATION LAYER: src/foundation/                      │
│  Types, config, globals, utilities, crypto, EIP-712     │
│  Zero dependencies on upper layers                      │
└─────────────────────────────────────────────────────────┘
```

**Dependency rule:** Dependencies flow **downward only**. Foundation knows nothing about transport. Transport knows nothing about services. Services know nothing about the API layer.

---

## Module Map

### Foundation (`src/foundation/`)

| File | Role |
|------|------|
| `hl_types.h` | All shared data structures: `OrderState`, `AssetInfo`, `PriceData`, `Position`, `OrderRequest`, `OrderResult`, enums (`OrderStatus`, `OrderSide`, `OrderType`, `TriggerType`) |
| `hl_config.h` | Compile-time constants: API endpoints, timeouts, cache durations, slippage, limits |
| `hl_globals.h` / `.cpp` | Runtime state singletons: `g_config`, `g_assets`, `g_trading`, `g_logger` |
| `hl_utils.h` / `.cpp` | String helpers, coin name normalization, time conversions (Unix <-> OLE DATE), price formatting |
| `hl_crypto.h` / `.cpp` | secp256k1 ECDSA signing, keccak256 hashing, Ethereum address derivation |
| `hl_eip712.h` / `.cpp` | EIP-712 typed data encoding (domain separator, agent type hash, order/cancel message hashing) |
| `hl_msgpack.h` / `.cpp` | MessagePack binary serialization for order and cancel actions (used by EIP-712) |

### Transport (`src/transport/`)

| File | Role |
|------|------|
| `hl_http.h` / `.cpp` | HTTP client wrapping Zorro's `http_request` function pointer. Provides `infoPost()` (query) and `exchangePost()` (signed actions) |
| `ws_types.h` | WebSocket-specific data structures: `PriceData`, `AccountData`, `PositionData`, `FillData` |
| `ws_price_cache.h` / `.cpp` | Thread-safe cache for prices, account data, positions, open orders, and fills. Single `CRITICAL_SECTION` protects all state |
| `ws_connection.h` / `.cpp` | IXWebSocket wrapper: connect, disconnect, poll messages, auto-reconnect with exponential backoff |
| `ws_manager.h` / `.cpp` | WebSocket orchestrator: subscription management, message routing, health monitoring, circuit breaker |
| `ws_parsers.h` / `.cpp` | JSON message parsers for WS channels (l2Book, clearinghouseState, userFills, orderUpdates). Uses yyjson |
| `json_helpers.h` | Thin yyjson wrappers for Hyperliquid's string-encoded numbers |

### Services (`src/services/`)

| File | Role |
|------|------|
| `hl_meta.h` / `.cpp` | Asset metadata: fetches perp universe from `/info`, builds `AssetInfo` entries with `szDecimals`, `pxDecimals`, min sizes |
| `hl_meta_spot.cpp` | Spot asset metadata (extension of `hl_meta`) |
| `hl_market_service.h` / `.cpp` | Price resolution (WS cache -> HTTP fallback), candle history, asset lookups |
| `hl_trading_service.h` / `.cpp` | Order placement pipeline: build request -> EIP-712 encode -> sign -> submit -> track |
| `hl_trading_cancel.cpp` | Order cancellation + dead man's switch (scheduleCancel) |
| `hl_trading_twap.h` / `.cpp` | TWAP order placement and cancellation |
| `hl_trading_modify.h` / `.cpp` | Atomic order modification via batchModify |
| `hl_trading_bracket.h` / `.cpp` | Bracket orders: entry + TP + SL with normalTpsl grouping |
| `hl_account_service.h` / `.cpp` | Balance, positions, margin queries. WS cache with HTTP fallback. Immediate fill application |

### API (`src/api/`)

| File | Role |
|------|------|
| `hl_broker_internal.h` | Shared preamble: Zorro SDK includes with macro cleanup, service includes, custom command codes |
| `hl_broker.cpp` | `zorro()`, `BrokerOpen`, `BrokerLogin`, `BrokerTime`. Plugin lifecycle, WS callback bridges |
| `hl_broker_market.cpp` | `BrokerAsset`, `BrokerAccount`, `BrokerHistory2`. Market data and account interface |
| `hl_broker_trade.cpp` | `BrokerBuy2`, `BrokerSell2`, `BrokerTrade`. Order placement and status polling |
| `hl_broker_commands.cpp` | `BrokerCommand` dispatch for all standard and custom command codes |

### Vendor (`src/vendor/`)

| File | Role |
|------|------|
| `yyjson/yyjson.h` + `yyjson.c` | Bundled fast read-only JSON library (used by WS parsers) |

---

## Global State

Four controlled singletons defined in `hl_globals.cpp`:

### `g_config` (RuntimeConfig)

Set at login, effectively read-only after. Holds:
- `walletAddress`, `privateKey` -- credentials
- `baseUrl` -- mainnet or testnet REST endpoint
- `isTestnet` -- network selection
- `diagLevel` -- logging verbosity (0-3)
- `enableWebSocket`, `useWsOrders` -- feature flags
- `orderType` -- default order type (IOC/GTC/ALO)
- `stopOrderPending` -- transient flag for SET_ORDERTYPE +8

### `g_assets` (AssetRegistry)

Thread-safe registry of all known assets. Protected by `CRITICAL_SECTION`.
- `assets[1024]` -- `AssetInfo` array indexed by asset index
- `findByName(name)`, `findByCoin(coin)`, `getByIndex(idx)`, `add(info)` -- accessors

### `g_trading` (TradingState)

Thread-safe trade management. Protected by `CRITICAL_SECTION tradeCs`.
- `nextTradeId` -- atomic counter for generating unique Zorro trade IDs
- `lastNonce` -- atomic counter for API request nonces
- `tradeMap` -- `std::map<int, OrderState>` mapping Zorro trade IDs to order state
- `currentSymbol`, `priceSymbol` -- context isolation for BrokerAsset vs GET_PRICE

### `g_logger` (Logger)

- `LogCallback` -- function pointer to Zorro's `BrokerMessage`
- Atomic log level
- `log()`, `error()`, `info()`, `debug()` -- level-filtered logging

### `g_wsManager` / `g_priceCache`

Typed as `void*` in Foundation to avoid a Foundation->Transport dependency. Cast to `hl::ws::WebSocketManager*` and `hl::ws::PriceCache*` at usage sites in the API and service layers.

---

## Thread Model

```
┌──────────────────────┐  reads PriceCache   ┌──────────────────────┐
│   ZORRO MAIN THREAD  │ ◄───────────────── │   WS MANAGER THREAD   │
│                      │                     │   (connectionLoop)    │
│  BrokerOpen          │  writes tradeMap    │   poll() messages     │
│  BrokerLogin         │ ──────────────────► │   dispatch to parsers │
│  BrokerAsset         │                     │   send subscriptions  │
│  BrokerBuy2          │                     │   health monitoring   │
│  BrokerTrade         │                     └────────┬─────────────┘
│  BrokerCommand       │                              │
└──────────────────────┘                              │ IXWebSocket
                                                      │ callbacks
                                              ┌───────┴─────────────┐
                                              │ IXWEBSOCKET THREAD  │
                                              │ (library-internal)  │
                                              │                     │
                                              │ TLS I/O             │
                                              │ ping/pong           │
                                              │ auto-reconnect      │
                                              └─────────────────────┘
```

**Synchronization:**
- `PriceCache` -- single `CRITICAL_SECTION` protecting all cached data (prices, positions, account, fills)
- `g_trading.tradeMap` -- separate `CRITICAL_SECTION` (`tradeCs`)
- IXWebSocket queues messages into `messageQueue_` (protected by its own `CRITICAL_SECTION`), drained by `poll()` on the manager thread

**Key invariant:** The Zorro main thread never touches WebSocket I/O directly. It reads from `PriceCache` and writes to `g_trading`. The WS manager thread writes to `PriceCache` and reads from `g_trading` (for fill callbacks).

---

## Zorro SDK Integration

### Function Pointer Initialization

Zorro calls `zorro(GLOBALS*)` **before** `BrokerOpen`. This function extracts runtime function pointers from `Globals->Functions[]`:

| Function | Index | Purpose |
|----------|-------|---------|
| `nap` | 13 | Non-blocking sleep (returns 0 if Zorro is aborting) |
| `http_status` | 142 | Check HTTP request completion |
| `http_result` | 143 | Read HTTP response body |
| `http_free` | 144 | Free HTTP request handle |
| `http_request` | 654 | Initiate HTTP request |

These are the **only** Zorro SDK functions the plugin uses. All HTTP communication goes through these function pointers.

### Macro Collision Management

The Zorro SDK `variables.h` defines hundreds of macros that map global variable names to `g->vXxx` struct fields. `hl_broker_internal.h` includes the Zorro headers inside `#pragma pack(push,4)` / `#pragma pack(pop)` and immediately undefines conflicting macros:

```cpp
#undef and        // C++ keyword
#undef or         // C++ keyword
#undef not        // C++ keyword
#undef function   // C++ identifier
#undef PI         // Math constant
#undef swap       // STL function
#undef MAX_ASSETS // Our config constant
#undef Balance    // Our struct name
```

See [GOTCHAS.md](GOTCHAS.md) for details.

### DLL Exports

All Zorro broker functions are exported with C linkage:

```cpp
#define DLLFUNC extern "C" __declspec(dllexport)

DLLFUNC int    BrokerOpen(char* Name, FARPROC fpMessage, FARPROC fpProgress);
DLLFUNC int    BrokerLogin(char* User, char* Pwd, char* Type, char* Accounts);
DLLFUNC int    BrokerTime(DATE* pTimeGMT);
DLLFUNC int    BrokerAsset(char* Asset, double* pPrice, double* pSpread, ...);
DLLFUNC int    BrokerAccount(char* Account, double* pBalance, ...);
DLLFUNC int    BrokerHistory2(char* Asset, DATE tStart, DATE tEnd, int nTickMinutes, ...);
DLLFUNC int    BrokerBuy2(char* Asset, int Amount, double StopDist, double Limit, ...);
DLLFUNC int    BrokerSell2(int TradeID, int Amount, double StopDist, double Limit, ...);
DLLFUNC int    BrokerTrade(int TradeID, double* pOpen, double* pClose, ...);
DLLFUNC double BrokerCommand(int Command, intptr_t Parameter);
```

---

## BrokerCommand Reference

### Standard Zorro Commands

Codes are those defined in the Zorro SDK header `include/trading.h`.

| Command | Code | Returns | Notes |
|---------|------|---------|-------|
| `GET_BROKERZONE` | 40 | `0` | UTC (no timezone offset) |
| `GET_MAXTICKS` | 43 | `5000` | Hyperliquid candleSnapshot limit |
| `GET_MAXREQUESTS` | 45 | `5` | Max concurrent HTTP requests |
| `GET_COMPLIANCE` | 51 | `0` | Returns 0; NFA mode controlled by Accounts.csv |
| `GET_POSITION` | 53 | lots | Per-symbol position, or rebuilds all positions if param=0 |
| `GET_PRICE` | 60 | price | Reads from PriceCache using `priceSymbol` |
| `GET_VOLUME` | 61 | `1` | Signals that tick activity exists |
| `GET_TRADES` | 71 | count | Fills `TRADE[]` array from live positions |
| `SET_SYMBOL` | 132 | `1` | Sets `g_trading.priceSymbol` for `GET_PRICE` context isolation |
| `SET_DIAGNOSTICS` | 138 | `1` | Sets `g_config.diagLevel` |
| `SET_AMOUNT` | 139 | `1` | Sets `g_trading.lotSize` |
| `GET_PRICETYPE` | 150 | `2` | Returns bid/ask prices |
| `SET_ORDERTYPE` | 157 | echo | Maps Zorro types: 0->IOC, 2->GTC, 4->ALO. Bit 8 sets `stopOrderPending`. Returns the accepted type (`1` when 0 was passed), or `0` for the unsupported AON variants |
| `SET_HWND` | 172 | `1` | Stores Zorro window handle for WS message routing |
| `DO_CANCEL` | 301 | 1/0 | Cancels order by trade ID |

### Custom Plugin Commands

| Command | Code | Parameter | Returns | Notes |
|---------|------|-----------|---------|-------|
| `HL_EXPORT_ASSETS` | 50001 | file path string | 1 on success | Writes CSV for BTC/ETH/SOL |
| `HL_EXPORT_META` | 50002 | file path string | asset count | Writes CSV for all assets |
| `HL_EXPORT_ACCOUNT` | 50003 | file path string | 1 on success | Writes Accounts.csv row |
| `HL_VALIDATE_PRICES` | 50010 | 0 | 1/0 | Health check: fetches BTC/ETH/SOL prices via HTTP |
| `HL_ENABLE_WEBSOCKET` | 50011 | 0/1 | 1 | Enable/disable WebSocket at runtime |
| `HL_SET_ORDER_TYPE` | 50012 | "Ioc"/"Gtc"/"Alo" | 1 on success | Set order type directly |
| `HL_GET_OPEN_ORDERS` | 50020 | symbol or 0 | count | Returns count of open orders |
| `HL_SET_ACCOUNT_MODE` | 50021 | 0/1 | 1 on success | 0=API wallet, 1=vault |
| `HL_FORCE_WS_DISCONNECT` | 50030 | 0 | 1 on success | Debug: force WS disconnect |
| `HL_GET_FUNDING_RATE` | 50031 | coin string or 0 | hourly rate (double) | Current funding rate |
| `HL_SCHEDULE_CANCEL` | 50032 | seconds (0=clear) | 1/0 | Dead man's switch |
| `HL_PLACE_TWAP` | 50040 | `TwapRequest*` | twapId (double) | Place TWAP order |
| `HL_CANCEL_TWAP` | 50041 | twapId (uint64) | 1/0 | Cancel active TWAP |
| `HL_MODIFY_ORDER` | 50042 | `ModifyRequest*` | 1/0 | Atomic order modify |
| `HL_PLACE_BRACKET` | 50043 | `BracketRequest*` | entryTradeId | Bracket order (entry+TP+SL) |

---

## Signing Pipeline

Every order and cancellation must be cryptographically signed using Ethereum's EIP-712 standard:

```
OrderRequest
    │
    ▼
OrderAction (hl_eip712.h)
    │  asset index, isBuy, price, size, reduceOnly, orderType, cloid
    ▼
MessagePack encode (hl_msgpack.cpp)
    │  binary serialization matching Hyperliquid's key ordering
    ▼
Append nonce (8 bytes big-endian) + vault flag (1 byte)
    │
    ▼
keccak256 hash → connectionId (32 bytes)
    │
    ▼
encodeAgentType(source, connectionId) → structHash
    │  source = "a" (mainnet) or "b" (testnet)
    ▼
EIP-712 message: keccak256("\x19\x01" || domainSeparator || structHash)
    │
    ▼
secp256k1 ECDSA sign → {r, s, v} signature
    │
    ▼
JSON payload: {"action":{...}, "nonce":N, "signature":{r,s,v}}
    │
    ▼
POST to /exchange endpoint
```

The domain separator is fixed: `name="Exchange"`, `version="1"`, `chainId=1337`, `verifyingContract=0x0...0`.

Cancel actions follow the same pipeline with `packCancelAction()` instead of `packOrderAction()`.

Trigger (stop-loss/take-profit) orders use `packTriggerOrderAction()` which adds `triggerIsMarket`, `triggerPx`, and `tpsl` fields.

### Additional Signing Pipelines

| Action type | MsgPack function | EIP-712 action type | Notes |
|-------------|------------------|---------------------|-------|
| Standard order | `packOrderAction()` | `"order"` | Grouping: `"na"` |
| Trigger order | `packTriggerOrderAction()` | `"order"` | Grouping: `"na"`, adds tpsl/trigger fields |
| Bracket order | `packOrderAction()` + `packTriggerOrderAction()` | `"order"` | Grouping: `"normalTpsl"`, bundles entry+TP+SL |
| Cancel | `packCancelAction()` | `"cancel"` | By OID |
| Cancel by CLOID | `packCancelByCloidAction()` | `"cancelByCloid"` | By client order ID |
| Batch modify | `packBatchModifyAction()` | `"batchModify"` | Atomic price/size change, preserves queue priority |
| TWAP place | `packTwapOrderAction()` | `"twapOrder"` | Exchange handles suborder splitting |
| TWAP cancel | `packTwapCancelAction()` | `"twapCancel"` | By twapId |
| Schedule cancel | `packScheduleCancelAction()` | `"scheduleCancel"` | Dead man's switch (set/clear) |
