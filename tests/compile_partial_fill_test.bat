@echo off
REM =============================================================================
REM compile_partial_fill_test.bat - Compile and run partial fill detection tests
REM =============================================================================
REM PREVENTS: OPM-91 (Partial fill threshold logic missing in BrokerTrade)
REM =============================================================================

call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1

cd /d "%~dp0"

echo.
echo ===================================================
echo  Compiling test_partial_fill.cpp
echo  Tests: Partial fill detection and status tracking
echo ===================================================
echo.

cl /nologo /EHsc /std:c++14 /I. unit\test_partial_fill.cpp /Fe:test_partial_fill.exe

if errorlevel 1 (
    echo.
    echo COMPILATION FAILED!
    exit /b 1
)

echo.
echo Running tests...
echo.
test_partial_fill.exe
set TEST_RESULT=%ERRORLEVEL%

echo.
echo Cleaning up...
del /Q *.obj 2>nul
del /Q test_partial_fill.exe 2>nul

if %TEST_RESULT% NEQ 0 (
    echo.
    echo TESTS FAILED!
    exit /b 1
)

echo.
echo All tests passed!
exit /b 0
