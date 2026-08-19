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

#include <string_view>
#include <turbo/unicode/engine/portability.h>
#include <turbo/unicode/engine/common_defs.h>

namespace turbo {

    enum TextEncoding {
        UTF8 = 1, // BOM 0xef 0xbb 0xbf
        UTF16_LE = 2, // BOM 0xff 0xfe
        UTF16_BE = 4, // BOM 0xfe 0xff
        UTF32_LE = 8, // BOM 0xff 0xfe 0x00 0x00
        UTF32_BE = 16, // BOM 0x00 0x00 0xfe 0xff
        Latin1 = 32,

        unspecified = 0
    };

#ifndef SIMDUTF_IS_BIG_ENDIAN
#error "SIMDUTF_IS_BIG_ENDIAN needs to be defined."
#endif

    enum endianness {
        LITTLE = 0,
        BIG = 1,
        NATIVE =
#if SIMDUTF_IS_BIG_ENDIAN
            BIG
#else
            LITTLE
#endif
    };

     [[nodiscard]] KUMO_FORCE_INLINE constexpr bool
    match_system(endianness e) {
        return e == endianness::NATIVE;
    }

     [[nodiscard]] std::string_view to_string(TextEncoding bom);

    // Note that BOM for UTF8 is discouraged.
    namespace BOM {

        /// Checks for a BOM. If not, returns unspecified
        /// @param byte          the string to process
        /// @param length        the length of the string in code units
        /// @return the corresponding encoding

         [[nodiscard]] TextEncoding check_bom(const uint8_t* byte, size_t length);
         [[nodiscard]] TextEncoding check_bom(const char* byte, size_t length);
        /// Returns the size, in bytes, of the BOM for a given encoding type.
        /// Note that UTF8 BOM are discouraged.
        /// @param bom         the encoding type
        /// @return the size in bytes of the corresponding BOM
         [[nodiscard]] size_t bom_byte_size(TextEncoding bom);

    } // namespace BOM

} // namespace turbo

