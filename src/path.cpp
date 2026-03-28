#include "xffs/path.hpp"
#include "xffs/dir.hpp"
#include "xffs/superblock.hpp"

#include <cstring>
#include <string>
#include <vector>

namespace xffs {

static std::vector<std::string> split_path(const char *path) {
    std::vector<std::string> parts;
    std::string current;
    for (const char *p = path; *p; p++) {
        if (*p == '/') {
            if (!current.empty()) {
                parts.push_back(current);
                current.clear();
            }
        } else {
            current += *p;
        }
    }
    if (!current.empty()) parts.push_back(current);
    return parts;
}

uint64_t path_resolve(disk &dsk, const superblock &sb, const char *path) {
    if (path == nullptr || path[0] != '/') return UINT64_MAX;

    // Handle root path
    if (std::strcmp(path, "/") == 0) return sb.root_inode;

    std::vector<std::string> parts = split_path(path);
    uint64_t current_inode = sb.root_inode;

    for (const auto &part : parts) {
        current_inode = dir_lookup(dsk, sb, current_inode, part.c_str());
        if (current_inode == UINT64_MAX) return UINT64_MAX;
    }

    return current_inode;
}

} // namespace xffs
