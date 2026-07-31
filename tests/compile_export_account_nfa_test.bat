@echo off
REM =============================================================================
REM compile_export_account_nfa_test.bat - HL_EXPORT_ACCOUNT NFA column [OPM-801]
REM =============================================================================
REM TESTS: The Accounts.csv template emitted by HL_EXPORT_ACCOUNT (50003) must
REM        leave the NFA compliance column neutral (0). The column is the user's
REM        setting - Accounts.csv or set(NFA)/Hedge in the script - so the
REM        plugin must not bake a compliance opinion into the template.
REM =============================================================================

call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars32.bat" >nul 2>&1

cd /d "%~dp0"

echo.
echo ===================================================
echo  Compiling test_export_account_nfa.cpp [OPM-801]
echo  Tests: exported Accounts.csv NFA column is neutral
echo ===================================================
echo.

cl /nologo /EHsc /std:c++17 ^
   /I..\src\foundation ^
   /I. ^
   unit\test_export_account_nfa.cpp ^
   /Fe:"%~dp0test_export_account_nfa.exe"

if errorlevel 1 (
    echo.
    echo COMPILATION FAILED!
    exit /b 1
)

echo.
echo Running tests...
echo.
"%~dp0test_export_account_nfa.exe"
set TEST_RESULT=%ERRORLEVEL%

echo.
echo Cleaning up...
del /Q *.obj 2>nul
del /Q test_export_account_nfa.exe 2>nul

if %TEST_RESULT% NEQ 0 (
    echo.
    echo TESTS FAILED!
    exit /b 1
)

echo.
echo All tests passed!
exit /b 0
