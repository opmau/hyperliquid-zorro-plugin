@echo off
REM =============================================================================
REM compile_bracket_order_test.bat - Compile and run bracket order tests [OPM-79]
REM =============================================================================
REM PREVENTS: Incorrect msgpack field ordering, wrong grouping string,
REM           missing orders in bracket array
REM =============================================================================

call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1

cd /d "%~dp0"

echo.
echo ===================================================
echo  Compiling test_bracket_order.cpp [OPM-79]
echo  Tests: Bracket order msgpack encoding, field ordering
echo ===================================================
echo.

cl /nologo /EHsc /std:c++14 /I. /I..\src\foundation unit\test_bracket_order.cpp ..\src\foundation\hl_msgpack.cpp /Fe:test_bracket_order.exe

if errorlevel 1 (
    echo.
    echo COMPILATION FAILED!
    exit /b 1
)

echo.
echo Running tests...
echo.
.\test_bracket_order.exe
set TEST_RESULT=%ERRORLEVEL%

echo.
echo Cleaning up...
del /Q *.obj 2>nul
del /Q test_bracket_order.exe 2>nul

if %TEST_RESULT% NEQ 0 (
    echo.
    echo TESTS FAILED!
    exit /b 1
)

echo.
echo All tests passed!
exit /b 0
