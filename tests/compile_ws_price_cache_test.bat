@echo off
setlocal

echo ============================================
echo   COMPILING ws_price_cache MODULE TEST
echo ============================================
echo.

:: Setup Visual Studio environment (32-bit for Zorro compatibility)
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars32.bat" >nul 2>&1
if errorlevel 1 (
    echo ERROR: Could not setup Visual Studio environment
    exit /b 1
)

cd /d "%~dp0"

echo Include paths:
echo   - ..\src\transport
echo.

echo Compiling...
cl /nologo /EHsc /std:c++17 ^
   /I..\src\transport ^
   test_ws_price_cache.cpp ^
   ..\src\transport\ws_price_cache.cpp ^
   /Fe:test_ws_price_cache.exe

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
test_ws_price_cache.exe

if errorlevel 1 (
    echo.
    echo TEST FAILED!
    exit /b 1
)

echo.
echo Cleaning up...
del /Q *.obj 2>nul
del /Q test_ws_price_cache.exe 2>nul

echo.
echo ALL TESTS PASSED!
