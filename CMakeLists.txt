cmake_minimum_required(VERSION 3.29)
project(YumeBoy LANGUAGES CXX)

# --- Language standard ---
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)         # Use -std=c++23 instead of -std=gnu++23
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

# Default to a Release build if the user didn't specify one.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Output binaries to build/<config>/ (e.g. build/Debug, build/Release).
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>)

# --- Shared compiler warnings ---
# This avoids setting warning flags globally, so third-party code (SDL3) isn't affected.
add_library(project_warnings INTERFACE)
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(project_warnings INTERFACE /W4)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(project_warnings INTERFACE -Wall -Wextra -Wunreachable-code)
endif()

# --- Include Directories ---
add_library(project_include_dirs INTERFACE)
target_include_directories(project_include_dirs INTERFACE ${CMAKE_SOURCE_DIR}/include)

# --- Dependencies ---
find_package(Boost REQUIRED COMPONENTS serialization)
add_subdirectory(third-party/SDL3)

# --- Main project sources ---
add_subdirectory(src)