# XFFS Specification (v1)

## Disk Layout
XFFS uses a 4KB block size.

| Block | Content | Description |
|---|---|---|
| 0 | Superblock | Filesystem metadata and layout pointers. |
| 1 | Bitmap | 1 bit per block (0=free, 1=used). |
| 2-65 | Inodes | Table of 1024 inodes (256 bytes each). |
| 66+ | Data Blocks | File and directory data. |

## Superblock Structure (Offset 0)
- `magic` (4 bytes): 0x58464653 ("XFFS")
- `block_size` (4 bytes): 4096
- `total_blocks` (8 bytes)
- `bitmap_start` (8 bytes)
- `bitmap_blocks` (8 bytes)
- `inode_start` (8 bytes)
- `inode_blocks` (8 bytes)
- `inode_count` (8 bytes)
- `data_start` (8 bytes)
- `root_inode` (8 bytes)

## Inode Structure (256 bytes)
- `mode` (2 bytes): 1=File, 2=Dir
- `links` (2 bytes): Reference count
- `size` (8 bytes): File size in bytes
- `direct[12]` (96 bytes): Pointers to first 12 data blocks
- `indirect` (8 bytes): Pointer to a block containing more pointers
