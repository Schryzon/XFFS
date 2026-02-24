#pragma once

#include <cstdint>

/**
 * @brief XFFS superblock structure.
 *
 * Stored at block 0, defines the filesystem layout.
 */

namespace xffs{
    constexpr uint32_t XFFS_MAGIC = 0x58464653; // "XFFS"
    constexpr uint32_t XFFS_BLOCK_SIZE = 4096;

    #pragma pack(push, 1)
    struct superblock{
        uint32_t magic;
        uint32_t block_size;
        uint64_t total_blocks;

        uint64_t bitmap_start;
        uint64_t bitmap_blocks;

        uint64_t inode_start;
        uint64_t inode_blocks;
        uint64_t inode_count;

        uint64_t data_start;

        uint64_t root_inode;
        uint64_t features;

        uint64_t checksum;
    };
    #pragma pack(pop)

}
