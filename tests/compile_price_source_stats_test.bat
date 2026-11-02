@echo off
REM =============================================================================
REM compile_price_source_stats_test.bat - Price source visibility [OPM-1113]
REM =============================================================================
REM TESTS: the tally that records whether a price lookup was answered from the
REM        live feed, a stale cached quote or an HTTP order-book request, and
REM        the one-line-per-interval summary it emits. Links
REM        src/services/hl_price_source_stats.h directly, so a change to the
REM        production counters or the log format fails this test.
REM =============================================================================

call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars32.bat" >nul 2>&1

cd /d "%~dp0"

echo.
echo ===================================================
echo  Compiling test_price_source_stats.cpp [OPM-1113]
echo  Tests: price source tally and interval reporting
echo ===================================================
echo.

cl /nologo /EHsc /std:c++17 ^
   /I..\src\foundation ^
   /I..\src\services ^
   /I..\src\transport ^
   /I. ^
   unit\test_price_source_stats.cpp ^
   /Fe:"%~dp0test_price_source_stats.exe"

if errorlevel 1 (
    echo.
    echo COMPILATION FAILED!
    exit /b 1
)

echo.
echo Running tests...
echo.
"%~dp0test_price_source_stats.exe"
set TEST_RESULT=%ERRORLEVEL%

echo.
echo Cleaning up...
del /Q *.obj 2>nul
del /Q test_price_source_stats.exe 2>nul

if %TEST_RESULT% NEQ 0 (
    echo.
    echo TESTS FAILED!
    exit /b 1
)

echo.
echo All tests passed!
exit /b 0
