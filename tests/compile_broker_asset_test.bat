@echo off
REM =============================================================================
REM compile_broker_asset_test.bat - Compile and run PIP/PIPCost formula tests
REM =============================================================================
REM PREVENTS: Bugs 6dfb104, 213643c, 8303e8b
REM =============================================================================

call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1

cd /d "%~dp0"

echo.
echo ===================================================
echo  Compiling test_broker_asset.cpp
echo  Tests: PIP, PIPCost, LotAmount, Lot Conversion
echo ===================================================
echo.

cl /nologo /EHsc /std:c++14 /I. /I..\src\foundation unit\test_broker_asset.cpp /Fe:test_broker_asset.exe

if errorlevel 1 (
    echo.
    echo COMPILATION FAILED!
    exit /b 1
)

echo.
echo Running tests...
echo.
test_broker_asset.exe
set TEST_RESULT=%ERRORLEVEL%

echo.
echo Cleaning up...
del /Q *.obj 2>nul
del /Q test_broker_asset.exe 2>nul

if %TEST_RESULT% NEQ 0 (
    echo.
    echo TESTS FAILED!
    exit /b 1
)

echo.
echo All tests passed!
exit /b 0