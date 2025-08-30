IF EXIST ..\..\build (
    rmdir /s /q ..\..\build
    echo build directory removed.
) ELSE (
    echo build directory does not exist.
)
call cmake -S ..\.. -B ..\..\build -DCMAKE_BUILD_TYPE=Debug -DENABLE_UPDATE_DEPS=ON
