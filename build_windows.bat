@echo off
rem ---------------------------------------------------------------------------
rem  Builds mess_tracker.exe and generate_sample_data.exe on Windows.
rem  Needs ONE of these free compilers:
rem    - MinGW-w64 (g++), e.g. the "WinLibs" zip or MSYS2, with its bin folder on PATH
rem    - Visual Studio / Build Tools: run this file from the "Developer Command Prompt"
rem  Double-click this file (or run it in a Command Prompt) from the project folder.
rem ---------------------------------------------------------------------------
setlocal
cd /d "%~dp0"

where g++ >nul 2>nul
if %errorlevel%==0 goto mingw
where cl >nul 2>nul
if %errorlevel%==0 goto msvc

echo No C++ compiler was found on this computer.
echo Install MinGW-w64 (for example the WinLibs zip from https://winlibs.com) and add its "bin" folder to PATH,
echo or install Visual Studio Build Tools, then run this file again.
pause
exit /b 1

:mingw
echo Building with g++ ...
set LIBSRC=src\Utils.cpp src\FileHandler.cpp src\Tracker.cpp src\Analyzer.cpp src\Forecaster.cpp src\Optimizer.cpp src\Report.cpp
g++ -std=c++17 -O2 -Iinclude -static src\main.cpp %LIBSRC% -o mess_tracker.exe
if errorlevel 1 goto fail
g++ -std=c++17 -O2 -Iinclude -static tools\generate_sample_data.cpp %LIBSRC% -o generate_sample_data.exe
if errorlevel 1 goto fail
goto done

:msvc
echo Building with cl ...
set LIBSRC=src\Utils.cpp src\FileHandler.cpp src\Tracker.cpp src\Analyzer.cpp src\Forecaster.cpp src\Optimizer.cpp src\Report.cpp
cl /nologo /std:c++17 /O2 /EHsc /utf-8 /Iinclude src\main.cpp %LIBSRC% /Fe:mess_tracker.exe
if errorlevel 1 goto fail
cl /nologo /std:c++17 /O2 /EHsc /utf-8 /Iinclude tools\generate_sample_data.cpp %LIBSRC% /Fe:generate_sample_data.exe
if errorlevel 1 goto fail
del /q *.obj >nul 2>nul
goto done

:done
echo.
echo Build finished. Created mess_tracker.exe and generate_sample_data.exe in this folder.
echo Run mess_tracker.exe from this folder (it reads the "data" folder next to it).
pause
exit /b 0

:fail
echo.
echo Build failed. Please check that your compiler supports C++17.
pause
exit /b 1
