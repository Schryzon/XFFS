#include "xffs/allocator.hpp"
#include "xffs/disk.hpp"
#include "xffs/bitmap.hpp"
#include "xffs/superblock.hpp"

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

    return dsk.read_block(sb.bitmap_start, out.data());
}

static bool store_bitmap(
    disk &dsk,
    const superblock &sb,
    const std::vector<uint8_t> &bitmap
) {
    return dsk.write_block(sb.bitmap_start, bitmap.data());
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

} // namespace xffs
