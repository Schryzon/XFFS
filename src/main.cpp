#include "xffs/disk.hpp"

#include <iostream>

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

    std::cout << "xffs formatted successfully\n";
    return 0;
}
