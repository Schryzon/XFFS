#include "xffs/dir.hpp"
#include "xffs/inode.hpp"
#include "xffs/inode_io.hpp"
#include "xffs/dirent.hpp"
#include "xffs/disk.hpp"
#include "xffs/superblock.hpp"

#include <cstring>
#include <limits>
#include <vector>

namespace xffs {

bool dir_init(disk &dsk, const superblock &sb, uint64_t inode_idx) {
    inode dir_inode {};
    if(!read_inode(dsk, sb, inode_idx, dir_inode)) {
        return false;
    }

    // Zero out the first data block (all dirent slots empty).
    std::vector<uint8_t> block(XFFS_BLOCK_SIZE, 0);
    return dsk.write_block(dir_inode.direct[0], block.data());
}

bool dir_add(disk &dsk, const superblock &sb, uint64_t dir_inode_idx,
             uint64_t child_inode, uint8_t type, const char *name) {
    inode dir_inode {};
    if(!read_inode(dsk, sb, dir_inode_idx, dir_inode)) {
        return false;
    }

    std::vector<uint8_t> block(XFFS_BLOCK_SIZE);
    if(!dsk.read_block(dir_inode.direct[0], block.data())) {
        return false;
    }

    const uint64_t slots = XFFS_BLOCK_SIZE / sizeof(dirent);

    for(uint64_t i = 0; i < slots; i++) {
        dirent *entry = reinterpret_cast<dirent *>(
            block.data() + i * sizeof(dirent));

        if(entry->inode == 0) {
            // Free slot — write the new entry.
            entry->inode    = child_inode;
            entry->type     = type;
            entry->name_len = static_cast<uint16_t>(std::strlen(name));

            std::memset(entry->name, 0, sizeof(entry->name));
            std::strncpy(entry->name, name, sizeof(entry->name) - 1);

            return dsk.write_block(dir_inode.direct[0], block.data());
        }
    }

    // No free slot in the first block.
    return false;
}

uint64_t dir_lookup(disk &dsk, const superblock &sb, uint64_t dir_inode_idx,
                    const char *name) {
    inode dir_inode {};
    if(!read_inode(dsk, sb, dir_inode_idx, dir_inode)) {
        return UINT64_MAX;
    }

    std::vector<uint8_t> block(XFFS_BLOCK_SIZE);
    if(!dsk.read_block(dir_inode.direct[0], block.data())) {
        return UINT64_MAX;
    }

    const uint64_t slots = XFFS_BLOCK_SIZE / sizeof(dirent);

    for(uint64_t i = 0; i < slots; i++) {
        const dirent *entry = reinterpret_cast<const dirent *>(
            block.data() + i * sizeof(dirent));

        if(entry->inode != 0 &&
           std::strncmp(entry->name, name, sizeof(entry->name)) == 0) {
            return entry->inode;
        }
    }

    return UINT64_MAX;
}

bool dir_remove(disk &dsk, const superblock &sb, uint64_t dir_inode_idx,
                const char *name) {
    inode dir_inode {};
    if(!read_inode(dsk, sb, dir_inode_idx, dir_inode)) {
        return false;
    }

    std::vector<uint8_t> block(XFFS_BLOCK_SIZE);
    if(!dsk.read_block(dir_inode.direct[0], block.data())) {
        return false;
    }

    const uint64_t slots = XFFS_BLOCK_SIZE / sizeof(dirent);

    for(uint64_t i = 0; i < slots; i++) {
        dirent *entry = reinterpret_cast<dirent *>(
            block.data() + i * sizeof(dirent));

        if(entry->inode != 0 &&
           std::strncmp(entry->name, name, sizeof(entry->name)) == 0) {
            entry->inode = 0; // Mark slot as free
            return dsk.write_block(dir_inode.direct[0], block.data());
        }
    }

    return false;
}

std::vector<dirent> dir_list(disk &dsk, const superblock &sb, uint64_t dir_inode_idx) {
    std::vector<dirent> entries;

    inode dir_inode {};
    if(!read_inode(dsk, sb, dir_inode_idx, dir_inode)) {
        return entries;
    }

    if (dir_inode.mode != INODE_DIR) {
        return entries;
    }

    std::vector<uint8_t> block(XFFS_BLOCK_SIZE);
    if(!dsk.read_block(dir_inode.direct[0], block.data())) {
        return entries;
    }

    const uint64_t slots = XFFS_BLOCK_SIZE / sizeof(dirent);

    for(uint64_t i = 0; i < slots; i++) {
        dirent *entry = reinterpret_cast<dirent *>(
            block.data() + i * sizeof(dirent));

        if(entry->inode != 0) {
            entries.push_back(*entry);
        }
    }

    return entries;
}

} // namespace xffs
