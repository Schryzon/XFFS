# XFFS Development Journal

This document records the design decisions, architectural changes, and implementation progress of the Xilaizong-Fuxin File System (XFFS).

---

## Phase 1: Foundation (Initial State)
- **Disk Abstraction**: Implemented a basic `disk` class backed by a `std::fstream` for block-level I/O.
- **Superblock**: Defined the `superblock` structure (block 0) to store filesystem metadata (magic number, block size, total blocks, offsets for bitmap and inode table).
- **Bitmap**: Implemented basic bitmap manipulation (`bitmap_set`, `bitmap_clear`, `bitmap_test`) for block allocation management.
- **Formatter**: Created `mkxffs` to initialize the superblock and bitmap on a virtual disk file.

---

## Phase 2: Block Allocators, Inodes, and Directories
- **Block Allocator Fixes**: 
    - Renamed exported `free_disk` to `free_block` for consistency.
    - Updated `load_bitmap` and `store_bitmap` to handle bitmaps spanning multiple blocks.
- **Inode I/O**:
    - Introduced `read_inode` and `write_inode` functions.
    - Used read-modify-write to handle multiple inodes packed into a single 4096-byte block (16 inodes per block).
- **Directory Operations**:
    - Implemented `dir_init`, `dir_add`, and `dir_lookup`.
    - Directories use a simple array of fixed-size `dirent` structures within their first data block.
- **Root Inode**:
    - Updated the formatter to automatically create the root inode (index 0, type `INODE_DIR`) during disk initialization.
    - Automated the zero-initialization of the root directory's data block.
- **Verification**: Added `format_test` to verify superblock integrity and root inode creation.

---

## Phase 3: File Operations & Path Resolution (Completed)
- **Inode Allocation**: Implemented `alloc_inode` to scan the inode table for free slots (`mode == 0`) and `free_inode` to reclaim them.
- **Directory Improvements**: Added `dir_remove` to support file deletion by clearing directory entries.
- **File Lifecycle**:
    - `file_create`: Handles inode and data block allocation, inode initialization, and directory linkage.
    - `file_write`/`file_read`: Implements data I/O for the first direct block (Phase 3 limitation).
    - `file_delete`: Performs a full cleanup of data blocks, the inode, and the directory entry.
- **Path Resolution**: Implemented `path_resolve` for UNIX-style absolute paths, allowing the system to traverse the directory tree.
- **Demonstration**: Updated `main.cpp` with a live creation-to-deletion demo of `/hello.txt`.
- **Validation**: Expanded `format_test.cpp` to verify the entire Phase 3 feature set.

---

## Phase 4: Reliability & Large Files (In Progress)
- **Checksum Support**:
    - Introduced `compute_checksum` in `utils.hpp` using a simple 64-bit XOR-sum.
    - Updated `inode_io.cpp` to compute and verify inode checksums.
    - Design pattern: Zero out the `checksum` field, compute the XOR across all 256 bytes, then write the result.
    - Failure mode: `read_inode` returns `false` if the computed checksum doesn't match the stored one (for non-free inodes).

---

## Phase 5: Interactive Shell & Directory Extensions (Completed)
- **Features**: Implemented `xffs_shell` (REPL environment) providing standard filesystem utilities: `ls`, `mkdir`, `touch`, `write`, `cat`, `stat`, and `rm`.
- **Directory Depth**: Introduced `dir_create`, which initializes empty data blocks for new directories and establishes `.` (self) and `..` (parent) linkages immediately upon creation.
- **Robustness**: Resolved a critical initialization bug where `dir_create` erroneously targeted block 0 (superblock). The shell now includes `recursive_rm` to safely traverse and deallocate directory trees (inodes and associated data blocks).

