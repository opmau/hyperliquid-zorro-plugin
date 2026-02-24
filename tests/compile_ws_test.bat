@echo off
echo ============================================
echo   COMPILE TEST: WebSocket Modules
echo ============================================
echo.

echo Setting up Visual Studio environment...
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars32.bat"
if errorlevel 1 (
    echo ERROR: Could not find Visual Studio Build Tools
    exit /b 1
)

cd /d "%~dp0.."
echo.

echo Compiling ws_price_cache.cpp...
cl /nologo /c /EHsc /std:c++17 /I"src/transport" ^
   src\transport\ws_price_cache.cpp ^
   /Fo"build\ws_price_cache.obj"
if errorlevel 1 (
    echo ERROR: ws_price_cache.cpp failed to compile
    exit /b 1
)
echo   OK

echo Compiling ws_connection.cpp...
cl /nologo /c /EHsc /std:c++17 /I"src/transport" ^
   src\transport\ws_connection.cpp ^
   /Fo"build\ws_connection.obj"
if errorlevel 1 (
    echo ERROR: ws_connection.cpp failed to compile
    exit /b 1
)
echo   OK

echo Compiling ws_manager.cpp...
cl /nologo /c /EHsc /std:c++17 /I"src/transport" ^
   src\transport\ws_manager.cpp ^
   /Fo"build\ws_manager.obj"
if errorlevel 1 (
    echo ERROR: ws_manager.cpp failed to compile
    exit /b 1
)
echo   OK

echo.
echo ============================================
echo   ALL WEBSOCKET MODULES COMPILED OK
echo ============================================

:: Cleanup
del /Q build\*.obj 2>nul

exit /b 0
