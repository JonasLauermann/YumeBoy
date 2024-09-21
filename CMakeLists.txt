cmake_minimum_required(VERSION 3.24)
project(YumeBoy)

set(CMAKE_CXX_STANDARD 23)

# Configure release builds
if(${is_release_build})
    add_compile_definitions(NDEBUG) # set NDEBUG macro
    add_compile_options(-O2)
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/W4)
    # There's no direct equivalent of `-Wunreachable-code` in MSVC,
    # but /W4 increases the warning level to show most warnings.
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wall -Wextra -Wunreachable-code)
endif()
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})

# Add SDL3
add_subdirectory(third-party/SDL3)
include_directories(third-party/SDL3/include)

# Include directories
include_directories(include src)

# Subdirectories
add_subdirectory(src)
