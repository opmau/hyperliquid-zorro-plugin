@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
cd /d "%~dp0"
echo Compiling test_config_compile.cpp...
cl /nologo /EHsc /std:c++14 test_config_compile.cpp /Fe:test_config.exe
if errorlevel 1 (
    echo COMPILATION FAILED!
    exit /b 1
)
echo.
echo Running test...
test_config.exe
echo.
echo Cleaning up...
del /Q *.obj 2>nul
