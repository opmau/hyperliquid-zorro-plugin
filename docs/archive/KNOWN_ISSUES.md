# Known Issues

Open bugs and gaps. For resolved issues, see [COMPLETED_ISSUES.md](COMPLETED_ISSUES.md).

**Last updated:** 2026-02-08

---

## 16. `s_seedCs` CRITICAL_SECTION Never Deleted

**Found in:** `src/services/hl_market_service.cpp:27-37`

**Symptom:**
`InitializeCriticalSection(&s_seedCs)` called on first use but `DeleteCriticalSection()` never called. OS kernel object leaked on each DLL load cycle.

**Fix Approach:**
Either: (a) Add cleanup to a module-level shutdown function called from `BrokerOpen` logout path, or (b) Use `std::mutex` instead which handles its own lifecycle via RAII.

**Priority:** Low (OS reclaims on process exit; only matters for repeated DLL load/unload cycles)

**Target Module:** `src/services/hl_market_service.cpp`

---

## 20. Export Commands (50001-50003) Defined But Not Implemented

**Found in:** `src/api/hl_broker_commands.cpp:71-73`

**Symptom:**
Strategies calling `brokerCommand(50001, ...)` etc. for asset/meta/account export get `0` back. Constants defined but no `case` handler exists.

**Fix Approach:**
Add `case` handlers that write CSV/JSON output files, matching legacy behavior.

**Priority:** Low (convenience feature, not required for trading)

**Target Module:** `src/api/hl_broker_commands.cpp`

---

## 21. Foundation Layer Depends on Transport Types (`hl_globals.h`)

**Found in:** `src/foundation/hl_globals.h:20-23, 141-142`

**Symptom:**
No runtime impact, but violates the dependency rule: Foundation must have zero knowledge of Transport types. Forward-declares `WebSocketManager` and `PriceCache` in Foundation layer.

**Fix Approach:**
Replace with `void*` opaque pointers in Foundation, cast in Transport/Services. Or move WS pointers to `src/transport/hl_ws_globals.h`.

**Priority:** Medium (architectural hygiene)

**Target Module:** `src/foundation/hl_globals.h`

---

## 22. API Layer Files Exceed Size Limits

**Found in:** `src/api/hl_broker.cpp` (~1000 lines), `src/api/hl_broker_commands.cpp` (~1000 lines)

**Symptom:**
Both API layer files are ~2.5x the 400-line limit from CLAUDE.md.

**Fix Approach:**
Split `hl_broker.cpp` into lifecycle/asset/trade files. Split `hl_broker_commands.cpp` into get/set files.

**Priority:** Low (does not affect functionality; improves developer experience)

**Target Module:** `src/api/`

---

## 23. Account Value Wrong — Unified Account Not Supported

**Found in:** `src/transport/ws_parsers.cpp:156`

**Symptom:**
Plugin reports equity ~$139 but Hyperliquid UI shows Portfolio Value ~$343. The gap is the spot/USDC portion of the unified account that `marginSummary` doesn't include.

**Root Cause:**
`parseClearinghouseState()` reads `"marginSummary"` which only covers perps. Unified accounts need `"crossMarginSummary"`.

**Fix Approach:**
Check for `"crossMarginSummary"` field; if present, use it instead of `"marginSummary"`. May also need `spotClearinghouseState` for spot balances.

**Priority:** High (account value is wrong for all unified account users)

**Target Module:** `src/transport/ws_parsers.cpp`, `src/services/hl_account_service.cpp`

---

## 24. No Support for Spot/Cash Trading (Perps Only)

**Found in:** Plugin-wide

**Symptom:**
Plugin only supports perpetual futures. Users with unified accounts cannot see or trade spot balances through Zorro.

**Fix Approach:**
Research spot API, add `spotClearinghouseState` WS subscription, spot order types, asset naming convention (`BTC` = perp, `BTC-SPOT` = spot).

**Priority:** Medium (feature request — perps trading works, spot is additive)

**Target Module:** Multiple — services, transport, API layers

---

## 25. GET_PRICE Returns Wrong Asset's Price (Global lastBid/lastAsk)

**Found in:** `src/api/hl_broker_commands.cpp:146-156`, `src/foundation/hl_globals.h`

