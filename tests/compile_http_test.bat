@echo off
setlocal

echo ============================================
echo   COMPILING hl_http MODULE TEST
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
echo   - ..\src\transport
echo.

echo Compiling...
cl /nologo /EHsc /std:c++14 ^
   /I..\src\foundation ^
   /I..\src\transport ^
   test_http_compile.cpp ^
   ..\src\transport\hl_http.cpp ^
   ..\src\foundation\hl_globals.cpp ^
   /Fe:test_http.exe

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
"%~dp0test_http.exe"

if errorlevel 1 (
    echo.
    echo TEST FAILED!
    exit /b 1
)

echo.
echo Cleaning up...
del /Q "%~dp0*.obj" 2>nul
del /Q "%~dp0test_http.exe" 2>nul

echo.
echo ALL TESTS PASSED!
