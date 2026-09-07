// Copyright 2017 The Abseil Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <turbo/strings/ascii.h>

#include <climits>
#include <cstddef>
#include <cstring>
#include <string>

#include <turbo/base/nullability.h>
#include <turbo/macros/config.h>

namespace turbo {

    namespace ascii_internal {

        // Returns whether `c` is in the a-z/A-Z range (w.r.t. `ToUpper`).
        // Implemented by:
        //  1. Pushing the a-z/A-Z range to [SCHAR_MIN, SCHAR_MIN + 26).
        //  2. Comparing to SCHAR_MIN + 26.
        template <bool ToUpper>
        constexpr bool AsciiInAZRange(unsigned char c) {
            constexpr unsigned char sub = (ToUpper ? 'a' : 'A') - SCHAR_MIN;
            constexpr signed char threshold = SCHAR_MIN + 26; // 26 = alphabet size.
            // Using unsigned arithmetic as overflows/underflows are well defined.
            unsigned char u = c - sub;
            // Using signed cmp, as SIMD unsigned cmp isn't available in many platforms.
            return static_cast<signed char>(u) < threshold;
        }

        template <bool ToUpper>
        constexpr bool AsciiInAZRangeNaive(unsigned char c) {
            constexpr unsigned char a = (ToUpper ? 'a' : 'A');
            constexpr unsigned char z = (ToUpper ? 'z' : 'Z');
            return a <= c && c <= z;
        }

        template <bool ToUpper, bool Naive>
        constexpr void AsciiStrCaseFoldImpl(char* turbo_nonnull dst,
            const char* turbo_nullable src,
            size_t size) {
            // The upper- and lowercase versions of ASCII characters differ by only 1 bit.
            // When we need to flip the case, we can xor with this bit to achieve the
            // desired result. Note that the choice of 'a' and 'A' here is arbitrary. We
            // could have chosen 'z' and 'Z', or any other pair of characters as they all
            // have the same single bit difference.
            constexpr unsigned char kAsciiCaseBitFlip = 'a' ^ 'A';

            for (size_t i = 0; i < size; ++i) {
                unsigned char v = static_cast<unsigned char>(src[i]);
                if constexpr (Naive) {
                    v ^= AsciiInAZRangeNaive<ToUpper>(v) ? kAsciiCaseBitFlip : 0;
                } else {
                    v ^= AsciiInAZRange<ToUpper>(v) ? kAsciiCaseBitFlip : 0;
                }
                dst[i] = static_cast<char>(v);
            }
        }

        // Splitting to short and long strings to allow vectorization decisions
        // to be made separately in the long and short cases.
        // Using slightly different implementations so the compiler won't optimize them
        // into the same code (the non-naive version is needed for SIMD, so for short
        // strings it's not important).
        // `src` may be null iff `size` is zero.
        template <bool ToUpper>
        constexpr void AsciiStrCaseFold(char* turbo_nonnull dst,
            const char* turbo_nullable src, size_t size) {
            size < 16 ? AsciiStrCaseFoldImpl<ToUpper, /*Naive=*/true>(dst, src, size)
                      : AsciiStrCaseFoldImpl<ToUpper, /*Naive=*/false>(dst, src, size);
        }

        void str_to_lower(char* turbo_nonnull dst, const char* turbo_nullable src,
            size_t n) {
            return AsciiStrCaseFold<false>(dst, src, n);
        }

        void str_to_upper(char* turbo_nonnull dst, const char* turbo_nullable src,
            size_t n) {
            return AsciiStrCaseFold<true>(dst, src, n);
        }

        static constexpr size_t ValidateAsciiCasefold() {
            constexpr size_t num_chars = 1 + CHAR_MAX - CHAR_MIN;
            size_t incorrect_index = 0;
            char lowered[num_chars] = { };
            char uppered[num_chars] = { };
            for (unsigned int i = 0; i < num_chars; ++i) {
                uppered[i] = lowered[i] = static_cast<char>(i);
            }
            AsciiStrCaseFold<false>(&lowered[0], &lowered[0], num_chars);
            AsciiStrCaseFold<true>(&uppered[0], &uppered[0], num_chars);
            for (size_t i = 0; i < num_chars; ++i) {
                const char ch = static_cast<char>(i),
                           ch_upper = ('a' <= ch && ch <= 'z' ? 'A' + (ch - 'a') : ch),
                           ch_lower = ('A' <= ch && ch <= 'Z' ? 'a' + (ch - 'A') : ch);
                if (uppered[i] != ch_upper || lowered[i] != ch_lower) {
                    incorrect_index = i > 0 ? i : num_chars;
                    break;
                }
            }
            return incorrect_index;
        }

        static_assert(ValidateAsciiCasefold() == 0, "error in case conversion");

    } // namespace ascii_internal

    void str_to_lower(std::string* turbo_nonnull s) {
        char* p = &(*s)[0];
        return ascii_internal::AsciiStrCaseFold<false>(p, p, s->size());
    }

    void str_to_upper(std::string* turbo_nonnull s) {
        char* p = &(*s)[0];
        return ascii_internal::AsciiStrCaseFold<true>(p, p, s->size());
    }

    bool ascii_has_upper_case(const char* input, size_t length) {
        auto broadcast = [](uint8_t v) -> uint64_t {
            return 0x101010101010101ull * v;
        };
        uint64_t broadcast_80 = broadcast(0x80);
        uint64_t broadcast_Ap = broadcast(128 - 'A');
        uint64_t broadcast_Zp = broadcast(128 - 'Z' - 1);
        size_t i = 0;

        uint64_t runner { 0 };

        for (; i + 7 < length; i += 8) {
            uint64_t word { };
            memcpy(&word, input + i, sizeof(word));
            runner |= (((word + broadcast_Ap) ^ (word + broadcast_Zp)) & broadcast_80);
        }
        if (i < length) {
            uint64_t word { };
            memcpy(&word, input + i, length - i);
            runner |= (((word + broadcast_Ap) ^ (word + broadcast_Zp)) & broadcast_80);
        }
        return runner != 0;
    }

    bool ascii_has_lower_case(const char* input, size_t length) {
        auto broadcast = [](uint8_t v) -> uint64_t {
            return 0x101010101010101ull * v;
        };
        uint64_t broadcast_80 = broadcast(0x80);
        uint64_t broadcast_ap = broadcast(128 - 'a');
        uint64_t broadcast_zp = broadcast(128 - 'z' - 1);
        size_t i = 0;

        uint64_t runner { 0 };

        for (; i + 7 < length; i += 8) {
            uint64_t word { };
            memcpy(&word, input + i, sizeof(word));
            runner |= (((word + broadcast_ap) ^ (word + broadcast_zp)) & broadcast_80);
        }
        if (i < length) {
            uint64_t word { };
            memcpy(&word, input + i, length - i);
            runner |= (((word + broadcast_ap) ^ (word + broadcast_zp)) & broadcast_80);
        }
        return runner != 0;
    }


} // namespace turbo
