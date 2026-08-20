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

#ifndef UNICODE_SWAP_BYTES_H
#define UNICODE_SWAP_BYTES_H

namespace turbo {
    namespace scalar {

        [[nodiscard]] constexpr inline uint16_t
        u16_swap_bytes(const uint16_t word) {
            return uint16_t((word >> 8) | (word << 8));
        }

        [[nodiscard]] constexpr inline uint32_t
        u32_swap_bytes(const uint32_t word) {
            return ((word >> 24) & 0xff) | // move byte 3 to byte 0
                ((word << 8) & 0xff0000) | // move byte 1 to byte 2
                ((word >> 8) & 0xff00) | // move byte 2 to byte 1
                ((word << 24) & 0xff000000); // byte 0 to byte 3
        }

        namespace utf32 {
            template <endianness big_endian>
            constexpr uint32_t swap_if_needed(uint32_t c) {
                return !match_system(big_endian) ? scalar::u32_swap_bytes(c) : c;
            }
        } // namespace utf32

        namespace utf16 {
            template <endianness big_endian>
            constexpr uint16_t swap_if_needed(uint16_t c) {
                return !match_system(big_endian) ? scalar::u16_swap_bytes(c) : c;
            }
        } // namespace utf16

    } // namespace scalar
} // namespace turbo

#endif
