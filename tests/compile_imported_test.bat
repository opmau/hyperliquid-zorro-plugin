@echo off
REM =============================================================================
REM compile_imported_test.bat - Compile and run imported trade tests
REM =============================================================================
REM PREVENTS: Bug 18c287c (IMPORTED trades return stale data)
REM =============================================================================

call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1

cd /d "%~dp0"

echo.
echo ===================================================
echo  Compiling test_imported_trades.cpp
echo  Tests: IMPORTED trade position tracking
echo ===================================================
echo.

cl /nologo /EHsc /std:c++14 /I. unit\test_imported_trades.cpp /Fe:test_imported_trades.exe

if errorlevel 1 (
    echo.
    echo COMPILATION FAILED!
    exit /b 1
)

echo.
echo Running tests...
echo.
test_imported_trades.exe
set TEST_RESULT=%ERRORLEVEL%

echo.
echo Cleaning up...
del /Q *.obj 2>nul
del /Q test_imported_trades.exe 2>nul

if %TEST_RESULT% NEQ 0 (
    echo.
    echo TESTS FAILED!
    exit /b 1
)

echo.
echo All tests passed!
exit /b 0