@echo off
REM =============================================================================
REM compile_trade_continuity_test.bat - Trade identity across logins [OPM-1552]
REM =============================================================================
REM TESTS: the UUID trade identifier format (src/foundation/hl_trade_uuid.h) and
REM        TradingState::beginSession, which keeps tracked trades across a
REM        logout/login on the same account. Links the production code
REM        directly, so a regression in either fails this test.
REM =============================================================================

call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars32.bat" >nul 2>&1

cd /d "%~dp0"

echo.
echo ===================================================
echo  Compiling test_trade_continuity.cpp [OPM-1552]
echo  Tests: UUID identifiers and session continuity
echo ===================================================
echo.

cl /nologo /EHsc /std:c++17 ^
   /I..\src\foundation ^
   /I. ^
   unit\test_trade_continuity.cpp ^
   ..\src\foundation\hl_globals.cpp ^
   /Fe:"%~dp0test_trade_continuity.exe"

if errorlevel 1 (
    echo.
    echo COMPILATION FAILED!
    exit /b 1
)

echo.
echo Running tests...
echo.
"%~dp0test_trade_continuity.exe"
set TEST_RESULT=%ERRORLEVEL%

echo.
echo Cleaning up...
del /Q *.obj 2>nul
del /Q test_trade_continuity.exe 2>nul

if %TEST_RESULT% NEQ 0 (
    echo.
    echo TESTS FAILED!
    exit /b 1
)

echo.
echo All tests passed!
exit /b 0
