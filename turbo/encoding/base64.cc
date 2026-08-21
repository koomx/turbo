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

#include <turbo/encoding/base64.h>
#include <turbo/base/nullability.h>
#include <turbo/bits/endian.h>
#include <turbo/base/internal/raw_logging.h>
#include <turbo/strings/ascii.h>
#include <limits>
#include <algorithm>

namespace turbo {

     // Reverses the mapping in Base64EscapeInternal; see that method's
        // documentation for details of the mapping.
        bool Base64UnescapeInternal(const char* turbo_nullable src_param, size_t szsrc,
            char* turbo_nullable dest, size_t szdest,
            const std::array<signed char, 256>& unbase64,
            size_t* turbo_nonnull len) {
            static const char kPad64Equals = '=';
            static const char kPad64Dot = '.';

            size_t destidx = 0;
            int decode = 0;
            int state = 0;
            unsigned char ch = 0;
            unsigned int temp = 0;

            // If "char" is signed by default, using *src as an array index results in
            // accessing negative array elements. Treat the input as a pointer to
            // unsigned char to avoid this.
            const unsigned char* src = reinterpret_cast<const unsigned char*>(src_param);

            // The GET_INPUT macro gets the next input character, skipping
            // over any whitespace, and stopping when we reach the end of the
            // string or when we read any non-data character.  The arguments are
            // an arbitrary identifier (used as a label for goto) and the number
            // of data bytes that must remain in the input to avoid aborting the
            // loop.
#define GET_INPUT(label, remain)                         \
    label:                                               \
    --szsrc;                                             \
    ch = *src++;                                         \
    decode = unbase64[ch];                               \
    if (decode < 0) {                                    \
        if (turbo::ascii_isspace(ch) && szsrc >= remain) \
            goto label;                                  \
        state = 4 - remain;                              \
        break;                                           \
    }

            // if dest is null, we're just checking to see if it's legal input
            // rather than producing output.  (I suspect this could just be done
            // with a regexp...).  We duplicate the loop so this test can be
            // outside it instead of in every iteration.

            if (dest) {
                // This loop consumes 4 input bytes and produces 3 output bytes
                // per iteration.  We can't know at the start that there is enough
                // data left in the string for a full iteration, so the loop may
                // break out in the middle; if so 'state' will be set to the
                // number of input bytes read.

                while (szsrc >= 4) {
                    // We'll start by optimistically assuming that the next four
                    // bytes of the string (src[0..3]) are four good data bytes
                    // (that is, no nulls, whitespace, padding chars, or illegal
                    // chars).  We need to test src[0..2] for nulls individually
                    // before constructing temp to preserve the property that we
                    // never read past a null in the string (no matter how long
                    // szsrc claims the string is).

                    if (!src[0] || !src[1] || !src[2] || ((temp = ((unsigned(unbase64[src[0]]) << 18) | (unsigned(unbase64[src[1]]) << 12) | (unsigned(unbase64[src[2]]) << 6) | (unsigned(unbase64[src[3]])))) & 0x80000000)) {
                        // Iff any of those four characters was bad (null, illegal,
                        // whitespace, padding), then temp's high bit will be set
                        // (because unbase64[] is -1 for all bad characters).
                        //
                        // We'll back up and resort to the slower decoder, which knows
                        // how to handle those cases.

                        GET_INPUT(first, 4);
                        temp = static_cast<unsigned char>(decode);
                        GET_INPUT(second, 3);
                        temp = (temp << 6) | static_cast<unsigned char>(decode);
                        GET_INPUT(third, 2);
                        temp = (temp << 6) | static_cast<unsigned char>(decode);
                        GET_INPUT(fourth, 1);
                        temp = (temp << 6) | static_cast<unsigned char>(decode);
                    } else {
                        // We really did have four good data bytes, so advance four
                        // characters in the string.

                        szsrc -= 4;
                        src += 4;
                    }

                    // temp has 24 bits of input, so write that out as three bytes.

                    if (destidx + 3 > szdest)
                        return false;
                    dest[destidx + 2] = static_cast<char>(temp);
                    temp >>= 8;
                    dest[destidx + 1] = static_cast<char>(temp);
                    temp >>= 8;
                    dest[destidx] = static_cast<char>(temp);
                    destidx += 3;
                }
            } else {
                while (szsrc >= 4) {
                    if (!src[0] || !src[1] || !src[2] || ((temp = ((unsigned(unbase64[src[0]]) << 18) | (unsigned(unbase64[src[1]]) << 12) | (unsigned(unbase64[src[2]]) << 6) | (unsigned(unbase64[src[3]])))) & 0x80000000)) {
                        GET_INPUT(first_no_dest, 4);
                        GET_INPUT(second_no_dest, 3);
                        GET_INPUT(third_no_dest, 2);
                        GET_INPUT(fourth_no_dest, 1);
                    } else {
                        szsrc -= 4;
                        src += 4;
                    }
                    destidx += 3;
                }
            }

#undef GET_INPUT

            // if the loop terminated because we read a bad character, return
            // now.
            if (decode < 0 && ch != kPad64Equals && ch != kPad64Dot && !turbo::ascii_isspace(ch))
                return false;

            if (ch == kPad64Equals || ch == kPad64Dot) {
                // if we stopped by hitting an '=' or '.', un-read that character -- we'll
                // look at it again when we count to check for the proper number of
                // equals signs at the end.
                ++szsrc;
                --src;
            } else {
                // This loop consumes 1 input byte per iteration.  It's used to
                // clean up the 0-3 input bytes remaining when the first, faster
                // loop finishes.  'temp' contains the data from 'state' input
                // characters read by the first loop.
                while (szsrc > 0) {
                    --szsrc;
                    ch = *src++;
                    decode = unbase64[ch];
                    if (decode < 0) {
                        if (turbo::ascii_isspace(ch)) {
                            continue;
                        } else if (ch == kPad64Equals || ch == kPad64Dot) {
                            // back up one character; we'll read it again when we check
                            // for the correct number of pad characters at the end.
                            ++szsrc;
                            --src;
                            break;
                        } else {
                            return false;
                        }
                    }

                    // Each input character gives us six bits of output.
                    temp = (temp << 6) | static_cast<unsigned char>(decode);
                    ++state;
                    if (state == 4) {
                        // If we've accumulated 24 bits of output, write that out as
                        // three bytes.
                        if (dest) {
                            if (destidx + 3 > szdest)
                                return false;
                            dest[destidx + 2] = static_cast<char>(temp);
                            temp >>= 8;
                            dest[destidx + 1] = static_cast<char>(temp);
                            temp >>= 8;
                            dest[destidx] = static_cast<char>(temp);
                        }
                        destidx += 3;
                        state = 0;
                        temp = 0;
                    }
                }
            }

            // Process the leftover data contained in 'temp' at the end of the input.
            int expected_equals = 0;
            switch (state) {
            case 0:
                // Nothing left over; output is a multiple of 3 bytes.
                break;

            case 1:
                // Bad input; we have 6 bits left over.
                return false;

            case 2:
                // Produce one more output byte from the 12 input bits we have left.
                if (dest) {
                    if (destidx + 1 > szdest)
                        return false;
                    temp >>= 4;
                    dest[destidx] = static_cast<char>(temp);
                }
                ++destidx;
                expected_equals = 2;
                break;

            case 3:
                // Produce two more output bytes from the 18 input bits we have left.
                if (dest) {
                    if (destidx + 2 > szdest)
                        return false;
                    temp >>= 2;
                    dest[destidx + 1] = static_cast<char>(temp);
                    temp >>= 8;
                    dest[destidx] = static_cast<char>(temp);
                }
                destidx += 2;
                expected_equals = 1;
                break;

            default:
                // state should have no other values at this point.
                TURBO_RAW_LOG(FATAL, "This can't happen; base64 decoder state = %d",
                    state);
            }

            // The remainder of the string should be all whitespace, mixed with
            // exactly 0 equals signs, or exactly 'expected_equals' equals
            // signs.  (Always accepting 0 equals signs is an Abseil extension
            // not covered in the RFC, as is accepting dot as the pad character.)

            int equals = 0;
            while (szsrc > 0) {
                if (*src == kPad64Equals || *src == kPad64Dot)
                    ++equals;
                else if (!turbo::ascii_isspace(*src))
                    return false;
                --szsrc;
                ++src;
            }

            const bool ok = (equals == 0 || equals == expected_equals);
            if (ok)
                *len = destidx;
            return ok;
        }