**Symptom:**
`brokerCommand(GET_PRICE, 0)` returns the price of the **last asset processed by BrokerAsset()**, not the currently selected asset. Smoke test: `asset("BTC"); brokerCommand(GET_PRICE, 0)` returned 87.17 (SOL's price).

**Root Cause:**
`hl::g_trading.lastBid`/`lastAsk` are a single global pair, not per-asset. Every `BrokerAsset()` call overwrites them.

**Fix Approach:**
Replace with per-asset map or query WS price cache directly in GET_PRICE handler.

**Priority:** Medium (Zorro's own price series is correct; GET_PRICE used by some strategies)

**Target Module:** `src/api/hl_broker_commands.cpp`, `src/foundation/hl_globals.h`

---

## 26. ~~Smoke Test enterLong() Returns 0 — Zorro Blocks Trade Internally~~ FIX APPLIED

**Status:** FIX APPLIED (2026-02-08) — Needs smoke test verification, then move to COMPLETED_ISSUES.md

**Root Cause (confirmed):** Missing `SET_ORDERTYPE` (command 157) handler in the broker plugin.

When `enterLong(1, limitPrice, 0, 0)` is called with a limit price, Zorro internally calls `BrokerCommand(SET_ORDERTYPE, 2)` to set GTC order mode. The plugin's `handleBrokerCommand` had no `case SET_ORDERTYPE:` — it fell through to `default` which returns 0, telling Zorro "this broker does not support GTC orders." Since the trade had a limit price (Entry > 0), Zorro required GTC/IOC support and blocked the trade before ever calling `BrokerBuy2`.

Evidence: Every other crypto plugin (Bittrex, Deribit, Binance, Kraken) handles `SET_ORDERTYPE`. The Bittrex reference plugin at `Source/BittrexPlugin/Bittrex.cpp:430` stores the type and returns `Parameter & 3`.

**Fix applied (2026-02-08):**

1. Added `SET_ORDERTYPE` handler to `src/api/hl_broker_commands.cpp`:
   - Type 0 (FOK) → HL "Ioc" (closest match)
   - Type 1 (IOC) → HL "Ioc"
   - Type 2 (GTC) → HL "Gtc"
   - Returns the order type code to confirm support

2. Added `BrokerTime` export to `src/api/hl_broker.cpp`:
   - Returns 2 (market open) — crypto markets are 24/7
   - Returns current UTC time as OLE DATE
   - Returns 0 if not logged in

**Verification needed:** Re-run smoke test to confirm enterLong succeeds with the fix

---

## 27. BrokerAsset PIPCost Warning 054 — Returns LotAmount Instead of PIP*LotAmount

**Found in:** `src/api/hl_broker.cpp` (BrokerAsset function)

**Symptom:**
Zorro emits `Warning 054: BTC PIPCost 0.000010000 -> 0.000001000` for every asset. Plugin returns `PIPCost = LotAmount` instead of `PIPCost = PIP * LotAmount = 0.000001`.

**Fix Approach:**
Change `*pPipCost = lotAmount` to `*pPipCost = pip * lotAmount` (or hardcode 1e-6).

**Priority:** Low (Zorro auto-corrects; cosmetic warning only)

**Target Module:** `src/api/hl_broker.cpp`

---

## 29. ~~WS Always Falls Back to HTTP~~ FIX APPLIED

**Status:** FIX APPLIED (2026-02-09) — Needs smoke test verification, then move to COMPLETED_ISSUES.md

**Found in:** `src/transport/ws_manager.cpp`

**Symptom:**
Every price query falls back to HTTP. The log shows `getPrice: BTC no WS data` followed by `HTTP fallback for BTC` for every asset, every bar. WebSocket works correctly for account data (fills, clearinghouse, orders) but never provides price data.

**Root Cause (confirmed):**
Thread contention on the WinHTTP WebSocket handle. The sender thread called `WinHttpWebSocketSend()` for l2Book subscriptions while the connection thread's `WinHttpWebSocketReceive()` was blocking on the same synchronous handle. Account data subscriptions worked by timing luck (sent during early-connection windows before the receive loop was blocking). l2Book subscriptions, queued later by BrokerAsset, competed with the blocking receive and were either silently dropped or never processed by HL.

**Fix applied (2026-02-09):**

1. Moved all subscription sends (l2Book + account) from the sender thread into the connection thread's receive loop, eliminating concurrent WinHTTP handle access
2. Sender thread now only handles keepalive pings (lightweight, infrequent — every 20s)
3. Added Diag=2 logging to `parseL2Book` success and subscription sends for diagnostics
4. Made `parseL2Book` coin extraction more robust (skip whitespace/quotes like legacy code)

**Verification needed:** Re-run smoke test to confirm l2Book data flows via WS and HTTP fallback stops

---

## 28. ~~Error 013: Invalid Expression — Crash During Smoke Test~~ RESOLVED

**Status:** RESOLVED (2026-02-08) — Move to COMPLETED_ISSUES.md

**Root Cause (confirmed):** Two test script bugs, not plugin bugs:

1. `BarPeriod=1` (1-min bars) + `SET_DIAGNOSTICS=2` (verbose WS) generated ~600 lines/min of diagnostic spam, overflowing Zorro's log buffer before Phase 3 could run at Bar 3 (3 minutes in)
2. `printf("%d", (var)Lots)` — using `%d` (expects 4-byte int) with `var` (8-byte double) corrupts the stack frame. Zorro docs warn this "can even cause a program crash." On x86, `%d` of double 21.0 reads low 4 bytes of IEEE 754 = 0x00000000 = displays 0, and corrupts all subsequent printf arguments.

**Fix applied:** Changed `BarPeriod = 1/6.0` (10-sec bars), `SET_DIAGNOSTICS = 1`, and `%d` → `%.0f` for `var` types. Smoke test now completes all 4 phases in ~70 seconds.

---

## Template for New Issues

```markdown
## [N]. [Short Title]

**Found in:** [file/module]

**Symptom:**
[What the user sees or what happens]

**Root Cause:**
[Why it happens, or "Suspected:" if unconfirmed]

**Fix Approach:**
[Brief description of how to fix]

**Priority:** [Critical | High | Medium | Low]

**Target Module:** [Which refactored module should contain the fix]
```
