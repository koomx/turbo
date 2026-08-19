#ifndef SIMDUTF_ICELAKE_BITMANIPULATION_H
#define SIMDUTF_ICELAKE_BITMANIPULATION_H

namespace turbo {
    namespace SIMDUTF_IMPLEMENTATION {
        namespace {

#ifdef SIMDUTF_REGULAR_VISUAL_STUDIO
            KUMO_FORCE_INLINE unsigned __int64 count_ones(uint64_t input_num) {
                // note: we do not support legacy 32-bit Windows
                return __popcnt64(input_num); // Visual Studio wants two underscores
            }
#else
            KUMO_FORCE_INLINE long long int count_ones(uint64_t input_num) {
                return _popcnt64(input_num);
            }
#endif

#ifdef SIMDUTF_REGULAR_VISUAL_STUDIO
            KUMO_FORCE_INLINE unsigned __int64 count_ones32(uint32_t input_num) {
                // note: we do not support legacy 32-bit Windows
                return __popcnt(input_num); // Visual Studio wants two underscores
            }
#else
            KUMO_FORCE_INLINE long long int count_ones32(uint32_t input_num) {
                return _popcnt32(input_num);
            }
#endif

#if SIMDUTF_NEED_TRAILING_ZEROES
// KUMO_FORCE_INLINE int trailing_zeroes(uint64_t input_num) {
//   #if SIMDUTF_REGULAR_VISUAL_STUDIO
//   return (int)_tzcnt_u64(input_num);
//   #else  // SIMDUTF_REGULAR_VISUAL_STUDIO
//   return __builtin_ctzll(input_num);
//   #endif // SIMDUTF_REGULAR_VISUAL_STUDIO
// }
#endif

        } // unnamed namespace
    } // namespace SIMDUTF_IMPLEMENTATION
} // namespace turbo

#endif // SIMDUTF_ICELAKE_BITMANIPULATION_H
