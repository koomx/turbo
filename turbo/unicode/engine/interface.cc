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

#include <turbo/unicode/engine/interface.h>

namespace turbo {

    bool implementation::supported_by_runtime_system() const {
        uint32_t required_instruction_sets = this->required_instruction_sets();
        uint32_t supported_instruction_sets = internal::detect_supported_architectures();
        return ((supported_instruction_sets & required_instruction_sets) == required_instruction_sets);
    }


     [[nodiscard]] TextEncoding implementation::autodetect_encoding(
        const char* input, size_t length) const noexcept {
        // If there is a BOM, then we trust it.
        auto bom_encoding = turbo::BOM::check_bom(input, length);
        if (bom_encoding != TextEncoding::unspecified) {
            return bom_encoding;
        }
        // UTF8 is common, it includes ASCII, and is commonly represented
        // without a BOM, so if it fits, go with that. Note that it is still
        // possible to get it wrong, we are only 'guessing'. If some has UTF-16
        // data without a BOM, it could pass as UTF-8.
        //
        // An interesting twist might be to check for UTF-16 ASCII first (every
        // other byte is zero).
        if (validate_utf8(input, length)) {
            return TextEncoding::UTF8;
        }
        // The next most common encoding that might appear without BOM is probably
        // UTF-16LE, so try that next.
        if ((length % 2) == 0) {
            // important: we need to divide by two
            if (validate_utf16le(reinterpret_cast<const char16_t*>(input),
                    length / 2)) {
                return TextEncoding::UTF16_LE;
                    }
        }
        if ((length % 4) == 0) {
            if (validate_utf32(reinterpret_cast<const char32_t*>(input), length / 4)) {
                return TextEncoding::UTF32_LE;
            }
        }
        return TextEncoding::unspecified;
    }

#ifdef SIMDUTF_INTERNAL_TESTS
    std::vector<implementation::TestProcedure>
    implementation::internal_tests() const {
        return {};
    }
#endif


     [[nodiscard]] size_t implementation::maximal_binary_length_from_base64(
        const char* input, size_t length) const noexcept {
        return scalar::base64::maximal_binary_length_from_base64(input, length);
    }

     [[nodiscard]] size_t implementation::maximal_binary_length_from_base64(
        const char16_t* input, size_t length) const noexcept {
        return scalar::base64::maximal_binary_length_from_base64(input, length);
    }

     [[nodiscard]] size_t implementation::binary_length_from_base64(
        const char* input, size_t length) const noexcept {
        return scalar::base64::binary_length_from_base64(input, length);
    }

     [[nodiscard]] size_t implementation::binary_length_from_base64(
    const char16_t* input, size_t length) const noexcept {
        return scalar::base64::binary_length_from_base64(input, length);
    }

     [[nodiscard]] size_t implementation::base64_length_from_binary(
    size_t length, Base64Options options) const noexcept {
        return scalar::base64::base64_length_from_binary(length, options);
    }

}  // namespace turbo
