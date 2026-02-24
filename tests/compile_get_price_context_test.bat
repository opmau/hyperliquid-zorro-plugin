@echo off
REM =============================================================================
REM compile_get_price_context_test.bat - GET_PRICE context isolation [OPM-6]
REM =============================================================================
REM PREVENTS: OPM-6 regression (GET_PRICE returns wrong asset's price)
REM =============================================================================

call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars32.bat" >nul 2>&1

cd /d "%~dp0"

echo.
echo ===================================================
echo  Compiling test_get_price_context.cpp [OPM-6]
echo  Tests: GET_PRICE asset context isolation
echo ===================================================
echo.

cl /nologo /EHsc /std:c++17 ^
   /I..\src\foundation ^
   /I..\src\transport ^
   /I. ^
   unit\test_get_price_context.cpp ^
   ..\src\foundation\hl_globals.cpp ^
   ..\src\transport\ws_price_cache.cpp ^
   /Fe:"%~dp0test_get_price_context.exe"

if errorlevel 1 (
    echo.
    echo COMPILATION FAILED!
    exit /b 1
)

echo.
echo Running tests...
echo.
"%~dp0test_get_price_context.exe"
set TEST_RESULT=%ERRORLEVEL%

echo.
echo Cleaning up...
del /Q *.obj 2>nul
del /Q test_get_price_context.exe 2>nul

if %TEST_RESULT% NEQ 0 (
    echo.
    echo TESTS FAILED!
    exit /b 1
)

echo.
echo All tests passed!
exit /b 0
