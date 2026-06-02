@echo off
REM build_dev_dll.bat - Build Hyperliquid_Dev.dll from modular sources (32-bit)
REM Run from anywhere; paths are resolved relative to this script's location.
REM NOTE: legacy direct-cl.exe helper. The supported build is CMake (see README);
REM this references the gitignored Source\ junction and only works on a machine
REM with a local Zorro install.

call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars32.bat" >nul 2>&1
if errorlevel 1 (
    echo ERROR: Could not find Visual Studio Build Tools
    exit /b 1
)

REM Repository root = parent of this script's build\ directory
cd /d "%~dp0.."

echo.
echo === Building Hyperliquid_Dev.dll (Modular 32-bit) ===
echo.

REM Collect all source files
set SOURCES=^
    src\foundation\hl_globals.cpp ^
    src\foundation\hl_utils.cpp ^
    src\foundation\hl_crypto.cpp ^
    src\foundation\hl_eip712.cpp ^
    src\transport\hl_http.cpp ^
    src\transport\ws_price_cache.cpp ^
    src\transport\ws_connection.cpp ^
    src\transport\ws_parsers.cpp ^
    src\transport\ws_manager.cpp ^
    src\services\hl_meta.cpp ^
    src\services\hl_market_service.cpp ^
    src\services\hl_trading_service.cpp ^
    src\services\hl_account_service.cpp ^
    src\api\hl_broker.cpp ^
    src\api\hl_broker_commands.cpp ^
    Source\HyperliquidPlugin\crypto\keccak256.c

REM Include paths
set INCLUDES=^
    /Isrc\foundation ^
    /Isrc\transport ^
    /Isrc\services ^
    /Isrc\api ^
    /ISource\HyperliquidPlugin\crypto

REM Defines
set DEFINES=/DWIN32 /DNDEBUG /D_WINDOWS /D_USRDLL /DDEV_BUILD /D_WIN32_WINNT=0x0601 /DWIN32_LEAN_AND_MEAN

REM Build DLL
echo Compiling and linking...
cl /nologo /O2 /MD /LD /EHsc /std:c++17 %INCLUDES% %DEFINES% /Fe"build\Hyperliquid_Dev.dll" %SOURCES% /link /SUBSYSTEM:WINDOWS winhttp.lib kernel32.lib user32.lib
if errorlevel 1 (
    echo.
    echo === BUILD FAILED! ===
    exit /b 1
)

REM Clean up obj files from project root
if exist *.obj del /Q *.obj
if exist *.lib del /Q hl_broker.lib hl_globals.lib 2>nul
if exist *.exp del /Q *.exp 2>nul

echo.
echo === BUILD SUCCESS ===
dir build\Hyperliquid_Dev.dll
echo.

REM Deploy to Zorro Plugin folder (set ZORRO_HOME to your Zorro install path)
if "%ZORRO_HOME%"=="" (
    echo Skipping deploy: set ZORRO_HOME to your Zorro install path to auto-deploy.
    goto :done
)
echo Deploying to Zorro Plugin folder...
copy /Y "build\Hyperliquid_Dev.dll" "%ZORRO_HOME%\Plugin\Hyperliquid_Dev.dll"
if errorlevel 1 (
    echo WARNING: Could not copy to Plugin folder
) else (
    echo Deployed to Plugin\Hyperliquid_Dev.dll
)

:done
echo.
echo Done!
