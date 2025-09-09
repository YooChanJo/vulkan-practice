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

echo Building %BUILD_TYPE%
echo Running: cmake --build ..\..\build --parallel --config %BUILD_TYPE%
call cmake --build ..\..\build --parallel --config %BUILD_TYPE%
if errorlevel 1 (
    echo Build failed!
    exit /b 1
)
echo Build finished successfully!
