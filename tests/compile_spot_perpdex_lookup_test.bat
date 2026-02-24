@echo off
REM =============================================================================
REM compile_spot_perpdex_lookup_test.bat - Spot & PerpDex asset lookup tests
REM =============================================================================
REM Tests: OPM-138 regression - spot/perpDex assets found by BrokerAsset lookup
REM =============================================================================

call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1

cd /d "%~dp0"

echo.
echo ===================================================
echo  Compiling test_spot_perpdex_lookup.cpp
echo  Tests: OPM-138 spot/perpDex registry lookup chain
echo ===================================================
echo.

cl /nologo /EHsc /std:c++14 /I. /I..\src\foundation unit\test_spot_perpdex_lookup.cpp ..\src\foundation\hl_globals.cpp /Fe:test_spot_perpdex_lookup.exe

if errorlevel 1 (
    echo.
    echo COMPILATION FAILED!
    exit /b 1
)

echo.
echo Running tests...
echo.
.\test_spot_perpdex_lookup.exe
set TEST_RESULT=%ERRORLEVEL%

echo.
echo Cleaning up...
del /Q *.obj 2>nul
del /Q test_spot_perpdex_lookup.exe 2>nul

if %TEST_RESULT% NEQ 0 (
    echo.
    echo TESTS FAILED!
    exit /b 1
)

echo.
echo All tests passed!
exit /b 0
