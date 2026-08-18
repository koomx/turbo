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

#include <algorithm>
#include <clocale>
#include <turbo/format/fast_to_buffer.h>

namespace turbo::format_internal {
    // Digit conversion.
    KUMO_CONST_INIT KUMO_DLL const char kHexChar[] = "0123456789abcdef";

    KUMO_CONST_INIT KUMO_DLL const char kHexTable[513] = "000102030405060708090a0b0c0d0e0f"
                                                         "101112131415161718191a1b1c1d1e1f"
                                                         "202122232425262728292a2b2c2d2e2f"
                                                         "303132333435363738393a3b3c3d3e3f"
                                                         "404142434445464748494a4b4c4d4e4f"
                                                         "505152535455565758595a5b5c5d5e5f"
                                                         "606162636465666768696a6b6c6d6e6f"
                                                         "707172737475767778797a7b7c7d7e7f"
                                                         "808182838485868788898a8b8c8d8e8f"
                                                         "909192939495969798999a9b9c9d9e9f"
                                                         "a0a1a2a3a4a5a6a7a8a9aaabacadaeaf"
                                                         "b0b1b2b3b4b5b6b7b8b9babbbcbdbebf"
                                                         "c0c1c2c3c4c5c6c7c8c9cacbcccdcecf"
                                                         "d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"
                                                         "e0e1e2e3e4e5e6e7e8e9eaebecedeeef"
                                                         "f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff";

    namespace {
        // Various routines to encode integers to strings.

        // We split data encodings into a group of 2 digits, 4 digits, 8 digits as
        // it's easier to combine powers of two into scalar arithmetic.

        // Previous implementation used a lookup table of 200 bytes for every 2 bytes
        // and it was memory bound, any L1 cache miss would result in a much slower
        // result. When benchmarking with a cache eviction rate of several percent,
        // this implementation proved to be better.

        // These constants represent '00', '0000' and '00000000' as ascii strings in
        // integers. We can add these numbers if we encode to bytes from 0 to 9. as
        // 'i' = '0' + i for 0 <= i <= 9.
        constexpr uint32_t kTwoZeroBytes = 0x0101 * '0';
        constexpr uint64_t kFourZeroBytes = 0x01010101 * '0';
        constexpr uint64_t kEightZeroBytes = 0x0101010101010101ull * '0';

        // * 103 / 1024 is a division by 10 for values from 0 to 99. It's also a
        // division of a structure [k takes 2 bytes][m takes 2 bytes], then * 103 / 1024
        // will be [k / 10][m / 10]. It allows parallel division.
        constexpr uint64_t kDivisionBy10Mul = 103u;
        constexpr uint64_t kDivisionBy10Div = 1 << 10;

        // * 10486 / 1048576 is a division by 100 for values from 0 to 9999.
        constexpr uint64_t kDivisionBy100Mul = 10486u;
        constexpr uint64_t kDivisionBy100Div = 1 << 20;

        // Encode functions write the ASCII output of input `n` to `out_str`.
        inline char* EncodeHundred(uint32_t n, char* turbo_nonnull out_str) {
            int num_digits = static_cast<int>(n - 10) >> 8;
            uint32_t div10 = (n * kDivisionBy10Mul) / kDivisionBy10Div;
            uint32_t mod10 = n - 10u * div10;
            uint32_t base = kTwoZeroBytes + div10 + (mod10 << 8);
            base >>= num_digits & 8;
            little_endian::Store16(out_str, static_cast<uint16_t>(base));
            return out_str + 2 + num_digits;
        }

        inline char* EncodeTenThousand(uint32_t n, char* turbo_nonnull out_str) {
            // We split lower 2 digits and upper 2 digits of n into 2 byte consecutive
            // blocks. 123 ->  [\0\1][\0\23]. We divide by 10 both blocks
            // (it's 1 division + zeroing upper bits), and compute modulo 10 as well "in
            // parallel". Then we combine both results to have both ASCII digits,
            // strip trailing zeros, add ASCII '0000' and return.
            uint32_t div100 = (n * kDivisionBy100Mul) / kDivisionBy100Div;
            uint32_t mod100 = n - 100ull * div100;
            uint32_t hundreds = (mod100 << 16) + div100;
            uint32_t tens = (hundreds * kDivisionBy10Mul) / kDivisionBy10Div;
            tens &= (0xFull << 16) | 0xFull;
            tens += (hundreds - 10ull * tens) << 8;
            KUMO_ASSUME(tens != 0);
            // The result can contain trailing zero bits, we need to strip them to a first
            // significant byte in a final representation. For example, for n = 123, we
            // have tens to have representation \0\1\2\3. We do `& -8` to round
            // to a multiple to 8 to strip zero bytes, not all zero bits.
            // countr_zero to help.
            // 0 minus 8 to make MSVC happy.
            uint32_t zeroes = static_cast<uint32_t>(turbo::countr_zero(tens)) & (0 - 8u);
            tens += kFourZeroBytes;
            tens >>= zeroes;
            little_endian::Store32(out_str, tens);
            return out_str + sizeof(tens) - zeroes / 8;
        }

