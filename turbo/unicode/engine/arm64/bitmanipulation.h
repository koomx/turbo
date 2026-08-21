#ifndef UNICODE_ARM64_BITMANIPULATION_H
#define UNICODE_ARM64_BITMANIPULATION_H

#include <turbo/bits/bits.h>

namespace turbo {
    namespace UNICODE_IMPLEMENTATION {
        namespace {

            template <typename T>
            T clear_least_significant_bit(T x) {
                return (x & (x - 1));
            }

        } // unnamed namespace
    } // namespace UNICODE_IMPLEMENTATION
} // namespace turbo

#endif // UNICODE_ARM64_BITMANIPULATION_H
