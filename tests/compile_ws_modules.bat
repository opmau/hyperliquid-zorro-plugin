@echo off
setlocal

echo ============================================
echo   COMPILE TEST: WebSocket Modules
echo ============================================
echo.

REM Try to find Visual Studio
set "VCVARS="
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars32.bat" (
    set "VCVARS=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars32.bat"
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat" (
    set "VCVARS=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat"
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars32.bat" (
    set "VCVARS=C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars32.bat"
)

if "%VCVARS%"=="" (
    echo ERROR: Could not find Visual Studio Build Tools
    exit /b 1
)

echo Using: %VCVARS%
call "%VCVARS%" >nul 2>&1

cd /d "%~dp0.."

REM Create build directory if needed
if not exist build mkdir build

echo.
echo [1/3] Compiling ws_price_cache.cpp...
cl /nologo /c /EHsc /std:c++17 /W3 ^
   /I"src\transport" ^
   src\transport\ws_price_cache.cpp ^
   /Fo"build\ws_price_cache.obj" 2>&1
if errorlevel 1 (
    echo   FAILED!
    exit /b 1
)
echo   OK

echo [2/3] Compiling ws_connection.cpp...
cl /nologo /c /EHsc /std:c++17 /W3 ^
   /I"src\transport" ^
   src\transport\ws_connection.cpp ^
   /Fo"build\ws_connection.obj" 2>&1
if errorlevel 1 (
    echo   FAILED!
    exit /b 1
)
echo   OK

echo [3/3] Compiling ws_manager.cpp...
cl /nologo /c /EHsc /std:c++17 /W3 ^
   /I"src\transport" ^
   src\transport\ws_manager.cpp ^
   /Fo"build\ws_manager.obj" 2>&1
if errorlevel 1 (
    echo   FAILED!
    exit /b 1
)
echo   OK

echo.
echo ============================================
echo   ALL WEBSOCKET MODULES COMPILED OK
echo ============================================

REM Cleanup object files
del /Q build\*.obj 2>nul

exit /b 0
