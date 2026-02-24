@echo off
REM =============================================================================
REM install_hooks.bat - Install Git pre-commit hooks
REM =============================================================================
REM Run this script once to enable automatic testing before commits.
REM =============================================================================

cd /d "%~dp0\.."

echo.
echo Installing Git hooks...
echo.

if not exist ".git\hooks" (
    echo ERROR: .git\hooks directory not found!
    echo Make sure you're running this from the repository root.
    exit /b 1
)

REM Copy the pre-commit hook
copy /Y "scripts\pre-commit" ".git\hooks\pre-commit" >nul
if errorlevel 1 (
    echo ERROR: Failed to copy pre-commit hook
    exit /b 1
)

echo Pre-commit hook installed successfully!
echo.
echo What happens now:
echo   - Before every 'git commit', unit tests will run automatically
echo   - If tests fail, the commit is blocked
echo   - Fix the issues and commit again
echo.
echo To test the hook manually:
echo   cd tests
echo   run_unit_tests.bat
echo.
echo To bypass the hook (NOT RECOMMENDED):
echo   git commit --no-verify
echo.