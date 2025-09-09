@echo off
REM This script removes all vendors and reinstalls them
pushd ..\..\
REM ==============================================================
REM Remove existing build directory
REM ==============================================================
IF EXIST vendor (
    rmdir /s /q vendor
    echo vendor directory removed.
) ELSE (
    echo vendor directory does not exist.
)
REM git submodule deinit -f .
git submodule update --init --recursive
popd