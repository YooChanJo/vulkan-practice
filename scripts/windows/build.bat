@echo off
REM Detect system architecture
if defined PROCESSOR_ARCHITEW6432 (
    set ARCH=x64
) else if "%PROCESSOR_ARCHITECTURE%"=="AMD64" (
    set ARCH=x64
) else (
    set ARCH=Win32
)
echo Detected architecture: %ARCH%

REM Configure CMake and Run
echo Running: cmake -S ..\.. -B ..\..\build -DCMAKE_BUILD_TYPE=Debug -G "Visual Studio 17 2022" -A %ARCH%
call cmake -S ..\.. -B ..\..\build -DCMAKE_BUILD_TYPE=Debug -G "Visual Studio 17 2022" -A %ARCH%
echo Running: cmake --build ..\..\build --parallel --config Debug
call cmake --build ..\..\build --parallel --config Debug