@echo off
echo Running: cmake --build ..\..\build --parallel
call cmake --build ..\..\build --parallel
if errorlevel 1 (
    echo Build failed!
    exit /b 1
)
echo Build finished successfully!
