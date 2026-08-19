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


namespace turbo {
    namespace scalar {
        namespace utf32 {

            template <typename InputPtr>
             [[nodiscard]]  bool validate(InputPtr data,
                size_t len) noexcept {
                uint64_t pos = 0;
                for (; pos < len; pos++) {
                    uint32_t word = data[pos];
                    if (word > 0x10FFFF || (word >= 0xD800 && word <= 0xDFFF)) {
                        return false;
                    }
                }
                return true;
            }

             [[nodiscard]] KUMO_FORCE_INLINE bool validate(const char32_t* buf,
                size_t len) noexcept {
                return validate(reinterpret_cast<const uint32_t*>(buf), len);
            }

            template <typename InputPtr>
             [[nodiscard]]  UnicodeResult
            validate_with_errors(InputPtr data, size_t len) noexcept {
                size_t pos = 0;
                for (; pos < len; pos++) {
                    uint32_t word = data[pos];
                    if (word > 0x10FFFF) {
                        return UnicodeResult(UnicodeError::TOO_LARGE, pos);
                    }
                    if (word >= 0xD800 && word <= 0xDFFF) {
                        return UnicodeResult(UnicodeError::SURROGATE, pos);
                    }
                }
                return UnicodeResult(UnicodeError::SUCCESS, pos);
            }

             [[nodiscard]] KUMO_FORCE_INLINE UnicodeResult
            validate_with_errors(const char32_t* buf, size_t len) noexcept {
                return validate_with_errors(reinterpret_cast<const uint32_t*>(buf), len);
            }

            inline  size_t utf8_length_from_utf32(const char32_t* p,
                size_t len) {
                // We are not BOM aware.
                size_t counter { 0 };
                for (size_t i = 0; i < len; i++) {
                    // credit: @ttsugriy  for the vectorizable approach
                    counter++; // ASCII
                    counter += static_cast<size_t>(p[i] > 0x7F); // two-byte
                    counter += static_cast<size_t>(p[i] > 0x7FF); // three-byte
                    counter += static_cast<size_t>(p[i] > 0xFFFF); // four-bytes
                }
                return counter;
            }

            [[nodiscard]] inline  size_t
            utf16_length_from_utf32(const char32_t* p, size_t len) {
                // We are not BOM aware.
                size_t counter { 0 };
                for (size_t i = 0; i < len; i++) {
                    counter++; // non-surrogate word
                    counter += static_cast<size_t>(p[i] > 0xFFFF); // surrogate pair
                }
                return counter;
            }

        } // namespace utf32
    } // namespace scalar
} // namespace turbo

