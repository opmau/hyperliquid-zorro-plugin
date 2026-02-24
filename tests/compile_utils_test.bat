@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
cd /d "%~dp0"
echo Compiling hl_utils.cpp and test...
cl /nologo /EHsc /std:c++14 /I..\src\foundation test_utils_compile.cpp ..\src\foundation\hl_utils.cpp /Fe:test_utils.exe
if errorlevel 1 (
    echo COMPILATION FAILED!
    exit /b 1
)
echo.
echo Running test...
test_utils.exe
echo.
echo Cleaning up...
del /Q *.obj 2>nul