        // Helper function to produce an ASCII representation of `i`.
        //
        // Function returns an 8-byte integer which when summed with `kEightZeroBytes`,
        // can be treated as a printable buffer with ascii representation of `i`,
        // possibly with leading zeros.
        //
        // Example:
        //
        //  uint64_t buffer = PrepareEightDigits(102030) + kEightZeroBytes;
        //  char* ascii = reinterpret_cast<char*>(&buffer);
        //  // Note two leading zeros:
        //  EXPECT_EQ(std::string_view(ascii, 8), "00102030");
        //
        // Pre-condition: `i` must be less than 100000000.
        inline uint64_t PrepareEightDigits(uint32_t i) {
            KUMO_ASSUME(i < 10000'0000);
            // Prepare 2 blocks of 4 digits "in parallel".
            uint32_t hi = i / 10000;
            uint32_t lo = i % 10000;
            uint64_t merged = hi | (uint64_t { lo } << 32);
            uint64_t div100 = ((merged * kDivisionBy100Mul) / kDivisionBy100Div) & ((0x7Full << 32) | 0x7Full);
            uint64_t mod100 = merged - 100ull * div100;
            uint64_t hundreds = (mod100 << 16) + div100;
            uint64_t tens = (hundreds * kDivisionBy10Mul) / kDivisionBy10Div;
            tens &= (0xFull << 48) | (0xFull << 32) | (0xFull << 16) | 0xFull;
            tens += (hundreds - 10ull * tens) << 8;
            return tens;
        }

        // Encodes v to buffer as 16 digits padded with leading zeros.
        // Pre-condition: v must be < 10^16.
        inline char* EncodePadded16(uint64_t v, char* turbo_nonnull buffer) {
            constexpr uint64_t k1e8 = 100000000;
            uint32_t hi = static_cast<uint32_t>(v / k1e8);
            uint32_t lo = static_cast<uint32_t>(v % k1e8);
            little_endian::Store64(buffer, PrepareEightDigits(hi) + kEightZeroBytes);
            little_endian::Store64(buffer + 8, PrepareEightDigits(lo) + kEightZeroBytes);
            return buffer + 16;
        }

        KUMO_FORCE_INLINE char* turbo_nonnull EncodeFullU32(
            uint32_t n, char* turbo_nonnull out_str) {
            if (n < 10) {
                *out_str = static_cast<char>('0' + n);
                return out_str + 1;
            }
            if (n < 100'000'000) {
                uint64_t bottom = PrepareEightDigits(n);
                KUMO_ASSUME(bottom != 0);
                // 0 minus 8 to make MSVC happy.
                uint32_t zeroes = static_cast<uint32_t>(turbo::countr_zero(bottom)) & (0 - 8u);
                little_endian::Store64(out_str, (bottom + kEightZeroBytes) >> zeroes);
                return out_str + sizeof(bottom) - zeroes / 8;
            }
            uint32_t div08 = n / 100'000'000;
            uint32_t mod08 = n % 100'000'000;
            uint64_t bottom = PrepareEightDigits(mod08) + kEightZeroBytes;
            out_str = EncodeHundred(div08, out_str);
            little_endian::Store64(out_str, bottom);
            return out_str + sizeof(bottom);
        }

        KUMO_FORCE_INLINE char* turbo_nonnull EncodeFullU64(
            uint64_t i, char* turbo_nonnull buffer) {
            if (i <= std::numeric_limits<uint32_t>::max()) {
                return EncodeFullU32(static_cast<uint32_t>(i), buffer);
            }
            uint32_t mod08;
            if (i < 1'0000'0000'0000'0000ull) {
                uint32_t div08 = static_cast<uint32_t>(i / 100'000'000ull);
                mod08 = static_cast<uint32_t>(i % 100'000'000ull);
                buffer = EncodeFullU32(div08, buffer);
            } else {
                uint64_t div08 = i / 100'000'000ull;
                mod08 = static_cast<uint32_t>(i % 100'000'000ull);
                uint32_t div016 = static_cast<uint32_t>(div08 / 100'000'000ull);
                uint32_t div08mod08 = static_cast<uint32_t>(div08 % 100'000'000ull);
                uint64_t mid_result = PrepareEightDigits(div08mod08) + kEightZeroBytes;
                buffer = EncodeTenThousand(div016, buffer);
                little_endian::Store64(buffer, mid_result);
                buffer += sizeof(mid_result);
            }
            uint64_t mod_result = PrepareEightDigits(mod08) + kEightZeroBytes;
            little_endian::Store64(buffer, mod_result);
            return buffer + sizeof(mod_result);
        }

        KUMO_FORCE_INLINE char* turbo_nonnull EncodeFullU128(
            uint128 i, char* turbo_nonnull buffer) {
            if (turbo::Uint128High64(i) == 0) {
                return EncodeFullU64(turbo::Uint128Low64(i), buffer);
            }
            // We divide the number into 16-digit chunks because `EncodePadded16` is
            // optimized to handle 16 digits at a time (as two 8-digit chunks).
            constexpr uint64_t k1e16 = uint64_t { 10'000'000'000'000'000 };
            uint128 high = i / k1e16;
            uint64_t low = turbo::Uint128Low64(i % k1e16);
            uint64_t mid = turbo::Uint128Low64(high % k1e16);
            high /= k1e16;

            if (high == 0) {
                buffer = EncodeFullU64(mid, buffer);
                buffer = EncodePadded16(low, buffer);
            } else {
                buffer = EncodeFullU64(turbo::Uint128Low64(high), buffer);
                buffer = EncodePadded16(mid, buffer);
                buffer = EncodePadded16(low, buffer);
            }
            return buffer;
        }
    }

    char* turbo_nonnull fast_int_to_buffer(
        uint32_t n, char* turbo_nonnull out_str) {
        out_str = EncodeFullU32(n, out_str);
        *out_str = '\0';
        return out_str;
    }

    char* turbo_nonnull fast_int_to_buffer(
        int32_t i, char* turbo_nonnull buffer) {
        uint32_t u = static_cast<uint32_t>(i);
        if (i < 0) {
            *buffer++ = '-';
            // We need to do the negation in modular (i.e., "unsigned")
            // arithmetic; MSVC++ apparently warns for plain "-u", so
            // we write the equivalent expression "0 - u" instead.
            u = 0 - u;
        }
        buffer = EncodeFullU32(u, buffer);
        *buffer = '\0';
        return buffer;
    }

    char* turbo_nonnull fast_int_to_buffer(
        uint64_t i, char* turbo_nonnull buffer) {
        buffer = EncodeFullU64(i, buffer);
        *buffer = '\0';
        return buffer;
    }

    char* turbo_nonnull fast_int_to_buffer(
        int64_t i, char* turbo_nonnull buffer) {
        uint64_t u = static_cast<uint64_t>(i);
        if (i < 0) {
            *buffer++ = '-';
            // We need to do the negation in modular (i.e., "unsigned")
            // arithmetic; MSVC++ apparently warns for plain "-u", so
            // we write the equivalent expression "0 - u" instead.
            u = 0 - u;
        }
        buffer = EncodeFullU64(u, buffer);
        *buffer = '\0';
        return buffer;
    }

    char* turbo_nonnull fast_int_to_buffer(
        uint128 i, char* turbo_nonnull buffer) {
        buffer = EncodeFullU128(i, buffer);
        *buffer = '\0';
        return buffer;
    }

    char* turbo_nonnull fast_int_to_buffer(
        int128 i, char* turbo_nonnull buffer) {
        uint128 u = static_cast<uint128>(i);
        if (i < 0) {
            *buffer++ = '-';
            u = -u;
        }
        buffer = EncodeFullU128(u, buffer);
        *buffer = '\0';
        return buffer;
    }

    void put_two_digits(uint32_t i, char* turbo_nonnull buf) {
        assert(i < 100);
        uint32_t base = kTwoZeroBytes;
        uint32_t div10 = (i * kDivisionBy10Mul) / kDivisionBy10Div;
        uint32_t mod10 = i - 10u * div10;
        base += div10 + (mod10 << 8);
        little_endian::Store16(buf, static_cast<uint16_t>(base));
    }

    // Utility routine(s) for round_trip_float_to_buffer:
    // OutputNecessaryDigits takes two 11-digit numbers, whose integer portion
    // represents the fractional part of a floating-point number, and outputs a
    // number that is in-between them, with the fewest digits possible. For
    // instance, given 12345678900 and 12345876900, it would output "0123457".
    // When there are multiple final digits that would satisfy this requirement,
    // this routine attempts to use a digit that would represent the average of
    // lower_double and upper_double.
    //
    // Although the routine works using integers, all callers use doubles, so
    // for their convenience this routine accepts doubles.
    static char* turbo_nonnull OutputNecessaryDigits(double lower_double,
        double upper_double,
        char* turbo_nonnull out) {
        assert(lower_double > 0);
        assert(lower_double < upper_double - 10);
        assert(upper_double < 100000000000.0);

        // Narrow the range a bit; without this bias, an input of lower=87654320010.0
        // and upper=87654320100.0 would produce an output of 876543201
        //
        // We do this in three steps: first, we lower the upper bound and truncate it
        // to an integer.  Then, we increase the lower bound by exactly the amount we
        // just decreased the upper bound by - at that point, the midpoint is exactly
        // where it used to be.  Then we truncate the lower bound.

        uint64_t upper64 = static_cast<uint64_t>(upper_double - (1.0 / 1024));
        double shrink = upper_double - upper64;
        uint64_t lower64 = static_cast<uint64_t>(lower_double + shrink);

        // Theory of operation: we convert the lower number to ascii representation,
        // two digits at a time.  As we go, we remove the same digits from the upper
        // number.  When we see the upper number does not share those same digits, we
        // know we can stop converting. When we stop, the last digit we output is
        // taken from the average of upper and lower values, rounded up.
        char buf[2];
        uint32_t lodigits = static_cast<uint32_t>(lower64 / 1000000000); // 1,000,000,000
        uint64_t mul64 = lodigits * uint64_t { 1000000000 };

        put_two_digits(lodigits, out);
        out += 2;
        if (upper64 - mul64 >= 1000000000) {
            // digit mismatch!
            put_two_digits(static_cast<uint32_t>(upper64 / 1000000000),
                buf);
            if (out[-2] != buf[0]) {
                out[-2] = static_cast<char>('0' + (upper64 + lower64 + 10000000000) / 20000000000);
                --out;
            } else {
                put_two_digits(
                    static_cast<uint32_t>((upper64 + lower64 + 1000000000) / 2000000000),
                    out - 2);
            }
            *out = '\0';
            return out;
        }
        uint32_t lower = static_cast<uint32_t>(lower64 - mul64);
        uint32_t upper = static_cast<uint32_t>(upper64 - mul64);

        lodigits = lower / 10000000; // 10,000,000
        uint32_t mul = lodigits * 10000000;
        put_two_digits(lodigits, out);
        out += 2;
        if (upper - mul >= 10000000) {
            // digit mismatch!
            put_two_digits(upper / 10000000, buf);
            if (out[-2] != buf[0]) {
                out[-2] = '0' + (upper + lower + 100000000) / 200000000;
                --out;
            } else {
                put_two_digits((upper + lower + 10000000) / 20000000,
                    out - 2);
            }
            *out = '\0';
            return out;
        }
        lower -= mul;
        upper -= mul;

        lodigits = lower / 100000; // 100,000
        mul = lodigits * 100000;
        put_two_digits(lodigits, out);
        out += 2;
        if (upper - mul >= 100000) {
            // digit mismatch!
            put_two_digits(upper / 100000, buf);
            if (out[-2] != buf[0]) {
                out[-2] = static_cast<char>('0' + (upper + lower + 1000000) / 2000000);
                --out;
            } else {
                put_two_digits((upper + lower + 100000) / 200000,
                    out - 2);
            }
            *out = '\0';
            return out;
        }
        lower -= mul;
        upper -= mul;

        lodigits = lower / 1000;
        mul = lodigits * 1000;
        put_two_digits(lodigits, out);
        out += 2;
        if (upper - mul >= 1000) {
            // digit mismatch!
            put_two_digits(upper / 1000, buf);
            if (out[-2] != buf[0]) {
                out[-2] = static_cast<char>('0' + (upper + lower + 10000) / 20000);
                --out;
            } else {
                put_two_digits((upper + lower + 1000) / 2000, out - 2);
            }
            *out = '\0';
            return out;
        }
        lower -= mul;
        upper -= mul;

        put_two_digits(lower / 10, out);
        out += 2;
        put_two_digits(upper / 10, buf);
        if (out[-2] != buf[0]) {
            out[-2] = static_cast<char>('0' + (upper + lower + 100) / 200);
            --out;
        } else {
            put_two_digits((upper + lower + 10) / 20, out - 2);
        }
        *out = '\0';
        return out;
    }

    // This table is used to quickly calculate the base-ten exponent of a given
    // float, and then to provide a multiplier to bring that number into the
    // range 1-999,999,999, that is, into uint32_t range.  Finally, the exp
    // string is made available so there is one less int-to-string conversion
    // to be done.
    struct Spec {
        double min_range;
        double multiplier;
        const char expstr[5];
    };

    // clang-format off
    constexpr Spec kPosExpTable[] = {
        Spec{1e+08f, 1e+02, "e+08"},
        Spec{1e+09f, 1e+01, "e+09"},
        Spec{1e+10f, 1e+00, "e+10"},
        Spec{1e+11f, 1e-01, "e+11"},
        Spec{1e+12f, 1e-02, "e+12"},
        Spec{1e+13f, 1e-03, "e+13"},
        Spec{1e+14f, 1e-04, "e+14"},
        Spec{1e+15f, 1e-05, "e+15"},
        Spec{1e+16f, 1e-06, "e+16"},
        Spec{1e+17f, 1e-07, "e+17"},
        Spec{1e+18f, 1e-08, "e+18"},
        Spec{1e+19f, 1e-09, "e+19"},
        Spec{1e+20f, 1e-10, "e+20"},
        Spec{1e+21f, 1e-11, "e+21"},
        Spec{1e+22f, 1e-12, "e+22"},
        Spec{1e+23f, 1e-13, "e+23"},
        Spec{1e+24f, 1e-14, "e+24"},
        Spec{1e+25f, 1e-15, "e+25"},
        Spec{1e+26f, 1e-16, "e+26"},
        Spec{1e+27f, 1e-17, "e+27"},
        Spec{1e+28f, 1e-18, "e+28"},
        Spec{1e+29f, 1e-19, "e+29"},
        Spec{1e+30f, 1e-20, "e+30"},
        Spec{1e+31f, 1e-21, "e+31"},
        Spec{1e+32f, 1e-22, "e+32"},
        Spec{1e+33f, 1e-23, "e+33"},
        Spec{1e+34f, 1e-24, "e+34"},
        Spec{1e+35f, 1e-25, "e+35"},
        Spec{1e+36f, 1e-26, "e+36"},
        Spec{1e+37f, 1e-27, "e+37"},
        Spec{1e+38f, 1e-28, "e+38"},
        Spec{1e+39,  1e-29, "e+39"},
    };
    // clang-format on

    // clang-format off
    constexpr Spec kNegExpTable[] = {
        Spec{1.4e-45f, 1e+55, "e-45"},
        Spec{1e-44f, 1e+54, "e-44"},
        Spec{1e-43f, 1e+53, "e-43"},
        Spec{1e-42f, 1e+52, "e-42"},
        Spec{1e-41f, 1e+51, "e-41"},
        Spec{1e-40f, 1e+50, "e-40"},
        Spec{1e-39f, 1e+49, "e-39"},
        Spec{1e-38f, 1e+48, "e-38"},
        Spec{1e-37f, 1e+47, "e-37"},
        Spec{1e-36f, 1e+46, "e-36"},
        Spec{1e-35f, 1e+45, "e-35"},
        Spec{1e-34f, 1e+44, "e-34"},
        Spec{1e-33f, 1e+43, "e-33"},
        Spec{1e-32f, 1e+42, "e-32"},
        Spec{1e-31f, 1e+41, "e-31"},
        Spec{1e-30f, 1e+40, "e-30"},
        Spec{1e-29f, 1e+39, "e-29"},
        Spec{1e-28f, 1e+38, "e-28"},
        Spec{1e-27f, 1e+37, "e-27"},
        Spec{1e-26f, 1e+36, "e-26"},
        Spec{1e-25f, 1e+35, "e-25"},
        Spec{1e-24f, 1e+34, "e-24"},
        Spec{1e-23f, 1e+33, "e-23"},
        Spec{1e-22f, 1e+32, "e-22"},
        Spec{1e-21f, 1e+31, "e-21"},
        Spec{1e-20f, 1e+30, "e-20"},
        Spec{1e-19f, 1e+29, "e-19"},
        Spec{1e-18f, 1e+28, "e-18"},
        Spec{1e-17f, 1e+27, "e-17"},
        Spec{1e-16f, 1e+26, "e-16"},
        Spec{1e-15f, 1e+25, "e-15"},
        Spec{1e-14f, 1e+24, "e-14"},
        Spec{1e-13f, 1e+23, "e-13"},
        Spec{1e-12f, 1e+22, "e-12"},
        Spec{1e-11f, 1e+21, "e-11"},
        Spec{1e-10f, 1e+20, "e-10"},
        Spec{1e-09f, 1e+19, "e-09"},
        Spec{1e-08f, 1e+18, "e-08"},
        Spec{1e-07f, 1e+17, "e-07"},
        Spec{1e-06f, 1e+16, "e-06"},
        Spec{1e-05f, 1e+15, "e-05"},
        Spec{1e-04f, 1e+14, "e-04"},
    };
    // clang-format on

    struct ExpCompare {
        bool operator()(const Spec& spec, double d) const {
            return spec.min_range < d;
        }
    };

    // round_trip_float_to_buffer converts the given float into a string which, if
    // passed to strtof, will produce the exact same original float.  It does this
    // by computing the range of possible doubles which map to the given float, and
    // then examining the digits of the doubles in that range.  If all the doubles
    // in the range start with "2.37", then clearly our float does, too.  As soon as
    // they diverge, only one more digit is needed.
    char* turbo_nonnull round_trip_float_to_buffer(
        float f, char* turbo_nonnull buffer) {
        static_assert(std::numeric_limits<float>::is_iec559,
            "IEEE-754/IEC-559 support only");

        // We write data to out, incrementing as we go, but FloatToBuffer always
        // returns the address of the buffer passed in.
        char* out = buffer;

        if (std::isnan(f)) {
            strcpy(out, "nan"); // NOLINT(runtime/printf)
            return buffer;
        }
        if (f == 0) {
            // +0 and -0 are handled here
            if (std::signbit(f)) {
                strcpy(out, "-0"); // NOLINT(runtime/printf)
            } else {
                strcpy(out, "0"); // NOLINT(runtime/printf)
            }
            return buffer;
        }
        if (f < 0) {
            *out++ = '-';
            f = -f;
        }
        if (f > std::numeric_limits<float>::max()) {
            strcpy(out, "inf"); // NOLINT(runtime/printf)
            return buffer;
        }

        double next_lower = nextafterf(f, 0.0f);
        // For all doubles in the range lower_bound < f < upper_bound, the
        // nearest float is f.
        double lower_bound = (f + next_lower) * 0.5;
        double upper_bound = f + (f - lower_bound);
        // Note: because std::nextafter is slow, we calculate upper_bound
        // assuming that it is the same distance from f as lower_bound is.
        // For exact powers of two, upper_bound is actually twice as far
        // from f as lower_bound is, but this turns out not to matter.

        // Most callers pass floats that are either 0 or within the
        // range 0.0001 through 100,000,000, so handle those first,
        // since they don't need exponential notation.
        const Spec* spec = nullptr;
        if (f < 1.0) {
            if (f >= 0.0001f) {
                // For fractional values, we set up the multiplier at the same
                // time as we output the leading "0." / "0.0" / "0.00" / "0.000"
                double multiplier = 1e+11;
                *out++ = '0';
                *out++ = '.';
                if (f < 0.1f) {
                    multiplier = 1e+12;
                    *out++ = '0';
                    if (f < 0.01f) {
                        multiplier = 1e+13;
                        *out++ = '0';
                        if (f < 0.001f) {
                            multiplier = 1e+14;
                            *out++ = '0';
                        }
                    }
                }
                OutputNecessaryDigits(lower_bound * multiplier, upper_bound * multiplier,
                    out);
                return buffer;
            }
            spec = std::lower_bound(std::begin(kNegExpTable), std::end(kNegExpTable),
                double { f }, ExpCompare());
            if (spec == std::end(kNegExpTable))
                --spec;
        } else if (f < 1e8) {
            // Handling non-exponential format greater than 1.0 is similar to the above,
            // but instead of 0.0 / 0.00 / 0.000, the prefix is simply the truncated
            // integer part of f.
            int32_t as_int = static_cast<int32_t>(f);
            out = fast_int_to_buffer(as_int, out);
            // Easy: if the integer part is within (lower_bound, upper_bound), then we
            // are already done.
            if (as_int > lower_bound && as_int < upper_bound) {
                return buffer;
            }
            *out++ = '.';
            OutputNecessaryDigits((lower_bound - as_int) * 1e11,
                (upper_bound - as_int) * 1e11, out);
            return buffer;
        } else {
            spec = std::lower_bound(std::begin(kPosExpTable), std::end(kPosExpTable),
                double { f }, ExpCompare());
            if (spec == std::end(kPosExpTable))
                --spec;
        }
        // Exponential notation from here on.  "spec" was computed using lower_bound,
        // which means it's the first spec from the table where min_range is greater
        // or equal to f.
        // Unfortunately that's not quite what we want; we want a min_range that is
        // less or equal.  So first thing, if it was greater, back up one entry.
        if (spec->min_range > f)
            --spec;

        // The digits might be "237000123", but we want "2.37000123",
        // so we output the digits one character later, and then move the first
        // digit back so we can stick the "." in.
        char* start = out;
        out = OutputNecessaryDigits(lower_bound * spec->multiplier,
            upper_bound * spec->multiplier, start + 1);
        start[0] = start[1];
        start[1] = '.';

        // If it turns out there was only one digit output, then back up over the '.'
        if (out == &start[2])
            --out;

        // Now add the "e+NN" part.
        memcpy(out, spec->expstr, 4);
        out[4] = '\0';
        return buffer;
    }

    // Although DBL_DIG is typically 15, DBL_MAX is normally represented with 17
    // digits of precision. When converted to a string value with fewer digits
    // of precision using strtod(), the result can be bigger than DBL_MAX due to
    // a rounding error. Converting this value back to a double will produce an
    // Inf which will trigger a SIGFPE if FP exceptions are enabled. We skip
    // the precision check for sufficiently large values to avoid the SIGFPE.
    static constexpr double kDoublePrecisionCheckMax = std::numeric_limits<double>::max() / 1.000000000000001;

    char* turbo_nonnull round_trip_double_to_buffer(
        double d, char* turbo_nonnull buffer) {
        // DBL_DIG is 15 for IEEE-754 doubles, which are used on almost all
        // platforms these days.  Just in case some system exists where DBL_DIG
        // is significantly larger -- and risks overflowing our buffer -- we have
        // this assert.
        static_assert(std::numeric_limits<double>::digits10 < 20,
            "std::numeric_limits<double>::digits10 is too big");

        // We avoid snprintf for NaNs because it inconsistently outputs "-nan"
        // or "nan" when given a nan with the sign bit set.
        if (std::isnan(d)) {
            strcpy(buffer, "nan"); // NOLINT(runtime/printf)
            return buffer;
        }
        bool full_precision_needed = true;
        if (std::abs(d) <= kDoublePrecisionCheckMax) {
            int snprintf_result = snprintf(buffer, kFastToBufferSize, "%.*g",
                std::numeric_limits<double>::digits10, d);

            // The snprintf should never overflow because the buffer is significantly
            // larger than the precision we asked for.
            KUMO_ASSERT(snprintf_result > 0 && snprintf_result < kFastToBufferSize);

            full_precision_needed = strtod(buffer, nullptr) != d;
        }

        if (full_precision_needed) {
            int snprintf_result = snprintf(buffer, kFastToBufferSize, "%.*g",
                std::numeric_limits<double>::digits10 + 2, d);

            // Should never overflow; see above.
            KUMO_ASSERT(snprintf_result > 0 && snprintf_result < kFastToBufferSize);
        }

        // snprintf() writes the radix character chosen by the global C locale's
        // LC_NUMERIC category, so a process that has called setlocale() can end up
        // with a separator other than '.' here. The rest of Abseil's float formatting
        // (round_trip_float_to_buffer, six_digits_to_buffer) is locale- independent and
        // SimpleAtod() only accepts '.', so rewrite the radix back to '.' to keep
        // turbo::HighPrecision(double) locale-independent and round-trippable through
        // SimpleAtod().
        // TODO: b/526633099 - Once all supported compilers ship std::to_chars with
        // floating-point support, use it here for inherent locale independence.
        const char* radix = localeconv()->decimal_point;
        // Skip an empty decimal_point (some minimal environments leave it ""), which
        // would otherwise match the beginning of the buffer and corrupt it.
        if (radix[0] != '\0' && std::strcmp(radix, ".") != 0) {
            if (char* p = std::strstr(buffer, radix)) {
                const size_t radix_len = std::strlen(radix);
                *p = '.';
                // A multibyte radix (rare, but possible in some locales) leaves trailing
                // bytes behind; collapse them so the output is a single '.'.
                if (radix_len > 1) {
                    std::memmove(p + 1, p + radix_len, std::strlen(p + radix_len) + 1);
                }
            }
        }
        return buffer;
    }

    // Given a 128-bit number expressed as a pair of uint64_t, high half first,
    // return that number multiplied by the given 32-bit value.  If the result is
    // too large to fit in a 128-bit number, divide it by 2 until it fits.
    static std::pair<uint64_t, uint64_t> Mul32(std::pair<uint64_t, uint64_t> num,
        uint32_t mul) {
        uint64_t bits0_31 = num.second & 0xFFFFFFFF;
        uint64_t bits32_63 = num.second >> 32;
        uint64_t bits64_95 = num.first & 0xFFFFFFFF;
        uint64_t bits96_127 = num.first >> 32;

        // The picture so far: each of these 64-bit values has only the lower 32 bits
        // filled in.
        // bits96_127:          [ 00000000 xxxxxxxx ]
        // bits64_95:                    [ 00000000 xxxxxxxx ]
        // bits32_63:                             [ 00000000 xxxxxxxx ]
        // bits0_31:                                       [ 00000000 xxxxxxxx ]

        bits0_31 *= mul;
        bits32_63 *= mul;
        bits64_95 *= mul;
        bits96_127 *= mul;

        // Now the top halves may also have value, though all 64 of their bits will
        // never be set at the same time, since they are a result of a 32x32 bit
        // multiply.  This makes the carry calculation slightly easier.
        // bits96_127:          [ mmmmmmmm | mmmmmmmm ]
        // bits64_95:                    [ | mmmmmmmm mmmmmmmm | ]
        // bits32_63:                      |        [ mmmmmmmm | mmmmmmmm ]
        // bits0_31:                       |                 [ | mmmmmmmm mmmmmmmm ]
        // eventually:        [ bits128_up | ...bits64_127.... | ..bits0_63... ]

        uint64_t bits0_63 = bits0_31 + (bits32_63 << 32);
        uint64_t bits64_127 = bits64_95 + (bits96_127 << 32) + (bits32_63 >> 32) + (bits0_63 < bits0_31);
        uint64_t bits128_up = (bits96_127 >> 32) + (bits64_127 < bits64_95);
        if (bits128_up == 0)
            return { bits64_127, bits0_63 };

        auto shift = static_cast<unsigned>(bit_width(bits128_up));
        uint64_t lo = (bits0_63 >> shift) + (bits64_127 << (64 - shift));
        uint64_t hi = (bits64_127 >> shift) + (bits128_up << (64 - shift));
        return { hi, lo };
    }

    // Compute num * 5 ^ expfive, and return the first 128 bits of the result,
    // where the first bit is always a one.  So PowFive(1, 0) starts 0b100000,
    // PowFive(1, 1) starts 0b101000, PowFive(1, 2) starts 0b110010, etc.
    static std::pair<uint64_t, uint64_t> PowFive(uint64_t num, int expfive) {
        std::pair<uint64_t, uint64_t> result = { num, 0 };
        while (expfive >= 13) {
            // 5^13 is the highest power of five that will fit in a 32-bit integer.
            result = Mul32(result, 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5);
            expfive -= 13;
        }
        constexpr uint32_t powers_of_five[13] = {
            1,
            5,
            5 * 5,
            5 * 5 * 5,
            5 * 5 * 5 * 5,
            5 * 5 * 5 * 5 * 5,
            5 * 5 * 5 * 5 * 5 * 5,
            5 * 5 * 5 * 5 * 5 * 5 * 5,
            5 * 5 * 5 * 5 * 5 * 5 * 5 * 5,
            5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5,
            5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5,
            5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5,
            5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5 * 5
        };
        result = Mul32(result, powers_of_five[expfive & 15]);
        int shift = countl_zero(result.first);
        if (shift != 0) {
            result.first = (result.first << shift) + (result.second >> (64 - shift));
            result.second = (result.second << shift);
        }
        return result;
    }

    struct ExpDigits {
        int32_t exponent;
        char digits[6];
    };

    // SplitToSix converts value, a positive double-precision floating-point number,
    // into a base-10 exponent and 6 ASCII digits, where the first digit is never
    // zero.  For example, SplitToSix(1) returns an exponent of zero and a digits
    // array of {'1', '0', '0', '0', '0', '0'}.  If value is exactly halfway between
    // two possible representations, e.g. value = 100000.5, then "round to even" is
    // performed.
    static ExpDigits SplitToSix(const double value) {
        ExpDigits exp_dig;
        int exp = 5;
        double d = value;
        // First step: calculate a close approximation of the output, where the
        // value d will be between 100,000 and 999,999, representing the digits
        // in the output ASCII array, and exp is the base-10 exponent.  It would be
        // faster to use a table here, and to look up the base-2 exponent of value,
        // however value is an IEEE-754 64-bit number, so the table would have 2,000
        // entries, which is not cache-friendly.
        if (d >= 999999.5) {
            if (d >= 1e+261)
                exp += 256, d *= 1e-256;
            if (d >= 1e+133)
                exp += 128, d *= 1e-128;
            if (d >= 1e+69)
                exp += 64, d *= 1e-64;
            if (d >= 1e+37)
                exp += 32, d *= 1e-32;
            if (d >= 1e+21)
                exp += 16, d *= 1e-16;
            if (d >= 1e+13)
                exp += 8, d *= 1e-8;
            if (d >= 1e+9)
                exp += 4, d *= 1e-4;
            if (d >= 1e+7)
                exp += 2, d *= 1e-2;
            if (d >= 1e+6)
                exp += 1, d *= 1e-1;
        } else {
            if (d < 1e-250)
                exp -= 256, d *= 1e256;
            if (d < 1e-122)
                exp -= 128, d *= 1e128;
            if (d < 1e-58)
                exp -= 64, d *= 1e64;
            if (d < 1e-26)
                exp -= 32, d *= 1e32;
            if (d < 1e-10)
                exp -= 16, d *= 1e16;
            if (d < 1e-2)
                exp -= 8, d *= 1e8;
            if (d < 1e+2)
                exp -= 4, d *= 1e4;
            if (d < 1e+4)
                exp -= 2, d *= 1e2;
            if (d < 1e+5)
                exp -= 1, d *= 1e1;
        }
        // At this point, d is in the range [99999.5..999999.5) and exp is in the
        // range [-324..308]. Since we need to round d up, we want to add a half
        // and truncate.
        // However, the technique above may have lost some precision, due to its
        // repeated multiplication by constants that each may be off by half a bit
        // of precision.  This only matters if we're close to the edge though.
        // Since we'd like to know if the fractional part of d is close to a half,
        // we multiply it by 65536 and see if the fractional part is close to 32768.
        // (The number doesn't have to be a power of two,but powers of two are faster)
        uint64_t d64k = static_cast<uint64_t>(d * 65536);
        uint32_t dddddd; // A 6-digit decimal integer.
        if ((d64k % 65536) == 32767 || (d64k % 65536) == 32768) {
            // OK, it's fairly likely that precision was lost above, which is
            // not a surprise given only 52 mantissa bits are available.  Therefore
            // redo the calculation using 128-bit numbers.  (64 bits are not enough).

            // Start out with digits rounded down; maybe add one below.
            dddddd = static_cast<uint32_t>(d64k / 65536);

            // mantissa is a 64-bit integer representing M.mmm... * 2^63.  The actual
            // value we're representing, of course, is M.mmm... * 2^exp2.
            int exp2;
            double m = std::frexp(value, &exp2);
            uint64_t mantissa = static_cast<uint64_t>(m * (32768.0 * 65536.0 * 65536.0 * 65536.0));
            // std::frexp returns an m value in the range [0.5, 1.0), however we
            // can't multiply it by 2^64 and convert to an integer because some FPUs
            // throw an exception when converting an number higher than 2^63 into an
            // integer - even an unsigned 64-bit integer!  Fortunately it doesn't matter
            // since m only has 52 significant bits anyway.
            mantissa <<= 1;
            exp2 -= 64; // not needed, but nice for debugging

            // OK, we are here to compare:
            //     (dddddd + 0.5) * 10^(exp-5)  vs.  mantissa * 2^exp2
            // so we can round up dddddd if appropriate.  Those values span the full
            // range of 600 orders of magnitude of IEE 64-bit floating-point.
            // Fortunately, we already know they are very close, so we don't need to
            // track the base-2 exponent of both sides.  This greatly simplifies the
            // the math since the 2^exp2 calculation is unnecessary and the power-of-10
            // calculation can become a power-of-5 instead.

            std::pair<uint64_t, uint64_t> edge, val;
            if (exp >= 6) {
                // Compare (dddddd + 0.5) * 5 ^ (exp - 5) to mantissa
                // Since we're tossing powers of two, 2 * dddddd + 1 is the
                // same as dddddd + 0.5
                edge = PowFive(2 * dddddd + 1, exp - 5);

                val.first = mantissa;
                val.second = 0;
            } else {
                // We can't compare (dddddd + 0.5) * 5 ^ (exp - 5) to mantissa as we did
                // above because (exp - 5) is negative.  So we compare (dddddd + 0.5) to
                // mantissa * 5 ^ (5 - exp)
                edge = PowFive(2 * dddddd + 1, 0);

                val = PowFive(mantissa, 5 - exp);
            }
            // printf("exp=%d %016lx %016lx vs %016lx %016lx\n", exp, val.first,
            //        val.second, edge.first, edge.second);
            if (val > edge) {
                dddddd++;
            } else if (val == edge) {
                dddddd += (dddddd & 1);
            }
        } else {
            // Here, we are not close to the edge.
            dddddd = static_cast<uint32_t>((d64k + 32768) / 65536);
        }
        if (dddddd == 1000000) {
            dddddd = 100000;
            exp += 1;
        }
        exp_dig.exponent = exp;

        uint32_t two_digits = dddddd / 10000;
        dddddd -= two_digits * 10000;
        put_two_digits(two_digits, &exp_dig.digits[0]);

        two_digits = dddddd / 100;
        dddddd -= two_digits * 100;
        put_two_digits(two_digits, &exp_dig.digits[2]);

        put_two_digits(dddddd, &exp_dig.digits[4]);
        return exp_dig;
    }

    // Helper function for fast formatting of floating-point.
    // The result is the same as "%g", a.k.a. "%.6g".
    size_t six_digits_to_buffer(double d,
        char* turbo_nonnull const buffer) {
        static_assert(std::numeric_limits<float>::is_iec559,
            "IEEE-754/IEC-559 support only");

        char* out = buffer; // we write data to out, incrementing as we go, but
        // FloatToBuffer always returns the address of the buffer
        // passed in.

        if (std::isnan(d)) {
            strcpy(out, "nan"); // NOLINT(runtime/printf)
            return 3;
        }
        if (d == 0) {
            // +0 and -0 are handled here
            if (std::signbit(d))
                *out++ = '-';
            *out++ = '0';
            *out = 0;
            return static_cast<size_t>(out - buffer);
        }
        if (d < 0) {
            *out++ = '-';
            d = -d;
        }
        if (d > std::numeric_limits<double>::max()) {
            strcpy(out, "inf"); // NOLINT(runtime/printf)
            return static_cast<size_t>(out + 3 - buffer);
        }

        auto exp_dig = SplitToSix(d);
        int exp = exp_dig.exponent;
        const char* digits = exp_dig.digits;
        out[0] = '0';
        out[1] = '.';
        switch (exp) {
        case 5:
            memcpy(out, &digits[0], 6), out += 6;
            *out = 0;
            return static_cast<size_t>(out - buffer);
        case 4:
            memcpy(out, &digits[0], 5), out += 5;
            if (digits[5] != '0') {
                *out++ = '.';
                *out++ = digits[5];
            }
            *out = 0;
            return static_cast<size_t>(out - buffer);
        case 3:
            memcpy(out, &digits[0], 4), out += 4;
            if ((digits[5] | digits[4]) != '0') {
                *out++ = '.';
                *out++ = digits[4];
                if (digits[5] != '0')
                    *out++ = digits[5];
            }
            *out = 0;
            return static_cast<size_t>(out - buffer);
        case 2:
            memcpy(out, &digits[0], 3), out += 3;
            *out++ = '.';
            memcpy(out, &digits[3], 3);
            out += 3;
            while (out[-1] == '0')
                --out;
            if (out[-1] == '.')
                --out;
            *out = 0;
            return static_cast<size_t>(out - buffer);
        case 1:
            memcpy(out, &digits[0], 2), out += 2;
            *out++ = '.';
            memcpy(out, &digits[2], 4);
            out += 4;
            while (out[-1] == '0')
                --out;
            if (out[-1] == '.')
                --out;
            *out = 0;
            return static_cast<size_t>(out - buffer);
        case 0:
            memcpy(out, &digits[0], 1), out += 1;
            *out++ = '.';
            memcpy(out, &digits[1], 5);
            out += 5;
            while (out[-1] == '0')
                --out;
            if (out[-1] == '.')
                --out;
            *out = 0;
            return static_cast<size_t>(out - buffer);
        case -4:
            out[2] = '0';
            ++out;
            KUMO_FALLTHROUGH_INTENDED;
        case -3:
            out[2] = '0';
            ++out;
            KUMO_FALLTHROUGH_INTENDED;
        case -2:
            out[2] = '0';
            ++out;
            KUMO_FALLTHROUGH_INTENDED;
        case -1:
            out += 2;
            memcpy(out, &digits[0], 6);
            out += 6;
            while (out[-1] == '0')
                --out;
            *out = 0;
            return static_cast<size_t>(out - buffer);
        }
        assert(exp < -4 || exp >= 6);
        out[0] = digits[0];
        assert(out[1] == '.');
        out += 2;
        memcpy(out, &digits[1], 5), out += 5;
        while (out[-1] == '0')
            --out;
        if (out[-1] == '.')
            --out;
        *out++ = 'e';
        if (exp > 0) {
            *out++ = '+';
        } else {
            *out++ = '-';
            exp = -exp;
        }
        if (exp > 99) {
            int dig1 = exp / 100;
            exp -= dig1 * 100;
            *out++ = '0' + static_cast<char>(dig1);
        }
        put_two_digits(static_cast<uint32_t>(exp), out);
        out += 2;
        *out = 0;
        return static_cast<size_t>(out - buffer);
    }
} // namespace turbo::format_internal
