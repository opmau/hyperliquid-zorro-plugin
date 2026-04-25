@echo off
REM Compile-only check for API layer (hl_broker, hl_broker_commands)
REM API layer is the only layer that includes Zorro SDK headers

call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars32.bat" >nul 2>&1

cd /d "C:\Users\admki\OneDrive\Zorro-plugin"

echo.
echo === Compile API Layer ===
echo.

REM Note: Do NOT use /I"include" - Zorro SDK has custom windows.h for Lite-C
REM      The API files use relative paths like ../../include/trading.h instead

echo [1/2] hl_broker.cpp...
cl /nologo /c /EHsc /std:c++17 /W3 /DWIN32 /D_WINDOWS /D_USRDLL /I"src\foundation" /I"src\transport" /I"src\services" /I"src\api" src\api\hl_broker.cpp /Fo"build\hl_broker.obj"
if errorlevel 1 (
    echo.
    echo === hl_broker.cpp FAILED ===
    exit /b 1
)
echo       OK

echo [2/2] hl_broker_commands.cpp...
cl /nologo /c /EHsc /std:c++17 /W3 /DWIN32 /D_WINDOWS /D_USRDLL /I"src\foundation" /I"src\transport" /I"src\services" /I"src\api" src\api\hl_broker_commands.cpp /Fo"build\hl_broker_commands.obj"
if errorlevel 1 (
    echo.
    echo === hl_broker_commands.cpp FAILED ===
    exit /b 1
)
echo       OK

echo.
echo === API LAYER COMPILATION OK ===
dir build\hl_broker*.obj
exit /b 0
