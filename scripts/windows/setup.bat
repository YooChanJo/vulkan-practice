@echo off
REM WARNING: Using Setup command includes process of dep update, which is unnecessary after initial setup
REM If setup has already successfully been executed, use ./build instead.

REM ==============================================================
REM Remove existing build directory
REM ==============================================================
IF EXIST ..\..\build (
    rmdir /s /q ..\..\build
    echo Build directory removed.
) ELSE (
    echo Build directory does not exist.
)

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
REM Configure CMake with Vulkan dependencies
REM ==============================================================
echo Running: cmake -S ..\.. -B ..\..\build -DIS_INITIAL_SETUP=ON -G "Visual Studio 17 2022" -A %ARCH%
call cmake -S ..\.. -B ..\..\build -DIS_INITIAL_SETUP=ON -G "Visual Studio 17 2022" -A %ARCH%
if errorlevel 1 (
    echo CMake configuration failed!
    exit /b 1
)
echo CMake configuration completed successfully.
