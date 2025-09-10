cmake_minimum_required(VERSION 3.22.1)
project(VulkanPractice)

set(CMAKE_CXX_STANDARD 17)

# -------------------------------------------------------------
# Detect generator type
# -------------------------------------------------------------
get_property(IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
# -------------------------------------------------------------
# Set Runtime Library for msvc
# -------------------------------------------------------------
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

# -------------------------------------------------------------
# Create Output Directory Prefix
# -------------------------------------------------------------
if(IS_MULTI_CONFIG)
    if(DEFINED CMAKE_VS_PLATFORM_NAME)
        set(PLATFORM_FOLDER ${CMAKE_VS_PLATFORM_NAME})
    else()
        set(PLATFORM_FOLDER ${CMAKE_SYSTEM_PROCESSOR})
    endif()
    set(BIN_OUTPUT_DIRECTORY_PREFIX ${CMAKE_BINARY_DIR}/bin/$<CONFIG>/${PLATFORM_FOLDER})
else()
    if(DEFINED CMAKE_VS_PLATFORM_NAME)
        set(PLATFORM_FOLDER ${CMAKE_VS_PLATFORM_NAME})
    else()
        set(PLATFORM_FOLDER ${CMAKE_SYSTEM_PROCESSOR})
    endif()
    set(BIN_OUTPUT_DIRECTORY_PREFIX ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}/${PLATFORM_FOLDER})
endif()

# -------------------------------------------------------------
# Source files
# -------------------------------------------------------------
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.h")

# -------------------------------------------------------------
# Load Vulkan
# -------------------------------------------------------------
find_package(Vulkan REQUIRED)

# -------------------------------------------------------------
# Create main executable target
# -------------------------------------------------------------
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# -------------------------------------------------------------
# Detect compiler and architecture --> Further implementation is needed
# -------------------------------------------------------------
# Detect platform and compiler info
message(STATUS "Creating CMake generator flags...")
set(GENERATOR "")
set(ARCH_FLAG "")
set(OTHER_FLAGS "")

# Decide generator
if(MSVC)
    # Visual Studio
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(ARCH_FLAG "x64")
    else()
        set(ARCH_FLAG "Win32")
    endif()
    set(GENERATOR "Visual Studio 17 2022")  # adjust VS version
elseif(MINGW)
    # MinGW on Windows: usually use Ninja if available
    find_program(NINJA_EXE ninja)
    if(NINJA_EXE)
        set(GENERATOR "Ninja")
    else()
        set(GENERATOR "MinGW Makefiles")
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    # Unix-like (Linux/macOS)
    find_program(NINJA_EXE ninja)
    if(NINJA_EXE)
        set(GENERATOR "Ninja")
    else()
        set(GENERATOR "Unix Makefiles")
    endif()
endif()
# Optional extra flags depending on platform
if(APPLE)
    set(OTHER_FLAGS "-DCMAKE_OSX_ARCHITECTURES=x86_64;arm64")
endif()
# Construct command
set(COMMON_CMAKE_COMMAND_ARGS
    -G "${GENERATOR}"
)
if(ARCH_FLAG)
    list(APPEND COMMON_CMAKE_COMMAND_ARGS -A ${ARCH_FLAG})
endif()
if(OTHER_FLAGS)
    list(APPEND COMMON_CMAKE_COMMAND_ARGS ${OTHER_FLAGS})
