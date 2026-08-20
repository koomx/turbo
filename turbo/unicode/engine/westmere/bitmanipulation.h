#ifndef UNICODE_WESTMERE_BITMANIPULATION_H
#define UNICODE_WESTMERE_BITMANIPULATION_H

namespace turbo {
    namespace UNICODE_IMPLEMENTATION {
        namespace {

#if KUMO_COMPILER_MSVC
            KUMO_FORCE_INLINE unsigned __int64 count_ones(uint64_t input_num) {
                // note: we do not support legacy 32-bit Windows
                return __popcnt64(input_num); // Visual Studio wants two underscores
            }
#else
            KUMO_FORCE_INLINE long long int count_ones(uint64_t input_num) {
                return _popcnt64(input_num);
            }
#endif

#if UNICODE_NEED_TRAILING_ZEROES
            KUMO_FORCE_INLINE int trailing_zeroes(uint64_t input_num) {
#if KUMO_COMPILER_MSVC
                unsigned long ret;
                _BitScanForward64(&ret, input_num);
                return (int)ret;
#else // KUMO_COMPILER_MSVC
                return __builtin_ctzll(input_num);
#endif // KUMO_COMPILER_MSVC
            }
#endif

            template <typename T>
            bool is_power_of_two(T x) {
                return (x & (x - 1)) == 0;
            }

        } // unnamed namespace
    } // namespace UNICODE_IMPLEMENTATION
} // namespace turbo

#endif // UNICODE_WESTMERE_BITMANIPULATION_H
