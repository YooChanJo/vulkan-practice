cmake_minimum_required(VERSION 3.22.1)
set(CMAKE_CXX_STANDARD 17)
project(VulkanPractice)

# Force MSVC runtime consistency
if (MSVC)
    # Use Debug DLL for Debug, Release DLL for Release
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL" CACHE STRING "" FORCE)
endif()

# Add sub directory build
add_subdirectory(vendor/GLFW)
add_subdirectory(vendor/GLM)
if(ENABLE_UPDATE_DEPS)
    set(UPDATE_DEPS ON CACHE BOOL "Automatically update Vulkan dependencies")
    add_subdirectory(vendor/Vulkan-ValidationLayers)
    unset(UPDATE_DEPS CACHE)
else()
    add_subdirectory(vendor/Vulkan-ValidationLayers)
endif()
add_subdirectory(vendor/spdlog)

# Add files
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.h")

# Config
set(my_includes vendor/GLFW/include vendor/GLM vendor/spdlog/include)
set(my_libs glfw glm VkLayer_utils spdlog::spdlog $<$<BOOL:${MINGW}>:ws2_32>)
# $<$<BOOL:${MINGW}>:ws2_32> is for spdlog

# Added Vulkan
find_package(Vulkan REQUIRED)

# Add Exe
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE ${my_libs} ${Vulkan_LIBRARIES})

# Set Include directories
target_include_directories(${PROJECT_NAME} PRIVATE ${my_includes} ${Vulkan_INCLUDE_DIRS})

# Defines
# Forces lib compilation
target_compile_definitions(spdlog PRIVATE SPDLOG_COMPILED_LIB)

# Set Precompiled Headers
target_precompile_headers(${PROJECT_NAME} PRIVATE src/pch.h)

# Optimizations
# Set flags per build type
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE
        # Debug
        $<$<CONFIG:Debug>:-g -O0 -Wall -Wextra -Wshadow -Wformat=2 -Wpedantic -DDEBUG>
        # Release
        $<$<CONFIG:Release>:-O3 -march=native -flto -funroll-loops -DNDEBUG -fstrict-aliasing -ffast-math>
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:-flto>
    )
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(${PROJECT_NAME} PRIVATE
        # Debug
        $<$<CONFIG:Debug>:/Zi /Od /W4 /DDEBUG /MDd /EHsc /permissive->
        # Release
        $<$<CONFIG:Release>:/O2 /Ob2 /GL /DNDEBUG /MD /EHsc /fp:fast>
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:/LTCG>
    )
endif()

# Enable LTO for Release
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)

# TODO: remove vk validation layer for release build --> redundant