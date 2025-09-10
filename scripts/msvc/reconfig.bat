@echo off
REM This script applies CMAKE list changes to /build
REM cleaning former build
call clean
REM ==============================================================
REM Detect system architecture
REM ==============================================================
if defined PROCESSOR_ARCHITEW6432 (
    set ARCH=x64
) else (
    if "%PROCESSOR_ARCHITECTURE%"=="AMD64" (
        set ARCH=x64
    ) else (
        set ARCH=Win32
    )
)
echo Detected architecture: %ARCH%

REM ==============================================================
REM Reconfigure CMake
REM ==============================================================
echo Running: cmake -S ..\.. -B ..\..\build -DIS_INITIAL_SETUP=OFF -G "Visual Studio 17 2022" -A %ARCH%
call cmake -S ..\.. -B ..\..\build -DIS_INITIAL_SETUP=OFF -G "Visual Studio 17 2022" -A %ARCH%
if errorlevel 1 (
    echo CMake configuration failed!
    exit /b 1
)