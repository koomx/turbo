#ifndef UNICODE_LSX_BITMANIPULATION_H
#define UNICODE_LSX_BITMANIPULATION_H

#include <turbo/unicode/utf.h>
#include <limits>

namespace turbo {
    namespace UNICODE_IMPLEMENTATION {
        namespace {

            KUMO_FORCE_INLINE int count_ones(uint64_t input_num) {
                return __lsx_vpickve2gr_w(__lsx_vpcnt_d(__lsx_vreplgr2vr_d(input_num)), 0);
            }

#if UNICODE_NEED_TRAILING_ZEROES
            KUMO_FORCE_INLINE int trailing_zeroes(uint64_t input_num) {
                return __builtin_ctzll(input_num);
            }
#endif

        } // unnamed namespace
    } // namespace UNICODE_IMPLEMENTATION
} // namespace turbo

#endif // UNICODE_LSX_BITMANIPULATION_H
