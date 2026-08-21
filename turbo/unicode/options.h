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

#include <cstddef>

namespace turbo {


    constexpr size_t default_line_length = 76; ///< default line length for base64 encoding with lines

    // Base64Options are used to specify the base64 encoding options.
    // ASCII spaces are ' ', '\t', '\n', '\r', '\f'
    // garbage characters are characters that are not part of the base64 alphabet
    // nor ASCII spaces.
    constexpr uint64_t base64_reverse_padding = 2; /* modifier for base64_default and base64_url */
    enum Base64Options : uint64_t {
        base64_default = 0, /* standard base64 format (with padding) */
        base64_url = 1, /* base64url format (no padding) */
        base64_default_no_padding = base64_default | base64_reverse_padding, /* standard base64 format without padding */
        base64_url_with_padding = base64_url | base64_reverse_padding, /* base64url with padding */
        base64_default_accept_garbage = 4, /* standard base64 format accepting garbage characters, the input stops
                                              with the first '=' if any */
        base64_url_accept_garbage = 5, /* base64url format accepting garbage characters, the input stops with
                                          the first '=' if any */
        base64_default_or_url = 8, /* standard/base64url hybrid format (only meaningful for decoding!) */
        base64_default_or_url_accept_garbage = 12, /* standard/base64url hybrid format accepting garbage characters
                                                      (only meaningful for decoding!), the input stops with the first '='
                                                      if any */
    };

    // last_chunk_handling_options are used to specify the handling of the last
    // chunk in base64 decoding.
    // https://tc39.es/proposal-arraybuffer-base64/spec/#sec-frombase64
    enum last_chunk_handling_options : uint64_t {
        loose = 0, /* standard base64 format, decode partial final chunk */
        strict = 1, /* error when the last chunk is partial, 2 or 3 chars, and
                       unpadded, or non-zero bit padding */
        stop_before_partial = 2, /* if the last chunk is partial, ignore it (no error) */
        only_full_chunks = 3 /* only decode full blocks (4 base64 characters, no padding) */
    };

    inline  bool
    is_partial(last_chunk_handling_options options) {
        return (options == stop_before_partial) || (options == only_full_chunks);
    }

} // namespace turbo