    template <typename String>
bool Base64UnescapeInternal(const char* turbo_nullable src, size_t slen,
    String* turbo_nonnull dest,
    const std::array<signed char, 256>& unbase64) {
        // Determine the size of the output string.  Base64 encodes every 3 bytes into
        // 4 characters.  Any leftover chars are added directly for good measure.
        const size_t dest_len = 3 * (slen / 4) + (slen % 4);

        bool ok;
        StringResizeAndOverwrite(
            *dest, dest_len, [src, slen, unbase64, &ok](char* buf, size_t buf_size) {
                size_t len;
                ok = Base64UnescapeInternal(src, slen, buf, buf_size, unbase64, &len);
                if (!ok) {
                    len = 0;
                }
                assert(len <= buf_size); // Could be shorter if there was padding.
                return len;
            });
        return ok;
    }

    // clang-format off
    constexpr std::array<signed char, 256> kUnBase64 = {
    -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
    -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
    -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
    -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
    -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
    -1,      -1,      -1,      62/*+*/, -1,      -1,      -1,      63/*/ */,
    52/*0*/, 53/*1*/, 54/*2*/, 55/*3*/, 56/*4*/, 57/*5*/, 58/*6*/, 59/*7*/,
    60/*8*/, 61/*9*/, -1,      -1,      -1,      -1,      -1,      -1,
    -1,       0/*A*/,  1/*B*/,  2/*C*/,  3/*D*/,  4/*E*/,  5/*F*/,  6/*G*/,
    07/*H*/,  8/*I*/,  9/*J*/, 10/*K*/, 11/*L*/, 12/*M*/, 13/*N*/, 14/*O*/,
    15/*P*/, 16/*Q*/, 17/*R*/, 18/*S*/, 19/*T*/, 20/*U*/, 21/*V*/, 22/*W*/,
    23/*X*/, 24/*Y*/, 25/*Z*/, -1,      -1,      -1,      -1,      -1,
    -1,      26/*a*/, 27/*b*/, 28/*c*/, 29/*d*/, 30/*e*/, 31/*f*/, 32/*g*/,
    33/*h*/, 34/*i*/, 35/*j*/, 36/*k*/, 37/*l*/, 38/*m*/, 39/*n*/, 40/*o*/,
    41/*p*/, 42/*q*/, 43/*r*/, 44/*s*/, 45/*t*/, 46/*u*/, 47/*v*/, 48/*w*/,
    49/*x*/, 50/*y*/, 51/*z*/, -1,      -1,      -1,      -1,      -1,
    -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
    -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
    -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
    -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
    -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
    -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
    -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
    -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
    -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
    -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
    -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
    -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
    -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
    -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
    -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
    -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1
};