endif()
# Set MSVC runtime library only if MSVC
if(MSVC)
    list(APPEND COMMON_CMAKE_COMMAND_ARGS "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()
message(STATUS "COMMON_CMAKE_COMMAND_ARGS set to ${COMMON_CMAKE_COMMAND_ARGS}")

# -------------------------------------------------------------
# Build External Dependencies as Stand Alone
# -------------------------------------------------------------
if(IS_INITIAL_SETUP)
    find_package(Git REQUIRED)
    find_package(Python3 REQUIRED)
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/external"
    )
    message(WARNING "Current Process Requires Wifi")
    execute_process(
        COMMAND ${GIT_EXECUTABLE} clone "https://github.com/glfw/glfw.git"
        COMMAND ${GIT_EXECUTABLE} clone "https://github.com/g-truc/glm.git"
        COMMAND ${GIT_EXECUTABLE} clone "https://github.com/gabime/spdlog.git"
        COMMAND ${GIT_EXECUTABLE} clone "https://github.com/google/shaderc.git"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/external"
    )
    execute_process(
        COMMAND ${Python3_EXECUTABLE} "shaderc/utils/git-sync-deps"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/external"
    )
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/external/glfw/include/GLFW" "${CMAKE_SOURCE_DIR}/external/includes/GLFW"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/external/glm/glm" "${CMAKE_SOURCE_DIR}/external/includes/glm"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/external/spdlog/include/spdlog" "${CMAKE_SOURCE_DIR}/external/includes/spdlog"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/external/shaderc/libshaderc/include/shaderc" "${CMAKE_SOURCE_DIR}/external/includes/shaderc"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/external"
    )
    message(WARNING "Wifi No Longer Required")
    if(IS_MULTI_CONFIG)
        # glfw
        execute_process(
            COMMAND ${CMAKE_COMMAND} -S "${CMAKE_SOURCE_DIR}/external/glfw"
                                    -B "${CMAKE_SOURCE_DIR}/external/glfw/build"
                                    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG=${CMAKE_SOURCE_DIR}/external/glfw-extract/Debug
                                    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE=${CMAKE_SOURCE_DIR}/external/glfw-extract/Release
                                    ${COMMON_CMAKE_COMMAND_ARGS}
                                    -DGLFW_BUILD_EXAMPLES=OFF
                                    -DGLFW_BUILD_TESTS=OFF
                                    -DGLFW_BUILD_DOCS=OFF
                                    # -DGLFW_INSTALL=OFF
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/external/glfw"
        )
        execute_process(
            COMMAND ${CMAKE_COMMAND} --build "${CMAKE_SOURCE_DIR}/external/glfw/build" --parallel --config Debug
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/external/glfw"
        )
        execute_process(
            COMMAND ${CMAKE_COMMAND} --build "${CMAKE_SOURCE_DIR}/external/glfw/build" --parallel --config Release
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/external/glfw"
        )
        # glm
        execute_process(
            COMMAND ${CMAKE_COMMAND} -S "${CMAKE_SOURCE_DIR}/external/glm"
                                    -B "${CMAKE_SOURCE_DIR}/external/glm/build"
                                    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG=${CMAKE_SOURCE_DIR}/external/glm-extract/Debug
                                    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE=${CMAKE_SOURCE_DIR}/external/glm-extract/Release
                                    ${COMMON_CMAKE_COMMAND_ARGS}
                                    # -DGLM_BUILD_INSTALL=OFF
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/external/glm"
        )
        execute_process(
            COMMAND ${CMAKE_COMMAND} --build "${CMAKE_SOURCE_DIR}/external/glm/build" --parallel --config Debug
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/external/glm"
        )
        execute_process(
            COMMAND ${CMAKE_COMMAND} --build "${CMAKE_SOURCE_DIR}/external/glm/build" --parallel --config Release
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/external/glm"
        )
        # spdlog
        execute_process(
            COMMAND ${CMAKE_COMMAND} -S "${CMAKE_SOURCE_DIR}/external/spdlog"
                                    -B "${CMAKE_SOURCE_DIR}/external/spdlog/build"
                                    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG=${CMAKE_SOURCE_DIR}/external/spdlog-extract/Debug
                                    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE=${CMAKE_SOURCE_DIR}/external/spdlog-extract/Release
                                    ${COMMON_CMAKE_COMMAND_ARGS}
                                    -DSPDLOG_BUILD_EXAMPLE=OFF
                                    # -DSPDLOG_INSTALL=OFF
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/external/spdlog"
        )
        execute_process(
            COMMAND ${CMAKE_COMMAND} --build "${CMAKE_SOURCE_DIR}/external/spdlog/build" --parallel --config Debug
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/external/spdlog"
        )
        execute_process(
            COMMAND ${CMAKE_COMMAND} --build "${CMAKE_SOURCE_DIR}/external/spdlog/build" --parallel --config Release
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/external/spdlog"
        )
        # shaderc
        execute_process(
            COMMAND ${CMAKE_COMMAND} -S "${CMAKE_SOURCE_DIR}/external/shaderc"
                                    -B "${CMAKE_SOURCE_DIR}/external/shaderc/build"
                                    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG=${CMAKE_SOURCE_DIR}/external/shaderc-extract/Debug
                                    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE=${CMAKE_SOURCE_DIR}/external/shaderc-extract/Release
                                    ${COMMON_CMAKE_COMMAND_ARGS}
                                    -DSHADERC_ENABLE_SHARED_CRT=ON # For shared crt build in msvc
                                    -DSHADERC_ENABLE_WGSL_OUTPUT=OFF # No Web GL
                                    # -DSHADERC_SKIP_INSTALL=ON # Skips any installs
                                    -DSHADERC_SKIP_TESTS=ON
                                    -DSHADERC_SKIP_EXAMPLES=ON
                                    -DSHADERC_SKIP_EXECUTABLES=ON
                                    -DSHADERC_SKIP_COPYRIGHT_CHECK=ON
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/external/shaderc"
        )
        execute_process(
            COMMAND ${CMAKE_COMMAND} --build "${CMAKE_SOURCE_DIR}/external/shaderc/build" --parallel --config Debug
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/external/shaderc"
        )
        execute_process(
            COMMAND ${CMAKE_COMMAND} --build "${CMAKE_SOURCE_DIR}/external/shaderc/build" --parallel --config Release
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/external/shaderc"
        )
    else()
        # glfw
        execute_process(
            COMMAND ${CMAKE_COMMAND} -S "${CMAKE_SOURCE_DIR}/external/glfw"
                                    -B "${CMAKE_SOURCE_DIR}/external/glfw/build-debug"
                                    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=${CMAKE_SOURCE_DIR}/external/glfw-extract/Debug
                                    -DCMAKE_BUILD_TYPE=Debug
                                    ${COMMON_CMAKE_COMMAND_ARGS}
                                    -DGLFW_BUILD_EXAMPLES=OFF
                                    -DGLFW_BUILD_TESTS=OFF
                                    -DGLFW_BUILD_DOCS=OFF
                                    # -DGLFW_INSTALL=OFF
            COMMAND ${CMAKE_COMMAND} -S "${CMAKE_SOURCE_DIR}/external/glfw"
                                    -B "${CMAKE_SOURCE_DIR}/external/glfw/build-release"
                                    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=${CMAKE_SOURCE_DIR}/external/glfw-extract/Release
                                    -DCMAKE_BUILD_TYPE=Release
                                    ${COMMON_CMAKE_COMMAND_ARGS}
                                    -DGLFW_BUILD_EXAMPLES=OFF
                                    -DGLFW_BUILD_TESTS=OFF
                                    -DGLFW_BUILD_DOCS=OFF
                                    # -DGLFW_INSTALL=OFF
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/external/glfw"
        )
        execute_process(
            COMMAND ${CMAKE_COMMAND} --build "${CMAKE_SOURCE_DIR}/external/glfw/build-debug" --parallel
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/external/glfw"
        )
        execute_process(
            COMMAND ${CMAKE_COMMAND} --build "${CMAKE_SOURCE_DIR}/external/glfw/build-release" --parallel
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/external/glfw"
        )
        # glm
        execute_process(
            COMMAND ${CMAKE_COMMAND} -S "${CMAKE_SOURCE_DIR}/external/glm"
                                    -B "${CMAKE_SOURCE_DIR}/external/glm/build-debug"
                                    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=${CMAKE_SOURCE_DIR}/external/glm-extract/Debug
                                    -DCMAKE_BUILD_TYPE=Debug
                                    ${COMMON_CMAKE_COMMAND_ARGS}
                                    # -DGLM_BUILD_INSTALL=OFF
            COMMAND ${CMAKE_COMMAND} -S "${CMAKE_SOURCE_DIR}/external/glm"
                                    -B "${CMAKE_SOURCE_DIR}/external/glm/build-release"
                                    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=${CMAKE_SOURCE_DIR}/external/glm-extract/Release
                                    -DCMAKE_BUILD_TYPE=Release
                                    ${COMMON_CMAKE_COMMAND_ARGS}
                                    # -DGLM_BUILD_INSTALL=OFF
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/external/glm"
        )
        execute_process(
            COMMAND ${CMAKE_COMMAND} --build "${CMAKE_SOURCE_DIR}/external/glm/build-debug" --parallel
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/external/glm"
        )
        execute_process(
            COMMAND ${CMAKE_COMMAND} --build "${CMAKE_SOURCE_DIR}/external/glm/build-release" --parallel
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/external/glm"
        )
        # spdlog
        execute_process(
            COMMAND ${CMAKE_COMMAND} -S "${CMAKE_SOURCE_DIR}/external/spdlog"
                                    -B "${CMAKE_SOURCE_DIR}/external/spdlog/build-debug"
                                    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=${CMAKE_SOURCE_DIR}/external/spdlog-extract/Debug
                                    -DCMAKE_BUILD_TYPE=Debug
                                    ${COMMON_CMAKE_COMMAND_ARGS}
                                    -DSPDLOG_BUILD_EXAMPLE=OFF
                                    # -DSPDLOG_INSTALL=OFF
            COMMAND ${CMAKE_COMMAND} -S "${CMAKE_SOURCE_DIR}/external/spdlog"
                                    -B "${CMAKE_SOURCE_DIR}/external/spdlog/build-release"
                                    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=${CMAKE_SOURCE_DIR}/external/spdlog-extract/Release
                                    -DCMAKE_BUILD_TYPE=Release
                                    ${COMMON_CMAKE_COMMAND_ARGS}
                                    -DSPDLOG_BUILD_EXAMPLE=OFF
                                    # -DSPDLOG_INSTALL=OFF
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/external/spdlog"
        )
        execute_process(
            COMMAND ${CMAKE_COMMAND} --build "${CMAKE_SOURCE_DIR}/external/spdlog/build-debug" --parallel
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/external/spdlog"
        )
        execute_process(
            COMMAND ${CMAKE_COMMAND} --build "${CMAKE_SOURCE_DIR}/external/spdlog/build-release" --parallel
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/external/spdlog"
        )
        # shaderc
        execute_process(
            COMMAND ${CMAKE_COMMAND} -S "${CMAKE_SOURCE_DIR}/external/shaderc"
                                    -B "${CMAKE_SOURCE_DIR}/external/shaderc/build-debug"
                                    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=${CMAKE_SOURCE_DIR}/external/shaderc-extract/Debug
                                    -DCMAKE_BUILD_TYPE=Debug
                                    ${COMMON_CMAKE_COMMAND_ARGS}
                                    # -DSHADERC_ENABLE_SHARED_CRT=ON # For shared crt build in msvc
                                    -DSHADERC_ENABLE_WGSL_OUTPUT=OFF # No Web GL
                                    # -DSHADERC_SKIP_INSTALL=ON # Skips any installs
                                    -DSHADERC_SKIP_TESTS=ON
                                    -DSHADERC_SKIP_EXAMPLES=ON
                                    -DSHADERC_SKIP_EXECUTABLES=ON
                                    -DSHADERC_SKIP_COPYRIGHT_CHECK=ON
            COMMAND ${CMAKE_COMMAND} -S "${CMAKE_SOURCE_DIR}/external/shaderc"
                                    -B "${CMAKE_SOURCE_DIR}/external/shaderc/build-release"
                                    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=${CMAKE_SOURCE_DIR}/external/shaderc-extract/Release
                                    -DCMAKE_BUILD_TYPE=Release
                                    ${COMMON_CMAKE_COMMAND_ARGS}
                                    -DSHADERC_ENABLE_SHARED_CRT=ON # For shared crt build in msvc
                                    -DSHADERC_ENABLE_WGSL_OUTPUT=OFF # No Web GL
                                    # -DSHADERC_SKIP_INSTALL=ON # Skips any installs
                                    -DSHADERC_SKIP_TESTS=ON
                                    -DSHADERC_SKIP_EXAMPLES=ON
                                    -DSHADERC_SKIP_EXECUTABLES=ON
                                    -DSHADERC_SKIP_COPYRIGHT_CHECK=ON
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/external/shaderc"
        )
        execute_process(
            COMMAND ${CMAKE_COMMAND} --build "${CMAKE_SOURCE_DIR}/external/shaderc/build-debug" --parallel
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/external/shaderc"
        )
        execute_process(
            COMMAND ${CMAKE_COMMAND} --build "${CMAKE_SOURCE_DIR}/external/shaderc/build-release" --parallel
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/external/shaderc"
        )
    endif()
    if(MSVC)
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/external/glfw-extract/Debug/glfw3.lib" "${CMAKE_SOURCE_DIR}/external/builds/Debug/glfw3.lib"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/external/glfw-extract/Debug/glfw3.pdb" "${CMAKE_SOURCE_DIR}/external/builds/Debug/glfw3.pdb"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/external/glfw-extract/Release/glfw3.lib" "${CMAKE_SOURCE_DIR}/external/builds/Release/glfw3.lib"
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/external/glfw"
        )
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/external/glm-extract/Debug/glm.lib" "${CMAKE_SOURCE_DIR}/external/builds/Debug/glm.lib"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/external/glm-extract/Debug/glm.pdb" "${CMAKE_SOURCE_DIR}/external/builds/Debug/glm.pdb"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/external/glm-extract/Release/glm.lib" "${CMAKE_SOURCE_DIR}/external/builds/Release/glm.lib"
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/external/glm"
        )
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/external/spdlog-extract/Debug/spdlogd.lib" "${CMAKE_SOURCE_DIR}/external/builds/Debug/spdlogd.lib"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/external/spdlog-extract/Debug/spdlogd.pdb" "${CMAKE_SOURCE_DIR}/external/builds/Debug/spdlogd.pdb"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/external/spdlog-extract/Release/spdlog.lib" "${CMAKE_SOURCE_DIR}/external/builds/Release/spdlog.lib"
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/external/spdlog"
        )
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/external/shaderc-extract/Debug/shaderc_combined.lib" "${CMAKE_SOURCE_DIR}/external/builds/Debug/shaderc_combined.lib"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/external/shaderc-extract/Debug/shaderc.pdb" "${CMAKE_SOURCE_DIR}/external/builds/Debug/shaderc.pdb"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/external/shaderc-extract/Release/shaderc_combined.lib" "${CMAKE_SOURCE_DIR}/external/builds/Release/shaderc_combined.lib"
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/external/shaderc"
        )
    else()
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/external/glfw-extract/Debug/libglfw3.a" "${CMAKE_SOURCE_DIR}/external/builds/Debug/libglfw3.a"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/external/glfw-extract/Release/libglfw3.a" "${CMAKE_SOURCE_DIR}/external/builds/Release/libglfw3.a"
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/external/glfw"
        )
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/external/glm-extract/Debug/libglm.a" "${CMAKE_SOURCE_DIR}/external/builds/Debug/libglm.a"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/external/glm-extract/Release/libglm.a" "${CMAKE_SOURCE_DIR}/external/builds/Release/libglm.a"
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/external/glm"
        )
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/external/spdlog-extract/Debug/libspdlogd.a" "${CMAKE_SOURCE_DIR}/external/builds/Debug/libspdlogd.a"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/external/spdlog-extract/Release/libspdlog.a" "${CMAKE_SOURCE_DIR}/external/builds/Release/libspdlog.a"
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/external/spdlog"
        )
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/external/shaderc-extract/Debug/libshaderc_combined.a" "${CMAKE_SOURCE_DIR}/external/builds/Debug/libshaderc_combined.a"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/external/shaderc-extract/Release/libshaderc_combined.a" "${CMAKE_SOURCE_DIR}/external/builds/Release/libshaderc_combined.a"
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/external/shaderc"
        )
    endif()
    # cleanup
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_SOURCE_DIR}/external/glfw"
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_SOURCE_DIR}/external/glfw-extract"
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_SOURCE_DIR}/external/glm"
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_SOURCE_DIR}/external/glm-extract"
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_SOURCE_DIR}/external/spdlog"
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_SOURCE_DIR}/external/spdlog-extract"
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_SOURCE_DIR}/external/shaderc"
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_SOURCE_DIR}/external/shaderc-extract"
    )
