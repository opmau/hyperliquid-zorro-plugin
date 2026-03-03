@echo off
REM =============================================================================
REM compile_market_service_ws_test.bat - Market service WS cache tests [OPM-175]
REM =============================================================================
REM TESTS: hasRealtimePrice, getPrice WS cache read, stale fallback logic
REM =============================================================================

call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars32.bat" >nul 2>&1

cd /d "%~dp0"

echo.
echo ===================================================
echo  Compiling test_market_service_ws.cpp [OPM-175]
echo  Tests: Market service WS cache interaction
echo ===================================================
echo.

cl /nologo /EHsc /std:c++17 ^
   /I..\src\foundation ^
   /I..\src\transport ^
   /I. ^
   unit\test_market_service_ws.cpp ^
   ..\src\foundation\hl_globals.cpp ^
   ..\src\transport\ws_price_cache.cpp ^
   /Fe:"%~dp0test_market_service_ws.exe"

if errorlevel 1 (
    echo.
    echo COMPILATION FAILED!
    exit /b 1
)

echo.
echo Running tests...
echo.
"%~dp0test_market_service_ws.exe"
set TEST_RESULT=%ERRORLEVEL%

echo.
echo Cleaning up...
del /Q *.obj 2>nul
del /Q test_market_service_ws.exe 2>nul

if %TEST_RESULT% NEQ 0 (
    echo.
    echo TESTS FAILED!
    exit /b 1
)

echo.
echo All tests passed!
exit /b 0
