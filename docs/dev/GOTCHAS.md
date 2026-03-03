# Gotchas

Platform traps, API quirks, and hard-won lessons. Read this before modifying the plugin.

---

## Zorro Platform

### Macro Collisions from `variables.h`

Zorro's `include/variables.h` defines macros like:

```c
#define TakeProfit g->vTakeProfit
#define Stop       g->vStop
#define Lots       g->vLots
#define Balance    g->vBalance
```

Any C++ identifier matching a Zorro global variable name is **silently macro-expanded** in API-layer files. This causes cryptic compiler errors or wrong runtime behavior with no warning.

**Prevention:** Use abbreviated names (`SL`/`TP` instead of `StopLoss`/`TakeProfit`). All required `#undef` directives are in `src/api/hl_broker_internal.h` -- if you hit a new collision, add the `#undef` there.

### Zorro SDK Functions Are NULL Outside Zorro

`http_request`, `http_status`, `http_result`, `http_free`, and `nap` are `extern` function pointers initialized by `zorro()` at DLL load time. They are `nullptr` until Zorro calls the plugin.

**For tests:** Include `tests/mocks/mock_zorro.h` with `#define MOCK_ZORRO_IMPLEMENTATION` in exactly one `.cpp` file. This provides stub implementations that return configurable responses.

### SET_ORDERTYPE +8 Means a Separate Stop Order

When Zorro sets `ordertype |= 8` (the STOP flag), it calls `BrokerBuy2` **twice** for a single trade:

1. First call: entry order (normal)
2. Second call: stop-loss trigger order (`limit` parameter = trigger price)

The `g_config.stopOrderPending` flag bridges these two calls. It is set by `SET_ORDERTYPE` and consumed (reset) by `BrokerBuy2`. Always reset `stopOrderPending` after reading it, regardless of outcome.

### BrokerAsset Is Called for Every Subscribed Asset

`BrokerAsset` with `pPrice != NULL` is called for **every** subscribed asset during each bar update, not just the strategy's current asset. This means writing to `g_trading.currentSymbol` inside `BrokerAsset` would corrupt the `GET_PRICE` context. The fix: a separate `priceSymbol` field set only by `SET_SYMBOL`.

### NFA Mode (Hedge=0)

Hyperliquid has one position per asset (no hedging). The plugin operates in NFA mode:
- Closing is done by placing an opposite-side `reduceOnly` order
- `GET_COMPLIANCE` returns `2 + 8` (NFA + no hedging)
- `BrokerSell2` always sets `reduceOnly = true`

### UTC Everywhere

Hyperliquid returns UTC timestamps. Zorro uses UTC internally. Do not modify timezone settings in `ZorroFix.ini`.

`BrokerTime` returns: `25569.0 + time(NULL) / 86400.0` (OLE DATE format, UTC).

---

## Hyperliquid API

### PIPCost Is Constant (0.000001)

For **all** Hyperliquid perpetual assets:

```
pxDecimals + szDecimals = 6  (always)
PIP       = 10^(-pxDecimals)
LotAmount = 10^(-szDecimals)
PIPCost   = PIP * LotAmount = 10^(-6) = 0.000001
```

This seems wrong but is mathematically correct. Hyperliquid allocates exactly 6 decimal digits of precision, split between price and size. There is a regression test that validates this for every `szDecimals` value.

**Example (BTC, szDecimals=5):**
- pxDecimals = 6 - 5 = 1
- PIP = 0.1, LotAmount = 0.00001
- PIPCost = 0.1 * 0.00001 = 0.000001

### EIP-712 Source Field

The `source` parameter in EIP-712 signing **must** be:
- `"a"` for mainnet
- `"b"` for testnet

Sending the wrong value causes the exchange to reject the signature silently. This was a real bug that caused all orders to fail on testnet. There is a regression test for it.

### CLOID Format

Client Order IDs follow this format:

```
0x{tradeId:08x}{timestamp:016llx}{counter:08x}
```

The Zorro trade ID is embedded in the first 4 bytes. `extractTradeIdFromCloid()` recovers it, enabling O(1) lookup when WebSocket fill notifications arrive referencing only the CLOID.

### Agent Wallet vs Master Address

If the wallet address entered in Zorro's User field is an API/agent wallet (not the master wallet):
- `clearinghouseState` returns empty/zero data
- Position and balance queries will show nothing

The plugin detects this via `account::checkUserRole()` at login and warns the user. The master address must be used for account queries; the agent key is used for signing.

### Numbers Are JSON Strings

Hyperliquid encodes numeric values as JSON strings: `"accountValue":"511.47"` instead of `"accountValue":511.47`. The `json_helpers.h` wrappers handle both formats via yyjson.

### Asset Indexing

