#include "xffs/bitmap.hpp"
#include "xffs/superblock.hpp"
#include "xffs/disk.hpp"
#include "xffs/inode.hpp"
#include "xffs/inode_io.hpp"
#include "xffs/dir.hpp"

#include <cstdint>
#include <cstring>
#include <ctime>
#include <vector>
#include <iostream>

namespace xffs {

/**
 * @brief Format a new XFFS image.
 *
 * Writes the superblock, bitmap, and root inode (index 0).
 */
bool format_disk(disk &dsk, uint64_t total_blocks) {
    if(!dsk.is_open()) {
        return false;
    }

    // --- Superblock ---
    superblock sb {};
    sb.magic        = XFFS_MAGIC;
    sb.block_size   = XFFS_BLOCK_SIZE;
    sb.total_blocks = total_blocks;

    sb.bitmap_start  = 1;
    sb.bitmap_blocks = 1;

    sb.inode_start  = 2;
    sb.inode_blocks = XFFS_DEFAULT_INODE_BLOCKS;
    sb.inode_count  = XFFS_DEFAULT_INODE_COUNT;

    sb.data_start  = sb.inode_start + sb.inode_blocks; // 66
    sb.root_inode  = 0;
    sb.features    = 0;
    sb.checksum    = 0; // TODO: implement checksum

    // --- Bitmap ---
    const uint64_t bitmap_bytes = XFFS_BLOCK_SIZE * sb.bitmap_blocks;
    std::vector<uint8_t> bitmap(bitmap_bytes, 0);

    // Mark all metadata blocks as used (blocks 0..data_start-1).
    for(uint64_t i = 0; i < sb.data_start; i++) {
        bitmap_set(bitmap, i);
    }

    // Reserve block for the root directory data (block data_start == 66).
    bitmap_set(bitmap, sb.data_start);

    // --- Write superblock ---
    std::vector<uint8_t> blk(XFFS_BLOCK_SIZE, 0);
    std::memcpy(blk.data(), &sb, sizeof(sb));
    if(!dsk.write_block(0, blk.data())) {
        return false;
    }

    // --- Write bitmap ---
    for(uint64_t i = 0; i < sb.bitmap_blocks; i++) {
        if(!dsk.write_block(sb.bitmap_start + i,
                            bitmap.data() + i * XFFS_BLOCK_SIZE)) {
            return false;
        }
    }

    std::cout << "[xffs] reserved blocks: 0-" << (sb.data_start - 1) << "\n";
    std::cout << "[xffs] data blocks start at: " << sb.data_start << "\n";

    // --- Root inode (inode 0, INODE_DIR) ---
    const uint64_t root_data_block = sb.data_start; // first data block

    const uint64_t now = static_cast<uint64_t>(std::time(nullptr));

    inode root {};
    root.mode      = INODE_DIR;
    root.links     = 1;
    root.uid       = 0;
    root.gid       = 0;
    root.size      = XFFS_BLOCK_SIZE;
    root.atime     = now;
    root.mtime     = now;
    root.ctime     = now;
    root.direct[0] = root_data_block;
    root.checksum  = 0; // TODO: implement checksum

    if(!write_inode(dsk, sb, 0, root)) {
        return false;
    }

    // Zero-initialise the root directory data block.
    if(!dir_init(dsk, sb, 0)) {
        return false;
    }

    std::cout << "[xffs] root inode created (inode 0), data block: "
              << root_data_block << "\n";

    return true;
}

} // namespace xffs
