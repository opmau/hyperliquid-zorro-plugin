@echo off
REM =============================================================================
REM compile_get_position_after_fill_test.bat - GET_POSITION after fill tests
REM =============================================================================
REM PREVENTS: OPM-85 (GET_POSITION returns 0 immediately after order fill)
REM =============================================================================

call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars32.bat" >nul 2>&1

cd /d "%~dp0"

echo.
echo ===================================================
echo  Compiling test_get_position_after_fill.cpp
echo  Tests: GET_POSITION returns correct value after fill
echo ===================================================
echo.

cl /nologo /EHsc /std:c++17 /I..\src\transport unit\test_get_position_after_fill.cpp ..\src\transport\ws_price_cache.cpp /Fe:test_get_position_after_fill.exe

if errorlevel 1 (
    echo.
    echo COMPILATION FAILED!
    exit /b 1
)

echo.
echo Running tests...
echo.
test_get_position_after_fill.exe
set TEST_RESULT=%ERRORLEVEL%

echo.
echo Cleaning up...
del /Q *.obj 2>nul
del /Q test_get_position_after_fill.exe 2>nul

if %TEST_RESULT% NEQ 0 (
    echo.
    echo TESTS FAILED!
    exit /b 1
)

echo.
echo All tests passed!
exit /b 0