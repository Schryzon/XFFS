#pragma once

#include "xffs/disk.hpp"
#include "xffs/superblock.hpp"

#include "xffs/dirent.hpp"

#include <cstdint>
#include <vector>

namespace xffs {

/**
 * @brief Zero-initialise the data block of an existing directory inode.
 *
 * Must be called after the inode has been written and its direct[0] block
 * has been allocated.
 *
 * @param dsk       Open disk image.
 * @param sb        Superblock.
 * @param inode_idx Index of the directory inode.
 * @return true on success.
 */
bool dir_init(disk &dsk, const superblock &sb, uint64_t inode_idx);

/**
 * @brief Add a directory entry to an existing directory.
 *
 * Scans the first data block of the directory inode for a free slot
 * (inode field == 0) and writes the new entry there.
 *
 * @param dsk           Open disk image.
 * @param sb            Superblock.
 * @param dir_inode_idx Index of the parent directory inode.
 * @param child_inode   Inode index of the entry being added.
 * @param type          DIRENT_FILE or DIRENT_DIR.
 * @param name          Null-terminated name string.
 * @return true on success.
 */
bool dir_add(disk &dsk, const superblock &sb, uint64_t dir_inode_idx,
             uint64_t child_inode, uint8_t type, const char *name);

/**
 * @brief Look up a name in a directory.
 *
 * @param dsk           Open disk image.
 * @param sb            Superblock.
 * @param dir_inode_idx Index of the directory inode.
 * @param name          Null-terminated name to search for.
 * @return Inode index of the found entry, or UINT64_MAX if not found.
 */
uint64_t dir_lookup(disk &dsk, const superblock &sb, uint64_t dir_inode_idx,
                    const char *name);

/**
 * @brief Remove a directory entry by name.
 */
bool dir_remove(disk &dsk, const superblock &sb, uint64_t dir_inode_idx,
                const char *name);

    /**
     * @brief List all active entries in a directory.
     * 
     * @param dsk           Open disk image.
     * @param sb            Superblock.
     * @param dir_inode_idx Index of the directory inode.
     * @return Vector of active directory entries.
     */
    std::vector<dirent> dir_list(disk &dsk, const superblock &sb, uint64_t dir_inode_idx);

} // namespace xffs