endif()

# -------------------------------------------------------------
# Includes & Libraries
# -------------------------------------------------------------
set(my_includes external/includes)

find_library(GLFW_LIB_DEBUG NAMES glfw glfw3 PATHS ${CMAKE_SOURCE_DIR}/external/builds/Debug)
find_library(GLM_LIB_DEBUG NAMES glm PATHS ${CMAKE_SOURCE_DIR}/external/builds/Debug)
find_library(SPDLOG_LIB_DEBUG NAMES spdlog spdlogd PATHS ${CMAKE_SOURCE_DIR}/external/builds/Debug)
find_library(SHADERC_LIB_DEBUG NAMES shaderc_combined PATHS ${CMAKE_SOURCE_DIR}/external/builds/Debug)
message(STATUS "Found ${GLFW_LIB_DEBUG};${SPDLOG_LIB_DEBUG};${GLM_LIB_DEBUG};${SHADERC_LIB_DEBUG}")

find_library(GLFW_LIB_RELEASE NAMES glfw glfw3 PATHS ${CMAKE_SOURCE_DIR}/external/builds/Release)
find_library(GLM_LIB_RELEASE NAMES glm PATHS ${CMAKE_SOURCE_DIR}/external/builds/Release)
find_library(SPDLOG_LIB_RELEASE NAMES spdlog spdlogd PATHS ${CMAKE_SOURCE_DIR}/external/builds/Release)
find_library(SHADERC_LIB_RELEASE NAMES shaderc_combined PATHS ${CMAKE_SOURCE_DIR}/external/builds/Release)
message(STATUS "Found ${GLFW_LIB_RELEASE};${SPDLOG_LIB_RELEASE};${GLM_LIB_RELEASE};${SHADERC_LIB_RELEASE}")

