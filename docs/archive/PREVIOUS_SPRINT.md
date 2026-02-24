# Previous Sprint: Pre-Production Merge (Completed Items)

**Sprint started:** 2026-02-06
**Branch:** `refactor/modularize`
**Predecessor:** Refactoring complete (Phases 1-7). This sprint addressed gaps found during tech lead review.

Items completed during this sprint have been moved here from `CURRENT_SPRINT.md` on 2026-02-08.

---

## P0 — Blockers (COMPLETE)

### P0-1: Wire EIP-712 Signing Into Trading Service
**Issue:** KNOWN_ISSUES #18 (now in COMPLETED_ISSUES.md)
**Status:** COMPLETE (2026-02-06)

Implemented `placeOrderWithId()` with 7-step flow: build order action, EIP-712 hash, secp256k1 sign, JSON payload, submit via HTTP or WS, parse response, update TradeMap. Mirror flow for `cancelOrder()`. Nonce management ported from legacy.

**Files modified:** `src/services/hl_trading_service.cpp`

---

### P0-2: Fix Race Condition on `indexToCoin_` Map
**Issue:** KNOWN_ISSUES #14 (now in COMPLETED_ISSUES.md)
**Status:** COMPLETE (2026-02-06)

Added `mutable CRITICAL_SECTION indexMapCs_` to `ws_manager.h`. Wrapped `setIndexMapping()`, `clearIndexMappings()`, `getCoinByIndex()` with EnterCriticalSection/LeaveCriticalSection.

**Files modified:** `src/transport/ws_manager.h`, `src/transport/ws_manager.cpp`

---

## P1 — Should Fix Before Merge (COMPLETE)

### P1-1: Add NULL Checks for `CreateEvent`/`CreateThread`
**Issue:** KNOWN_ISSUES #15 (now in COMPLETED_ISSUES.md)
**Status:** COMPLETE (2026-02-06)

Added NULL checks after `CreateEvent()` and `CreateThread()` in `start()` and `sendOrderSync()`. Log messages for each failure case.

**Files modified:** `src/transport/ws_manager.cpp`

---

### P1-2: Add `build/` to `.gitignore`
**Status:** COMPLETE (2026-02-06)

Added `build/` and `build32/` to `.gitignore`. Removed tracked build files with `git rm -r --cached`.

---

### P1-3: Implement `refreshBalance()` HTTP Response Parsing
**Issue:** KNOWN_ISSUES #17 (now in COMPLETED_ISSUES.md)
**Status:** COMPLETE (2026-02-06)

Added `#include "../transport/ws_parsers.h"` and call to `hl::ws::parseClearinghouseState()` for HTTP response. Returns true only after successful parsing.

**Files modified:** `src/services/hl_account_service.cpp`

---

### P1-4: Implement `GET_PRICE` Command Handler
**Issue:** KNOWN_ISSUES #19 (now in COMPLETED_ISSUES.md)
**Status:** COMPLETE (2026-02-06) — but see Issue #25 (open bug: returns wrong asset)

Added `lastBid`/`lastAsk` to `TradingState` struct. `BrokerAsset()` caches prices. `GET_PRICE` handler returns bid/ask/mid based on priceType.

**Files modified:** `src/api/hl_broker_commands.cpp`, `src/api/hl_broker.cpp`, `src/foundation/hl_globals.h`

---

### P1-5: Create Pre-Merge Tag and Test on `dev` Branch
**Status:** PARTIALLY COMPLETE (2026-02-08) — DLL builds, smoke test run

- Tag `pre-merge-to-main` created at commit 5010013
- Fast-forward merged to `dev` (no conflicts)
- All 12 modules compile, unit tests pass
- DLL links successfully (1.49 MB, 9 exports)
- CMake DEV_BUILD option added — DLL output name is `Hyperliquid_Dev.dll`

**Smoke Test Run 1 (2026-02-08 morning):**

| # | Test | Result | Notes |
|---|------|--------|-------|
| 1 | Login | PASS | WS connects, account data received |
| 2 | GET_COMPLIANCE | PASS | 10 |
| 3 | GET_MAXREQUESTS | PASS | 5 |
| 4 | Balance > 0 | PASS | $139.19 (wrong — Issue #23, UI shows $343) |
| 5 | BTC PIP/LotAmount/PIPCost | PASS | Warning 054 auto-corrected (Issue #27) |
| 6 | ETH PIP/LotAmount | PASS | Warning 054 auto-corrected |
| 7 | SOL PIP | PASS | Warning 054 auto-corrected |
| 8 | BTC price > 1000 | PASS | $70,766 |
| 9 | BTC spread reasonable | PASS | $290 |
| 10 | ETH price > 100 | PASS | $2,190 |
| 11 | GET_PRICE(BTC) | **FAIL** | Returned 87.17 (SOL price) — Issue #25 |
| 12 | enterLong(1 lot) | **FAIL** | Returned 0, Zorro blocked — Issue #26 |

**Smoke Test Run 2 (2026-02-08 afternoon):**

13 PASS, 2 FAIL. Phase 3 & 4 now execute fully. Lot size fix attempt 1 used `(int)(expr)` cast which Lite-C evaluated as 0. Fixed to `int x = expr;` assignment.

| # | Test | Result | Notes |
|---|------|--------|-------|
| 1-10 | Phase 1-2 | PASS | Same as Run 1 |
| 11 | GET_PRICE(BTC) | **FAIL** | Returned 87.14 (SOL price) — Issue #25 confirmed |
| 12 | enterLong | **FAIL** | Lots=0 due to Lite-C `(int)` cast bug — fix applied, awaiting Run 3 |

**New issues found:** #25, #26, #27 added to KNOWN_ISSUES.md

---

## Sprint Overview (Original)

| Priority | Items | Estimated Hours |
|----------|-------|-----------------|
| P0 — Blockers | 2 | 6-10 |
| P1 — Should fix before merge | 5 | 4-6 |
| P2 — First sprint after merge | 4 | 12-20 |
| P3 — Backlog | 4 | 8-16 |
| **Total** | **15** | **30-52** |
