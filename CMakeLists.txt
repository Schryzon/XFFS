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
    src/inode_io.cpp
    src/dir.cpp
    src/file_ops.cpp
    src/path.cpp
)

target_include_directories(mkxffs PRIVATE include)

# --- Test executable ---
add_executable(format_test
    tests/format_test.cpp
    src/disk.cpp
    src/format.cpp
    src/allocator.cpp
    src/inode_io.cpp
    src/dir.cpp
    src/file_ops.cpp
    src/path.cpp
)

target_include_directories(format_test PRIVATE include)

# --- Shell executable ---
add_executable(xffs_shell
    src/shell.cpp
    src/disk.cpp
    src/format.cpp
    src/allocator.cpp
    src/inode_io.cpp
    src/dir.cpp
    src/file_ops.cpp
    src/path.cpp
)

target_include_directories(xffs_shell PRIVATE include)
