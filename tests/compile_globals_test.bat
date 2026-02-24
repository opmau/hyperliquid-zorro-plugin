@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
cd /d "%~dp0"
echo Compiling hl_globals.cpp and test...
cl /nologo /EHsc /std:c++14 /I..\src\foundation test_globals_compile.cpp ..\src\foundation\hl_globals.cpp /Fe:test_globals.exe
if errorlevel 1 (
    echo COMPILATION FAILED!
    exit /b 1
)
echo.
echo Running test...
test_globals.exe
echo.
echo Cleaning up...
del /Q *.obj 2>nul
