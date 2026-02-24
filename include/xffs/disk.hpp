#pragma once

#include <cstdint>
#include <fstream>
#include <string>

namespace xffs {

/**
 * @brief Simple block device backed by a file.
 */
class disk {
public:
    disk();
    ~disk();

    bool open(const std::string &path);
    void close();

    bool read_block(uint64_t blk, void *buf);
    bool write_block(uint64_t blk, const void *buf);

    bool is_open() const;

private:
    std::fstream file_;
};

} // namespace xffs
