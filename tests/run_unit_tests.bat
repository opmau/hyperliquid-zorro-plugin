@echo off
REM =============================================================================
REM run_unit_tests.bat - Run all fast unit tests
REM =============================================================================
REM This script is called by pre-commit hooks and can be run manually.
REM Tests are designed to be fast (<5 seconds total).
REM
REM Exit codes:
REM   0 = All tests passed
REM   1 = One or more tests failed
REM
REM [OPM-735] Each module runs via "cmd /c .\..." (NOT "call"): every
REM compile_*.bat re-runs vcvars, and chained calls grow this process's
REM environment until cmd aborts the batch SILENTLY with exit 0 (observed
REM after 3 modules). A child cmd per module discards the vcvars growth.
REM The ".\" prefix keeps resolution working when the CWD is excluded from
REM the executable search path (NoDefaultCurrentDirectoryInExePath).
REM =============================================================================

setlocal EnableDelayedExpansion

cd /d "%~dp0"

echo.
echo =====================================================
echo   HYPERLIQUID PLUGIN UNIT TEST SUITE
echo   Running fast tests before commit...
echo =====================================================
echo.

set TESTS_PASSED=0
set TESTS_FAILED=0

REM =============================================================================
REM Test 1: PIP/PIPCost/LotAmount Formulas
REM Prevents bugs: 6dfb104, 213643c, 8303e8b
REM =============================================================================
echo [1/28] Testing PIP/PIPCost/LotAmount formulas...
cmd /c .\compile_broker_asset_test.bat
if !ERRORLEVEL! EQU 0 (
    set /a TESTS_PASSED+=1
    echo       PASSED
) else (
    set /a TESTS_FAILED+=1
    echo       FAILED - PIP/PIPCost calculations incorrect!
)
echo.

REM =============================================================================
REM Test 2: Multi-Asset Position Parsing
REM Prevents bug: 81db4b6
REM =============================================================================
echo [2/28] Testing multi-asset position parsing...
cmd /c .\compile_position_parsing_test.bat
if !ERRORLEVEL! EQU 0 (
    set /a TESTS_PASSED+=1
    echo       PASSED
) else (
    set /a TESTS_FAILED+=1
    echo       FAILED - Position parsing returns wrong asset data!
)
echo.

REM =============================================================================
REM Test 3: IMPORTED Trade Position Tracking
REM Prevents bug: 18c287c
REM =============================================================================
echo [3/28] Testing IMPORTED trade position tracking...
cmd /c .\compile_imported_test.bat
if !ERRORLEVEL! EQU 0 (
    set /a TESTS_PASSED+=1
    echo       PASSED
) else (
    set /a TESTS_FAILED+=1
    echo       FAILED - IMPORTED trades return stale data!
)
echo.

REM =============================================================================
REM Test 4: EIP-712 Mainnet vs Testnet Source
REM Prevents bug: OPM-22 (e392a43)
REM =============================================================================
echo [4/28] Testing EIP-712 mainnet vs testnet source...
cmd /c .\compile_eip712_source_test.bat
if !ERRORLEVEL! EQU 0 (
    set /a TESTS_PASSED+=1
    echo       PASSED
) else (
    set /a TESTS_FAILED+=1
    echo       FAILED - EIP-712 testnet source incorrect!
)
echo.

REM =============================================================================
REM Test 5: Existing utils tests (if they exist)
REM =============================================================================
echo [5/28] Testing utility functions...
if exist compile_utils_test.bat (
    cmd /c .\compile_utils_test.bat >nul 2>&1
    if !ERRORLEVEL! EQU 0 (
        set /a TESTS_PASSED+=1
        echo       PASSED
    ) else (
        set /a TESTS_FAILED+=1
        echo       FAILED
    )
) else (
    echo       SKIPPED - compile_utils_test.bat not found
)
echo.

