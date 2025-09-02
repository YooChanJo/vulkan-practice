@echo off
IF EXIST ..\..\build-release (
    rmdir /s /q ..\..\build-release
    echo build release directory removed.
) ELSE (
    echo build release directory does not exist.
)
REM Detect system architecture
if defined PROCESSOR_ARCHITEW6432 (
    set ARCH=x64
) else if "%PROCESSOR_ARCHITECTURE%"=="AMD64" (
    set ARCH=x64
) else (
    set ARCH=Win32
)
echo Detected architecture: %ARCH%

REM Configure CMake
echo Running: cmake -S ..\.. -B ..\..\build-release -DCMAKE_BUILD_TYPE=Release -DENABLE_UPDATE_DEPS=ON -G "Visual Studio 17 2022" -A %ARCH%
call cmake -S ..\.. -B ..\..\build-release -DCMAKE_BUILD_TYPE=Release -DENABLE_UPDATE_DEPS=ON -G "Visual Studio 17 2022" -A %ARCH%
