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
echo [1/21] Testing PIP/PIPCost/LotAmount formulas...
call compile_broker_asset_test.bat
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
echo [2/21] Testing multi-asset position parsing...
call compile_position_parsing_test.bat
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
echo [3/21] Testing IMPORTED trade position tracking...
call compile_imported_test.bat
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
echo [4/21] Testing EIP-712 mainnet vs testnet source...
call compile_eip712_source_test.bat
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
echo [5/21] Testing utility functions...
if exist compile_utils_test.bat (
    call compile_utils_test.bat >nul 2>&1
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
echo [6/21] Testing GET_PRICE context isolation...
call compile_get_price_context_test.bat
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
echo [7/21] Testing trigger order construction...
call compile_trigger_order_test.bat
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
echo [8/21] Testing partial fill detection...
call compile_partial_fill_test.bat
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
echo [9/21] Testing lotSize division-by-zero guard...
call compile_lotsize_divzero_test.bat
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
echo [10/21] Testing WebSocket parsers...
call compile_ws_parsers_test.bat
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
echo [11/21] Testing TWAP order construction...
call compile_twap_test.bat
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
echo [12/21] Testing scheduleCancel signing...
call compile_schedule_cancel_test.bat
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
echo [13/21] Testing batchModify encoding...
call compile_batch_modify_test.bat
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
echo [14/21] Testing bracket order encoding...
call compile_bracket_order_test.bat
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
echo [15/21] Testing trading service logic...
call compile_trading_service_test.bat
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
echo [16/21] Testing account service logic...
call compile_account_service_test.bat
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
echo [17/21] Testing market service logic...
call compile_market_service_test.bat
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
echo [18/21] Testing market service HTTP parsing...
call compile_market_service_http_test.bat
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
echo [19/21] Testing account service HTTP parsing...
call compile_account_service_http_test.bat
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
echo [20/21] Testing account service WS cache interactions...
call compile_account_service_ws_test.bat
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
echo [21/21] Testing market service WS cache interactions...
call compile_market_service_ws_test.bat
if !ERRORLEVEL! EQU 0 (
    set /a TESTS_PASSED+=1
    echo       PASSED
) else (
    set /a TESTS_FAILED+=1
    echo       FAILED - Market service WS cache tests failed!
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