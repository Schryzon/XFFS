#include "xffs/disk.hpp"
#include "xffs/superblock.hpp"
#include "xffs/allocator.hpp"

#include <iostream>
#include <vector>

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

    if(!xffs::format_disk(dsk, 16384)) {
        std::cerr << "format failed\n";
        return 1;
    }

    // LOAD SUPERBLOCK PROPERLY
    std::vector<uint8_t> sb_block(xffs::XFFS_BLOCK_SIZE);
    if(!dsk.read_block(0, sb_block.data())) {
        std::cerr << "failed to read superblock\n";
        return 1;
    }

    const xffs::superblock *sb =
        reinterpret_cast<const xffs::superblock *>(sb_block.data());

    // TEST ALLOCATOR
    uint64_t blk = xffs::alloc_block(dsk, *sb);
    std::cout << "[test] allocated block: " << blk << "\n";

    std::cout << "xffs formatted successfully\n";
    return 0;
}
