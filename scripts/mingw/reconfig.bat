@echo off
REM This script applies CMAKE list changes to /build
REM cleaning former build
call clean
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
                        echo Debug: ./reconfig or ./reconfig --Debug
                        echo Release: ./reconfig --Release
                        echo RelWithDebuInfo: ./reconfig --RelWithDebInfo
                        echo MinSizeRel: ./reconfig --MinSizeRel
                        exit /b 0
                    ) else (
                        echo Invalid flag
                        echo ^<Available Commands^>
                        echo Debug: ./reconfig or ./reconfig --Debug
                        echo Release: ./reconfig --Release
                        echo RelWithDebuInfo: ./reconfig --RelWithDebInfo
                        echo MinSizeRel: ./reconfig --MinSizeRel
                        exit /b 1
                    )
                )
            )
        )
    )
)

echo Configuring %BUILD_TYPE%
REM ==============================================================
REM Reconfigure CMake
REM ==============================================================
echo Running: cmake -S ..\.. -B ..\..\build -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DIS_INITIAL_SETUP=OFF -G "MinGW Makefiles"
call cmake -S ..\.. -B ..\..\build -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DIS_INITIAL_SETUP=OFF -G "MinGW Makefiles"
if errorlevel 1 (
    echo CMake configuration failed!
    exit /b 1
)