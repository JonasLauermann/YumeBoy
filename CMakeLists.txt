cmake_minimum_required(VERSION 3.24)
project(YumeBoy)

set(CMAKE_CXX_STANDARD 23)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/W4)
    # There's no direct equivalent of `-Wunreachable-code` in MSVC,
    # but /W4 increases the warning level to show most warnings.
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wall -Wextra -Wunreachable-code)
endif()

# # Find SDL2
find_package(SDL2 REQUIRED)

add_executable(YumeBoy main.cpp
        CPU.cpp
        Cartridge.cpp
        MemoryStub.hpp
        PPU.cpp
        Joypad.cpp)
target_link_libraries(YumeBoy SDL2::SDL2 SDL2::SDL2main)
