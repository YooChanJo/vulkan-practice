cmake_minimum_required(VERSION 3.22.1)
project(VulkanPractice)

set(CMAKE_CXX_STANDARD 17)

# -------------------------------------------------------------
# Detect generator type
# -------------------------------------------------------------
get_property(IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

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
    set(LIB_OUTPUT_DIRECTORY_PREFIX ${CMAKE_BINARY_DIR}/lib/$<CONFIG>/${PLATFORM_FOLDER})
else()
    if(DEFINED CMAKE_VS_PLATFORM_NAME)
        set(PLATFORM_FOLDER ${CMAKE_VS_PLATFORM_NAME})
    else()
        set(PLATFORM_FOLDER ${CMAKE_SYSTEM_PROCESSOR})
    endif()
    set(BIN_OUTPUT_DIRECTORY_PREFIX ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}/${PLATFORM_FOLDER})
    set(LIB_OUTPUT_DIRECTORY_PREFIX ${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE}/${PLATFORM_FOLDER})
endif()
set(COPY_FROM_LIB_DIRECTORY ${CMAKE_SOURCE_DIR}/vendor/builds/lib)
set(COPY_FROM_BIN_DIRECTORY ${CMAKE_SOURCE_DIR}/vendor/builds/bin) # Currently not copying from
set(COPY_TO_LIB_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

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
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    # On Unix-like, use Ninja if available
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

if(MSVC)
    list(APPEND COMMON_CMAKE_COMMAND_ARGS "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

message("COMMON_CMAKE_COMMAND_ARGS set to ${COMMON_CMAKE_COMMAND_ARGS}")

# -------------------------------------------------------------
# Build External Dependencies as Stand Alone
# -------------------------------------------------------------
if(IS_INITIAL_SETUP)
    # Remove previous build
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_SOURCE_DIR}/vendor/builds"
    )
    if(IS_MULTI_CONFIG)
        message(WARNING "Warning: Every possible configuration is built (Debug, Release, RelWithDebInfo, MinSizeRel). This may take some time.")
        # Vulkan Validation Layers
        message(WARNING "Wifi is required for this process, please keep device online.")
        message(STATUS "Configuring Vulkan Validation Layers Debug Build")
        execute_process(
            COMMAND ${CMAKE_COMMAND} -S "${CMAKE_SOURCE_DIR}/vendor/Vulkan-Validationlayers"
                                    -B "${CMAKE_SOURCE_DIR}/vendor/Vulkan-Validationlayers/build/Debug"
                                    ${COMMON_CMAKE_COMMAND_ARGS}
                                    -DCMAKE_BUILD_TYPE=Debug
                                    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG=${COPY_FROM_LIB_DIRECTORY}/Debug/${PLATFORM_FOLDER}
                                    -DCMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG=${COPY_FROM_LIB_DIRECTORY}/Debug/${PLATFORM_FOLDER}
                                    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG=${COPY_FROM_BIN_DIRECTORY}/Debug/${PLATFORM_FOLDER}
                                    -DUPDATE_DEPS=ON
                                    "-DCMAKE_PREFIX_PATH=${CMAKE_SOURCE_DIR}/vendor/Vulkan-Validationlayers/external/Debug"  
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/vendor/Vulkan-Validationlayers"
        )
        message(STATUS "Configuring Vulkan Validation Layers RelWithDebInfo Build")
        execute_process(
            COMMAND ${CMAKE_COMMAND} -S "${CMAKE_SOURCE_DIR}/vendor/Vulkan-Validationlayers"
                                    -B "${CMAKE_SOURCE_DIR}/vendor/Vulkan-Validationlayers/build/RelWithDebInfo"
                                    ${COMMON_CMAKE_COMMAND_ARGS}
                                    -DCMAKE_BUILD_TYPE=RelWithDebInfo
                                    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO=${COPY_FROM_LIB_DIRECTORY}/RelWithDebInfo/${PLATFORM_FOLDER}
                                    -DCMAKE_LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO=${COPY_FROM_LIB_DIRECTORY}/RelWithDebInfo/${PLATFORM_FOLDER}
                                    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO=${COPY_FROM_BIN_DIRECTORY}/RelWithDebInfo/${PLATFORM_FOLDER}
                                    -DUPDATE_DEPS=ON
                                    "-DCMAKE_PREFIX_PATH=${CMAKE_SOURCE_DIR}/vendor/Vulkan-Validationlayers/external/RelWithDebInfo"  
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/vendor/Vulkan-Validationlayers"
        )
        # message(STATUS "Updating third parties for shaderc")
        # find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
        # execute_process(
        #     COMMAND ${Python3_EXECUTABLE} "${CMAKE_SOURCE_DIR}/vendor/shaderc/utils/git-sync-deps"
        #     WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/vendor/shaderc"
        #     RESULT_VARIABLE Pyresult
        #     OUTPUT_VARIABLE Pyout
        #     ERROR_VARIABLE Pyerr
        # )
        # message(STATUS "Exit code: ${Pyresult}")
        # message(STATUS "Stdout:\n${Pyout}")
        # if(NOT "${Pyerr}" STREQUAL "") # If Error exists
        #     message(STATUS "Python stderr:\n${Pyerr}")
        # endif()
        message(WARNING "Wifi no longer required.")
        # Vulkan Validation Layers
        message(STATUS "Building Vulkan Validation Layers")
        execute_process(
            COMMAND ${CMAKE_COMMAND} --build "${CMAKE_SOURCE_DIR}/vendor/Vulkan-Validationlayers/build/Debug" --parallel --config Debug
            COMMAND ${CMAKE_COMMAND} --build "${CMAKE_SOURCE_DIR}/vendor/Vulkan-Validationlayers/build/RelWithDebInfo" --parallel --config RelWithDebInfo
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/vendor/Vulkan-Validationlayers"
        )
        # shaderc
        # message(STATUS "Building shaderc")
        # execute_process(
        #     COMMAND ${CMAKE_COMMAND} -S "${CMAKE_SOURCE_DIR}/vendor/shaderc"
        #                             -B "${CMAKE_SOURCE_DIR}/vendor/shaderc/build"
        #                             ${COMMON_CMAKE_COMMAND_ARGS}
        #                             -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG=${COPY_FROM_LIB_DIRECTORY}/Debug/${PLATFORM_FOLDER}
        #                             -DCMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG=${COPY_FROM_LIB_DIRECTORY}/Debug/${PLATFORM_FOLDER}
        #                             -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG=${COPY_FROM_BIN_DIRECTORY}/Debug/${PLATFORM_FOLDER}

        #                             -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE=${COPY_FROM_LIB_DIRECTORY}/Release/${PLATFORM_FOLDER}
        #                             -DCMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE=${COPY_FROM_LIB_DIRECTORY}/Release/${PLATFORM_FOLDER}
        #                             -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE=${COPY_FROM_BIN_DIRECTORY}/Release/${PLATFORM_FOLDER}

        #                             -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO=${COPY_FROM_LIB_DIRECTORY}/RelWithDebInfo/${PLATFORM_FOLDER}
        #                             -DCMAKE_LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO=${COPY_FROM_LIB_DIRECTORY}/RelWithDebInfo/${PLATFORM_FOLDER}
        #                             -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO=${COPY_FROM_BIN_DIRECTORY}/RelWithDebInfo/${PLATFORM_FOLDER}

        #                             -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL=${COPY_FROM_LIB_DIRECTORY}/MinSizeRel/${PLATFORM_FOLDER}
        #                             -DCMAKE_LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL=${COPY_FROM_LIB_DIRECTORY}/MinSizeRel/${PLATFORM_FOLDER}
        #                             -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL=${COPY_FROM_BIN_DIRECTORY}/MinSizeRel/${PLATFORM_FOLDER}

        #     COMMAND ${CMAKE_COMMAND} --build "${CMAKE_SOURCE_DIR}/vendor/shaderc/build" --parallel --config Debug
        #     COMMAND ${CMAKE_COMMAND} --build "${CMAKE_SOURCE_DIR}/vendor/shaderc/build" --parallel --config Release
        #     COMMAND ${CMAKE_COMMAND} --build "${CMAKE_SOURCE_DIR}/vendor/shaderc/build" --parallel --config RelWithDebInfo
        #     COMMAND ${CMAKE_COMMAND} --build "${CMAKE_SOURCE_DIR}/vendor/shaderc/build" --parallel --config MinSizeRel
        #     WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/vendor/shaderc"
        # )
        # GLFW
        message(STATUS "Building GLFW")
        execute_process(
            COMMAND ${CMAKE_COMMAND} -S "${CMAKE_SOURCE_DIR}/vendor/GLFW"
                                    -B "${CMAKE_SOURCE_DIR}/vendor/GLFW/build"
                                    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG=${COPY_FROM_LIB_DIRECTORY}/Debug/${PLATFORM_FOLDER}
                                    -DCMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG=${COPY_FROM_LIB_DIRECTORY}/Debug/${PLATFORM_FOLDER}
                                    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG=${COPY_FROM_BIN_DIRECTORY}/Debug/${PLATFORM_FOLDER}

                                    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE=${COPY_FROM_LIB_DIRECTORY}/Release/${PLATFORM_FOLDER}
                                    -DCMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE=${COPY_FROM_LIB_DIRECTORY}/Release/${PLATFORM_FOLDER}
                                    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE=${COPY_FROM_BIN_DIRECTORY}/Release/${PLATFORM_FOLDER}

                                    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO=${COPY_FROM_LIB_DIRECTORY}/RelWithDebInfo/${PLATFORM_FOLDER}
                                    -DCMAKE_LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO=${COPY_FROM_LIB_DIRECTORY}/RelWithDebInfo/${PLATFORM_FOLDER}
                                    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO=${COPY_FROM_BIN_DIRECTORY}/RelWithDebInfo/${PLATFORM_FOLDER}

                                    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL=${COPY_FROM_LIB_DIRECTORY}/MinSizeRel/${PLATFORM_FOLDER}
                                    -DCMAKE_LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL=${COPY_FROM_LIB_DIRECTORY}/MinSizeRel/${PLATFORM_FOLDER}
                                    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL=${COPY_FROM_BIN_DIRECTORY}/MinSizeRel/${PLATFORM_FOLDER}

                                    ${COMMON_CMAKE_COMMAND_ARGS}
            COMMAND ${CMAKE_COMMAND} --build "${CMAKE_SOURCE_DIR}/vendor/GLFW/build" --parallel --config Debug
            COMMAND ${CMAKE_COMMAND} --build "${CMAKE_SOURCE_DIR}/vendor/GLFW/build" --parallel --config Release
            COMMAND ${CMAKE_COMMAND} --build "${CMAKE_SOURCE_DIR}/vendor/GLFW/build" --parallel --config RelWithDebInfo
            COMMAND ${CMAKE_COMMAND} --build "${CMAKE_SOURCE_DIR}/vendor/GLFW/build" --parallel --config MinSizeRel
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/vendor/GLFW"
        )
        # GLM
        message(STATUS "Building GLM")
        execute_process(
            COMMAND ${CMAKE_COMMAND} -S "${CMAKE_SOURCE_DIR}/vendor/GLM"
                                    -B "${CMAKE_SOURCE_DIR}/vendor/GLM/build"
                                    ${COMMON_CMAKE_COMMAND_ARGS}
                                    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG=${COPY_FROM_LIB_DIRECTORY}/Debug/${PLATFORM_FOLDER}
                                    -DCMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG=${COPY_FROM_LIB_DIRECTORY}/Debug/${PLATFORM_FOLDER}
                                    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG=${COPY_FROM_BIN_DIRECTORY}/Debug/${PLATFORM_FOLDER}

                                    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE=${COPY_FROM_LIB_DIRECTORY}/Release/${PLATFORM_FOLDER}
                                    -DCMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE=${COPY_FROM_LIB_DIRECTORY}/Release/${PLATFORM_FOLDER}
                                    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE=${COPY_FROM_BIN_DIRECTORY}/Release/${PLATFORM_FOLDER}

                                    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO=${COPY_FROM_LIB_DIRECTORY}/RelWithDebInfo/${PLATFORM_FOLDER}
                                    -DCMAKE_LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO=${COPY_FROM_LIB_DIRECTORY}/RelWithDebInfo/${PLATFORM_FOLDER}
                                    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO=${COPY_FROM_BIN_DIRECTORY}/RelWithDebInfo/${PLATFORM_FOLDER}

                                    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL=${COPY_FROM_LIB_DIRECTORY}/MinSizeRel/${PLATFORM_FOLDER}
                                    -DCMAKE_LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL=${COPY_FROM_LIB_DIRECTORY}/MinSizeRel/${PLATFORM_FOLDER}
                                    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL=${COPY_FROM_BIN_DIRECTORY}/MinSizeRel/${PLATFORM_FOLDER}

            COMMAND ${CMAKE_COMMAND} --build "${CMAKE_SOURCE_DIR}/vendor/GLM/build" --parallel --config Debug
            COMMAND ${CMAKE_COMMAND} --build "${CMAKE_SOURCE_DIR}/vendor/GLM/build" --parallel --config Release
            COMMAND ${CMAKE_COMMAND} --build "${CMAKE_SOURCE_DIR}/vendor/GLM/build" --parallel --config RelWithDebInfo
            COMMAND ${CMAKE_COMMAND} --build "${CMAKE_SOURCE_DIR}/vendor/GLM/build" --parallel --config MinSizeRel
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/vendor/GLM"
        )
        # spdlog
        message(STATUS "Building spdlog")
        execute_process(
            COMMAND ${CMAKE_COMMAND} -S "${CMAKE_SOURCE_DIR}/vendor/spdlog"
                                    -B "${CMAKE_SOURCE_DIR}/vendor/spdlog/build"
                                    ${COMMON_CMAKE_COMMAND_ARGS}
                                    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG=${COPY_FROM_LIB_DIRECTORY}/Debug/${PLATFORM_FOLDER}
                                    -DCMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG=${COPY_FROM_LIB_DIRECTORY}/Debug/${PLATFORM_FOLDER}
                                    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG=${COPY_FROM_BIN_DIRECTORY}/Debug/${PLATFORM_FOLDER}

                                    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE=${COPY_FROM_LIB_DIRECTORY}/Release/${PLATFORM_FOLDER}
                                    -DCMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE=${COPY_FROM_LIB_DIRECTORY}/Release/${PLATFORM_FOLDER}
                                    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE=${COPY_FROM_BIN_DIRECTORY}/Release/${PLATFORM_FOLDER}

                                    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO=${COPY_FROM_LIB_DIRECTORY}/RelWithDebInfo/${PLATFORM_FOLDER}
                                    -DCMAKE_LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO=${COPY_FROM_LIB_DIRECTORY}/RelWithDebInfo/${PLATFORM_FOLDER}
                                    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO=${COPY_FROM_BIN_DIRECTORY}/RelWithDebInfo/${PLATFORM_FOLDER}

                                    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL=${COPY_FROM_LIB_DIRECTORY}/MinSizeRel/${PLATFORM_FOLDER}
                                    -DCMAKE_LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL=${COPY_FROM_LIB_DIRECTORY}/MinSizeRel/${PLATFORM_FOLDER}
                                    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL=${COPY_FROM_BIN_DIRECTORY}/MinSizeRel/${PLATFORM_FOLDER}

            COMMAND ${CMAKE_COMMAND} --build "${CMAKE_SOURCE_DIR}/vendor/spdlog/build" --parallel --config Debug
            COMMAND ${CMAKE_COMMAND} --build "${CMAKE_SOURCE_DIR}/vendor/spdlog/build" --parallel --config Release
            COMMAND ${CMAKE_COMMAND} --build "${CMAKE_SOURCE_DIR}/vendor/spdlog/build" --parallel --config RelWithDebInfo
            COMMAND ${CMAKE_COMMAND} --build "${CMAKE_SOURCE_DIR}/vendor/spdlog/build" --parallel --config MinSizeRel
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/vendor/spdlog"
        )
    else()
        message(WARNING "Not Supported yet: To be implemented.")
        # -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=/path/to/libs
    endif()
