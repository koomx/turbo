#ifndef SIMDUTF_LSX_BITMANIPULATION_H
#define SIMDUTF_LSX_BITMANIPULATION_H

#include <turbo/unicode/utf.h>
#include <limits>

namespace turbo {
    namespace SIMDUTF_IMPLEMENTATION {
        namespace {

            KUMO_FORCE_INLINE int count_ones(uint64_t input_num) {
                return __lsx_vpickve2gr_w(__lsx_vpcnt_d(__lsx_vreplgr2vr_d(input_num)), 0);
            }

#if SIMDUTF_NEED_TRAILING_ZEROES
            KUMO_FORCE_INLINE int trailing_zeroes(uint64_t input_num) {
                return __builtin_ctzll(input_num);
            }
#endif

        } // unnamed namespace
    } // namespace SIMDUTF_IMPLEMENTATION
} // namespace turbo

#endif // SIMDUTF_LSX_BITMANIPULATION_H
