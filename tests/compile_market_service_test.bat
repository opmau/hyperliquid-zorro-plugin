@echo off
REM =============================================================================
REM compile_market_service_test.bat - Market service tests [OPM-9]
REM =============================================================================
REM TESTS: Candle intervals, HTTP seed cooldown
REM =============================================================================

call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars32.bat" >nul 2>&1

cd /d "%~dp0"

echo.
echo ===================================================
echo  Compiling test_market_service.cpp [OPM-9]
echo  Tests: Market service pure logic
echo ===================================================
echo.

cl /nologo /EHsc /std:c++17 ^
   /I..\src\foundation ^
   /I..\src\transport ^
   /I. ^
   unit\test_market_service.cpp ^
   ..\src\foundation\hl_globals.cpp ^
   /Fe:"%~dp0test_market_service.exe"

if errorlevel 1 (
    echo.
    echo COMPILATION FAILED!
    exit /b 1
)

echo.
echo Running tests...
echo.
"%~dp0test_market_service.exe"
set TEST_RESULT=%ERRORLEVEL%

echo.
echo Cleaning up...
del /Q *.obj 2>nul
del /Q test_market_service.exe 2>nul

if %TEST_RESULT% NEQ 0 (
    echo.
    echo TESTS FAILED!
    exit /b 1
)

echo.
echo All tests passed!
exit /b 0
