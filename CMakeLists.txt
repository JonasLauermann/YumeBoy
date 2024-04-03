cmake_minimum_required(VERSION 3.24)
project(YumeBoy)

set(CMAKE_CXX_STANDARD 23)

add_executable(YumeBoy main.cpp
        CPU.cpp
        Cartridge.cpp
        MemoryStub.hpp)
