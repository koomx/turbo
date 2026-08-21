#ifndef UNICODE_HASWELL_BITMANIPULATION_H
#define UNICODE_HASWELL_BITMANIPULATION_H

#include <turbo/bits/bits.h>

namespace turbo {
    namespace UNICODE_IMPLEMENTATION {
        namespace {

            template <typename T>
            bool is_power_of_two(T x) {
                return (x & (x - 1)) == 0;
            }

        } // unnamed namespace
    } // namespace UNICODE_IMPLEMENTATION
} // namespace turbo

#endif // UNICODE_HASWELL_BITMANIPULATION_H
