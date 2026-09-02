// Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#pragma once

#ifdef __SSSE3__
#include <tmmintrin.h>
#endif

#ifdef _MSC_VER
#include <intrin.h>
#endif

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <limits>
#include <string>
#include <type_traits>

#include <string_view>
#include <turbo/base/nullability.h>
#include <turbo/bits/bits.h>
#include <turbo/bits/endian.h>
#include <turbo/macros/config.h>
#include <turbo/numeric/int128.h>

namespace turbo::format_internal {
    template <typename int_type>
    constexpr bool is_signed() {
        if constexpr (std::is_arithmetic_v<int_type>) {
            // Use std::numeric_limits<T>::is_signed where it's defined to work.
            return std::numeric_limits<int_type>::is_signed;
        }
        // TODO(jorg): This signed-ness check is used because it works correctly
        // with enums, and it also serves to check that int_type is not a pointer.
        // If one day something like std::is_signed<enum E> works, switch to it.
        return static_cast<int_type>(1) - 2 < 0;
    }

    inline constexpr int kFastToBufferSize = 32;

    inline constexpr int kFastToBuffer128Size = 41;
    inline constexpr int kSixDigitsToBufferSize = 16;

    // WARNING: These functions may write more characters than necessary, because
    // they are intended for speed. All functions take an output buffer
    // as an argument and return a pointer to the last byte they wrote, which is the
    // terminating '\0'. The maximum size written is `kFastToBufferSize` for 64-bit
    // integers or less, and `kFastToBuffer128Size` for 128-bit integers.
    char* turbo_nonnull fast_int_to_buffer(int32_t i, char* turbo_nonnull buffer)

        KUMO_INTERNAL_NEED_MIN_SIZE(buffer, kFastToBufferSize);

    char* turbo_nonnull fast_int_to_buffer(uint32_t n, char* turbo_nonnull out_str)

        KUMO_INTERNAL_NEED_MIN_SIZE(out_str, kFastToBufferSize);

    char* turbo_nonnull fast_int_to_buffer(int64_t i, char* turbo_nonnull buffer)

        KUMO_INTERNAL_NEED_MIN_SIZE(buffer, kFastToBufferSize);

    char* turbo_nonnull fast_int_to_buffer(uint64_t i, char* turbo_nonnull buffer)

        KUMO_INTERNAL_NEED_MIN_SIZE(buffer, kFastToBufferSize);

    char* turbo_nonnull fast_int_to_buffer(int128 i, char* turbo_nonnull buffer)

        KUMO_INTERNAL_NEED_MIN_SIZE(buffer, kFastToBuffer128Size);

    char* turbo_nonnull fast_int_to_buffer(uint128 i, char* turbo_nonnull buffer)

        KUMO_INTERNAL_NEED_MIN_SIZE(buffer, kFastToBuffer128Size);

    // For enums and integer types that are up to 128 bits and are not an exact
    // match for the types above, use templates to call the appropriate one of the
    // four overloads above.
    template <typename int_type>
    char* turbo_nonnull fast_int_to_buffer(int_type i,
        char* turbo_nonnull buffer)

        KUMO_INTERNAL_NEED_MIN_SIZE(
            buffer, (sizeof(int_type) > 8 ? kFastToBuffer128Size : kFastToBufferSize)) {
        // These conditions are constexpr bools to suppress MSVC warning C4127.
        constexpr bool kIsSigned = is_signed<int_type>();
        constexpr bool kUse64Bit = sizeof(i) > 32 / 8;
        constexpr bool kUse128Bit = sizeof(i) > 64 / 8;
        if (kIsSigned) {
            if constexpr (kUse128Bit) {
                return fast_int_to_buffer(static_cast<int128>(i), buffer);
            } else if constexpr (kUse64Bit) {
                return fast_int_to_buffer(static_cast<int64_t>(i), buffer);
            } else {
                return fast_int_to_buffer(static_cast<int32_t>(i), buffer);
            }
        } else {
            if constexpr (kUse128Bit) {
                return fast_int_to_buffer(static_cast<uint128>(i), buffer);
            } else if constexpr (kUse64Bit) {
                return fast_int_to_buffer(static_cast<uint64_t>(i), buffer);
            } else {
                return fast_int_to_buffer(static_cast<uint32_t>(i), buffer);
            }
        }
    }

    // Digit conversion.
    KUMO_DLL extern const char kHexChar[17]; // 0123456789abcdef
    KUMO_DLL extern const char
        kHexTable[513]; // 000102030405060708090a0b0c0d0e0f1011...

    // Writes a two-character representation of 'i' to 'buf'. 'i' must be in the
    // range 0 <= i < 100, and buf must have space for two characters. Example:
    //   char buf[2];
    //   put_two_digits(42, buf);
    //   // buf[0] == '4'
    //   // buf[1] == '2'
    void put_two_digits(uint32_t i, char* turbo_nonnull buf);

    // Helper function used to implement turbo::HighPrecision().
    char* turbo_nonnull round_trip_double_to_buffer(double d, char* turbo_nonnull buffer);

    char* turbo_nonnull round_trip_float_to_buffer(float f, char* turbo_nonnull buffer);

    // Helper function for fast formatting of floating-point values.
    // The result is the same as printf's "%g", a.k.a. "%.6g"; that is, six
    // significant digits are returned, trailing zeros are removed, and numbers
    // outside the range 0.0001-999999 are output using scientific notation
    // (1.23456e+06). This routine is heavily optimized.
    // Required buffer size is `kSixDigitsToBufferSize`.
    size_t six_digits_to_buffer(double d, char* turbo_nonnull buffer);


    inline uint32_t fast_digit_count(uint32_t x) noexcept {
        auto int_log2 = [](uint32_t z) -> int {
            return 31 - countl_zero(z | 1);
        };
        const static uint64_t kTable[] = {
            4294967296,  8589934582,  8589934582,  8589934582,  12884901788,
            12884901788, 12884901788, 17179868184, 17179868184, 17179868184,
            21474826480, 21474826480, 21474826480, 21474826480, 25769703776,
            25769703776, 25769703776, 30063771072, 30063771072, 30063771072,
            34349738368, 34349738368, 34349738368, 34349738368, 38554705664,
            38554705664, 38554705664, 41949672960, 41949672960, 41949672960,
            42949672960, 42949672960};
        return static_cast<uint32_t>((x + kTable[int_log2(x)]) >> 32);
    }

} // namespace turbo::format_internal
