@echo off
setlocal enabledelayedexpansion

:: The root directory where you want to start searching
set "rootDir=..\..\build\bin"

:: The file name (with extension) you are searching for
set "fileName=VulkanPractice.exe"

set "foundPath="

for /r "%rootDir%" %%F in (*) do (
    if /i "%%~nxF"=="%fileName%" (
        set "foundPath=%%F"
        goto :found
    )
)

:found
if defined foundPath (
    %foundPath%
) else (
    echo File not found
)
endlocal
