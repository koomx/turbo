#ifndef UNICODE_ARM64_BITMANIPULATION_H
#define UNICODE_ARM64_BITMANIPULATION_H

namespace turbo {
    namespace UNICODE_IMPLEMENTATION {
        namespace {

            /* result might be undefined when input_num is zero */
            KUMO_FORCE_INLINE int count_ones(uint64_t input_num) {
#if KUMO_COMPILER_MSVC
                return vaddv_u8(vcnt_u8(vcreate_u8(input_num)));
#else
                // if the system supports SVE or CSSC, __builtin_popcountll
                // might be compiled to fewer single instructions. For CSSC,
                // __builtin_popcountll is compiled to a single instruction.
                return __builtin_popcountll(input_num);
#endif
            }

#if UNICODE_NEED_TRAILING_ZEROES
            KUMO_FORCE_INLINE int trailing_zeroes(uint64_t input_num) {
#if KUMO_COMPILER_MSVC
                unsigned long ret;
                // Search the mask data from least significant bit (LSB)
                // to the most significant bit (MSB) for a set bit (1).
                _BitScanForward64(&ret, input_num);
                return (int)ret;
#else // KUMO_COMPILER_MSVC
                return __builtin_ctzll(input_num);
#endif // KUMO_COMPILER_MSVC
            }
#endif
            template <typename T>
            T clear_least_significant_bit(T x) {
                return (x & (x - 1));
            }

        } // unnamed namespace
    } // namespace UNICODE_IMPLEMENTATION
} // namespace turbo

#endif // UNICODE_ARM64_BITMANIPULATION_H
