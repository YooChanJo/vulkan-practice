@echo off
REM WARNING: Using Setup command includes process of dep update, which is unnecessary after initial setup
REM If setup has already successfully been executed, use ./reconfig instead.

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
REM Remove existing build directory
REM ==============================================================
IF EXIST ..\..\external (
    rmdir /s /q ..\..\external
    echo External directory removed.
) ELSE (
    echo External directory does not exist.
)

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
                        echo Debug: ./setup or ./setup --Debug
                        echo Release: ./setup --Release
                        echo RelWithDebuInfo: ./setup --RelWithDebInfo
                        echo MinSizeRel: ./setup --MinSizeRel
                        exit /b 0
                    ) else (
                        echo Invalid flag
                        echo ^<Available Commands^>
                        echo Debug: ./setup or ./setup --Debug
                        echo Release: ./setup --Release
                        echo RelWithDebuInfo: ./setup --RelWithDebInfo
                        echo MinSizeRel: ./setup --MinSizeRel
                        exit /b 1
                    )
                )
            )
        )
    )
)
echo Configuring %BUILD_TYPE%

REM ==============================================================
REM Configure CMake with Vulkan dependencies
REM ==============================================================
echo Running: cmake -S ..\.. -B ..\..\build -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DIS_INITIAL_SETUP=ON -G "MinGW Makefiles"
call cmake -S ..\.. -B ..\..\build -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DIS_INITIAL_SETUP=ON -G "MinGW Makefiles"
if errorlevel 1 (
    echo CMake configuration failed!
    exit /b 1
)
echo CMake configuration completed successfully.
