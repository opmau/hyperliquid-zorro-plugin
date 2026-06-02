@echo off
REM Compile-only test for WebSocket modules (/c = compile only, no linking)
REM Run from anywhere (paths resolved relative to this script)

call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars32.bat"

cd /d "%~dp0.."

echo.
echo === Compile-only check for WebSocket modules ===
echo.

echo [1/4] ws_price_cache.cpp...
cl /nologo /c /EHsc /std:c++14 /W3 /I"src\transport" src\transport\ws_price_cache.cpp /Fo"build\ws_price_cache.obj"
if errorlevel 1 goto :fail
echo       OK

echo [2/4] ws_connection.cpp...
cl /nologo /c /EHsc /std:c++14 /W3 /I"src\transport" src\transport\ws_connection.cpp /Fo"build\ws_connection.obj"
if errorlevel 1 goto :fail
echo       OK

echo [3/4] ws_parsers.cpp...
cl /nologo /c /EHsc /std:c++14 /W3 /I"src\transport" src\transport\ws_parsers.cpp /Fo"build\ws_parsers.obj"
if errorlevel 1 goto :fail
echo       OK

echo [4/4] ws_manager.cpp...
cl /nologo /c /EHsc /std:c++14 /W3 /I"src\transport" src\transport\ws_manager.cpp /Fo"build\ws_manager.obj"
if errorlevel 1 goto :fail
echo       OK

echo.
echo === ALL MODULES COMPILE OK ===
echo.
dir build\*.obj
goto :end

:fail
echo.
echo === COMPILATION FAILED ===
exit /b 1

:end
exit /b 0
