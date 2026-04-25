@echo off
REM Compile-only test for EIP-712 module (/c = compile only, no linking)
REM Run from: c:\Users\admki\OneDrive\Zorro-plugin\build\

call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars32.bat"

cd /d "C:\Users\admki\OneDrive\Zorro-plugin"

echo.
echo === Compile-only check for EIP-712 module ===
echo.

echo [1/1] hl_eip712.cpp...
cl /nologo /c /EHsc /std:c++17 /W3 /I"src\foundation" /I"Source\HyperliquidPlugin\crypto" src\foundation\hl_eip712.cpp /Fo"build\hl_eip712.obj"
if errorlevel 1 goto :fail
echo       OK

echo.
echo === EIP-712 MODULE COMPILES OK ===
echo.
dir build\hl_eip712.obj
goto :end

:fail
echo.
echo === COMPILATION FAILED ===
exit /b 1

:end
exit /b 0
