#pragma once

#include "xffs/disk.hpp"
#include "xffs/superblock.hpp"

#include <cstdint>

namespace xffs {

/**
 * @brief Create a regular file in a directory.
 *
 * @param dsk           Open disk image.
 * @param sb            Superblock.
 * @param dir_inode_idx Index of the parent directory.
 * @param name          Name of the new file.
 * @return New inode index or UINT64_MAX on failure.
 */
uint64_t file_create(disk &dsk, const superblock &sb,
                     uint64_t dir_inode_idx, const char *name);

/**
 * @brief Create a subdirectory.
 *
 * @param dsk           Open disk image.
 * @param sb            Superblock.
 * @param dir_inode_idx Index of the parent directory.
 * @param name          Name of the new directory.
 * @return New inode index or UINT64_MAX on failure.
 */
uint64_t dir_create(disk &dsk, const superblock &sb,
                    uint64_t dir_inode_idx, const char *name);

/**
 * @brief Write data to a file.
 *
 * @param dsk       Open disk image.
 * @param sb        Superblock.
 * @param inode_idx Index of the file inode.
 * @param offset    Byte offset to start writing at.
 * @param buf       Data to write.
 * @param len       Number of bytes to write.
 * @return true on success.
 */
bool file_write(disk &dsk, const superblock &sb, uint64_t inode_idx,
                uint64_t offset, const uint8_t *buf, uint64_t len);

/**
 * @brief Read data from a file.
 *
 * @param dsk       Open disk image.
 * @param sb        Superblock.
 * @param inode_idx Index of the file inode.
 * @param offset    Byte offset to start reading from.
 * @param buf       Destination buffer.
 * @param len       Number of bytes to read.
 * @return true on success.
 */
bool file_read(disk &dsk, const superblock &sb, uint64_t inode_idx,
               uint64_t offset, uint8_t *buf, uint64_t len);

/**
 * @brief Delete a file from a directory.
 *
 * @param dsk           Open disk image.
 * @param sb            Superblock.
 * @param dir_inode_idx Index of the parent directory.
 * @param name          Name of the file to remove.
 * @return true on success.
 */
bool file_delete(disk &dsk, const superblock &sb,
                 uint64_t dir_inode_idx, const char *name);

} // namespace xffs
