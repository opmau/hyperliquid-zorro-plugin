@echo off
REM Compile-only test for hl_meta module (/c = compile only, no linking)
REM Run from: tests\ directory

call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1

cd /d "%~dp0.."

echo.
echo === Compile-only check for hl_meta module (Services Layer) ===
echo.

echo [1/1] hl_meta.cpp...
cl /nologo /c /EHsc /std:c++14 /W3 ^
   /I"src\foundation" ^
   /I"src\transport" ^
   /I"src\services" ^
   src\services\hl_meta.cpp ^
   /Fo"build\hl_meta.obj"
if errorlevel 1 goto :fail
echo       OK

echo.
echo === hl_meta MODULE COMPILES OK ===
echo.
dir build\hl_meta.obj
goto :end

:fail
echo.
echo === COMPILATION FAILED ===
exit /b 1

:end
exit /b 0
