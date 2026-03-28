#pragma once

#include <cstdint>

namespace xffs {

/**
 * @brief Inode type identifiers.
 */
constexpr uint16_t INODE_FREE = 0;
constexpr uint16_t INODE_FILE = 1;
constexpr uint16_t INODE_DIR  = 2;

constexpr uint32_t XFFS_DIRECT_BLOCKS   = 12;
constexpr uint32_t XFFS_INODE_SIZE      = 256;
constexpr uint32_t XFFS_INODE_PAYLOAD   = 128; // Size of non-reserved fields
constexpr uint32_t XFFS_INODE_RESERVED  = XFFS_INODE_SIZE - XFFS_INODE_PAYLOAD;

/**
 * @brief On-disk inode (256 bytes).
 */
#pragma pack(push, 1)
struct inode {
    uint16_t mode;
    uint16_t links;
    uint32_t uid;
    uint32_t gid;

    uint64_t size;
    uint64_t atime;
    uint64_t mtime;
    uint64_t ctime;

    uint64_t direct[XFFS_DIRECT_BLOCKS];
    uint64_t indirect;

    uint64_t checksum;

    uint8_t reserved[XFFS_INODE_RESERVED];
};
#pragma pack(pop)

}