endif()

# Copy ${CMAKE_SOURCE_DIR}/vendor/builds/lib/<config> to ${LIB_OUTPUT_DIRECTORY_PREFIX}
execute_process( # doesnt know config copy all
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${COPY_FROM_LIB_DIRECTORY}
            ${COPY_TO_LIB_DIRECTORY}
)

# -------------------------------------------------------------
# Includes & Libraries
# -------------------------------------------------------------
set(my_includes vendor/GLFW/include vendor/GLM vendor/spdlog/include)
find_library(GLFW_LIB_DEBUG NAMES glfw glfw3 PATHS ${COPY_TO_LIB_DIRECTORY}/Debug/${PLATFORM_FOLDER})
find_library(SPDLOG_LIB_DEBUG NAMES spdlog spdlogd PATHS ${COPY_TO_LIB_DIRECTORY}/Debug/${PLATFORM_FOLDER})
find_library(GLM_LIB_DEBUG NAMES glm PATHS ${COPY_TO_LIB_DIRECTORY}/Debug/${PLATFORM_FOLDER})
find_library(VK_LAYER_LIB_DEBUG NAMES VkLayer_utils PATHS ${COPY_TO_LIB_DIRECTORY}/Debug/${PLATFORM_FOLDER})
message(STATUS "Found ${GLFW_LIB_DEBUG};${SPDLOG_LIB_DEBUG};${GLM_LIB_DEBUG};${VK_LAYER_LIB_DEBUG}")

