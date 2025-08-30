IF EXIST ..\..\build-release (
    rmdir /s /q ..\..\build-release
    echo build release directory removed.
) ELSE (
    echo build release directory does not exist.
)
call cmake -S ..\.. -B ..\..\build-release -DCMAKE_BUILD_TYPE=Release -DENABLE_UPDATE_DEPS=ON
