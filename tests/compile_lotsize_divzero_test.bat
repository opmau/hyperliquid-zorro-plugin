@echo off
REM =============================================================================
REM compile_lotsize_divzero_test.bat - Compile and run lotSize div-by-zero tests
REM =============================================================================
REM PREVENTS: OPM-158 (lotSize division-by-zero in fill calculation)
REM =============================================================================

call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1

cd /d "%~dp0"

echo.
echo ===================================================
echo  Compiling test_lotsize_divzero.cpp
echo  Tests: lotSize division-by-zero guard
echo ===================================================
echo.

cl /nologo /EHsc /std:c++14 /I. unit\test_lotsize_divzero.cpp /Fe:test_lotsize_divzero.exe

if errorlevel 1 (
    echo.
    echo COMPILATION FAILED!
    exit /b 1
)

echo.
echo Running tests...
echo.
.\test_lotsize_divzero.exe
set TEST_RESULT=%ERRORLEVEL%

echo.
echo Cleaning up...
del /Q *.obj 2>nul
del /Q test_lotsize_divzero.exe 2>nul

if %TEST_RESULT% NEQ 0 (
    echo.
    echo TESTS FAILED!
    exit /b 1
)

echo.
echo All tests passed!
exit /b 0
