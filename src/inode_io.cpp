#include "xffs/inode_io.hpp"
#include "xffs/disk.hpp"
#include "xffs/superblock.hpp"
#include "xffs/inode.hpp"
#include "xffs/utils.hpp"

#include <cstring>
#include <vector>

namespace xffs {

bool read_inode(disk &dsk, const superblock &sb, uint64_t index, inode &out) {
    const uint64_t inodes_per_block = XFFS_BLOCK_SIZE / sizeof(inode);
    const uint64_t block_offset     = index / inodes_per_block;
    const uint64_t slot_in_block    = index % inodes_per_block;

    std::vector<uint8_t> block(XFFS_BLOCK_SIZE);
    if (!dsk.read_block(sb.inode_start + block_offset, block.data())) {
        return false;
    }

    inode *nodes = reinterpret_cast<inode *>(block.data());
    out = nodes[slot_in_block];

    if (out.mode != INODE_FREE) {
        // Verify checksum
        inode tmp = out;
        uint64_t stored = tmp.checksum;
        tmp.checksum = 0;
        uint64_t computed = compute_checksum(&tmp, sizeof(inode));
        if (computed != stored) {
            return false; // Checksum error
        }
    }

    return true;
}

bool write_inode(disk &dsk, const superblock &sb, uint64_t index, const inode &in) {
    const uint64_t inodes_per_block = XFFS_BLOCK_SIZE / sizeof(inode);
    const uint64_t block_offset     = index / inodes_per_block;
    const uint64_t slot_in_block    = index % inodes_per_block;

    std::vector<uint8_t> block(XFFS_BLOCK_SIZE);
    if (!dsk.read_block(sb.inode_start + block_offset, block.data())) {
        return false;
    }

    inode to_write = in;
    if (to_write.mode != INODE_FREE) {
        to_write.checksum = 0;
        to_write.checksum = compute_checksum(&to_write, sizeof(inode));
    }

    inode *nodes = reinterpret_cast<inode *>(block.data());
    nodes[slot_in_block] = to_write;

    return dsk.write_block(sb.inode_start + block_offset, block.data());
}

} // namespace xffs
