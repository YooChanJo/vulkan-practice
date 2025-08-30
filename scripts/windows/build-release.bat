call cmake -S ..\.. -B ..\..\build-release -DCMAKE_BUILD_TYPE=Release
call cmake --build ..\..\build-release --parallel --config Release