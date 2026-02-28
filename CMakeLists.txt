cmake_minimum_required(VERSION 3.16)
project(xffs LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(mkxffs
    src/main.cpp
    src/disk.cpp
    src/format.cpp
    src/allocator.cpp
)

target_include_directories(mkxffs PRIVATE include)
