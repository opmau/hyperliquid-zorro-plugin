@echo off
REM =============================================================================
REM compile_eip712_source_test.bat - Compile and run EIP-712 source tests
REM =============================================================================
REM PREVENTS: Bug OPM-22 (e392a43) - testnet signing used wrong source
REM =============================================================================

call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1

cd /d "%~dp0"

echo.
echo ===================================================
echo  Compiling test_eip712_source.cpp
echo  Tests: Mainnet vs Testnet EIP-712 signing source
echo ===================================================
echo.

cl /nologo /EHsc /std:c++14 ^
   /I. /I..\src\foundation ^
   /I..\Source\HyperliquidPlugin\crypto ^
   unit\test_eip712_source.cpp ^
   ..\src\foundation\hl_eip712.cpp ^
   ..\src\foundation\hl_msgpack.cpp ^
   ..\src\foundation\hl_crypto.cpp ^
   ..\Source\HyperliquidPlugin\crypto\keccak256.c ^
   /Fe:test_eip712_source.exe

if errorlevel 1 (
    echo.
    echo COMPILATION FAILED!
    exit /b 1
)

echo.
echo Running tests...
echo.
.\test_eip712_source.exe
set TEST_RESULT=%ERRORLEVEL%

echo.
echo Cleaning up...
del /Q *.obj 2>nul
del /Q test_eip712_source.exe 2>nul

if %TEST_RESULT% NEQ 0 (
    echo.
    echo TESTS FAILED!
    exit /b 1
)

echo.
echo All tests passed!
exit /b 0
