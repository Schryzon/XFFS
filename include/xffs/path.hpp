#pragma once

#include "xffs/disk.hpp"
#include "xffs/superblock.hpp"

#include <cstdint>

namespace xffs {

/**
 * @brief Resolve an absolute path into an inode index.
 * 
 * Walks the directory tree starting from the root inode (0).
 * Path must start with '/'.
 * 
 * @param dsk   Open disk image.
 * @param sb    Superblock.
 * @param path  Null-terminated absolute path string.
 * @return Inode index or UINT64_MAX if not found.
 */
uint64_t path_resolve(disk &dsk, const superblock &sb, const char *path);

} // namespace xffs
