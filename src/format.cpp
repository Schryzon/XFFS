#include "xffs/superblock.hpp"
#include "xffs/disk.hpp"

#include <cstring>
#include <vector>

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

    std::vector<uint8_t> block(XFFS_BLOCK_SIZE);
    std::memcpy(block.data(), &sb, sizeof(sb));

    return dsk.write_block(0, block.data());
}

} // namespace xffs
