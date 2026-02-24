@echo off
REM =============================================================================
REM compile_trigger_order_test.bat - Compile and run trigger order tests [OPM-77]
REM =============================================================================
REM PREVENTS: Silent STOP flag discard, incorrect trigger order JSON
REM =============================================================================

call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1

cd /d "%~dp0"

echo.
echo ===================================================
echo  Compiling test_trigger_order.cpp [OPM-77]
echo  Tests: Trigger types, msgpack, STOP flag, slippage
echo ===================================================
echo.

cl /nologo /EHsc /std:c++14 /I. /I..\src\foundation unit\test_trigger_order.cpp ..\src\foundation\hl_msgpack.cpp /Fe:test_trigger_order.exe

if errorlevel 1 (
    echo.
    echo COMPILATION FAILED!
    exit /b 1
)

echo.
echo Running tests...
echo.
.\test_trigger_order.exe
set TEST_RESULT=%ERRORLEVEL%

echo.
echo Cleaning up...
del /Q *.obj 2>nul
del /Q test_trigger_order.exe 2>nul

if %TEST_RESULT% NEQ 0 (
    echo.
    echo TESTS FAILED!
    exit /b 1
)

echo.
echo All tests passed!
exit /b 0
