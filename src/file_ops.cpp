#include "xffs/file_ops.hpp"
#include "xffs/inode.hpp"
#include "xffs/inode_io.hpp"
#include "xffs/allocator.hpp"
#include "xffs/dir.hpp"
#include "xffs/dirent.hpp"
#include "xffs/disk.hpp"
#include "xffs/superblock.hpp"

#include <algorithm>
#include <cstring>
#include <ctime>
#include <vector>

namespace xffs {

uint64_t file_create(disk &dsk, const superblock &sb,
                     uint64_t dir_inode_idx, const char *name) {
    // 1. Check if it already exists
    if (dir_lookup(dsk, sb, dir_inode_idx, name) != UINT64_MAX) {
        return UINT64_MAX;
    }

    // 2. Allocate inode
    uint64_t ino = alloc_inode(dsk, sb);
    if (ino == UINT64_MAX) return UINT64_MAX;

    // 3. Allocate initial data block
    uint64_t blk = alloc_block(dsk, sb);
    if (blk == UINT64_MAX) {
        free_inode(dsk, sb, ino);
        return UINT64_MAX;
    }

    // 4. Initialize inode
    const uint64_t now = static_cast<uint64_t>(std::time(nullptr));
    inode file_ino {};
    file_ino.mode = INODE_FILE;
    file_ino.links = 1;
    file_ino.size = 0;
    file_ino.atime = now;
    file_ino.mtime = now;
    file_ino.ctime = now;
    file_ino.direct[0] = blk;

    if (!write_inode(dsk, sb, ino, file_ino)) {
        free_block(dsk, sb, blk);
        free_inode(dsk, sb, ino);
        return UINT64_MAX;
    }

    // 5. Add to directory
    if (!dir_add(dsk, sb, dir_inode_idx, ino, DIRENT_FILE, name)) {
        free_block(dsk, sb, blk);
        free_inode(dsk, sb, ino);
        return UINT64_MAX;
    }

    return ino;
}

uint64_t dir_create(disk &dsk, const superblock &sb,
                    uint64_t dir_inode_idx, const char *name) {
    // 1. Check if it already exists
    if (dir_lookup(dsk, sb, dir_inode_idx, name) != UINT64_MAX) {
        return UINT64_MAX;
    }

    // 2. Allocate inode
    uint64_t ino = alloc_inode(dsk, sb);
    if (ino == UINT64_MAX) return UINT64_MAX;

    // 3. Allocate initial data block
    uint64_t blk = alloc_block(dsk, sb);
    if (blk == UINT64_MAX) {
        free_inode(dsk, sb, ino);
        return UINT64_MAX;
    }

    // Initialize the directory block (zero it out)
    std::vector<uint8_t> zero_block(XFFS_BLOCK_SIZE, 0);
    if (!dsk.write_block(blk, zero_block.data())) {
        free_block(dsk, sb, blk);
        free_inode(dsk, sb, ino);
        return UINT64_MAX;
    }

    // 4. Initialize inode
    const uint64_t now = static_cast<uint64_t>(std::time(nullptr));
    inode new_dir_ino {};
    new_dir_ino.mode = INODE_DIR;
    new_dir_ino.links = 2; // '.' and parent's link
    new_dir_ino.size = XFFS_BLOCK_SIZE;
    new_dir_ino.atime = now;
    new_dir_ino.mtime = now;
    new_dir_ino.ctime = now;
    new_dir_ino.direct[0] = blk;

    if (!write_inode(dsk, sb, ino, new_dir_ino)) {
        free_block(dsk, sb, blk);
        free_inode(dsk, sb, ino);
        return UINT64_MAX;
    }

    // 5. Add to parent directory
    if (!dir_add(dsk, sb, dir_inode_idx, ino, DIRENT_DIR, name)) {
        free_block(dsk, sb, blk);
        free_inode(dsk, sb, ino);
        return UINT64_MAX;
    }

    // (Optional but good practice) Add "." and ".." to the new directory
    dir_add(dsk, sb, ino, ino, DIRENT_DIR, ".");
    dir_add(dsk, sb, ino, dir_inode_idx, DIRENT_DIR, "..");

    return ino;
}

bool file_write(disk &dsk, const superblock &sb, uint64_t inode_idx,
                uint64_t offset, const uint8_t *buf, uint64_t len) {
    inode file_ino {};
    if (!read_inode(dsk, sb, inode_idx, file_ino)) return false;

    // Simple implementation: only support single direct block for Phase 3
    if (offset + len > XFFS_BLOCK_SIZE) return false;

    std::vector<uint8_t> block(XFFS_BLOCK_SIZE);
    if (!dsk.read_block(file_ino.direct[0], block.data())) return false;

    std::memcpy(block.data() + offset, buf, len);
    if (!dsk.write_block(file_ino.direct[0], block.data())) return false;

    // Update size if it grew
    if (offset + len > file_ino.size) {
        file_ino.size = offset + len;
        file_ino.mtime = static_cast<uint64_t>(std::time(nullptr));
        return write_inode(dsk, sb, inode_idx, file_ino);
    }

    return true;
}

bool file_read(disk &dsk, const superblock &sb, uint64_t inode_idx,
               uint64_t offset, uint8_t *buf, uint64_t len) {
    inode file_ino {};
    if (!read_inode(dsk, sb, inode_idx, file_ino)) return false;

    if (offset >= file_ino.size) return false;
    uint64_t actual_len = std::min(len, file_ino.size - offset);

    std::vector<uint8_t> block(XFFS_BLOCK_SIZE);
    if (!dsk.read_block(file_ino.direct[0], block.data())) return false;

    std::memcpy(buf, block.data() + offset, actual_len);
    return true;
}

bool file_delete(disk &dsk, const superblock &sb,
                 uint64_t dir_inode_idx, const char *name) {
    uint64_t ino = dir_lookup(dsk, sb, dir_inode_idx, name);
    if (ino == UINT64_MAX) return false;

    inode file_ino {};
    if (!read_inode(dsk, sb, ino, file_ino)) return false;

    // Free all direct blocks
    for (int i = 0; i < XFFS_DIRECT_BLOCKS; i++) {
        if (file_ino.direct[i] != 0) {
            free_block(dsk, sb, file_ino.direct[i]);
        }
    }

    // Free inode
    if (!free_inode(dsk, sb, ino)) return false;

    // Remove directory entry
    return dir_remove(dsk, sb, dir_inode_idx, name);
}

} // namespace xffs
