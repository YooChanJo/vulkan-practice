@echo off
REM ==============================================================
REM Run the executable based on argument
REM ==============================================================
set RUN_TYPE=Debug
if /I "%1"=="--Debug" (
    set RUN_TYPE=Debug
) else (
    if /I "%1"=="" (
        set RUN_TYPE=Debug
    ) else (
        if /I "%1"=="--Release" (
            set RUN_TYPE=Release
        ) else (
            if /I "%1"=="--RelWithDebInfo" (
                set RUN_TYPE=RelWithDebInfo
            ) else (
                if /I "%1"=="--MinSizeRel" (
                    set RUN_TYPE=MinSizeRel
                ) else (
                    if /I "%1"=="--help" (
                        echo ^<Available Commands^>
                        echo Debug: ./run or ./run --Debug
                        echo Release: ./run --Release
                        echo RelWithDebuInfo: ./run --RelWithDebInfo
                        echo MinSizeRel: ./run --MinSizeRel
                        exit /b 0
                    ) else (
                        echo Invalid flag
                        echo ^<Available Commands^>
                        echo Debug: ./run or ./run --Debug
                        echo Release: ./run --Release
                        echo RelWithDebuInfo: ./run --RelWithDebInfo
                        echo MinSizeRel: ./run --MinSizeRel
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
call ..\..\build\bin\%RUN_TYPE%\%ARCH%\VulkanPractice\VulkanPractice.exe