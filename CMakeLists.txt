cmake_minimum_required(VERSION 3.24)
project(YumeBoy)

set(CMAKE_CXX_STANDARD 23)

# # Find SDL2
find_package(SDL2 REQUIRED)

add_executable(YumeBoy main.cpp
        CPU.cpp
        Cartridge.cpp
        MemoryStub.hpp
        PPU.cpp)
target_link_libraries(YumeBoy SDL2::SDL2 SDL2::SDL2main)
