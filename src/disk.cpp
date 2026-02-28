#include "xffs/disk.hpp"
#include "xffs/superblock.hpp"

namespace xffs {

disk::disk() = default;
disk::~disk() {
    close();
}

bool disk::open(const std::string &path) {
    file_.open(path,
        std::ios::in |
        std::ios::out |
        std::ios::binary);

    if(!file_.is_open()) {
        // try create
        file_.clear();
        file_.open(path,
            std::ios::out |
            std::ios::binary);
        file_.close();

        file_.open(path,
            std::ios::in |
            std::ios::out |
            std::ios::binary);
    }

    return file_.is_open();
}

void disk::close() {
    if(file_.is_open()) {
        file_.close();
    }
}

bool disk::is_open() const {
    return file_.is_open();
}

bool disk::read_block(uint64_t blk, void *buf) {
    if(!file_.is_open()) {
        return false;
    }

    file_.seekg(blk * XFFS_BLOCK_SIZE);
    file_.read(reinterpret_cast<char *>(buf), XFFS_BLOCK_SIZE);
    return file_.good();
}

bool disk::write_block(uint64_t blk, const void *buf) {
    if(!file_.is_open()) {
        return false;
    }

    file_.seekp(blk * XFFS_BLOCK_SIZE);
    file_.write(reinterpret_cast<const char *>(buf), XFFS_BLOCK_SIZE);
    file_.flush();
    return file_.good();
}

} // namespace xffs
