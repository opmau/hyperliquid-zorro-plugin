# Testing Framework Implementation Checklist

**Purpose:** Track implementation progress of the Hyperliquid Plugin testing framework.
**Status:** PHASE 1 COMPLETE
**Started:** 2026-02-06
**Last Updated:** 2026-02-06

---

## Quick Status Overview

| Phase | Description | Status | Files Created |
|-------|-------------|--------|---------------|
| 0 | Infrastructure Setup | [x] DONE | 5/5 |
| 1 | Regression Tests (Real Bugs) | [x] DONE | 4/4 |
| 2 | EIP-712 Signature Tests | [ ] Pending | 0/3 |
| 3 | State Machine Tests | [ ] Pending | 0/2 |
| 4 | Chaos/Resilience Tests | [ ] Pending | 0/2 |
| 5 | Pre-Commit Hooks | [x] DONE | 3/3 |
| 6 | Integration Tests (Optional) | [ ] Pending | 0/2 |

---

## Phase 0: Infrastructure Setup - COMPLETE

**Goal:** Create directory structure, mock files, and test utilities.

### Checklist

- [x] **0.1** Create directory structure
  ```
  tests/
  ├── vectors/           # Test data (created)
  ├── unit/              # Fast unit tests (created)
  ├── chaos/             # Fault injection (created)
  ├── mocks/             # Test doubles (created)
  └── integration/       # Testnet tests (created)
  ```

- [x] **0.2** Create `tests/mocks/mock_zorro.h`
  - Mock Zorro function pointers (http_request, http_status, etc.)
  - Allows tests to run without Zorro DLL

- [ ] **0.3** Create `tests/mocks/mock_transport.h` (deferred)
  - MockHttp class for injecting HTTP responses
  - MockWebSocket class for simulating WS events

- [x] **0.4** Create `tests/test_framework.h`
  - ASSERT_EQ, ASSERT_FLOAT_EQ, ASSERT_STREQ macros
  - RUN_TEST macro and printTestSummary()
  - Lightweight, no external dependencies

- [x] **0.5** Pre-commit infrastructure
  - `tests/run_unit_tests.bat` - Main test runner
  - `scripts/pre-commit` - Git hook script
  - `scripts/install_hooks.bat` - Hook installer

---

## Phase 1: Regression Tests (Real Bugs from Git) - COMPLETE

**Goal:** Prevent the 11 bugs found in git history from recurring.

### Position/Sizing Bugs - COMPLETE

