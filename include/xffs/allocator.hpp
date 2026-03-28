#pragma once

#include <cstdint>

namespace xffs {
    class disk;
    struct superblock;

    /**
     * @brief Allocate one data block.
     *
     * @return block index or UINT64_MAX on failure
     */
    uint64_t alloc_block(disk &dsk, const superblock &sb);

    /**
     * @brief Free a previously allocated block.
     */
    bool free_block(disk &dsk, const superblock &sb, uint64_t blk);

    /**
     * @brief Allocate a free inode from the inode table.
     * @return inode index or UINT64_MAX on failure.
     */
    uint64_t alloc_inode(disk &dsk, const superblock &sb);

    /**
     * @brief Free an inode (mark as INODE_FREE).
     */
    bool free_inode(disk &dsk, const superblock &sb, uint64_t index);
}
