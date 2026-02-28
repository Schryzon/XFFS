#include "xffs/bitmap.hpp"
#include "xffs/superblock.hpp"
#include "xffs/disk.hpp"

#include <cstdint>
#include <cstring>
#include <vector>
#include <iostream>

namespace xffs {

    /**
    * @brief Format a new XFFS image.
    */
    bool format_disk(disk &dsk, uint64_t total_blocks) {
        if(!dsk.is_open()) {
            return false;
        }

        superblock sb {};
        sb.magic = XFFS_MAGIC;
        sb.block_size = XFFS_BLOCK_SIZE;
        sb.total_blocks = total_blocks;

        sb.bitmap_start = 1;
        sb.bitmap_blocks = 1;

        sb.inode_start = 2;
        sb.inode_blocks = 64;
        sb.inode_count = 1024;

        sb.data_start = sb.inode_start + sb.inode_blocks;
        sb.root_inode = 0;
        sb.features = 0;
        sb.checksum = 0; // TODO: implement checksum

        // Create bitmap
        const uint64_t bitmap_bytes = XFFS_BLOCK_SIZE * sb.bitmap_blocks;
        std::vector<uint8_t> bitmap(bitmap_bytes, 0);

        // Reserve metadata blocks
        for(uint64_t i = 0; i < sb.data_start; i++){
            bitmap_set(bitmap, i);
        }

        // Write superblock
        std::vector<uint8_t> block(XFFS_BLOCK_SIZE, 0);
        std::memcpy(block.data(), &sb, sizeof(sb));

        if(!dsk.write_block(0, block.data())){
            return false;
        }

        // Write bitmap
        if(!dsk.write_block(sb.bitmap_start, bitmap.data())){
            return false;
        }

        std::cout << "[xffs] reserved blocks: 0-" << (sb.data_start - 1) << "\n";
        std::cout << "[xffs] data blocks start at: " << sb.data_start << "\n";

        return true;
    }

} // namespace xffs
