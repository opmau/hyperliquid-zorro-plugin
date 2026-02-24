# Completed Issues

Issues resolved during the `refactor/modularize` sprint. Moved from `KNOWN_ISSUES.md` on 2026-02-08.

For open issues, see [KNOWN_ISSUES.md](KNOWN_ISSUES.md).

---

## 1. WebSocket diagLevel Not Synchronized — FIXED

**Status:** FIXED

**Root Cause:** Two issues compounded:
1. `SET_DIAGNOSTICS` called before `BrokerLogin` — `g_wsManager` doesn't exist yet
2. `BrokerLogin` did `memset` to reset config but only preserved `zorroWindow`, not `diagLevel`

**Fix:** In `hl_broker.cpp` BrokerLogin, preserve `diagLevel` the same way `zorroWindow` is preserved.

---

## 2. initialSubscriptionsQueued Gate May Block L2Book Subscriptions — INVESTIGATED

**Status:** INVESTIGATED — Flag IS set correctly; diagnostic logging added

**Investigation:** Flag IS being set at `hl_broker.cpp:256`. Refactored code uses `initialSubsQueued_`. Sender loop logic is correct — checks both `connection_.isConnected()` AND `initialSubsQueued_`. Diagnostic logging added for Diag>=2 and Diag>=3.

---

## 3. Price Cache Data May Be Present But Stale — LOGGING ADDED

**Status:** Diagnostic logging added to trace l2Book parsing. Related to Issue #2.

---

## 4. BrokerTrade WS Cache Miss on Every Call — FIXED

**Status:** FIXED — parsers implemented in `ws_parsers.cpp`

**Root Cause:** During Phase 4 refactoring, `parseClearinghouseState()`, `parseOpenOrders()`, and `parseUserFills()` were left as stubs, causing HTTP fallback on every call.

**Fix:** Created `ws_parsers.h/.cpp` with full parsing implementations ported from legacy. Also fixes Issue #7.

---

## 5. JSON Parsing Fragility — MITIGATED

**Status:** MITIGATED — one fragile call site fixed, documentation added

**Fix:** `hl_market_service.cpp:getPerpDexPrice()` replaced with scoped parsing. `hl_utils.h` JSON helpers section now includes scoping warning.

---

## 6. Thread Safety Audit — COMPLETE

**Status:** COMPLETE — Audit done 2026-02-06. One race condition found (Issue #14) and fixed.

All shared state properly protected: `ws_price_cache`, `l2SubCs_`, `accountSubCs_`, `postCs_`, `responseCs_`, `g_trading.tradeCs`, `indexToCoin_`.

---

## 7. WebSocket Manager Parsing Simplified (Placeholder) — FIXED

**Status:** FIXED — see Issue #4 fix. Created `ws_parsers.h/.cpp` with full parsing for clearinghouseState, openOrders, and userFills.

---

## 8. WebSocket Manager Missing Index Mapping Methods — FIXED

**Status:** FIXED — index mapping methods added to `ws_manager.h/.cpp`, `populateWsIndexMappings()` implemented in `hl_meta.cpp`.

---

## 9. Position Parser Test Doesn't Handle Spaces in JSON — FIXED

**Status:** FIXED — whitespace-tolerant coin matching in test parser. All 9 tests pass (previously 5/9 failing).

---

## 10. CMakeLists.txt Zorro SDK Include Path Conflict — FIXED

**Status:** FIXED — removed Zorro SDK include path from CMake targets. API files use relative paths (`../../include/trading.h`).

---

## 11. time.h Conflicts with Zorro trading.h — FIXED

**Status:** FIXED — root cause was missing `DATE` typedef from `WIN32_LEAN_AND_MEAN` stripping `<wtypes.h>`. Added `#include <wtypes.h>` after `<windows.h>` in broker files.

---

## 12. Plugin Using HTTP Fallback Instead of WS — PARTIALLY FIXED

**Status:** PARTIALLY FIXED — multiple contributing causes addressed

**Fixes applied:**
1. WS parser stubs populated (Issues #4/#7) — dominant cause of excessive HTTP
2. HTTP seed cooldown aligned with cache maxAge (reduced from 3000ms to 1500ms)
3. Stale WS prices returned (capped at 5s) instead of empty data
4. BrokerAsset subscription race is by-design (first call per asset uses HTTP)

**Remaining:** `refreshBalance()` HTTP fallback doesn't populate WS cache (edge case, WS handles steady-state).

---

## 13. Startup HTTP Burst — Use allMids Instead of Per-Asset l2Book — FIXED

**Status:** FIXED — BrokerAsset subscription protocol corrected. Added `pPrice==NULL` early return to skip price fetch during subscription phase.

---

## 14. Race Condition on `indexToCoin_` Map — FIXED

**Status:** FIXED (2026-02-06, sprint P0-2). Added `CRITICAL_SECTION indexMapCs_` wrapping all 3 access methods.

---

## 15. `CreateEvent()`/`CreateThread()` Return Values Not Checked — FIXED

**Status:** FIXED (2026-02-06, sprint P1-1). Added NULL checks with cleanup and log messages.

---

## 17. `refreshBalance()` HTTP Fallback Doesn't Parse Response — FIXED

**Status:** FIXED (2026-02-06, sprint P1-3). Reuses `ws_parsers::parseClearinghouseState()` for HTTP response parsing.

---

## 18. EIP-712 Signing Not Wired Into Trading Service — FIXED

**Status:** FIXED (2026-02-06, sprint P0-1). Full `placeOrderWithId()` with 7-step EIP-712 signing flow. Mirror flow for `cancelOrder()`.

---

## 19. `GET_PRICE` Command Handler Returns Zero — FIXED

**Status:** FIXED (2026-02-06, sprint P1-4). Added `lastBid`/`lastAsk` to `TradingState`, cached in `BrokerAsset()`, returned by `GET_PRICE` handler.

**Note:** Subsequent testing (Issue #25) found the cached values are global, not per-asset. The handler works but returns the wrong asset's price. See open Issue #25.
