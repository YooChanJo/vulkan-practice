@echo off
REM ==============================================================
REM Determine build configuration
REM ==============================================================
set BUILD_TYPE=Debug
if /I "%1"=="--Debug" (
    set BUILD_TYPE=Debug
) else (
    if /I "%1"=="" (
        set BUILD_TYPE=Debug
    ) else (
        if /I "%1"=="--Release" (
            set BUILD_TYPE=Release
        ) else (
            if /I "%1"=="--RelWithDebInfo" (
                set BUILD_TYPE=RelWithDebInfo
            ) else (
                if /I "%1"=="--MinSizeRel" (
                    set BUILD_TYPE=MinSizeRel
                ) else (
                    if /I "%1"=="--help" (
                        echo ^<Available Commands^>
                        echo Debug: ./build or ./build --Debug
                        echo Release: ./build --Release
                        echo RelWithDebuInfo: ./build --RelWithDebInfo
                        echo MinSizeRel: ./build --MinSizeRel
                        exit /b 0
                    ) else (
                        echo Invalid flag
                        echo ^<Available Commands^>
                        echo Debug: ./build or ./build --Debug
                        echo Release: ./build --Release
                        echo RelWithDebuInfo: ./build --RelWithDebInfo
                        echo MinSizeRel: ./build --MinSizeRel
                        exit /b 1
                    )
                )
            )
        )
    )
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
REM Configure CMake
REM ==============================================================
echo Running: cmake -S ..\.. -B ..\..\build -DIS_INITIAL_SETUP=OFF -G "Visual Studio 17 2022" -A %ARCH%
call cmake -S ..\.. -B ..\..\build -DIS_INITIAL_SETUP=OFF -G "Visual Studio 17 2022" -A %ARCH%
if errorlevel 1 (
    echo CMake configuration failed!
    exit /b 1
)

echo Building %BUILD_TYPE%
echo Running: cmake --build ..\..\build --parallel --config %BUILD_TYPE%
call cmake --build ..\..\build --parallel --config %BUILD_TYPE%
if errorlevel 1 (
    echo Build failed!
    exit /b 1
)
echo Build finished successfully!
