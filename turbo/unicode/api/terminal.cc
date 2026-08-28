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

#include <turbo/unicode/api/terminal.h>
#include <turbo/unicode/api/utf8.h>
#include <turbo/unicode/api/wchar.h>

#include <algorithm>

namespace turbo {
namespace {

// based on https://bjoern.hoehrmann.de/utf-8/decoder/dfa/
// Copyright (c) 2008-2009 Bjoern Hoehrmann <bjoern@hoehrmann.de>
static const uint8_t kUtf8DfaTable[] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 00..1f
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 20..3f
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 40..5f
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 60..7f
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, // 80..9f
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7, // a0..bf
    8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, // c0..df
    0xa,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x4,0x3,0x3, // e0..ef
    0xb,0x6,0x6,0x6,0x5,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8, // f0..ff
    0x0,0x1,0x2,0x3,0x5,0x8,0x7,0x1,0x1,0x1,0x4,0x6,0x1,0x1,0x1,0x1, // s0..s0
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1, // s1..s2
    1,2,1,1,1,1,1,2,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1, // s3..s4
    1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,3,1,3,1,1,1,1,1,1, // s5..s6
    1,3,1,1,1,1,1,3,1,3,1,1,1,1,1,1,1,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // s7..s8
};

struct Utf8Decoder {
    enum { ACCEPT = 0, REJECT = 1 };

    uint32_t decode(uint8_t byte) {
        uint32_t type = kUtf8DfaTable[byte];
        codepoint = (state != ACCEPT) ? (byte & 0x3fu) | (codepoint << 6)
                                      : (0xff >> type) & byte;
        state = kUtf8DfaTable[256 + state * 16 + type];
        return state;
    }

    void reset() {
        state = ACCEPT;
        codepoint = 0xfffdU;
    }

    uint8_t state { ACCEPT };
    uint32_t codepoint { 0 };
};

bool is_printable_ascii(uint8_t c) { return c >= 0x20 && c <= 0x7E; }
bool is_csi_parameter_byte(uint8_t c) { return c >= 0x30 && c <= 0x3F; }
bool is_csi_intermediate_byte(uint8_t c) { return c >= 0x20 && c <= 0x2F; }
bool is_csi_final_byte(uint8_t c) { return c >= 0x40 && c <= 0x7E; }

int mapped_wcwidth(wchar_t wc) {
    int width = widechar_wcwidth(wc);
    switch (width) {
    case widechar_nonprint:
    case widechar_combining:
    case widechar_unassigned:
        return 0;
    case widechar_ambiguous:
    case widechar_private_use:
    case widechar_widened_in_9:
        return 1;
    default:
        return width;
    }
}

enum ComputeWidthMode {
    Width,
    BytesBeforeLimit
};

template <ComputeWidthMode mode>
size_t compute_width_impl(const uint8_t* data, size_t size, size_t prefix,
    size_t limit) noexcept {
    Utf8Decoder decoder;
    bool is_escape_sequence = false;
    size_t width = 0;
    size_t rollback = 0;
    for (size_t i = 0; i < size; ++i) {
        while (i < size && is_printable_ascii(data[i])) {
            bool ignore_width = is_escape_sequence &&
                (is_csi_parameter_byte(data[i]) || is_csi_intermediate_byte(data[i]));

            if (ignore_width || (data[i] == '[' && is_escape_sequence)) {
            } else if (is_escape_sequence && is_csi_final_byte(data[i])) {
                is_escape_sequence = false;
            } else {
                ++width;
            }
            ++i;
        }

        if (mode == BytesBeforeLimit && width > limit)
            return i - (width - limit);

        if (i < size) {
            switch (decoder.decode(data[i])) {
            case Utf8Decoder::REJECT: {
                decoder.reset();
                i -= rollback;
                rollback = 0;
                break;
            }
            case Utf8Decoder::ACCEPT: {
                size_t next_width = width;
                if (decoder.codepoint == '\x1b')
                    is_escape_sequence = true;
                else if (decoder.codepoint == '\t')
                    next_width += 8 - (prefix + width) % 8;
                else
                    next_width += static_cast<size_t>(
                        mapped_wcwidth(static_cast<wchar_t>(decoder.codepoint)));

                if (mode == BytesBeforeLimit && next_width > limit)
                    return i - rollback;
                width = next_width;

                rollback = 0;
                break;
            }
            default:
                ++rollback;
            }
        }
    }

    return (mode == BytesBeforeLimit) ? size : width;
}

} // namespace

size_t utf8_display_width(const uint8_t* data, size_t size, size_t prefix) noexcept {
    return compute_width_impl<Width>(data, size, prefix, 0);
}

size_t utf8_display_bytes_for_width(const uint8_t* data, size_t size, size_t prefix,
    size_t limit) noexcept {
    return compute_width_impl<BytesBeforeLimit>(data, size, prefix, limit);
}

} // namespace turbo