# Create an imported target for each dependency
add_library(GLFW::GLFW UNKNOWN IMPORTED)
add_library(GLM::GLM UNKNOWN IMPORTED)
add_library(SPDLOG::SPDLOG UNKNOWN IMPORTED)
add_library(SHADERC::COMBINED UNKNOWN IMPORTED)

# Assign config-specific locations
set_target_properties(GLFW::GLFW PROPERTIES
    IMPORTED_LOCATION_DEBUG        "${GLFW_LIB_DEBUG}"
    IMPORTED_LOCATION_RELEASE      "${GLFW_LIB_RELEASE}"
    IMPORTED_LOCATION_RELWITHDEBINFO "${GLFW_LIB_RELEASE}"
    IMPORTED_LOCATION_MINSIZEREL   "${GLFW_LIB_RELEASE}"
)

set_target_properties(SPDLOG::SPDLOG PROPERTIES
    IMPORTED_LOCATION_DEBUG        "${SPDLOG_LIB_DEBUG}"
    IMPORTED_LOCATION_RELEASE      "${SPDLOG_LIB_RELEASE}"
    IMPORTED_LOCATION_RELWITHDEBINFO "${SPDLOG_LIB_RELEASE}"
    IMPORTED_LOCATION_MINSIZEREL   "${SPDLOG_LIB_RELEASE}"
)

