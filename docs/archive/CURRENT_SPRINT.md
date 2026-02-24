# Current Sprint: Pre-Production Merge

**Goal:** Get `refactor/modularize` branch production-ready and merged to `main`.

**Started:** 2026-02-06
**Branch:** `refactor/modularize`

For completed P0/P1 items and smoke test results, see [PREVIOUS_SPRINT.md](PREVIOUS_SPRINT.md).

---

## Remaining Work

### P1-6: Fix Account Value for Unified Accounts

**Issue:** Account value wrong / unified account not supported (tracked in Linear)
**Status:** NOT STARTED
**Estimate:** 2-4 hours

Plugin reads `marginSummary.accountValue` (~$139) instead of the unified account total (~$343).

**Steps:**
1. [ ] Check Hyperliquid API docs for `crossMarginSummary` vs `marginSummary` response structure
2. [ ] Capture a raw `clearinghouseState` WS message from testnet (Diag=3)
3. [ ] Update `ws_parsers.cpp:parseClearinghouseState()` to prefer `crossMarginSummary`
4. [ ] Update HTTP fallback parser in `hl_account_service.cpp` to match
5. [ ] Verify account value matches Hyperliquid UI
6. [ ] Run `compile_broker.bat` and `run_unit_tests.bat`

**Files in scope:**
- `src/transport/ws_parsers.cpp` (primary)
- `src/services/hl_account_service.cpp` (HTTP fallback)

---

## P2 — First Sprint After Merge

Address after the refactored code is on `main` and stable.

### P2-1: Add Service Layer Tests
**Estimate:** 8-12 hours

The Services and API layers have **0% test coverage**. Tests to create: `test_eip712.cpp`, `test_trading_service.cpp`, `test_account_service.cpp`, `test_market_service.cpp`.

---

### P2-2: Add WebSocket Parser Tests
**Estimate:** 4-6 hours

`ws_parsers.cpp` has 3 complex JSON parsers with zero test coverage.

---

### P2-3: Fix Foundation->Transport Layer Violation
**Issue:** Foundation layer depends on transport types in hl_globals.h (tracked in Linear)
**Estimate:** 1-2 hours

Move `g_wsManager` and `g_priceCache` out of `hl_globals.h`.

---

### P2-4: Add Spot/Cash Trading Support
**Issue:** No support for spot/cash trading, perps only (tracked in Linear)
**Estimate:** 8-16 hours

---

### P2-5: Startup Performance — allMids Optimization
**Estimate:** 4-6 hours

Replace per-asset l2Book HTTP seeding with a single `allMids` request.

---

## P3 — Backlog

### P3-1: Split API Layer Files
**Issue:** API layer files exceed size limits (tracked in Linear)

### P3-2: Implement Export Commands (50001-50003)
**Issue:** Export commands 50001-50003 defined but not implemented (tracked in Linear)

### P3-3: Fix `s_seedCs` Resource Leak
**Issue:** s_seedCs CRITICAL_SECTION never deleted (tracked in Linear)

### P3-4: Expand Mock Infrastructure for WebSocket Testing

---

## Merge Checklist

Before merging `refactor/modularize` -> `dev` -> `main`:

- [x] **P0-1** EIP-712 signing wired into `placeOrder()`/`cancelOrder()`
- [x] **P0-2** Race condition on `indexToCoin_` fixed
- [x] **P1-1** NULL checks on `CreateEvent`/`CreateThread`
- [x] **P1-2** `build/` added to `.gitignore`
- [x] **P1-3** `refreshBalance()` parses HTTP response
- [~] **P1-4** `GET_PRICE` returns cached prices — **BUG: returns wrong asset** (Issue #25)
- [x] `run_unit_tests.bat` — all tests pass
- [x] `compile_broker.bat` — DLL builds without errors
- [~] **P1-5** Tag created, merged to `dev`, compiled — smoke test run 2026-02-08
- [x] **P1-6** Account value correct for unified accounts (Issue #23) — uses max(marginSummary, crossMarginSummary)
- [~] Manual smoke test Run 2 (2026-02-08): 13 PASS, 2 FAIL — lot size Lite-C cast fixed
- [~] Manual smoke test Run 3 (2026-02-08): 13 PASS, 2 FAIL — Error 013 crash fixed (BarPeriod+diagnostics), printf format crash fixed. Remaining FAILs: GET_PRICE wrong asset (Issue #25), enterLong returns 0 (Issue #26)
- [x] **P1-7** WS price data fix — moved subscription sends to connection thread (Issue #29)
- [ ] Manual smoke test Run 4 — verify WS l2Book data flows, HTTP fallback stops
- [ ] User explicitly approves merge to `main`

---

## Test Coverage Targets

| Layer | Current | Target (P2) | Target (long-term) |
|-------|---------|-------------|---------------------|
| Foundation | 71% | 85% | 95% |
| Transport | 40% | 60% | 80% |
| Services | 0% | 50% | 80% |
| API | 0% | 25% | 60% |
| **Overall** | **39%** | **55%** | **80%** |

---

## Session Pattern for Sprint Work

```
1. Pick ONE item from this sprint doc
2. State: "Working on P[X]-[N]: [title]"
3. Read the relevant files (max 2-3)
4. Implement the fix
5. Run: compile_broker.bat
6. Run: cd tests && run_unit_tests.bat
7. git commit with descriptive message
8. Update this doc: check the box, note any complications
9. /clear
```

**Rule:** One item per session. If a fix touches more than 5 files, STOP and re-evaluate scope.
