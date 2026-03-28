#include "xffs/disk.hpp"
#include "xffs/superblock.hpp"
#include "xffs/inode.hpp"
#include "xffs/inode_io.hpp"
#include "xffs/file_ops.hpp"
#include "xffs/path.hpp"

#include <iostream>
#include <vector>
#include <cstring>

namespace xffs {
    bool format_disk(disk &dsk, uint64_t total_blocks);
}

int main(int argc, char **argv) {
    if(argc < 2) {
        std::cout << "usage: mkxffs <image>\n";
        return 1;
    }

    xffs::disk dsk;
    if(!dsk.open(argv[1])) {
        std::cerr << "failed to open image\n";
        return 1;
    }

    if(!xffs::format_disk(dsk, xffs::XFFS_DEFAULT_TOTAL_BLOCKS)) {
        std::cerr << "format failed\n";
        return 1;
    }

    // --- Demo Phase 3: File Operations ---
    std::cout << "\n--- Phase 3: File Operations Demo ---\n";

    std::vector<uint8_t> sb_block(xffs::XFFS_BLOCK_SIZE);
    if (!dsk.read_block(0, sb_block.data())) return 1;
    const xffs::superblock *sb = reinterpret_cast<const xffs::superblock *>(sb_block.data());

    // 1. Create file "/hello.txt"
    uint64_t ino = xffs::file_create(dsk, *sb, 0, "hello.txt");
    if (ino != UINT64_MAX) {
        std::cout << "[demo] created /hello.txt at inode " << ino << "\n";

        // 2. Write data
        const char *data = "XFFS says hello! This is a test string for Phase 3.";
        uint64_t len = std::strlen(data);
        if (xffs::file_write(dsk, *sb, ino, 0, reinterpret_cast<const uint8_t *>(data), len)) {
            std::cout << "[demo] wrote data to /hello.txt\n";
        }

        // 3. Resolve path
        uint64_t resolved_ino = xffs::path_resolve(dsk, *sb, "/hello.txt");
        std::cout << "[demo] resolved /hello.txt to inode " << resolved_ino << "\n";

        // 4. Read back
        std::vector<uint8_t> read_buf(len + 1, 0);
        if (xffs::file_read(dsk, *sb, resolved_ino, 0, read_buf.data(), len)) {
            std::cout << "[demo] read back: \"" << reinterpret_cast<char *>(read_buf.data()) << "\"\n";
        }

        // 5. Delete file
        if (xffs::file_delete(dsk, *sb, 0, "hello.txt")) {
            std::cout << "[demo] deleted /hello.txt\n";
            uint64_t deleted_ino = xffs::path_resolve(dsk, *sb, "/hello.txt");
            if (deleted_ino == UINT64_MAX) {
                std::cout << "[demo] verified /hello.txt is gone\n";
            }
        }
    } else {
        std::cerr << "[demo] failed to create /hello.txt\n";
    }

    std::cout << "xffs formatted successfully\n";
    return 0;
}