REM =============================================================================
REM Test 6: GET_PRICE Context Isolation [OPM-6]
REM Prevents bug: OPM-6 (GET_PRICE returns wrong asset's price)
REM =============================================================================
echo [6/28] Testing GET_PRICE context isolation...
cmd /c .\compile_get_price_context_test.bat
if !ERRORLEVEL! EQU 0 (
    set /a TESTS_PASSED+=1
    echo       PASSED
) else (
    set /a TESTS_FAILED+=1
    echo       FAILED - GET_PRICE returns wrong asset price!
)
echo.

REM =============================================================================
REM Test 7: Trigger Order Construction [OPM-77]
REM Prevents bug: Silent STOP flag discard, incorrect trigger JSON
REM =============================================================================
echo [7/28] Testing trigger order construction...
cmd /c .\compile_trigger_order_test.bat
if !ERRORLEVEL! EQU 0 (
    set /a TESTS_PASSED+=1
    echo       PASSED
) else (
    set /a TESTS_FAILED+=1
    echo       FAILED - Trigger order tests failed!
)
echo.

REM =============================================================================
REM Test 8: Partial Fill Detection [OPM-91]
REM Prevents bug: Missing PartialFill status, HTTP fallback guard
REM =============================================================================
echo [8/28] Testing partial fill detection...
cmd /c .\compile_partial_fill_test.bat
if !ERRORLEVEL! EQU 0 (
    set /a TESTS_PASSED+=1
    echo       PASSED
) else (
    set /a TESTS_FAILED+=1
    echo       FAILED - Partial fill detection tests failed!
)
echo.

REM =============================================================================
REM Test 9: lotSize Division-by-Zero Guard [OPM-158]
REM Prevents bug: Division by zero when lotSize is 0 (uninitialized state)
REM =============================================================================
echo [9/28] Testing lotSize division-by-zero guard...
cmd /c .\compile_lotsize_divzero_test.bat
if !ERRORLEVEL! EQU 0 (
    set /a TESTS_PASSED+=1
    echo       PASSED
) else (
    set /a TESTS_FAILED+=1
    echo       FAILED - lotSize division-by-zero guard tests failed!
)
echo.

REM =============================================================================
REM Test 10: WebSocket Parser Unit Tests [OPM-10]
REM Tests all 6 ws_parsers.cpp functions with canned JSON fixtures
REM =============================================================================
echo [10/28] Testing WebSocket parsers...
cmd /c .\compile_ws_parsers_test.bat
if !ERRORLEVEL! EQU 0 (
    set /a TESTS_PASSED+=1
    echo       PASSED
) else (
    set /a TESTS_FAILED+=1
    echo       FAILED - WebSocket parser tests failed!
)
echo.

REM =============================================================================
REM Test 11: TWAP Order Construction [OPM-81]
REM Prevents: Incorrect msgpack field ordering, wrong TWAP action types
REM =============================================================================
echo [11/28] Testing TWAP order construction...
cmd /c .\compile_twap_test.bat
if !ERRORLEVEL! EQU 0 (
    set /a TESTS_PASSED+=1
    echo       PASSED
) else (
    set /a TESTS_FAILED+=1
    echo       FAILED - TWAP order tests failed!
)
echo.

REM =============================================================================
REM Test 12: scheduleCancel (Dead Man's Switch) [OPM-83]
REM Prevents: Incorrect msgpack encoding, signature mismatch
REM =============================================================================
echo [12/28] Testing scheduleCancel signing...
cmd /c .\compile_schedule_cancel_test.bat
if !ERRORLEVEL! EQU 0 (
    set /a TESTS_PASSED+=1
    echo       PASSED
) else (
    set /a TESTS_FAILED+=1
    echo       FAILED - scheduleCancel signing tests failed!
)
echo.

REM =============================================================================
REM Test 13: batchModify (Atomic Order Modify) [OPM-80]
REM Prevents: Incorrect msgpack encoding, wrong oid type, field ordering
REM =============================================================================
echo [13/28] Testing batchModify encoding...
cmd /c .\compile_batch_modify_test.bat
if !ERRORLEVEL! EQU 0 (
    set /a TESTS_PASSED+=1
    echo       PASSED
) else (
    set /a TESTS_FAILED+=1
    echo       FAILED - batchModify encoding tests failed!
)
echo.

REM =============================================================================
REM Test 14: Bracket Order Encoding [OPM-79]
REM Prevents: Wrong grouping, missing orders, incorrect trigger fields
REM =============================================================================
echo [14/28] Testing bracket order encoding...
cmd /c .\compile_bracket_order_test.bat
if !ERRORLEVEL! EQU 0 (
    set /a TESTS_PASSED+=1
    echo       PASSED
) else (
    set /a TESTS_FAILED+=1
    echo       FAILED - Bracket order encoding tests failed!
)
echo.

REM =============================================================================
REM Test 15: Trading Service [OPM-9]
REM Tests: CLOID gen/parse, trade ID, nonce, order storage, fill status
REM =============================================================================
echo [15/28] Testing trading service logic...
cmd /c .\compile_trading_service_test.bat
if !ERRORLEVEL! EQU 0 (
    set /a TESTS_PASSED+=1
    echo       PASSED
) else (
    set /a TESTS_FAILED+=1
    echo       FAILED - Trading service tests failed!
)
echo.

REM =============================================================================
REM Test 16: Account Service [OPM-9]
REM Tests: PositionInfo, Balance, applyFill, Zorro account values
REM =============================================================================
echo [16/28] Testing account service logic...
cmd /c .\compile_account_service_test.bat
if !ERRORLEVEL! EQU 0 (
    set /a TESTS_PASSED+=1
    echo       PASSED
) else (
    set /a TESTS_FAILED+=1
    echo       FAILED - Account service tests failed!
)
echo.

REM =============================================================================
REM Test 17: Market Service [OPM-9]
REM Tests: Candle intervals, HTTP seed cooldown
REM =============================================================================
echo [17/28] Testing market service logic...
cmd /c .\compile_market_service_test.bat
if !ERRORLEVEL! EQU 0 (
    set /a TESTS_PASSED+=1
    echo       PASSED
) else (
    set /a TESTS_FAILED+=1
    echo       FAILED - Market service tests failed!
)
echo.

REM =============================================================================
REM Test 18: Market Service HTTP Parsing [OPM-174]
REM Tests: l2Book, candleSnapshot, metaAndAssetCtxs parsing
REM =============================================================================
echo [18/28] Testing market service HTTP parsing...
cmd /c .\compile_market_service_http_test.bat
if !ERRORLEVEL! EQU 0 (
    set /a TESTS_PASSED+=1
    echo       PASSED
) else (
    set /a TESTS_FAILED+=1
    echo       FAILED - Market service HTTP parsing tests failed!
)
echo.

REM =============================================================================
REM Test 19: Account Service HTTP Parsing [OPM-174]
REM Tests: spotBalance, userRole, orderStatus parsing
REM =============================================================================
echo [19/28] Testing account service HTTP parsing...
cmd /c .\compile_account_service_http_test.bat
if !ERRORLEVEL! EQU 0 (
    set /a TESTS_PASSED+=1
    echo       PASSED
) else (
    set /a TESTS_FAILED+=1
    echo       FAILED - Account service HTTP parsing tests failed!
)
echo.

REM =============================================================================
REM Test 20: Account Service WS Cache Tests [OPM-175]
REM Tests: getBalance, hasRealtimeBalance, getPosition with PriceCache
REM =============================================================================
echo [20/28] Testing account service WS cache interactions...
cmd /c .\compile_account_service_ws_test.bat
if !ERRORLEVEL! EQU 0 (
    set /a TESTS_PASSED+=1
    echo       PASSED
) else (
    set /a TESTS_FAILED+=1
    echo       FAILED - Account service WS cache tests failed!
)
echo.

REM =============================================================================
REM Test 21: Market Service WS Cache Tests [OPM-175]
REM Tests: getPrice WS reads, stale-data fallback, HTTP seed cooldown
REM =============================================================================
echo [21/28] Testing market service WS cache interactions...
cmd /c .\compile_market_service_ws_test.bat
if !ERRORLEVEL! EQU 0 (
    set /a TESTS_PASSED+=1
    echo       PASSED
) else (
    set /a TESTS_FAILED+=1
    echo       FAILED - Market service WS cache tests failed!
)
echo.

REM =============================================================================
REM Test 22: ALO order type (CR-1, CR-4, CR-5) [OPM-790]
REM Prevents: ALO clobbered by auto SET_ORDERTYPE, market orders inheriting Alo,
REM           TIF casing signature mismatch, silent exchange rejects.
REM =============================================================================
echo [22/28] Testing ALO order type (CR-1, CR-4, CR-5)...
cmd /c .\compile_alo_ordertype_test.bat
if !ERRORLEVEL! EQU 0 (
    set /a TESTS_PASSED+=1
    echo       PASSED
) else (
    set /a TESTS_FAILED+=1
    echo       FAILED - ALO order-type regressions detected!
)
echo.

REM =============================================================================
REM Test 23: ALO execution (CR-2, CR-6..CR-9) [OPM-790]
REM Prevents: symmetric price rounding across the spread, phantom full closes
REM           on resting close orders, cancels targeting oid 0, partial fills
REM           discarded, IOC modifies the exchange is certain to reject.
REM =============================================================================
echo [23/28] Testing ALO execution (CR-2, CR-6..CR-9)...
cmd /c .\compile_alo_execution_test.bat
if !ERRORLEVEL! EQU 0 (
    set /a TESTS_PASSED+=1
    echo       PASSED
) else (
    set /a TESTS_FAILED+=1
    echo       FAILED - ALO execution regressions detected!
)
echo.

REM =============================================================================
REM Test 24: HL_EXPORT_ACCOUNT NFA column [OPM-801]
REM Prevents: the exported Accounts.csv template imposing a compliance setting
REM           on the user - the NFA column belongs to Accounts.csv / set(NFA).
REM =============================================================================
echo [24/28] Testing HL_EXPORT_ACCOUNT NFA column...
cmd /c .\compile_export_account_nfa_test.bat
if !ERRORLEVEL! EQU 0 (
    set /a TESTS_PASSED+=1
    echo       PASSED
) else (
    set /a TESTS_FAILED+=1
    echo       FAILED - exported NFA column is not neutral!
)
echo.

REM =============================================================================
REM Test 25: Unified account collateral [OPM-824]
REM Prevents: reporting perps-only equity on a unified account - under-reports
REM           usable collateral ~37%% on mainnet and reads 0 on testnet, which
REM           trips the zero-balance guard and halts the strategy.
REM =============================================================================
echo [25/28] Testing unified account collateral...
cmd /c .\compile_unified_collateral_test.bat
if !ERRORLEVEL! EQU 0 (
    set /a TESTS_PASSED+=1
    echo       PASSED
) else (
    set /a TESTS_FAILED+=1
    echo       FAILED - unified account equity is wrong!
)
echo.

REM =============================================================================
REM Test 26: HTTP transport [OPM-823, OPM-877]
REM Tests: null-body retry contract, request timeout bounds
REM =============================================================================
echo [26/28] Testing HTTP transport...
cmd /c .\compile_http_test.bat
if !ERRORLEVEL! EQU 0 (
    set /a TESTS_PASSED+=1
    echo       PASSED
) else (
    set /a TESTS_FAILED+=1
    echo       FAILED - HTTP transport tests failed!
)
echo.

REM =============================================================================
REM Test 27: Exchange order ID for a trade ID [OPM-1085]
REM Prevents: handing a strategy an order ID the exchange would not recognise -
REM           a number parsed out of a synthetic trade-map ID, or one rounded to
REM           fit a double. Either matches the wrong fill, or none, silently.
REM =============================================================================
echo [27/28] Testing exchange order ID lookup...
cmd /c .\compile_order_oid_test.bat
if !ERRORLEVEL! EQU 0 (
    set /a TESTS_PASSED+=1
    echo       PASSED
) else (
    set /a TESTS_FAILED+=1
    echo       FAILED - exchange order ID lookup is wrong!
)
echo.

REM =============================================================================
REM Test 28: Price source visibility [OPM-1113]
REM Prevents: a session answering every price lookup over the HTTP fallback
REM           logging exactly like one running off the live feed - the
REM           degradation that most affects timing leaving no trace.
REM =============================================================================
echo [28/28] Testing price source visibility...
cmd /c .\compile_price_source_stats_test.bat
if !ERRORLEVEL! EQU 0 (
    set /a TESTS_PASSED+=1
    echo       PASSED
) else (
    set /a TESTS_FAILED+=1
    echo       FAILED - price source reporting is wrong!
)
echo.

REM =============================================================================
REM SUMMARY
REM =============================================================================
echo =====================================================
if !TESTS_FAILED! EQU 0 (
    echo   ALL TESTS PASSED ^(!TESTS_PASSED! tests^)
    echo =====================================================
    exit /b 0
) else (
    echo   TEST FAILURES: !TESTS_PASSED! passed, !TESTS_FAILED! failed
    echo =====================================================
    echo.
    echo   COMMIT BLOCKED - Fix failing tests before committing!
    echo.
    exit /b 1
)