set_target_properties(GLM::GLM PROPERTIES
    IMPORTED_LOCATION_DEBUG        "${GLM_LIB_DEBUG}"
    IMPORTED_LOCATION_RELEASE      "${GLM_LIB_RELEASE}"
    IMPORTED_LOCATION_RELWITHDEBINFO "${GLM_LIB_RELEASE}"
    IMPORTED_LOCATION_MINSIZEREL   "${GLM_LIB_RELEASE}"
)

set_target_properties(SHADERC::COMBINED PROPERTIES
    IMPORTED_LOCATION_DEBUG        "${SHADERC_LIB_DEBUG}"
    IMPORTED_LOCATION_RELEASE      "${SHADERC_LIB_RELEASE}"
    IMPORTED_LOCATION_RELWITHDEBINFO "${SHADERC_LIB_RELEASE}"
    IMPORTED_LOCATION_MINSIZEREL   "${SHADERC_LIB_RELEASE}"
)

set(my_libs GLFW::GLFW SPDLOG::SPDLOG GLM::GLM SHADERC::COMBINED $<$<BOOL:${MINGW}>:ws2_32>)

# -------------------------------------------------------------
# Add Target Properties etc.
# -------------------------------------------------------------
target_include_directories(${PROJECT_NAME} PRIVATE ${my_includes} ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${my_libs} ${Vulkan_LIBRARIES})
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:INCLUDE_DEBUG_INFO> SHADER_DIR="${CMAKE_SOURCE_DIR}/assets/shaders")

