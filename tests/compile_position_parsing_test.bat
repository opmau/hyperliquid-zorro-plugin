@echo off
REM =============================================================================
REM compile_position_parsing_test.bat - Compile and run position parsing tests
REM =============================================================================
REM PREVENTS: Bug 81db4b6 (wrong asset's position data in multi-asset JSON)
REM =============================================================================

call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1

cd /d "%~dp0"

echo.
echo ===================================================
echo  Compiling test_position_parsing.cpp
echo  Tests: Multi-asset JSON parsing correctness
echo ===================================================
echo.

cl /nologo /EHsc /std:c++14 /I. unit\test_position_parsing.cpp /Fe:test_position_parsing.exe

if errorlevel 1 (
    echo.
    echo COMPILATION FAILED!
    exit /b 1
)

echo.
echo Running tests...
echo.
test_position_parsing.exe
set TEST_RESULT=%ERRORLEVEL%

echo.
echo Cleaning up...
del /Q *.obj 2>nul
del /Q test_position_parsing.exe 2>nul

if %TEST_RESULT% NEQ 0 (
    echo.
    echo TESTS FAILED!
    exit /b 1
)

echo.
echo All tests passed!
exit /b 0