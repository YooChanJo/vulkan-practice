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
    set(COPY_FROM_LIB_DIRECTORY ${CMAKE_SOURCE_DIR}/vendor/builds/lib/$<CONFIG>)
else()
    if(DEFINED CMAKE_VS_PLATFORM_NAME)
        set(PLATFORM_FOLDER ${CMAKE_VS_PLATFORM_NAME})
    else()
        set(PLATFORM_FOLDER ${CMAKE_SYSTEM_PROCESSOR})
    endif()
    set(BIN_OUTPUT_DIRECTORY_PREFIX ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}/${PLATFORM_FOLDER})
    set(LIB_OUTPUT_DIRECTORY_PREFIX ${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE}/${PLATFORM_FOLDER})
    set(COPY_FROM_LIB_DIRECTORY ${CMAKE_SOURCE_DIR}/vendor/builds/lib/${CMAKE_BUILD_TYPE})
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
function(run_and_log)
    execute_process(
        ${ARGV}
        RESULT_VARIABLE result
        OUTPUT_VARIABLE output
        ERROR_VARIABLE error
    )

    if(NOT result EQUAL 0)
        message(SEND_ERROR "Command failed with exit code ${result}")
    endif()

    if(output)
        message(STATUS "Output:\n${output}")
    endif()

    if(error)
        message(WARNING "Error:\n${error}")
    endif()
endfunction()


