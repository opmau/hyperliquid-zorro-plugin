# Testing

Tests in this project are regression tests for bugs that caused real financial losses. Every test carries a commit hash or issue ID showing which bug it prevents.

---

## Running Tests

```batch
cd tests
run_unit_tests.bat
```

This runs all 17 tests in under 10 seconds. Exit code 0 = all pass, 1 = failures.

Run specific tests by calling the individual script:

```batch
cd tests
compile_broker_asset_test.bat       # Just PIP/PIPCost formulas
compile_eip712_source_test.bat      # Just EIP-712 signing
```

---

## Test Suite

| # | Script | What it tests | Bug prevented |
|---|--------|--------------|---------------|
| 1 | `compile_broker_asset_test.bat` | PIP, PIPCost, LotAmount formulas + lot conversion | 6dfb104, 213643c, 8303e8b (profit display in millions) |
| 2 | `compile_position_parsing_test.bat` | Multi-asset JSON position extraction | 81db4b6 (wrong asset's data returned) |
| 3 | `compile_imported_test.bat` | IMPORTED trade position tracking | 18c287c (stale position data) |
| 4 | `compile_eip712_source_test.bat` | EIP-712 source="a" (mainnet) vs "b" (testnet) | OPM-22 (orders rejected on testnet) |
| 5 | `compile_utils_test.bat` | Utility functions (string, time, numeric) | -- |
| 6 | `compile_get_price_context_test.bat` | GET_PRICE uses priceSymbol, not currentSymbol | OPM-6 (wrong asset price) |
| 7 | `compile_trigger_order_test.bat` | Trigger order JSON construction + STOP flag handling | OPM-77 (stop orders silently discarded) |
| 8 | `compile_partial_fill_test.bat` | PartialFill detection + 0.999 threshold | OPM-91 (partial fills not detected) |
| 9 | `compile_lotsize_divzero_test.bat` | Division-by-zero guard when lotSize=0 | OPM-158 (crash on uninitialized state) |
| 10 | `compile_ws_parsers_test.bat` | All 6 WS message parsers (l2Book, allMids, clearinghouse, fills, orders, userEvents) | OPM-10 |
| 11 | `compile_twap_test.bat` | TWAP order msgpack + EIP-712 signing | OPM-81 |
| 12 | `compile_schedule_cancel_test.bat` | scheduleCancel signing (set + clear) | OPM-83 |
| 13 | `compile_batch_modify_test.bat` | batchModify msgpack encoding | OPM-80 |
| 14 | `compile_bracket_order_test.bat` | Bracket order encoding (entry + TP + SL) | OPM-79 |
| 15 | `compile_trading_service_test.bat` | CLOID roundtrip, nonce monotonicity, TradeMap CRUD, fill status | OPM-9 |
| 16 | `compile_account_service_test.bat` | Balance parsing, position parsing, HTTP fallback | OPM-9 |
| 17 | `compile_market_service_test.bat` | Candle interval mapping, asset metadata, funding rate | OPM-9 |

### Test-to-File Mapping

| If you modify... | Run this test |
|-----------------|---------------|
| PIP, PIPCost, LotAmount, szDecimals | `compile_broker_asset_test.bat` |
| Position parsing, JSON extraction | `compile_position_parsing_test.bat` |
| IMPORTED trades, BrokerTrade | `compile_imported_test.bat` |
| EIP-712 signing, `source` field | `compile_eip712_source_test.bat` |
| `hl_utils.h/cpp` | `compile_utils_test.bat` |
| BrokerCommand, GET_PRICE, SET_SYMBOL | `compile_get_price_context_test.bat` |
| Trigger orders, SET_ORDERTYPE +8 | `compile_trigger_order_test.bat` |
| OrderStatus, fill detection | `compile_partial_fill_test.bat` |
| Lot conversion, lotSize usage | `compile_lotsize_divzero_test.bat` |
| WS message parsing, ws_parsers | `compile_ws_parsers_test.bat` |
| TWAP orders, `hl_trading_twap.cpp` | `compile_twap_test.bat` |
| scheduleCancel, dead man's switch | `compile_schedule_cancel_test.bat` |
| batchModify, `hl_trading_modify.cpp` | `compile_batch_modify_test.bat` |
| Bracket orders, normalTpsl grouping | `compile_bracket_order_test.bat` |
| Trading service logic, CLOID, nonce | `compile_trading_service_test.bat` |
| Account service, balance, positions | `compile_account_service_test.bat` |
| Market service, candles, funding rate | `compile_market_service_test.bat` |
| Any broker/trading code | `run_unit_tests.bat` (all tests) |

---

## Test Infrastructure

### test_framework.h

Lightweight assertion macros in `namespace hl::test`. No external dependencies.

**Test registration:**
```cpp
TEST_CASE(my_test_name) {
    // test body
}

int main() {
    RUN_TEST(my_test_name);
    return hl::test::printTestSummary();
}
```

**Available assertions:**

| Macro | Purpose |
|-------|---------|
| `ASSERT_TRUE(cond)` | Boolean check |
| `ASSERT_FALSE(cond)` | Inverse boolean |
| `ASSERT_EQ(actual, expected)` | Equality (`!=` comparison) |
| `ASSERT_NE(actual, expected)` | Inequality |
| `ASSERT_STREQ(actual, expected)` | String equality (`strcmp`) |
| `ASSERT_STRNE(actual, expected)` | String inequality |
| `ASSERT_FLOAT_EQ(actual, expected)` | Float equality (epsilon=1e-9) |
| `ASSERT_FLOAT_EQ_TOL(a, e, tol)` | Float equality with custom tolerance |
| `ASSERT_GT/GE/LT/LE(a, b)` | Comparison operators |
| `ASSERT_NOT_NULL(ptr)` | Pointer non-null |
| `ASSERT_NULL(ptr)` | Pointer null |
| `ASSERT_MSG(cond, msg)` | Custom failure message |

All macros print file:line on failure and return from the test function (non-fatal at test level, fatal at assertion level).

### mock_zorro.h

Provides mock implementations of Zorro SDK function pointers for standalone testing:

```cpp
// In exactly ONE .cpp file:
#define MOCK_ZORRO_IMPLEMENTATION
#include "mocks/mock_zorro.h"

// In other files:
#include "mocks/mock_zorro.h"  // (without MOCK_ZORRO_IMPLEMENTATION)
```

**Configuration:**
```cpp
hl::mock::g_httpMock.responseBody = R"({"status":"ok","response":{"type":"default"}})";
hl::mock::g_httpMock.statusCode = 1;      // 1 = ready
hl::mock::g_httpMock.shouldFail = false;
```

**What it mocks:** `http_request`, `http_status`, `http_result`, `http_free`, `nap` -- the five Zorro function pointers the plugin uses. Tests that don't use HTTP can ignore mock configuration entirely.

---

## Writing a New Test

### 1. Create the test source file

```cpp
// tests/unit/test_my_feature.cpp
//=============================================================================
// test_my_feature.cpp - Description of what this test validates
//=============================================================================
// PREVENTS BUGS: OPM-XXX (brief description)
//=============================================================================

// Include mock Zorro (only needed if test uses HTTP or nap)
#define MOCK_ZORRO_IMPLEMENTATION
#include "../mocks/mock_zorro.h"

// Include test framework
#include "../test_framework.h"

// Include the code under test
#include "../../src/foundation/hl_types.h"
// ... other includes as needed

using namespace hl::test;

//=============================================================================
// Tests
//=============================================================================

TEST_CASE(feature_basic_case) {
    // Setup
    // ...

    // Exercise
    auto result = someFunction();

    // Assert
    ASSERT_EQ(result, expected);
}

TEST_CASE(feature_edge_case) {
    // ...
    ASSERT_TRUE(edgeCondition);
}

//=============================================================================
// Main
//=============================================================================

int main() {
    printf("test_my_feature: Validating [feature name]\n\n");

    RUN_TEST(feature_basic_case);
    RUN_TEST(feature_edge_case);

    return printTestSummary();
}
```

### 2. Create the compile script

```batch
@echo off
REM tests/compile_my_feature_test.bat

call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1

cd /d "%~dp0"

echo.
echo ===================================================
echo  Compiling test_my_feature.cpp
echo  Tests: [brief description]
echo ===================================================
echo.

cl /nologo /EHsc /std:c++14 /I. /I..\src\foundation unit\test_my_feature.cpp /Fe:test_my_feature.exe

if errorlevel 1 (
    echo COMPILATION FAILED!
    exit /b 1
)

echo Running tests...
test_my_feature.exe
set TEST_RESULT=%ERRORLEVEL%

del /Q *.obj 2>nul
del /Q test_my_feature.exe 2>nul

if %TEST_RESULT% NEQ 0 (
    echo TESTS FAILED!
    exit /b 1
)

echo All tests passed!
exit /b 0
```

The `cl` flags:
- `/EHsc` -- C++ exception handling
- `/std:c++14` -- minimum C++ standard (tests don't need C++17)
- `/I. /I..\src\foundation` -- include paths (add more as needed)

### 3. Add to the test suite

Edit `run_unit_tests.bat` to add your test as the next numbered entry:

```batch
echo [10/10] Testing my feature...
call compile_my_feature_test.bat
if !ERRORLEVEL! EQU 0 (
    set /a TESTS_PASSED+=1
    echo       PASSED
) else (
    set /a TESTS_FAILED+=1
    echo       FAILED - My feature tests failed!
)
```

Update the `[N/N]` counter on all test lines.

---

## Critical Formulas

These formulas are protected by regression tests. **Do not change them without updating the corresponding test.**

```
Hyperliquid total precision:
  pxDecimals + szDecimals = 6  (always)

Price decimals:
  pxDecimals = 6 - szDecimals

PIP (minimum price movement):
  PIP = 10^(-pxDecimals)

LotAmount (minimum trade unit):
  LotAmount = 10^(-szDecimals)

PIPCost (value of 1 PIP per 1 lot):
  PIPCost = PIP * LotAmount = 10^(-6) = 0.000001  (constant!)

Lot conversion:
  contracts = lots * LotAmount
  lots = contracts / LotAmount
```

**Example (BTC, szDecimals=5):**
| Value | Calculation | Result |
|-------|-------------|--------|
| pxDecimals | 6 - 5 | 1 |
| PIP | 10^(-1) | 0.1 |
| LotAmount | 10^(-5) | 0.00001 |
| PIPCost | 0.1 * 0.00001 | 0.000001 |
| 2.0 BTC in lots | 2.0 / 0.00001 | 200,000 |

---

## Pre-Commit Hook

Install the hook to automatically run all tests before every commit:

```batch
cd scripts
install_hooks.bat
```

After installation, `git commit` will be blocked if any test fails. The hook calls `run_unit_tests.bat` and checks its exit code.

---

## CMake Test Targets

In addition to the standalone `.bat` tests, CMakeLists.txt defines these test executables for the transport layer:

| Target | What it tests | Requires network? |
|--------|--------------|-------------------|
| `test_ws_price_cache` | PriceCache thread-safety and correctness | No |
| `compile_test_transport` | Transport layer compiles cleanly | No |
| `test_poll_timeout` | WebSocket poll() timeout contract | Yes (testnet) |
| `test_ws_connection_ix` | IXWebSocket connection lifecycle | Yes (testnet) |
| `test_ws_auto_reconnect` | Auto-reconnect after disconnect | Yes (testnet) |
| `test_ixwebsocket_connect` | Raw IXWebSocket API connectivity | Yes (testnet) |
| `test_ws_l2book_integration` | L2 book multi-asset subscription | Yes (testnet) |

Build and run CMake tests:
```batch
cd build_vcpkg
cmake --build . --config Release --target test_ws_price_cache
Release\test_ws_price_cache.exe
```

Network-dependent tests connect to Hyperliquid testnet and are intended for manual verification, not CI.
