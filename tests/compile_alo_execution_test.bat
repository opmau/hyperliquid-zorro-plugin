@echo off
REM =============================================================================
REM compile_alo_execution_test.bat - ALO execution regression tests [OPM-790]
REM =============================================================================
REM TESTS: CR-2, CR-6, CR-7, CR-8, CR-9 — side-aware price rounding +
REM        integer-price exemption, honest close reporting, synthetic-oid
REM        cancel guard, partial-fill preservation, modify TIF vs the
REM        always_place=false rule.
REM Sibling: compile_alo_ordertype_test.bat (CR-1, CR-4, CR-5).
REM =============================================================================

call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars32.bat" >nul 2>&1

cd /d "%~dp0"

echo.
echo ===================================================
echo  Compiling test_alo_execution.cpp [OPM-790]
echo  Tests: ALO execution path - rounding, closes, cancels
echo ===================================================
echo.

cl /nologo /EHsc /std:c++17 ^
   /I..\src\foundation ^
   /I..\src\services ^
   /I..\src\vendor\yyjson ^
   /I. ^
   unit\test_alo_execution.cpp ^
   ..\src\foundation\hl_utils.cpp ^
   ..\src\services\hl_trading_response.cpp ^
   ..\src\vendor\yyjson\yyjson.c ^
   /Fe:"%~dp0test_alo_execution.exe"

if errorlevel 1 (
    echo.
    echo COMPILATION FAILED!
    exit /b 1
)

echo.
echo Running tests...
echo.
"%~dp0test_alo_execution.exe"
set TEST_RESULT=%ERRORLEVEL%

echo.
echo Cleaning up...
del /Q *.obj 2>nul
del /Q test_alo_execution.exe 2>nul

if %TEST_RESULT% NEQ 0 (
    echo.
    echo TESTS FAILED!
    exit /b 1
)

echo.
echo All tests passed!
exit /b 0
