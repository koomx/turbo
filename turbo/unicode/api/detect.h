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

#include <turbo/unicode/engine/common_defs.h>
#include <turbo/unicode/engine/compiler_check.h>
#include <turbo/unicode/text_encoding.h>
#include <turbo/unicode/error.h>

namespace turbo {

    /// Autodetect the encoding of the input, a single encoding is recommended.
    /// E.g., the function might return turbo::encoding_type::UTF8,
    /// turbo::encoding_type::UTF16_LE, turbo::encoding_type::UTF16_BE, or
    /// turbo::encoding_type::UTF32_LE.
    ///
    /// @param input the string to analyze.
    /// @param length the length of the string in bytes.
    /// @return the detected encoding type
    simdutf_warn_unused turbo::encoding_type
    autodetect_encoding(const char* input, size_t length) noexcept;
    simdutf_really_inline simdutf_warn_unused turbo::encoding_type
    autodetect_encoding(const uint8_t* input, size_t length) noexcept {
        return autodetect_encoding(reinterpret_cast<const char*>(input), length);
    }


    /// Autodetect the possible encodings of the input in one pass.
    /// E.g., if the input might be UTF-16LE or UTF-8, this function returns
    /// the value (turbo::encoding_type::UTF8 | turbo::encoding_type::UTF16_LE).
    ///
    /// Overridden by each implementation.
    ///
    /// @param input the string to analyze.
    /// @param length the length of the string in bytes.
    /// @return the detected encoding type
    simdutf_warn_unused int detect_encodings(const char* input,
        size_t length) noexcept;
    simdutf_really_inline simdutf_warn_unused int
    detect_encodings(const uint8_t* input, size_t length) noexcept {
        return detect_encodings(reinterpret_cast<const char*>(input), length);
    }

}  // namespace turbo
