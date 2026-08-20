@echo off
REM =============================================================================
REM compile_order_oid_test.bat - Exchange order ID for a trade ID [OPM-1085]
REM =============================================================================
REM TESTS: HL_GET_ORDER_OID (50045) hands a strategy the exchange's own order ID
REM        for one of its trades, to be used as a join key against the fills the
REM        exchange reports. It must report "no ID" (0) rather than a number the
REM        exchange would not recognise: a synthetic ID from the trade map
REM        (PENDING_/RESUMED_/IMPORTED_/DRY_RUN) must not be parsed into one, and
REM        an ID a double cannot carry exactly must not be rounded into one.
REM =============================================================================

call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars32.bat" >nul 2>&1

cd /d "%~dp0"

echo.
echo ===================================================
echo  Compiling test_order_oid.cpp [OPM-1085]
echo  Tests: exchange order ID lookup by trade ID
echo ===================================================
echo.

cl /nologo /EHsc /std:c++17 ^
   /I..\src\foundation ^
   /I. ^
   unit\test_order_oid.cpp ^
   ..\src\foundation\hl_utils.cpp ^
   ..\src\foundation\hl_globals.cpp ^
   /Fe:"%~dp0test_order_oid.exe"

if errorlevel 1 (
    echo.
    echo COMPILATION FAILED!
    exit /b 1
)

echo.
echo Running tests...
echo.
"%~dp0test_order_oid.exe"
set TEST_RESULT=%ERRORLEVEL%

echo.
echo Cleaning up...
del /Q *.obj 2>nul
del /Q test_order_oid.exe 2>nul

if %TEST_RESULT% NEQ 0 (
    echo.
    echo TESTS FAILED!
    exit /b 1
)

echo.
echo All tests passed!
exit /b 0
