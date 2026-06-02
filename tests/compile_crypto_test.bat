@echo off
setlocal

echo ============================================
echo   COMPILING hl_crypto MODULE TEST
echo ============================================
echo.

:: Setup Visual Studio environment (64-bit)
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
if errorlevel 1 (
    echo ERROR: Could not setup Visual Studio environment
    exit /b 1
)

cd /d "%~dp0"

echo Include paths:
echo   - ..\src\foundation
echo   - ..\src\vendor\secp256k1
echo.

echo Compiling...
cl /nologo /EHsc /std:c++14 ^
   /I..\src\foundation ^
   /I..\src\vendor\secp256k1 ^
   test_crypto_compile.cpp ^
   ..\src\foundation\hl_crypto.cpp ^
   ..\src\vendor\secp256k1\keccak256.c ^
   /Fe:test_crypto.exe

if errorlevel 1 (
    echo.
    echo ============================================
    echo   COMPILATION FAILED!
    echo ============================================
    exit /b 1
)

echo.
echo ============================================
echo   COMPILATION SUCCESSFUL
echo ============================================
echo.

echo Running test...
echo.
test_crypto.exe

if errorlevel 1 (
    echo.
    echo TEST FAILED!
    exit /b 1
)

echo.
echo Cleaning up...
del /Q *.obj 2>nul
del /Q test_crypto.exe 2>nul

echo.
echo ALL TESTS PASSED!
