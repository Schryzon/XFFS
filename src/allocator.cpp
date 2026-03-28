#include "xffs/allocator.hpp"
#include "xffs/disk.hpp"
#include "xffs/bitmap.hpp"
#include "xffs/superblock.hpp"
#include "xffs/inode.hpp"
#include "xffs/inode_io.hpp"

#include <vector>
#include <limits>

namespace xffs {

static bool load_bitmap(
    disk &dsk,
    const superblock &sb,
    std::vector<uint8_t> &out
) {
    const uint64_t bytes = sb.bitmap_blocks * XFFS_BLOCK_SIZE;
    out.resize(bytes);

    for(uint64_t i = 0; i < sb.bitmap_blocks; i++) {
        if(!dsk.read_block(sb.bitmap_start + i, out.data() + i * XFFS_BLOCK_SIZE)) {
            return false;
        }
    }
    return true;
}

static bool store_bitmap(
    disk &dsk,
    const superblock &sb,
    const std::vector<uint8_t> &bitmap
) {
    for(uint64_t i = 0; i < sb.bitmap_blocks; i++) {
        if(!dsk.write_block(sb.bitmap_start + i, bitmap.data() + i * XFFS_BLOCK_SIZE)) {
            return false;
        }
    }
    return true;
}

uint64_t alloc_block(disk &dsk, const superblock &sb) {
    std::vector<uint8_t> bitmap;
    if(!load_bitmap(dsk, sb, bitmap)) {
        return UINT64_MAX;
    }

    const uint64_t bit_count = sb.total_blocks;

    uint64_t blk = bitmap_alloc(bitmap, bit_count);
    if(blk == UINT64_MAX) {
        return UINT64_MAX;
    }

    if(!store_bitmap(dsk, sb, bitmap)) {
        return UINT64_MAX;
    }

    return blk;
}

bool free_block(disk &dsk, const superblock &sb, uint64_t blk) {
    if(blk >= sb.total_blocks) {
        return false;
    }

    std::vector<uint8_t> bitmap;
    if(!load_bitmap(dsk, sb, bitmap)) {
        return false;
    }

    bitmap_clear(bitmap, blk);
    return store_bitmap(dsk, sb, bitmap);
}

uint64_t alloc_inode(disk &dsk, const superblock &sb) {
    for(uint64_t i = 0; i < sb.inode_count; i++) {
        inode cur {};
        if(!read_inode(dsk, sb, i, cur)) {
            continue;
        }

        if(cur.mode == INODE_FREE) {
            // Reserve it by writing a dummy mode
            cur.mode = INODE_FILE;
            if(write_inode(dsk, sb, i, cur)) {
                return i;
            }
        }
    }
    return UINT64_MAX;
}

bool free_inode(disk &dsk, const superblock &sb, uint64_t index) {
    if(index >= sb.inode_count) {
        return false;
    }

    inode zero {};
    return write_inode(dsk, sb, index, zero);
}

} // namespace xffs