target_precompile_headers(${PROJECT_NAME} PRIVATE src/pch.h)
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${BIN_OUTPUT_DIRECTORY_PREFIX}/${PROJECT_NAME}
)

# -------------------------------------------------------------
# Visual Studio startup project
# -------------------------------------------------------------
if(MSVC)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
endif()

# -------------------------------------------------------------
# Compiler options
# -------------------------------------------------------------
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:-g -O0 -Wall -Wextra -Wshadow -Wformat=2 -Wpedantic -DDEBUG>
        $<$<CONFIG:Release>:-O3 -march=native -flto -funroll-loops -DNDEBUG -fstrict-aliasing -ffast-math>
        $<$<CONFIG:RelWithDebInfo>:-O2 -g -DNDEBUG>
        $<$<CONFIG:MinSizeRel>:-Os -DNDEBUG>
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:-flto>
        $<$<CONFIG:RelWithDebInfo>:-flto>
    )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:/Zi /Od /W4 /DDEBUG /EHsc /permissive->
        $<$<CONFIG:Release>:/O2 /Ob2 /GL /DNDEBUG /EHsc /fp:fast>
        $<$<CONFIG:RelWithDebInfo>:/O2 /Zi /DNDEBUG /EHsc>
        $<$<CONFIG:MinSizeRel>:/O1 /Os /DNDEBUG /EHsc>
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:/LTCG>
        $<$<CONFIG:RelWithDebInfo>:/LTCG>
    )
endif()
if(MSVC)
    target_compile_options(${PROJECT_NAME} PUBLIC /utf-8)
endif()

# -------------------------------------------------------------
# Enable LTO for release-like builds
# -------------------------------------------------------------
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
# set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO TRUE)