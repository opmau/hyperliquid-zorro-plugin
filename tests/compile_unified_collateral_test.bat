@echo off
REM =============================================================================
REM compile_unified_collateral_test.bat - Unified collateral tests [OPM-824]
REM =============================================================================
REM TESTS: spotClearinghouseState parsing, abstraction-mode mapping (including
REM        "default"), and the collateral model that decides the equity Zorro
REM        sizes positions from. Links src/services/hl_account_collateral.h
REM        directly, so a change to production maths fails this test.
REM =============================================================================

call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars32.bat" >nul 2>&1

cd /d "%~dp0"

echo.
echo ===================================================
echo  Compiling test_unified_collateral.cpp [OPM-824]
echo  Tests: unified account equity and free collateral
echo ===================================================
echo.

cl /nologo /EHsc /std:c++17 ^
   /I..\src\foundation ^
   /I..\src\services ^
   /I..\src\transport ^
   /I..\src\vendor\yyjson ^
   /I. ^
   unit\test_unified_collateral.cpp ^
   ..\src\vendor\yyjson\yyjson.c ^
   /Fe:"%~dp0test_unified_collateral.exe"

if errorlevel 1 (
    echo.
    echo COMPILATION FAILED!
    exit /b 1
)

echo.
echo Running tests...
echo.
"%~dp0test_unified_collateral.exe"
set TEST_RESULT=%ERRORLEVEL%

echo.
echo Cleaning up...
del /Q *.obj 2>nul
del /Q test_unified_collateral.exe 2>nul

if %TEST_RESULT% NEQ 0 (
    echo.
    echo TESTS FAILED!
    exit /b 1
)

echo.
echo All tests passed!
exit /b 0
