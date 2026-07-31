@echo off
REM =============================================================================
REM compile_alo_enablement_test.bat - ALO enablement regression tests [OPM-790]
REM =============================================================================
REM TESTS: CR-1..CR-9 — sticky ALO order type, market-order TIF forcing, TIF
REM        casing canonicalization, exchange error parsing/classification,
REM        side-aware price rounding + integer-price exemption, honest close
REM        reporting, synthetic-oid cancel guard, partial-fill preservation,
REM        modify TIF vs the always_place=false rule.
REM =============================================================================

call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars32.bat" >nul 2>&1

cd /d "%~dp0"

echo.
echo ===================================================
echo  Compiling test_alo_enablement.cpp [OPM-790]
echo  Tests: ALO / post-only maker execution enablement
echo ===================================================
echo.

cl /nologo /EHsc /std:c++17 ^
   /I..\src\foundation ^
   /I..\src\services ^
   /I..\src\vendor\yyjson ^
   /I. ^
   unit\test_alo_enablement.cpp ^
   ..\src\foundation\hl_utils.cpp ^
   ..\src\services\hl_trading_response.cpp ^
   ..\src\vendor\yyjson\yyjson.c ^
   /Fe:"%~dp0test_alo_enablement.exe"

if errorlevel 1 (
    echo.
    echo COMPILATION FAILED!
    exit /b 1
)

echo.
echo Running tests...
echo.
"%~dp0test_alo_enablement.exe"
set TEST_RESULT=%ERRORLEVEL%

echo.
echo Cleaning up...
del /Q *.obj 2>nul
del /Q test_alo_enablement.exe 2>nul

if %TEST_RESULT% NEQ 0 (
    echo.
    echo TESTS FAILED!
    exit /b 1
)

echo.
echo All tests passed!
exit /b 0
