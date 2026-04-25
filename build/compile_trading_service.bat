@echo off
REM Compile-only check for hl_trading_service

call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars32.bat" >nul 2>&1

cd /d "C:\Users\admki\OneDrive\Zorro-plugin"

echo.
echo === Compile hl_trading_service.cpp ===
echo.

cl /nologo /c /EHsc /std:c++14 /W3 /I"src\foundation" /I"src\transport" /I"src\services" src\services\hl_trading_service.cpp /Fo"build\hl_trading_service.obj"
if errorlevel 1 (
    echo.
    echo === COMPILATION FAILED ===
    exit /b 1
)

echo.
echo === COMPILATION OK ===
dir build\hl_trading_service.obj
exit /b 0
