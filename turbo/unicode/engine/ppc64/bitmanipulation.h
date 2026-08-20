#ifndef UNICODE_PPC64_BITMANIPULATION_H
#define UNICODE_PPC64_BITMANIPULATION_H

namespace turbo {
    namespace UNICODE_IMPLEMENTATION {
        namespace {

#if KUMO_COMPILER_MSVC
            KUMO_FORCE_INLINE int count_ones(uint64_t input_num) {
                // note: we do not support legacy 32-bit Windows
                return __popcnt64(input_num); // Visual Studio wants two underscores
            }
#else
            KUMO_FORCE_INLINE int count_ones(uint64_t input_num) {
                return __builtin_popcountll(input_num);
            }
#endif

#if UNICODE_NEED_TRAILING_ZEROES
            KUMO_FORCE_INLINE int trailing_zeroes(uint64_t input_num) {
                return __builtin_ctzll(input_num);
            }
#endif

        } // unnamed namespace
    } // namespace UNICODE_IMPLEMENTATION
} // namespace turbo

#endif // UNICODE_PPC64_BITMANIPULATION_H