    constexpr std::array<signed char, 256> kUnWebSafeBase64 = {
    -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
    -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
    -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
    -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
    -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
    -1,      -1,      -1,      -1,      -1,      62/*-*/, -1,      -1,
    52/*0*/, 53/*1*/, 54/*2*/, 55/*3*/, 56/*4*/, 57/*5*/, 58/*6*/, 59/*7*/,
    60/*8*/, 61/*9*/, -1,      -1,      -1,      -1,      -1,      -1,
    -1,       0/*A*/,  1/*B*/,  2/*C*/,  3/*D*/,  4/*E*/,  5/*F*/,  6/*G*/,
    07/*H*/,  8/*I*/,  9/*J*/, 10/*K*/, 11/*L*/, 12/*M*/, 13/*N*/, 14/*O*/,
    15/*P*/, 16/*Q*/, 17/*R*/, 18/*S*/, 19/*T*/, 20/*U*/, 21/*V*/, 22/*W*/,
    23/*X*/, 24/*Y*/, 25/*Z*/, -1,      -1,      -1,      -1,      63/*_*/,
    -1,      26/*a*/, 27/*b*/, 28/*c*/, 29/*d*/, 30/*e*/, 31/*f*/, 32/*g*/,
    33/*h*/, 34/*i*/, 35/*j*/, 36/*k*/, 37/*l*/, 38/*m*/, 39/*n*/, 40/*o*/,
    41/*p*/, 42/*q*/, 43/*r*/, 44/*s*/, 45/*t*/, 46/*u*/, 47/*v*/, 48/*w*/,
    49/*x*/, 50/*y*/, 51/*z*/, -1,      -1,      -1,      -1,      -1,
    -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
    -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
    -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
    -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
    -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
    -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
    -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
    -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
    -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
    -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
    -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
    -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
    -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
    -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
    -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,
    -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1
};
    // clang-format on



