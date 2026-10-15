@echo off
REM =============================================================================
REM compile_margin_actions_test.bat - Compile and run leverage/margin tests
REM =============================================================================
REM PREVENTS: updateLeverage / updateIsolatedMargin packed or signed unlike the
REM           SDK; USD->ntli scaling; isolated-only assets read as cross [OPM-1584]
REM VERIFIED AGAINST: msgpack-python packb + Python SDK sign_l1_action
REM =============================================================================

call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1

cd /d "%~dp0"

echo.
echo ===================================================
echo  Compiling test_margin_actions.cpp [OPM-1584]
echo  Tests: leverage / isolated margin msgpack + signing
echo ===================================================
echo.

cl /nologo /EHsc /std:c++14 ^
   /I. /I..\src\foundation /I..\src\services ^
   /I..\src\vendor\secp256k1 ^
   unit\test_margin_actions.cpp ^
   ..\src\foundation\hl_eip712.cpp ^
   ..\src\foundation\hl_msgpack.cpp ^
   ..\src\foundation\hl_crypto.cpp ^
   ..\src\vendor\secp256k1\keccak256.c ^
   /Fe:test_margin_actions.exe

if errorlevel 1 (
    echo.
    echo COMPILATION FAILED!
    exit /b 1
)

echo.
echo Running tests...
echo.
.\test_margin_actions.exe
set TEST_RESULT=%ERRORLEVEL%

echo.
echo Cleaning up...
del /Q *.obj 2>nul
del /Q test_margin_actions.exe 2>nul

if %TEST_RESULT% NEQ 0 (
    echo.
    echo TESTS FAILED!
    exit /b 1
)

echo.
echo All tests passed!
exit /b 0
