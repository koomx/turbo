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

#include <turbo/unicode/api/wchar.h>
#include <turbo/unicode/api/utf16.h>
#include <turbo/unicode/api/utf32.h>
#include <turbo/unicode/api/utf8.h>

#include <string>
#include <string_view>

namespace turbo {


        size_t convert_wchar_to_utf8(wchar_t wc, char* buf, ShiftState& s) {
            // Reinterpret the output buffer `buf` as `unsigned char*` for subsequent
            // bitwise operations. This ensures well-defined behavior for bit
            // manipulations (avoiding issues with signed `char`) and is safe under C++
            // aliasing rules, as `unsigned char` can alias any type.
            auto* ubuf = reinterpret_cast<unsigned char*>(buf);
            const uint32_t v = static_cast<uint32_t>(wc);
            constexpr size_t kError = static_cast<size_t>(-1);

            if (v <= 0x007F) {
                // 1-byte sequence (U+0000 to U+007F).
                // 0xxxxxxx.
                ubuf[0] = (0b0111'1111 & v);
                s = { }; // Reset surrogate state.
                return 1;
            } else if (0x0080 <= v && v <= 0x07FF) {
                // 2-byte sequence (U+0080 to U+07FF).
                // 110xxxxx 10xxxxxx.
                ubuf[0] = 0b1100'0000 | (0b0001'1111 & (v >> 6));
                ubuf[1] = 0b1000'0000 | (0b0011'1111 & v);
                s = { }; // Reset surrogate state.
                return 2;
            } else if ((0x0800 <= v && v <= 0xD7FF) || (0xE000 <= v && v <= 0xFFFF)) {
                // 3-byte sequence (U+0800 to U+D7FF or U+E000 to U+FFFF).
                // Excludes surrogate code points U+D800-U+DFFF.
                // 1110xxxx 10xxxxxx 10xxxxxx.
                ubuf[0] = 0b1110'0000 | (0b0000'1111 & (v >> 12));
                ubuf[1] = 0b1000'0000 | (0b0011'1111 & (v >> 6));
                ubuf[2] = 0b1000'0000 | (0b0011'1111 & v);
                s = { }; // Reset surrogate state.
                return 3;
            } else if (0xD800 <= v && v <= 0xDBFF) {
                // High Surrogate (U+D800 to U+DBFF).
                // This part forms the first two bytes of an eventual 4-byte UTF-8 sequence.
                const unsigned char high_bits_val = (0b0000'1111 & (v >> 6)) + 1;

                // First byte of the 4-byte UTF-8 sequence (11110xxx).
                ubuf[0] = 0b1111'0000 | (0b0000'0111 & (high_bits_val >> 2));
                // Second byte of the 4-byte UTF-8 sequence (10xxxxxx).
                ubuf[1] = 0b1000'0000 | //
                    (0b0011'0000 & (high_bits_val << 4)) | //
                    (0b0000'1111 & (v >> 2));
                // Set state for high surrogate after writing to buffer.
                s = { true, static_cast<unsigned char>(0b0000'0011 & v) };
                return 2; // Wrote 2 bytes, expecting 2 more from a low surrogate.
            } else if (0xDC00 <= v && v <= 0xDFFF) {
                // Low Surrogate (U+DC00 to U+DFFF).
                // This part forms the last two bytes of a 4-byte UTF-8 sequence,
                // using state from a preceding high surrogate.
                if (!s.saw_high_surrogate) {
                    // Error: Isolated low surrogate without a preceding high surrogate.
                    // s remains in its current (problematic) state.
                    // Caller should handle error.
                    return kError;
                }

                // Third byte of the 4-byte UTF-8 sequence (10xxxxxx).
                ubuf[0] = 0b1000'0000 | //
                    (0b0011'0000 & (s.bits << 4)) | //
                    (0b0000'1111 & (v >> 6));
                // Fourth byte of the 4-byte UTF-8 sequence (10xxxxxx).
                ubuf[1] = 0b1000'0000 | (0b0011'1111 & v);

                s = { }; // Reset surrogate state, pair complete.
                return 2; // Wrote 2 more bytes, completing the 4-byte sequence.
            } else if constexpr (0xFFFF < std::numeric_limits<wchar_t>::max()) {
                // Conditionally compile the 4-byte direct conversion branch.
                // This block is compiled only if wchar_t can represent values > 0xFFFF.
                // It's placed after surrogate checks to ensure surrogates are handled by
                // their specific logic. This inner 'if' is the runtime check for the 4-byte
                // range. At this point, v is known not to be in the 1, 2, or 3-byte BMP
                // ranges, nor is it a surrogate code point.
                if (0x10000 <= v && v <= 0x10FFFF) {
                    // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx.
                    ubuf[0] = 0b1111'0000 | (0b0000'0111 & (v >> 18));
                    ubuf[1] = 0b1000'0000 | (0b0011'1111 & (v >> 12));
                    ubuf[2] = 0b1000'0000 | (0b0011'1111 & (v >> 6));
                    ubuf[3] = 0b1000'0000 | (0b0011'1111 & v);
                    s = { }; // Reset surrogate state.
                    return 4;
                }
            }

            // Invalid wchar_t value (e.g., out of Unicode range, or unhandled after all
            // checks).
            s = { }; // Reset surrogate state.
            return kError;
        }

    std::string convert_wchar_to_utf8(std::wstring_view wc) {
        if (wc.empty()) {
            return {};
        }
        if constexpr (sizeof(wchar_t) == 2) {
            const auto* in = reinterpret_cast<const char16_t*>(wc.data());
            std::string out(utf8_length_from_utf16(in, wc.size()), '\0');
            out.resize(convert_utf16_to_utf8(in, wc.size(), out.data()));
            return out;
        } else {
            const auto* in = reinterpret_cast<const char32_t*>(wc.data());
            std::string out(utf8_length_from_utf32(in, wc.size()), '\0');
            out.resize(convert_utf32_to_utf8(in, wc.size(), out.data()));
            return out;
        }
    }

    std::wstring convert_utf8_to_wchar(std::string_view utf8) {
        if (utf8.empty()) {
            return {};
        }
        if constexpr (sizeof(wchar_t) == 2) {
            std::wstring out(utf16_length_from_utf8(utf8.data(), utf8.size()), L'\0');
            out.resize(convert_utf8_to_utf16(utf8.data(), utf8.size(),
                reinterpret_cast<char16_t*>(out.data())));
            return out;
        } else {
            std::wstring out(utf32_length_from_utf8(utf8.data(), utf8.size()), L'\0');
            out.resize(convert_utf8_to_utf32(utf8.data(), utf8.size(),
                reinterpret_cast<char32_t*>(out.data())));
            return out;
        }
    }
}  // namespace turbo