- [x] **1.1** `tests/unit/test_position_parsing.cpp`
  - Prevents bug `81db4b6` (JSON found wrong asset's data)
  - Tests: Multi-asset JSON, correct asset isolation
  - Batch: `compile_position_parsing_test.bat`

- [x] **1.2** `tests/unit/test_broker_asset.cpp`
  - Prevents bugs `6dfb104` + `213643c` (PIP/PIPCost errors)
  - Tests: PIP = 10^(-pxDecimals), PIPCost = PIP * LotAmount
  - Tests: Multiple assets (BTC, ETH, DOGE, SOL)
  - Batch: `compile_broker_asset_test.bat`

- [x] **1.3** Lot sizing tests (included in test_broker_asset.cpp)
  - Prevents bug `8303e8b` (2 lots instead of 20,000)
  - Tests: contracts = lots * LotAmount
  - Tests: lotsToContracts and contractsToLots conversions

### Order Tracking Bugs - COMPLETE

- [x] **1.4** `tests/unit/test_imported_trades.cpp`
  - Prevents bug `18c287c` (IMPORTED returned stale data)
  - Tests: IMPORTED trade queries CURRENT position
  - Tests: Position reversal (long->short) returns 0
  - Batch: `compile_imported_test.bat`

---

## Phase 2: EIP-712 Signature Tests - PENDING

**Goal:** Prove C++ signatures match Python SDK exactly.

- [ ] **2.1** `tests/vectors/generate_vectors.py`
  - Python script to generate test vectors from SDK

- [ ] **2.2** `tests/vectors/order_signing.json`
  - Test vectors with expected hashes

- [ ] **2.3** `tests/unit/test_signature_vectors.cpp`
  - Cross-validate with Python SDK

---

## Phase 3: State Machine Tests - PENDING

**Goal:** Prove order lifecycle tracking is correct.

- [ ] **3.1** `tests/unit/test_order_state.cpp`
  - Order state transitions

- [ ] **3.2** `tests/unit/test_position_tracking.cpp`
  - Position updates on fill

---

## Phase 4: Chaos/Resilience Tests - PENDING

**Goal:** Prove graceful degradation under failure.

- [ ] **4.1** `tests/chaos/test_ws_failures.cpp`
  - WS disconnect, reconnect, stale data

- [ ] **4.2** `tests/chaos/test_zorro_boundary.cpp`
  - NULL params, zero amounts, edge cases

---

## Phase 5: Pre-Commit Hook Setup - COMPLETE

**Goal:** Automatic test execution before every commit.

- [x] **5.1** `tests/run_unit_tests.bat`
  - Runs: PIP/PIPCost, Position Parsing, Imported Trades, Utils
  - Returns non-zero on failure

- [x] **5.2** `scripts/pre-commit`
  - Git hook script (runs from Git Bash)

- [x] **5.3** `scripts/install_hooks.bat`
  - Copies hook to .git/hooks/

**To Install:**
```batch
cd scripts
install_hooks.bat
```

---

## Phase 6: Integration Tests (Optional) - PENDING

**Goal:** Verify round-trip with real Hyperliquid testnet.

- [ ] **6.1** `tests/integration/test_testnet_roundtrip.cpp`

- [ ] **6.2** `Strategy/ZorroTestnet.c`

---

## Zorro Formula Reference (CRITICAL)

These formulas are encoded in tests to prevent Claude from getting them wrong:

### PIP Calculation
```
szDecimals = from Hyperliquid meta (e.g., BTC=4, DOGE=0)
pxDecimals = 6 - szDecimals
PIP = 10^(-pxDecimals)

Examples:
  BTC:  szDecimals=4, pxDecimals=2, PIP=0.01
  DOGE: szDecimals=0, pxDecimals=6, PIP=0.000001
```

### LotAmount Calculation
```
LotAmount = 10^(-szDecimals)

Examples:
  BTC:  szDecimals=4, LotAmount=0.0001
  DOGE: szDecimals=0, LotAmount=1.0
```

### PIPCost Calculation (CONSTANT FOR ALL ASSETS!)
```
PIPCost = PIP * LotAmount = 10^(-6) = 0.000001

Why constant? Because pxDecimals + szDecimals = 6 always on Hyperliquid.
```

### Lot Conversion
```
contracts = lots * LotAmount
lots = contracts / LotAmount

Example:
  Want 2.0 BTC contracts with LotAmount=0.0001
  lots = 2.0 / 0.0001 = 20,000 lots
```

---

## Files Created Summary

```
tests/
├── test_framework.h                    # [x] Assertion macros
├── run_unit_tests.bat                  # [x] Main test runner
├── compile_broker_asset_test.bat       # [x] PIP/PIPCost tests
├── compile_position_parsing_test.bat   # [x] Multi-asset JSON tests
├── compile_imported_test.bat           # [x] IMPORTED trade tests
├── mocks/
│   └── mock_zorro.h                    # [x] Zorro function mocks
├── unit/
│   ├── test_broker_asset.cpp           # [x] PIP/PIPCost/LotAmount
│   ├── test_position_parsing.cpp       # [x] Multi-asset parsing
│   └── test_imported_trades.cpp        # [x] IMPORTED trades
├── vectors/                            # (empty, for Phase 2)
├── chaos/                              # (empty, for Phase 4)
└── integration/                        # (empty, for Phase 6)

scripts/
├── pre-commit                          # [x] Git hook
└── install_hooks.bat                   # [x] Hook installer
```

---

## How to Run Tests Manually

```batch
cd tests

REM Run all tests
run_unit_tests.bat

REM Run individual test suites
compile_broker_asset_test.bat      REM PIP/PIPCost/Lot formulas
compile_position_parsing_test.bat  REM Multi-asset JSON
compile_imported_test.bat          REM IMPORTED trade tracking
```

---

## When Tests Fail: Document, Don't Fix

**CRITICAL: Do NOT fix bugs on the fly during testing.**

### Protocol

1. **STOP** - Do not attempt an immediate fix
2. **DOCUMENT** - Add issue to `docs/KNOWN_ISSUES.md`:

```markdown
## [Test name] - [Brief description]
- **Test:** `compile_broker_asset_test.bat`
- **Error:** [Exact error message from test output]
- **Expected:** [What the test expected]
- **Actual:** [What the code returned]
- **Likely cause:** [Your analysis]
- **Files to check:** [Relevant source files]
```

3. **CREATE TODO** - Track with clear description:

```
- [ ] Fix: test_broker_asset - PIPCost returns 1.0 instead of 0.000001
- [ ] Fix: test_position_parsing - ETH position returned for BTC query
```

4. **CONTINUE** - Proceed with other work
5. **FIX LATER** - Dedicated session for each bug

### Why This Matters

- On-the-fly fixes often introduce NEW bugs
- Documenting first ensures the issue is fully understood
- Clear todos prevent issues from being forgotten
- Dedicated fix sessions allow proper testing of the fix

### Issue Template (Copy/Paste)

```markdown
## [TEST] - [SYMPTOM]
- **Test:**
- **Error:**
- **Expected:**
- **Actual:**
- **Likely cause:**
- **Files to check:**
```

---

## Next Session Resume Point

**Current Phase:** Phases 0, 1, 5 COMPLETE
**Next Task:** Phase 2 - EIP-712 signature validation with Python SDK test vectors
**Priority:** Optional - main regression tests already cover financial bugs