| Asset type | Index range | Example |
|-----------|-------------|---------|
| Standard perps | 0-999 | BTC=0, ETH=1, ... |
| PerpDex assets | 110000+, 120000+, ... | First perpDex starts at 110000 |
| Spot assets | 10000+ | 10000 + spotIndex |

API calls use the **full** index. Position updates from the exchange reference perpDex assets as `"@110001"` -- use `meta::convertPerpDexIndexToCoin()` to resolve to a coin name.

### Price Formatting

All prices sent to the exchange must be rounded to 5 significant figures via `utils::formatPriceForExchange()`. Sending more precision causes order rejection.

---

## Build System

### Always 32-bit

Zorro is a 32-bit (x86) process. The DLL **must** be built with `-A Win32`. An x64 DLL will be silently ignored by Zorro -- no error, it just won't appear in the broker list.

The vcpkg triplet must match: `x86-windows-static`.

### DEV_BUILD Flag

| Flag | DLL reports as | DLL filename |
|------|---------------|-------------|
| `DEV_BUILD=ON` (default) | "Hyperliquid-DEV" | `Hyperliquid_Dev.dll` |
| `DEV_BUILD=OFF` | "Hyperliquid" | `Hyperliquid.dll` |

Both dev and production DLLs can coexist in Zorro's plugin directory.

### Paths Have Spaces

The project lives in OneDrive. Always quote paths in shell commands:

```powershell
# Correct
cmake --build "c:\Users\admki\OneDrive\Zorro-plugin\build_vcpkg" --config Release

# Wrong -- will fail
cmake --build c:\Users\admki\OneDrive\Zorro-plugin\build_vcpkg --config Release
```

---

## Threading

### Three Threads at Runtime

| Thread | Owner | What it does |
|--------|-------|-------------|
| Main (Zorro) | Zorro process | Calls broker functions, reads from PriceCache |
| WS Manager | `ws_manager.cpp` | Polls IXWebSocket, dispatches messages, sends subscriptions |
| IXWebSocket internal | IXWebSocket library | TLS I/O, ping/pong, auto-reconnect |

### Shared State Synchronization

All shared state flows through `PriceCache`, protected by a single `CRITICAL_SECTION`. The WS manager thread writes; the Zorro main thread reads.

`g_trading.tradeMap` has its own `CRITICAL_SECTION` (`tradeCs`). Any function that creates or updates a trade ID must acquire this lock.

### Deadlock Prevention

`logCallback_` is set to `nullptr` before `WaitForSingleObject` in `WebSocketManager::stop()`. Both the WS thread and IXWebSocket's thread call `logCallback_` which routes to `BrokerMessage` which calls `SendMessage` to the Zorro GUI. If the main (GUI) thread is blocked waiting for the WS thread to stop, `SendMessage` deadlocks. Nulling the callback first breaks this cycle.

### IXWebSocket Auto-Reconnect

IXWebSocket handles reconnection natively with exponential backoff (1s to 30s). On reconnect, the WS manager re-subscribes all channels via `requeueSubscriptionsAfterReconnect()`. A circuit breaker stops reconnect storms after 15 consecutive failures (5-minute cooldown).

---

## Fatal Error Handling (OPM-170)

### Global Fatal Error Flag

`g_fatalError` is a `std::atomic<bool>` in `hl_globals.h`. When set to `true`, all broker functions (`BrokerAsset`, `BrokerBuy2`, `BrokerAccount`) return 0 immediately. The WS manager sets this flag when it detects an unrecoverable state (e.g., toxic subscriptions causing reconnect loops).

`zorroQuit()` in `hl_broker.cpp` calls Zorro's `quit("!...")` to halt the strategy. The `static bool halted` flag ensures it fires only once.

### Toxic L2Book Subscriptions

If a coin's l2Book subscription causes repeated reconnects with no data received, the WS manager:
1. Tracks per-coin reconnect failures via `l2RequeueFailCount_`
2. After 3 consecutive reconnects without data, bans the coin (`bannedL2Coins_`)
3. Sets `g_fatalError = true` and stops auto-reconnect
4. Logs the toxic coin name for debugging

**Symptom:** Strategy halts with "FATAL: WebSocket entered unrecoverable state".

### PerpDex Name Separator (OPM-169)

PerpDex assets use **underscore** (`_`) as the separator between the base symbol and dex name:
- `GOLD-USDC_xyz` (Zorro display name) -> `xyz:GOLD` (API format)

Previously used dot (`.`), which caused conflicts with Zorro's history file paths (Zorro interprets dots in filenames as file extension separators).

### HTTP Fallback for Positions (OPM-134)

`account::ensurePositionData()` adds an HTTP fallback when WebSocket `clearinghouseState` hasn't delivered position data. This prevents `getPosition()` and `getAllPositions()` from returning empty results during WS warmup or after reconnection.
