#pragma once

#include <cstdint>

namespace xffs {

/**
 * @brief Directory entry types.
 */
constexpr uint8_t DIRENT_FILE = 1;
constexpr uint8_t DIRENT_DIR  = 2;

/**
 * @brief Fixed-size directory entry.
 */
#pragma pack(push, 1)
struct dirent {
    uint64_t inode;
    uint16_t name_len;
    uint8_t  type;
    char     name[245];
};
#pragma pack(pop)

}
