#pragma once

#include <cstdint>
#include <vector>

namespace xffs {
    /**
     * @brief Set a bit in the bitmap.
     */
    inline void bitmap_set(std::vector<uint8_t> &bitmap, uint64_t index){
        bitmap[index / 8] |= static_cast<uint8_t>(1u << (index % 8));
    }

    /**
     * @brief Clear a bit in the bitmap.
     */
    inline void bitmap_clear(std::vector<uint8_t> &bitmap, uint64_t index){
        bitmap[index / 8] &= static_cast<uint8_t>(~(1u << (index % 8)));
    }

    /**
     * @brief Test a bit.
     */
    inline bool bitmap_test(std::vector<uint8_t> &bitmap, uint64_t index){
        return (bitmap[index / 8] >> (index % 8)) & 1u;
    }

    /**
     * @brief Find first zero bit and set it.
     *
     * @return block index or UINT64_MAX if full
     */
    inline uint64_t bitmap_alloc(std::vector<uint8_t> &bitmap, uint64_t max_bits){
        for(uint64_t i = 0; i <max_bits; i++){
            if(!bitmap_test(bitmap, i)){
                bitmap_set(bitmap, i);
                return i;
            }
        }
        return UINT64_MAX;
    }
}
