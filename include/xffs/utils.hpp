#pragma once

#include <cstdint>
#include <cstddef>

namespace xffs {

/**
 * @brief Simple XOR-based checksum for teaching purposes.
 * 
 * Computes a 64-bit checksum by XORing 8-byte chunks.
 * 
 * @param data Pointer to the data buffer.
 * @param len  Length of data in bytes (should be multiple of 8).
 * @return 64-bit checksum.
 */
inline uint64_t compute_checksum(const void *data, size_t len) {
    const uint64_t *ptr = static_cast<const uint64_t *>(data);
    uint64_t sum = 0;
    for (size_t i = 0; i < len / sizeof(uint64_t); i++) {
        sum ^= ptr[i];
    }
    return sum;
}

} // namespace xffs
