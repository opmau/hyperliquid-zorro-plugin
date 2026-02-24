@echo off
REM =============================================================================
REM compile_spot_asset_test.bat - Compile and run spot asset formula tests
REM =============================================================================
REM Tests: Spot pxDecimals, PIPCost (10^-8), index mapping, normalizeCoin
REM =============================================================================

call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1

cd /d "%~dp0"

echo.
echo ===================================================
echo  Compiling test_spot_asset.cpp
echo  Tests: Spot formulas, index mapping, normalizeCoin
echo ===================================================
echo.

cl /nologo /EHsc /std:c++14 /I. /I..\src\foundation unit\test_spot_asset.cpp ..\src\foundation\hl_utils.cpp /Fe:test_spot_asset.exe

if errorlevel 1 (
    echo.
    echo COMPILATION FAILED!
    exit /b 1
)

echo.
echo Running tests...
echo.
test_spot_asset.exe
set TEST_RESULT=%ERRORLEVEL%

echo.
echo Cleaning up...
del /Q *.obj 2>nul
del /Q test_spot_asset.exe 2>nul

if %TEST_RESULT% NEQ 0 (
    echo.
    echo TESTS FAILED!
    exit /b 1
)

echo.
echo All tests passed!
exit /b 0
