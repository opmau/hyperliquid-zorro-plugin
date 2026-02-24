@echo off
REM =============================================================================
REM compile_meta_szdecimals_test.bat - Test szDecimals=0 parsing [OPM-131]
REM =============================================================================
REM PREVENTS: szDecimals=0 clamped to 4 in hl_meta.cpp
REM =============================================================================

call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1

cd /d "%~dp0"

echo.
echo ===================================================
echo  Compiling test_meta_szdecimals.cpp [OPM-131]
echo  Tests: szDecimals=0 preservation in meta parsing
echo ===================================================
echo.

cl /nologo /EHsc /std:c++14 /I. /I..\src\foundation /I..\src\transport /I..\src\vendor\yyjson ^
   unit\test_meta_szdecimals.cpp ..\src\vendor\yyjson\yyjson.c ^
   /Fe:"%~dp0test_meta_szdecimals.exe"

if errorlevel 1 (
    echo.
    echo COMPILATION FAILED!
    exit /b 1
)

echo.
echo Running tests...
echo.
"%~dp0test_meta_szdecimals.exe"
set TEST_RESULT=%ERRORLEVEL%

echo.
echo Cleaning up...
del /Q "%~dp0*.obj" 2>nul
del /Q "%~dp0test_meta_szdecimals.exe" 2>nul

if %TEST_RESULT% NEQ 0 (
    echo.
    echo TESTS FAILED!
    exit /b 1
)

echo.
echo All tests passed!
exit /b 0
