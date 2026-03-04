@echo off
REM =============================================================================
REM compile_schedule_cancel_test.bat - Compile and run scheduleCancel tests
REM =============================================================================
REM PREVENTS: Incorrect msgpack encoding, signature mismatch [OPM-83]
REM VERIFIED AGAINST: Python SDK signing_test.py test_schedule_cancel_action
REM =============================================================================

call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1

cd /d "%~dp0"

echo.
echo ===================================================
echo  Compiling test_schedule_cancel.cpp [OPM-83]
echo  Tests: scheduleCancel msgpack + EIP-712 signing
echo ===================================================
echo.

cl /nologo /EHsc /std:c++14 ^
   /I. /I..\src\foundation ^
   /I..\Source\HyperliquidPlugin\crypto ^
   unit\test_schedule_cancel.cpp ^
   ..\src\foundation\hl_eip712.cpp ^
   ..\src\foundation\hl_msgpack.cpp ^
   ..\src\foundation\hl_crypto.cpp ^
   ..\Source\HyperliquidPlugin\crypto\keccak256.c ^
   /Fe:test_schedule_cancel.exe

if errorlevel 1 (
    echo.
    echo COMPILATION FAILED!
    exit /b 1
)

echo.
echo Running tests...
echo.
.\test_schedule_cancel.exe
set TEST_RESULT=%ERRORLEVEL%

echo.
echo Cleaning up...
del /Q *.obj 2>nul
del /Q test_schedule_cancel.exe 2>nul

if %TEST_RESULT% NEQ 0 (
    echo.
    echo TESTS FAILED!
    exit /b 1
)

echo.
echo All tests passed!
exit /b 0