    // The two strings below provide maps from normal 6-bit characters to their
        // base64-escaped equivalent.
        // For the inverse case, see kUn(WebSafe)Base64 in the external
        // escaping.cc.
        constexpr char kBase64Chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        constexpr char kWebSafeBase64Chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

        // ----------------------------------------------------------------------
        //   Take the input in groups of 4 characters and turn each
        //   character into a code 0 to 63 thus:
        //           A-Z map to 0 to 25
        //           a-z map to 26 to 51
        //           0-9 map to 52 to 61
        //           +(- for WebSafe) maps to 62
        //           /(_ for WebSafe) maps to 63
        //   There will be four numbers, all less than 64 which can be represented
        //   by a 6 digit binary number (aaaaaa, bbbbbb, cccccc, dddddd respectively).
        //   Arrange the 6 digit binary numbers into three bytes as such:
        //   aaaaaabb bbbbcccc ccdddddd
        //   Equals signs (one or two) are used at the end of the encoded block to
        //   indicate that the text was not an integer multiple of three bytes long.
        // ----------------------------------------------------------------------
        size_t Base64EscapeInternal(const unsigned char* src, size_t szsrc, char* dest,
            size_t szdest, const char* base64,
            bool do_padding) {
            constexpr char kPad64 = '=';

            constexpr size_t kMaxSize = (std::numeric_limits<size_t>::max() - 1) / 4 * 3;
            if (KUMO_UNLIKELY(szsrc > kMaxSize || szsrc * 4 > szdest * 3))
                return 0;

            char* cur_dest = dest;
            const unsigned char* cur_src = src;

            char* const limit_dest = dest + szdest;
            const unsigned char* const limit_src = src + szsrc;

            // (from https://tools.ietf.org/html/rfc3548)
            // Special processing is performed if fewer than 24 bits are available
            // at the end of the data being encoded.  A full encoding quantum is
            // always completed at the end of a quantity.  When fewer than 24 input
            // bits are available in an input group, zero bits are added (on the
            // right) to form an integral number of 6-bit groups.
            //
            // If do_padding is true, padding at the end of the data is performed. This
            // output padding uses the '=' character.

            // Three bytes of data encodes to four characters of cyphertext.
            // So we can pump through three-byte chunks atomically.
            if (szsrc >= 3) { // "limit_src - 3" is UB if szsrc < 3.
                while (cur_src < limit_src - 3) { // While we have >= 32 bits.
                    uint32_t in = turbo::big_endian::Load32(cur_src) >> 8;

                    cur_dest[0] = base64[in >> 18];
                    in &= 0x3FFFF;
                    cur_dest[1] = base64[in >> 12];
                    in &= 0xFFF;
                    cur_dest[2] = base64[in >> 6];
                    in &= 0x3F;
                    cur_dest[3] = base64[in];

                    cur_dest += 4;
                    cur_src += 3;
                }
            }
            // To save time, we didn't update szdest or szsrc in the loop.  So do it now.
            szdest = static_cast<size_t>(limit_dest - cur_dest);
            szsrc = static_cast<size_t>(limit_src - cur_src);

            /* now deal with the tail (<=3 bytes) */
            switch (szsrc) {
            case 0:
                // Nothing left; nothing more to do.
                break;
            case 1: {
                // One byte left: this encodes to two characters, and (optionally)
                // two pad characters to round out the four-character cypherblock.
                if (szdest < 2)
                    return 0;
                uint32_t in = cur_src[0];
                cur_dest[0] = base64[in >> 2];
                in &= 0x3;
                cur_dest[1] = base64[in << 4];
                cur_dest += 2;
                szdest -= 2;
                if (do_padding) {
                    if (szdest < 2)
                        return 0;
                    cur_dest[0] = kPad64;
                    cur_dest[1] = kPad64;
                    cur_dest += 2;
                    szdest -= 2;
                }
                break;
            }
            case 2: {
                // Two bytes left: this encodes to three characters, and (optionally)
                // one pad character to round out the four-character cypherblock.
                if (szdest < 3)
                    return 0;
                uint32_t in = turbo::big_endian::Load16(cur_src);
                cur_dest[0] = base64[in >> 10];
                in &= 0x3FF;
                cur_dest[1] = base64[in >> 4];
                in &= 0x00F;
                cur_dest[2] = base64[in << 2];
                cur_dest += 3;
                szdest -= 3;
                if (do_padding) {
                    if (szdest < 1)
                        return 0;
                    cur_dest[0] = kPad64;
                    cur_dest += 1;
                    szdest -= 1;
                }
                break;
            }
            case 3: {
                // Three bytes left: same as in the big loop above.  We can't do this in
                // the loop because the loop above always reads 4 bytes, and the fourth
                // byte is past the end of the input.
                if (szdest < 4)
                    return 0;
                uint32_t in = (uint32_t { cur_src[0] } << 16) + turbo::big_endian::Load16(cur_src + 1);
                cur_dest[0] = base64[in >> 18];
                in &= 0x3FFFF;
                cur_dest[1] = base64[in >> 12];
                in &= 0xFFF;
                cur_dest[2] = base64[in >> 6];
                in &= 0x3F;
                cur_dest[3] = base64[in];
                cur_dest += 4;
                szdest -= 4;
                break;
            }
            default:
                // Should not be reached: blocks of 4 bytes are handled
                // in the while loop before this switch statement.
                TURBO_RAW_LOG(FATAL, "Logic problem? szsrc = %zu", szsrc);
                break;
            }
            return static_cast<size_t>(cur_dest - dest);
        }