find_library(GLFW_LIB_RELEASE NAMES glfw glfw3 PATHS ${COPY_TO_LIB_DIRECTORY}/Release/${PLATFORM_FOLDER})
find_library(SPDLOG_LIB_RELEASE NAMES spdlog spdlogd PATHS ${COPY_TO_LIB_DIRECTORY}/Release/${PLATFORM_FOLDER})
find_library(GLM_LIB_RELEASE NAMES glm PATHS ${COPY_TO_LIB_DIRECTORY}/Release/${PLATFORM_FOLDER})
message(STATUS "Found ${GLFW_LIB_RELEASE};${SPDLOG_LIB_RELEASE};${GLM_LIB_RELEASE}")

find_library(GLFW_LIB_RELWITHDEBINFO NAMES glfw glfw3 PATHS ${COPY_TO_LIB_DIRECTORY}/RelWithDebInfo/${PLATFORM_FOLDER})
find_library(SPDLOG_LIB_RELWITHDEBINFO NAMES spdlog spdlogd PATHS ${COPY_TO_LIB_DIRECTORY}/RelWithDebInfo/${PLATFORM_FOLDER})
find_library(GLM_LIB_RELWITHDEBINFO NAMES glm PATHS ${COPY_TO_LIB_DIRECTORY}/RelWithDebInfo/${PLATFORM_FOLDER})
find_library(VK_LAYER_LIB_RELWITHDEBINFO NAMES VkLayer_utils PATHS ${COPY_TO_LIB_DIRECTORY}/RelWithDebInfo/${PLATFORM_FOLDER})
message(STATUS "Found ${GLFW_LIB_RELWITHDEBINFO};${SPDLOG_LIB_RELWITHDEBINFO};${GLM_LIB_RELWITHDEBINFO};${VK_LAYER_LIB_RELWITHDEBINFO}")

