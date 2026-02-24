@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
cd /d "%~dp0"
echo Compiling OPM-140 perpDex prefix stripping test...
cl /nologo /EHsc /std:c++14 /I..\src\foundation unit\test_perpdex_prefix.cpp ..\src\foundation\hl_utils.cpp /Fe:test_perpdex_prefix.exe
if errorlevel 1 (
    echo COMPILATION FAILED!
    exit /b 1
)
echo.
echo Running test...
test_perpdex_prefix.exe
set EXITCODE=%ERRORLEVEL%
echo.
echo Cleaning up...
del /Q *.obj 2>nul
exit /b %EXITCODE%
