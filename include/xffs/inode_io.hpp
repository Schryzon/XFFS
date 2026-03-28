#pragma once

#include "xffs/disk.hpp"
#include "xffs/superblock.hpp"
#include "xffs/inode.hpp"

#include <cstdint>

namespace xffs {

/**
 * @brief Read an inode by index from the inode table.
 *
 * @param dsk   Open disk image.
 * @param sb    Superblock (provides inode_start, block_size).
 * @param index Zero-based inode index.
 * @param out   Destination inode structure.
 * @return true on success.
 */
bool read_inode(disk &dsk, const superblock &sb, uint64_t index, inode &out);

/**
 * @brief Write an inode by index into the inode table.
 *
 * @param dsk   Open disk image.
 * @param sb    Superblock (provides inode_start, block_size).
 * @param index Zero-based inode index.
 * @param in    Inode data to persist.
 * @return true on success.
 */
bool write_inode(disk &dsk, const superblock &sb, uint64_t index, const inode &in);

} // namespace xffs