find_library(GLFW_LIB_MINSIZEREL NAMES glfw glfw3 PATHS ${COPY_TO_LIB_DIRECTORY}/MinSizeRel/${PLATFORM_FOLDER})
find_library(SPDLOG_LIB_MINSIZEREL NAMES spdlog spdlogd PATHS ${COPY_TO_LIB_DIRECTORY}/MinSizeRel/${PLATFORM_FOLDER})
find_library(GLM_LIB_MINSIZEREL NAMES glm PATHS ${COPY_TO_LIB_DIRECTORY}/MinSizeRel/${PLATFORM_FOLDER})
message(STATUS "Found ${GLFW_LIB_MINSIZEREL};${SPDLOG_LIB_MINSIZEREL};${GLM_LIB_MINSIZEREL}")

# Create an imported target for each dependency
add_library(GLFW::GLFW UNKNOWN IMPORTED)
add_library(SPDLOG::SPDLOG UNKNOWN IMPORTED)
add_library(GLM::GLM UNKNOWN IMPORTED)
add_library(VK_LAYER::UTILS UNKNOWN IMPORTED)

# Assign config-specific locations
set_target_properties(GLFW::GLFW PROPERTIES
    IMPORTED_LOCATION_DEBUG        "${GLFW_LIB_DEBUG}"
    IMPORTED_LOCATION_RELEASE      "${GLFW_LIB_RELEASE}"
    IMPORTED_LOCATION_RELWITHDEBINFO "${GLFW_LIB_RELWITHDEBINFO}"
    IMPORTED_LOCATION_MINSIZEREL   "${GLFW_LIB_MINSIZEREL}"
)

