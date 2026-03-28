#include "xffs/disk.hpp"
#include "xffs/superblock.hpp"
#include "xffs/inode.hpp"
#include "xffs/inode_io.hpp"
#include "xffs/file_ops.hpp"
#include "xffs/path.hpp"

#include <cassert>
#include <cstdio>
#include <cstring>
#include <vector>

namespace xffs {
    bool format_disk(disk &dsk, uint64_t total_blocks);
}

int main() {
    const char *img = "test_phase3.img";

    {
        std::FILE *f = std::fopen(img, "wb");
        assert(f != nullptr);
        const uint64_t img_size = 8ULL * 1024 * 1024;
        std::vector<uint8_t> buf(img_size, 0);
        std::fwrite(buf.data(), 1, img_size, f);
        std::fclose(f);
    }

    xffs::disk dsk;
    assert(dsk.open(img));

    const uint64_t total_blocks = 2048;
    assert(xffs::format_disk(dsk, total_blocks));

    std::vector<uint8_t> sb_block(xffs::XFFS_BLOCK_SIZE);
    assert(dsk.read_block(0, sb_block.data()));
    const xffs::superblock *sb = reinterpret_cast<const xffs::superblock *>(sb_block.data());

    // --- Phase 3 Tests ---

    // 1. File Creation
    uint64_t ino = xffs::file_create(dsk, *sb, 0, "test.txt");
    assert(ino != UINT64_MAX);
    assert(ino > 0); // Root is 0, this should be next available

    // 2. Path Resolution
    uint64_t resolved = xffs::path_resolve(dsk, *sb, "/test.txt");
    assert(resolved == ino);

    // 3. File Writing
    const char *msg = "Phase 3 test data";
    uint64_t len = std::strlen(msg);
    assert(xffs::file_write(dsk, *sb, ino, 0, reinterpret_cast<const uint8_t *>(msg), len));

    // 4. File Reading
    char read_buf[64] = {0};
    assert(xffs::file_read(dsk, *sb, ino, 0, reinterpret_cast<uint8_t *>(read_buf), len));
    assert(std::strcmp(read_buf, msg) == 0);

    // 5. File Deletion
    assert(xffs::file_delete(dsk, *sb, 0, "test.txt"));
    assert(xffs::path_resolve(dsk, *sb, "/test.txt") == UINT64_MAX);

    // 6. Non-existent path
    assert(xffs::path_resolve(dsk, *sb, "/nonexistent") == UINT64_MAX);

    // 7. Checksum Corruption Test
    uint64_t corrupt_ino = xffs::file_create(dsk, *sb, 0, "corrupt.txt");
    assert(corrupt_ino != UINT64_MAX);
    
    // Manually corrupt the block on disk
    const uint64_t inodes_per_block = xffs::XFFS_BLOCK_SIZE / sizeof(xffs::inode);
    const uint64_t block_offset     = corrupt_ino / inodes_per_block;
    std::vector<uint8_t> corrupt_block(xffs::XFFS_BLOCK_SIZE);
    assert(dsk.read_block(sb->inode_start + block_offset, corrupt_block.data()));
    
    // Flip a bit in the middle of the inode payload
    corrupt_block[ (corrupt_ino % inodes_per_block) * sizeof(xffs::inode) + 10 ] ^= 0xFF;
    assert(dsk.write_block(sb->inode_start + block_offset, corrupt_block.data()));

    // Attempt to read it - should fail due to checksum mismatch
    xffs::inode dummy;
    assert(!xffs::read_inode(dsk, *sb, corrupt_ino, dummy));
    std::printf("[phase4_test] checksum corruption detected successfully\n");

    std::printf("[phase3_test] all assertions passed\n");

    dsk.close();
    std::remove(img);
    return 0;
}