    size_t CalculateBase64EscapedLenInternal(size_t input_len, bool do_padding) {
            // Base64 encodes three bytes of input at a time. If the input is not
            // divisible by three, we pad as appropriate.
            //
            // Base64 encodes each three bytes of input into four bytes of output.
            constexpr size_t kMaxSize = (std::numeric_limits<size_t>::max() - 1) / 4 * 3;
            TURBO_INTERNAL_CHECK(input_len <= kMaxSize,
                "CalculateBase64EscapedLenInternal() overflow");
            size_t len = (input_len / 3) * 4;

            // Since all base 64 input is an integral number of octets, only the following
            // cases can arise:
            if (input_len % 3 == 0) {
                // (from https://tools.ietf.org/html/rfc3548)
                // (1) the final quantum of encoding input is an integral multiple of 24
                // bits; here, the final unit of encoded output will be an integral
                // multiple of 4 characters with no "=" padding,
            } else if (input_len % 3 == 1) {
                // (from https://tools.ietf.org/html/rfc3548)
                // (2) the final quantum of encoding input is exactly 8 bits; here, the
                // final unit of encoded output will be two characters followed by two
                // "=" padding characters, or
                len += 2;
                if (do_padding) {
                    len += 2;
                }
            } else { // (input_len % 3 == 2)
                // (from https://tools.ietf.org/html/rfc3548)
                // (3) the final quantum of encoding input is exactly 16 bits; here, the
                // final unit of encoded output will be three characters followed by one
                // "=" padding character.
                len += 3;
                if (do_padding) {
                    len += 1;
                }
            }

            return len;
        }


    std::string Base64EscapeToStringInternal(const unsigned char* src, size_t szsrc,
        bool do_padding,
        const char* base64_chars) {
        std::string escaped;
        const size_t calc_escaped_size = CalculateBase64EscapedLenInternal(szsrc, do_padding);
        StringResizeAndOverwrite(
            escaped, calc_escaped_size,
            [src, szsrc, base64_chars, do_padding](char* buf, size_t buf_size) {
                const size_t escaped_len = Base64EscapeInternal(
                    src, szsrc, buf, buf_size, base64_chars, do_padding);
                assert(escaped_len == buf_size);
                return escaped_len;
            });
        return escaped;
    }

    bool base64_decode(std::string_view src, std::string* turbo_nonnull dest) {
        return Base64UnescapeInternal(src.data(), src.size(), dest, kUnBase64);
    }

    bool web_safe_base64_decode(std::string_view src,
    std::string* turbo_nonnull dest) {
        return Base64UnescapeInternal(src.data(), src.size(), dest, kUnWebSafeBase64);
    }


    std::string base64_encode(std::string_view src) {
        return Base64EscapeToStringInternal(
            reinterpret_cast<const unsigned char*>(src.data()), src.size(), true,
            kBase64Chars);
    }

    std::string web_safe_base64_encode(std::string_view src) {
        return Base64EscapeToStringInternal(
            reinterpret_cast<const unsigned char*>(src.data()), src.size(), false,
            kWebSafeBase64Chars);
    }

} // namespace turbo
