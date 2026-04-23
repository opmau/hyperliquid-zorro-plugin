@echo off
REM build_prod_dll.bat - Build Hyperliquid.dll (PRODUCTION) via CMake + vcpkg (32-bit)
REM Run from: c:\Users\admki\OneDrive\Zorro-plugin\build\
REM
REM Output: build_prod\Release\Hyperliquid.dll  (no _Dev suffix)
REM
REM This is the RELEASE build. Does NOT auto-deploy to Zorro\Plugin — that step
REM is manual after mainnet smoke tests pass.

setlocal
set PROJECT_ROOT=C:\Users\admki\OneDrive\Zorro-plugin
set BUILD_DIR=%PROJECT_ROOT%\build_prod
set CMAKE_EXE="C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
set VCVARS="C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars32.bat"

REM First-time configure (creates build_prod\CMakeCache.txt)
if not exist "%BUILD_DIR%\CMakeCache.txt" (
    echo === Configuring production build (first time) ===
    call %VCVARS% >nul
    if errorlevel 1 (
        echo ERROR: Visual Studio Build Tools not found at %VCVARS%
        exit /b 1
    )
    if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
    cd /d "%BUILD_DIR%"
    %CMAKE_EXE% "%PROJECT_ROOT%" -G "Visual Studio 17 2022" -A Win32 -DDEV_BUILD=OFF ^
        -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake ^
        -DVCPKG_TARGET_TRIPLET=x86-windows-static
    if errorlevel 1 (
        echo === CONFIGURE FAILED ===
        exit /b 1
    )
)

echo.
echo === Building Hyperliquid.dll (production) ===
echo.
%CMAKE_EXE% --build "%BUILD_DIR%" --config Release
if errorlevel 1 (
    echo.
    echo === BUILD FAILED ===
    exit /b 1
)

if not exist "%BUILD_DIR%\Release\Hyperliquid.dll" (
    echo.
    echo ERROR: Hyperliquid.dll was not produced.
    echo Check DEV_BUILD=OFF is set in %BUILD_DIR%\CMakeCache.txt
    exit /b 1
)

echo.
echo === BUILD SUCCESS ===
dir "%BUILD_DIR%\Release\Hyperliquid.dll" | findstr /R "Hyperliquid.dll"
echo.
echo Production DLL ready at:
echo   %BUILD_DIR%\Release\Hyperliquid.dll
echo.
echo To deploy (only after mainnet smoke tests pass):
echo   copy /Y "%BUILD_DIR%\Release\Hyperliquid.dll" "C:\Users\admki\OneDrive\Zorro\Plugin\Hyperliquid.dll"
echo.
endlocal