set_target_properties(SPDLOG::SPDLOG PROPERTIES
    IMPORTED_LOCATION_DEBUG        "${SPDLOG_LIB_DEBUG}"
    IMPORTED_LOCATION_RELEASE      "${SPDLOG_LIB_RELEASE}"
    IMPORTED_LOCATION_RELWITHDEBINFO "${SPDLOG_LIB_RELWITHDEBINFO}"
    IMPORTED_LOCATION_MINSIZEREL   "${SPDLOG_LIB_MINSIZEREL}"
)

set_target_properties(GLM::GLM PROPERTIES
    IMPORTED_LOCATION_DEBUG        "${GLM_LIB_DEBUG}"
    IMPORTED_LOCATION_RELEASE      "${GLM_LIB_RELEASE}"
    IMPORTED_LOCATION_RELWITHDEBINFO "${GLM_LIB_RELWITHDEBINFO}"
    IMPORTED_LOCATION_MINSIZEREL   "${GLM_LIB_MINSIZEREL}"
)

set_target_properties(VK_LAYER::UTILS PROPERTIES
    IMPORTED_LOCATION_DEBUG        "${VK_LAYER_LIB_DEBUG}"
    IMPORTED_LOCATION_RELWITHDEBINFO "${VK_LAYER_LIB_RELWITHDEBINFO}"
)

# set(my_libs glfw glm::glm $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:VkLayer_utils> spdlog::spdlog $<$<BOOL:${MINGW}>:ws2_32>)
set(my_libs GLFW::GLFW SPDLOG::SPDLOG GLM::GLM $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:VK_LAYER::UTILS> $<$<BOOL:${MINGW}>:ws2_32>)

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
        $<$<CONFIG:Debug>:/Zi /Od /W4 /DDEBUG /MDd /EHsc /permissive->
        $<$<CONFIG:Release>:/O2 /Ob2 /GL /DNDEBUG /MD /EHsc /fp:fast>
        $<$<CONFIG:RelWithDebInfo>:/O2 /Zi /DNDEBUG /MD /EHsc>
        $<$<CONFIG:MinSizeRel>:/O1 /Os /DNDEBUG /MD /EHsc>
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