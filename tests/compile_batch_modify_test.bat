@echo off
REM =============================================================================
REM compile_batch_modify_test.bat - Compile and run batchModify tests [OPM-80]
REM =============================================================================
REM PREVENTS: Incorrect msgpack field ordering, wrong oid encoding
REM =============================================================================

call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1

cd /d "%~dp0"

echo.
echo ===================================================
echo  Compiling test_batch_modify.cpp [OPM-80]
echo  Tests: batchModify msgpack encoding, field ordering
echo ===================================================
echo.

cl /nologo /EHsc /std:c++14 /I. /I..\src\foundation unit\test_batch_modify.cpp ..\src\foundation\hl_msgpack.cpp /Fe:test_batch_modify.exe

if errorlevel 1 (
    echo.
    echo COMPILATION FAILED!
    exit /b 1
)

echo.
echo Running tests...
echo.
.\test_batch_modify.exe
set TEST_RESULT=%ERRORLEVEL%

echo.
echo Cleaning up...
del /Q *.obj 2>nul
del /Q test_batch_modify.exe 2>nul

if %TEST_RESULT% NEQ 0 (
    echo.
    echo TESTS FAILED!
    exit /b 1
)

echo.
echo All tests passed!
exit /b 0
