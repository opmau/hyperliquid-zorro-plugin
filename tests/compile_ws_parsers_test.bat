@echo off
setlocal

echo ============================================
echo   COMPILING ws_parsers UNIT TEST [OPM-10]
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
echo   - ..\src\vendor\yyjson
echo.

echo Compiling...
cl /nologo /EHsc /std:c++17 ^
   /I..\src\transport ^
   /I..\src\vendor\yyjson ^
   unit\test_ws_parsers.cpp ^
   ..\src\transport\ws_parsers.cpp ^
   ..\src\transport\ws_price_cache.cpp ^
   ..\src\vendor\yyjson\yyjson.c ^
   /Fe:test_ws_parsers.exe

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
test_ws_parsers.exe
set TEST_RESULT=%ERRORLEVEL%

echo.
echo Cleaning up...
del /Q *.obj 2>nul
del /Q test_ws_parsers.exe 2>nul

if %TEST_RESULT% NEQ 0 (
    echo.
    echo TEST FAILED!
    exit /b 1
)

echo.
echo ALL TESTS PASSED!
exit /b 0
