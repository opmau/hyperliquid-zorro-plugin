@echo off
REM =============================================================================
REM compile_market_service_http_test.bat - Market service HTTP parsing tests [OPM-174]
REM =============================================================================
REM TESTS: l2Book, candleSnapshot, metaAndAssetCtxs parsing
REM DEPS:  yyjson (JSON parsing), json_helpers.h (header-only)
REM =============================================================================

call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars32.bat" >nul 2>&1

cd /d "%~dp0"

echo.
echo ===================================================
echo  Compiling test_market_service_http.cpp [OPM-174]
echo  Tests: l2Book, candle, funding rate parsing
echo ===================================================
echo.

cl /nologo /EHsc /std:c++17 ^
   /I..\src\transport ^
   /I..\src\vendor\yyjson ^
   /I. ^
   unit\test_market_service_http.cpp ^
   ..\src\vendor\yyjson\yyjson.c ^
   /Fe:"%~dp0test_market_service_http.exe"

if errorlevel 1 (
    echo.
    echo COMPILATION FAILED!
    exit /b 1
)

echo.
echo Running tests...
echo.
"%~dp0test_market_service_http.exe"
set TEST_RESULT=%ERRORLEVEL%

echo.
echo Cleaning up...
del /Q *.obj 2>nul
del /Q test_market_service_http.exe 2>nul

if %TEST_RESULT% NEQ 0 (
    echo.
    echo TESTS FAILED!
    exit /b 1
)

echo.
echo All tests passed!
exit /b 0
