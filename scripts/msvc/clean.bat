@echo off
REM ==============================================================
REM Remove existing build directory
REM ==============================================================
IF EXIST ..\..\build (
    rmdir /s /q ..\..\build
    echo Build directory removed.
) ELSE (
    echo Build directory does not exist.
)