if(IS_INITIAL_SETUP)
    if(IS_MULTI_CONFIG)
        message(WARNING "Warning: Every possible configuration is built (Debug, Release, RelWithDebInfo, MinSizeRel). This may take some time.")
        # Vulkan Validation Layers
        message(WARNING "Wifi is required for this process, please keep device online.")
        message(STATUS "Configuring Vulkan Validation Layers Debug Build")
        run_and_log(
            COMMAND ${CMAKE_COMMAND} -S "${CMAKE_SOURCE_DIR}/vendor/Vulkan-Validationlayers"
                                    -B "${CMAKE_SOURCE_DIR}/vendor/Vulkan-Validationlayers/build/Debug"
                                    ${COMMON_CMAKE_COMMAND_ARGS}
                                    -DCMAKE_BUILD_TYPE=Debug
                                    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG=${CMAKE_SOURCE_DIR}/vendor/builds/lib/Debug
                                    -DUPDATE_DEPS=ON
                                    "-DCMAKE_PREFIX_PATH=${CMAKE_SOURCE_DIR}/vendor/Vulkan-Validationlayers/external/Debug"  
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/vendor/Vulkan-Validationlayers"
        )
        message(STATUS "Configuring Vulkan Validation Layers RelWithDebInfo Build")
        run_and_log(
            COMMAND ${CMAKE_COMMAND} -S "${CMAKE_SOURCE_DIR}/vendor/Vulkan-Validationlayers"
                                    -B "${CMAKE_SOURCE_DIR}/vendor/Vulkan-Validationlayers/build/RelWithDebInfo"
                                    ${COMMON_CMAKE_COMMAND_ARGS}
                                    -DCMAKE_BUILD_TYPE=RelWithDebInfo
                                    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO=${CMAKE_SOURCE_DIR}/vendor/builds/lib/RelWithDebInfo
                                    -DUPDATE_DEPS=ON
                                    "-DCMAKE_PREFIX_PATH=${CMAKE_SOURCE_DIR}/vendor/Vulkan-Validationlayers/external/RelWithDebInfo"  
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/vendor/Vulkan-Validationlayers"
        )
        message(WARNING "Wifi no longer required.")
        # Vulkan Validation Layers
        message(STATUS "Building Vulkan Validation Layers")
        run_and_log(
            COMMAND ${CMAKE_COMMAND} --build "${CMAKE_SOURCE_DIR}/vendor/Vulkan-Validationlayers/build/Debug" --parallel --config Debug
            COMMAND ${CMAKE_COMMAND} --build "${CMAKE_SOURCE_DIR}/vendor/Vulkan-Validationlayers/build/RelWithDebInfo" --parallel --config RelWithDebInfo
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/vendor/Vulkan-Validationlayers"
        )
        # GLFW
        message(STATUS "Building Vulkan GLFW")
        run_and_log(
            COMMAND ${CMAKE_COMMAND} -S "${CMAKE_SOURCE_DIR}/vendor/GLFW"
                                    -B "${CMAKE_SOURCE_DIR}/vendor/GLFW/build"
                                    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG=${CMAKE_SOURCE_DIR}/vendor/builds/lib/Debug
                                    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE=${CMAKE_SOURCE_DIR}/vendor/builds/lib/Release
                                    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO=${CMAKE_SOURCE_DIR}/vendor/builds/lib/RelWithDebInfo
                                    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL=${CMAKE_SOURCE_DIR}/vendor/builds/lib/MinSizeRel
                                    ${COMMON_CMAKE_COMMAND_ARGS}
            COMMAND ${CMAKE_COMMAND} --build "${CMAKE_SOURCE_DIR}/vendor/GLFW/build" --parallel --config Debug
            COMMAND ${CMAKE_COMMAND} --build "${CMAKE_SOURCE_DIR}/vendor/GLFW/build" --parallel --config Release
            COMMAND ${CMAKE_COMMAND} --build "${CMAKE_SOURCE_DIR}/vendor/GLFW/build" --parallel --config RelWithDebInfo
            COMMAND ${CMAKE_COMMAND} --build "${CMAKE_SOURCE_DIR}/vendor/GLFW/build" --parallel --config MinSizeRel
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/vendor/GLFW"
        )
        # GLM
        message(STATUS "Building Vulkan GLM")
        run_and_log(
            COMMAND ${CMAKE_COMMAND} -S "${CMAKE_SOURCE_DIR}/vendor/GLM"
                                    -B "${CMAKE_SOURCE_DIR}/vendor/GLM/build"
                                    ${COMMON_CMAKE_COMMAND_ARGS}
                                    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG=${CMAKE_SOURCE_DIR}/vendor/builds/lib/Debug
                                    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE=${CMAKE_SOURCE_DIR}/vendor/builds/lib/Release
                                    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO=${CMAKE_SOURCE_DIR}/vendor/builds/lib/RelWithDebInfo
                                    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL=${CMAKE_SOURCE_DIR}/vendor/builds/lib/MinSizeRel
            COMMAND ${CMAKE_COMMAND} --build "${CMAKE_SOURCE_DIR}/vendor/GLM/build" --parallel --config Debug
            COMMAND ${CMAKE_COMMAND} --build "${CMAKE_SOURCE_DIR}/vendor/GLM/build" --parallel --config Release
            COMMAND ${CMAKE_COMMAND} --build "${CMAKE_SOURCE_DIR}/vendor/GLM/build" --parallel --config RelWithDebInfo
            COMMAND ${CMAKE_COMMAND} --build "${CMAKE_SOURCE_DIR}/vendor/GLM/build" --parallel --config MinSizeRel
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/vendor/GLM"
        )
        # spdlog
        message(STATUS "Building Vulkan spdlog")
        run_and_log(
            COMMAND ${CMAKE_COMMAND} -S "${CMAKE_SOURCE_DIR}/vendor/spdlog"
                                    -B "${CMAKE_SOURCE_DIR}/vendor/spdlog/build"
                                    ${COMMON_CMAKE_COMMAND_ARGS}
                                    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG=${CMAKE_SOURCE_DIR}/vendor/builds/lib/Debug
                                    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE=${CMAKE_SOURCE_DIR}/vendor/builds/lib/Release
                                    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO=${CMAKE_SOURCE_DIR}/vendor/builds/lib/RelWithDebInfo
                                    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL=${CMAKE_SOURCE_DIR}/vendor/builds/lib/MinSizeRel
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
add_custom_target(copy_contents ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${COPY_FROM_LIB_DIRECTORY}
            ${LIB_OUTPUT_DIRECTORY_PREFIX}
)
add_dependencies(${PROJECT_NAME} copy_contents)

# -------------------------------------------------------------
# Includes & Libraries
# -------------------------------------------------------------
set(my_includes vendor/GLFW/include vendor/GLM vendor/spdlog/include)
file(GLOB_RECURSE LIB_FILES CONFIGURE_DEPENDS
    "${LIB_OUTPUT_DIRECTORY_PREFIX}/*"
)
message(STATUS "My Libs being linked: ${LIB_FILES}")
# set(my_libs glfw glm::glm $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:VkLayer_utils> spdlog::spdlog $<$<BOOL:${MINGW}>:ws2_32>)
set(my_libs ${LIB_FILES} $<$<BOOL:${MINGW}>:ws2_32>)

# -------------------------------------------------------------
# Add Target Properties etc.
# -------------------------------------------------------------
target_include_directories(${PROJECT_NAME} PRIVATE ${my_includes} ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${my_libs} ${Vulkan_LIBRARIES})
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:INCLUDE_DEBUG_INFO>)
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

# -------------------------------------------------------------
# Enable LTO for release-like builds
# -------------------------------------------------------------
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
# set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO TRUE)

# # Suppose this is the full path from LOCATION
# get_target_property(FULL_PATH VendorLib LOCATION)
# message(STATUS "Full path: ${FULL_PATH}")

# # Extract just the filename with extension
# get_filename_component(FILE_NAME ${FULL_PATH} NAME)
# message(STATUS "Filename with extension: ${FILE_NAME}")

# # Optional: extract directory only
# get_filename_component(DIR_NAME ${FULL_PATH} DIRECTORY)
# message(STATUS "Directory: ${DIR_NAME}")

# # Optional: extract filename without extension
# get_filename_component(FILE_NO_EXT ${FULL_PATH} NAME_WE)
# message(STATUS "Filename without extension: ${FILE_NO_EXT}")

# # Optional: extract just the extension
# get_filename_component(EXT ${FULL_PATH} EXT)
# message(STATUS "Extension: ${EXT}")