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

#include <climits>
#include <initializer_list>
#include <turbo/macros/macros/pragma/pragma.h>
#include <turbo/unicode/engine/backend_select.h>
#include <turbo/unicode/utf.h>
#include <type_traits>
// The best choice should always come first!
KUMO_DISABLE_UNUSED_WARNING
#include <turbo/unicode/engine/arm64.h>
#include <turbo/unicode/engine/icelake.h>
#include <turbo/unicode/engine/haswell.h>
#include <turbo/unicode/engine/westmere.h>
#include <turbo/unicode/engine/ppc64.h>
#include <turbo/unicode/engine/rvv.h>
#include <turbo/unicode/engine/lasx.h>
#include <turbo/unicode/engine/lsx.h>
#include <turbo/unicode/engine/fallback.h> // have it always last.
KUMO_RESTORE_UNUSED_WARNING


#include <turbo/unicode/engine/backend_select.h>

static_assert(sizeof(uint8_t) == sizeof(char),
    "simdutf requires that uint8_t be a char");
static_assert(sizeof(uint16_t) == sizeof(char16_t),
    "simdutf requires that char16_t be 16 bits");
static_assert(sizeof(uint32_t) == sizeof(char32_t),
    "simdutf requires that char32_t be 32 bits");
// next line is redundant, but it is kept to catch defective systems.
static_assert(CHAR_BIT == 8, "simdutf requires 8-bit bytes");

namespace turbo {

    namespace internal {
// When there is a single implementation, we should not pay a price
// for dispatching to the best implementation. We should just use the
// one we have. This is a compile-time check.
#define UNICODE_SINGLE_IMPLEMENTATION \
    (UNICODE_IMPLEMENTATION_ICELAKE + UNICODE_IMPLEMENTATION_HASWELL + UNICODE_IMPLEMENTATION_WESTMERE + UNICODE_IMPLEMENTATION_ARM64 + UNICODE_IMPLEMENTATION_PPC64 + UNICODE_IMPLEMENTATION_LSX + UNICODE_IMPLEMENTATION_LASX + UNICODE_IMPLEMENTATION_FALLBACK == 1)

#if UNICODE_IMPLEMENTATION_ICELAKE
        static const icelake::implementation* get_icelake_singleton() {
            static const icelake::implementation icelake_singleton { };
            return &icelake_singleton;
        }
#endif
#if UNICODE_IMPLEMENTATION_HASWELL
        static const haswell::implementation* get_haswell_singleton() {
            static const haswell::implementation haswell_singleton { };
            return &haswell_singleton;
        }
#endif
#if UNICODE_IMPLEMENTATION_WESTMERE
        static const westmere::implementation* get_westmere_singleton() {
            static const westmere::implementation westmere_singleton { };
            return &westmere_singleton;
        }
#endif
#if UNICODE_IMPLEMENTATION_ARM64
        static const arm64::implementation* get_arm64_singleton() {
            static const arm64::implementation arm64_singleton { };
            return &arm64_singleton;
        }
#endif
#if UNICODE_IMPLEMENTATION_PPC64
        static const ppc64::implementation* get_ppc64_singleton() {
            static const ppc64::implementation ppc64_singleton { };
            return &ppc64_singleton;
        }
#endif
#if UNICODE_IMPLEMENTATION_RVV
        static const rvv::implementation* get_rvv_singleton() {
            static const rvv::implementation rvv_singleton { };
            return &rvv_singleton;
        }
#endif
#if UNICODE_IMPLEMENTATION_LASX
        static const lasx::implementation* get_lasx_singleton() {
            static const lasx::implementation lasx_singleton { };
            return &lasx_singleton;
        }
#endif
#if UNICODE_IMPLEMENTATION_LSX
        static const lsx::implementation* get_lsx_singleton() {
            static const lsx::implementation lsx_singleton { };
            return &lsx_singleton;
        }
#endif
#if UNICODE_IMPLEMENTATION_FALLBACK
        static const fallback::implementation* get_fallback_singleton() {
            static const fallback::implementation fallback_singleton { };
            return &fallback_singleton;
        }
#endif

#if UNICODE_SINGLE_IMPLEMENTATION
        KUMO_FORCE_INLINE static const implementation* get_single_implementation() {
            return
#if UNICODE_IMPLEMENTATION_ICELAKE
                get_icelake_singleton();
#endif
#if UNICODE_IMPLEMENTATION_HASWELL
            get_haswell_singleton();
#endif
#if UNICODE_IMPLEMENTATION_WESTMERE
            get_westmere_singleton();
#endif
#if UNICODE_IMPLEMENTATION_ARM64
            get_arm64_singleton();
#endif
#if UNICODE_IMPLEMENTATION_PPC64
            get_ppc64_singleton();
#endif
#if UNICODE_IMPLEMENTATION_LASX
            get_lasx_singleton();
#endif
#if UNICODE_IMPLEMENTATION_LSX
            get_lsx_singleton();
#endif
#if UNICODE_IMPLEMENTATION_FALLBACK
            get_fallback_singleton();
#endif
        }
#endif

        /// @private Detects best supported implementation on first use, and sets it
        class detect_best_supported_implementation_on_first_use
            : public implementation {
        public:
            std::string_view name() const noexcept final { return set_best()->name(); }
            std::string_view description() const noexcept final {
                return set_best()->description();
            }
            uint32_t required_instruction_sets() const noexcept final {
                return set_best()->required_instruction_sets();
            }

            [[nodiscard]] int
            detect_encodings(const char* input, size_t length) const noexcept override {
                return set_best()->detect_encodings(input, length);
            }

            [[nodiscard]] bool
            validate_utf8(const char* buf, size_t len) const noexcept final {
                return set_best()->validate_utf8(buf, len);
            }

            [[nodiscard]] UnicodeResult validate_utf8_with_errors(
                const char* buf, size_t len) const noexcept final {
                return set_best()->validate_utf8_with_errors(buf, len);
            }

            [[nodiscard]] bool
            validate_ascii(const char* buf, size_t len) const noexcept final {
                return set_best()->validate_ascii(buf, len);
            }
            [[nodiscard]] UnicodeResult validate_ascii_with_errors(
                const char* buf, size_t len) const noexcept final {
                return set_best()->validate_ascii_with_errors(buf, len);
            }

            [[nodiscard]] bool
            validate_utf16le_as_ascii(const char16_t* buf,
                size_t len) const noexcept final {
                return set_best()->validate_utf16le_as_ascii(buf, len);
            }
            [[nodiscard]] bool
            validate_utf16be_as_ascii(const char16_t* buf,
                size_t len) const noexcept final {
                return set_best()->validate_utf16be_as_ascii(buf, len);
            }

            [[nodiscard]] bool
            validate_utf16le(const char16_t* buf,
                size_t len) const noexcept final {
                return set_best()->validate_utf16le(buf, len);
            }

            [[nodiscard]] bool
            validate_utf16be(const char16_t* buf,
                size_t len) const noexcept final {
                return set_best()->validate_utf16be(buf, len);
            }

            [[nodiscard]] UnicodeResult validate_utf16le_with_errors(
                const char16_t* buf, size_t len) const noexcept final {
                return set_best()->validate_utf16le_with_errors(buf, len);
            }

            [[nodiscard]] UnicodeResult validate_utf16be_with_errors(
                const char16_t* buf, size_t len) const noexcept final {
                return set_best()->validate_utf16be_with_errors(buf, len);
            }
            void to_well_formed_utf16be(const char16_t* input, size_t len,
                char16_t* output) const noexcept final {
                return set_best()->to_well_formed_utf16be(input, len, output);
            }
            void to_well_formed_utf16le(const char16_t* input, size_t len,
                char16_t* output) const noexcept final {
                return set_best()->to_well_formed_utf16le(input, len, output);
            }

            [[nodiscard]] bool
            validate_utf32(const char32_t* buf,
                size_t len) const noexcept final {
                return set_best()->validate_utf32(buf, len);
            }

            [[nodiscard]] UnicodeResult validate_utf32_with_errors(
                const char32_t* buf, size_t len) const noexcept final {
                return set_best()->validate_utf32_with_errors(buf, len);
            }

            [[nodiscard]] size_t
            convert_latin1_to_utf8(const char* buf, size_t len,
                char* utf8_output) const noexcept final {
                return set_best()->convert_latin1_to_utf8(buf, len, utf8_output);
            }

            [[nodiscard]] size_t convert_latin1_to_utf16le(
                const char* buf, size_t len,
                char16_t* utf16_output) const noexcept final {
                return set_best()->convert_latin1_to_utf16le(buf, len, utf16_output);
            }

            [[nodiscard]] size_t convert_latin1_to_utf16be(
                const char* buf, size_t len,
                char16_t* utf16_output) const noexcept final {
                return set_best()->convert_latin1_to_utf16be(buf, len, utf16_output);
            }

            [[nodiscard]] size_t convert_latin1_to_utf32(
                const char* buf, size_t len,
                char32_t* latin1_output) const noexcept final {
                return set_best()->convert_latin1_to_utf32(buf, len, latin1_output);
            }

            [[nodiscard]] size_t
            convert_utf8_to_latin1(const char* buf, size_t len,
                char* latin1_output) const noexcept final {
                return set_best()->convert_utf8_to_latin1(buf, len, latin1_output);
            }

            [[nodiscard]] UnicodeResult convert_utf8_to_latin1_with_errors(
                const char* buf, size_t len,
                char* latin1_output) const noexcept final {
                return set_best()->convert_utf8_to_latin1_with_errors(buf, len,
                    latin1_output);
            }

            [[nodiscard]] size_t convert_valid_utf8_to_latin1(
                const char* buf, size_t len,
                char* latin1_output) const noexcept final {
                return set_best()->convert_valid_utf8_to_latin1(buf, len, latin1_output);
            }

            [[nodiscard]] size_t convert_utf8_to_utf16le(
                const char* buf, size_t len,
                char16_t* utf16_output) const noexcept final {
                return set_best()->convert_utf8_to_utf16le(buf, len, utf16_output);
            }

            [[nodiscard]] size_t convert_utf8_to_utf16be(
                const char* buf, size_t len,
                char16_t* utf16_output) const noexcept final {
                return set_best()->convert_utf8_to_utf16be(buf, len, utf16_output);
            }

            [[nodiscard]] UnicodeResult convert_utf8_to_utf16le_with_errors(
                const char* buf, size_t len,
                char16_t* utf16_output) const noexcept final {
                return set_best()->convert_utf8_to_utf16le_with_errors(buf, len,
                    utf16_output);
            }

            [[nodiscard]] UnicodeResult convert_utf8_to_utf16be_with_errors(
                const char* buf, size_t len,
                char16_t* utf16_output) const noexcept final {
                return set_best()->convert_utf8_to_utf16be_with_errors(buf, len,
                    utf16_output);
            }

            [[nodiscard]] size_t convert_valid_utf8_to_utf16le(
                const char* buf, size_t len,
                char16_t* utf16_output) const noexcept final {
                return set_best()->convert_valid_utf8_to_utf16le(buf, len, utf16_output);
            }

            [[nodiscard]] size_t convert_valid_utf8_to_utf16be(
                const char* buf, size_t len,
                char16_t* utf16_output) const noexcept final {
                return set_best()->convert_valid_utf8_to_utf16be(buf, len, utf16_output);
            }
            [[nodiscard]] UnicodeResult utf8_length_from_utf16le_with_replacement(
                const char16_t* input, size_t length) const noexcept final {
                return set_best()->utf8_length_from_utf16le_with_replacement(input, length);
            }

            [[nodiscard]] UnicodeResult utf8_length_from_utf16be_with_replacement(
                const char16_t* input, size_t length) const noexcept final {
                return set_best()->utf8_length_from_utf16be_with_replacement(input, length);
            }

            [[nodiscard]] size_t convert_utf16le_to_utf8_with_replacement(
                const char16_t* input, size_t length,
                char* utf8_buffer) const noexcept final {
                return set_best()->convert_utf16le_to_utf8_with_replacement(input, length,
                    utf8_buffer);
            }

            [[nodiscard]] size_t convert_utf16be_to_utf8_with_replacement(
                const char16_t* input, size_t length,
                char* utf8_buffer) const noexcept final {
                return set_best()->convert_utf16be_to_utf8_with_replacement(input, length,
                    utf8_buffer);
            }

            [[nodiscard]] size_t
            convert_utf8_to_utf32(const char* buf, size_t len,
                char32_t* utf32_output) const noexcept final {
                return set_best()->convert_utf8_to_utf32(buf, len, utf32_output);
            }

            [[nodiscard]] UnicodeResult convert_utf8_to_utf32_with_errors(
                const char* buf, size_t len,
                char32_t* utf32_output) const noexcept final {
                return set_best()->convert_utf8_to_utf32_with_errors(buf, len,
                    utf32_output);
            }

            [[nodiscard]] size_t convert_valid_utf8_to_utf32(
                const char* buf, size_t len,
                char32_t* utf32_output) const noexcept final {
                return set_best()->convert_valid_utf8_to_utf32(buf, len, utf32_output);
            }

            [[nodiscard]] size_t
            convert_utf16le_to_latin1(const char16_t* buf, size_t len,
                char* latin1_output) const noexcept final {
                return set_best()->convert_utf16le_to_latin1(buf, len, latin1_output);
            }

            [[nodiscard]] size_t
            convert_utf16be_to_latin1(const char16_t* buf, size_t len,
                char* latin1_output) const noexcept final {
                return set_best()->convert_utf16be_to_latin1(buf, len, latin1_output);
            }

            [[nodiscard]] UnicodeResult convert_utf16le_to_latin1_with_errors(
                const char16_t* buf, size_t len,
                char* latin1_output) const noexcept final {
                return set_best()->convert_utf16le_to_latin1_with_errors(buf, len,
                    latin1_output);
            }

            [[nodiscard]] UnicodeResult convert_utf16be_to_latin1_with_errors(
                const char16_t* buf, size_t len,
                char* latin1_output) const noexcept final {
                return set_best()->convert_utf16be_to_latin1_with_errors(buf, len,
                    latin1_output);
            }

            [[nodiscard]] size_t convert_valid_utf16le_to_latin1(
                const char16_t* buf, size_t len,
                char* latin1_output) const noexcept final {
                return set_best()->convert_valid_utf16le_to_latin1(buf, len, latin1_output);
            }

            [[nodiscard]] size_t convert_valid_utf16be_to_latin1(
                const char16_t* buf, size_t len,
                char* latin1_output) const noexcept final {
                return set_best()->convert_valid_utf16be_to_latin1(buf, len, latin1_output);
            }

            [[nodiscard]] size_t
            convert_utf16le_to_utf8(const char16_t* buf, size_t len,
                char* utf8_output) const noexcept final {
                return set_best()->convert_utf16le_to_utf8(buf, len, utf8_output);
            }

            [[nodiscard]] size_t
            convert_utf16be_to_utf8(const char16_t* buf, size_t len,
                char* utf8_output) const noexcept final {
                return set_best()->convert_utf16be_to_utf8(buf, len, utf8_output);
            }

            [[nodiscard]] UnicodeResult convert_utf16le_to_utf8_with_errors(
                const char16_t* buf, size_t len,
                char* utf8_output) const noexcept final {
                return set_best()->convert_utf16le_to_utf8_with_errors(buf, len,
                    utf8_output);
            }

            [[nodiscard]] UnicodeResult convert_utf16be_to_utf8_with_errors(
                const char16_t* buf, size_t len,
                char* utf8_output) const noexcept final {
                return set_best()->convert_utf16be_to_utf8_with_errors(buf, len,
                    utf8_output);
            }

            [[nodiscard]] size_t convert_valid_utf16le_to_utf8(
                const char16_t* buf, size_t len,
                char* utf8_output) const noexcept final {
                return set_best()->convert_valid_utf16le_to_utf8(buf, len, utf8_output);
            }

            [[nodiscard]] size_t convert_valid_utf16be_to_utf8(
                const char16_t* buf, size_t len,
                char* utf8_output) const noexcept final {
                return set_best()->convert_valid_utf16be_to_utf8(buf, len, utf8_output);
            }

            [[nodiscard]] size_t
            convert_utf32_to_latin1(const char32_t* buf, size_t len,
                char* latin1_output) const noexcept final {
                return set_best()->convert_utf32_to_latin1(buf, len, latin1_output);
            }

            [[nodiscard]] UnicodeResult convert_utf32_to_latin1_with_errors(
                const char32_t* buf, size_t len,
                char* latin1_output) const noexcept final {
                return set_best()->convert_utf32_to_latin1_with_errors(buf, len,
                    latin1_output);
            }

            [[nodiscard]] size_t convert_valid_utf32_to_latin1(
                const char32_t* buf, size_t len,
                char* latin1_output) const noexcept final {
                return set_best()->convert_utf32_to_latin1(buf, len, latin1_output);
            }

            [[nodiscard]] size_t
            convert_utf32_to_utf8(const char32_t* buf, size_t len,
                char* utf8_output) const noexcept final {
                return set_best()->convert_utf32_to_utf8(buf, len, utf8_output);
            }

            [[nodiscard]] UnicodeResult convert_utf32_to_utf8_with_errors(
                const char32_t* buf, size_t len,
                char* utf8_output) const noexcept final {
                return set_best()->convert_utf32_to_utf8_with_errors(buf, len, utf8_output);
            }

            [[nodiscard]] size_t
            convert_valid_utf32_to_utf8(const char32_t* buf, size_t len,
                char* utf8_output) const noexcept final {
                return set_best()->convert_valid_utf32_to_utf8(buf, len, utf8_output);
            }

            [[nodiscard]] size_t convert_utf32_to_utf16le(
                const char32_t* buf, size_t len,
                char16_t* utf16_output) const noexcept final {
                return set_best()->convert_utf32_to_utf16le(buf, len, utf16_output);
            }

            [[nodiscard]] size_t convert_utf32_to_utf16be(
                const char32_t* buf, size_t len,
                char16_t* utf16_output) const noexcept final {
                return set_best()->convert_utf32_to_utf16be(buf, len, utf16_output);
            }

            [[nodiscard]] UnicodeResult convert_utf32_to_utf16le_with_errors(
                const char32_t* buf, size_t len,
                char16_t* utf16_output) const noexcept final {
                return set_best()->convert_utf32_to_utf16le_with_errors(buf, len,
                    utf16_output);
            }

            [[nodiscard]] UnicodeResult convert_utf32_to_utf16be_with_errors(
                const char32_t* buf, size_t len,
                char16_t* utf16_output) const noexcept final {
                return set_best()->convert_utf32_to_utf16be_with_errors(buf, len,
                    utf16_output);
            }

            [[nodiscard]] size_t convert_valid_utf32_to_utf16le(
                const char32_t* buf, size_t len,
                char16_t* utf16_output) const noexcept final {
                return set_best()->convert_valid_utf32_to_utf16le(buf, len, utf16_output);
            }

            [[nodiscard]] size_t convert_valid_utf32_to_utf16be(
                const char32_t* buf, size_t len,
                char16_t* utf16_output) const noexcept final {
                return set_best()->convert_valid_utf32_to_utf16be(buf, len, utf16_output);
            }

            [[nodiscard]] size_t convert_utf16le_to_utf32(
                const char16_t* buf, size_t len,
                char32_t* utf32_output) const noexcept final {
                return set_best()->convert_utf16le_to_utf32(buf, len, utf32_output);
            }

            [[nodiscard]] size_t convert_utf16be_to_utf32(
                const char16_t* buf, size_t len,
                char32_t* utf32_output) const noexcept final {
                return set_best()->convert_utf16be_to_utf32(buf, len, utf32_output);
            }

            [[nodiscard]] UnicodeResult convert_utf16le_to_utf32_with_errors(
                const char16_t* buf, size_t len,
                char32_t* utf32_output) const noexcept final {
                return set_best()->convert_utf16le_to_utf32_with_errors(buf, len,
                    utf32_output);
            }

            [[nodiscard]] UnicodeResult convert_utf16be_to_utf32_with_errors(
                const char16_t* buf, size_t len,
                char32_t* utf32_output) const noexcept final {
                return set_best()->convert_utf16be_to_utf32_with_errors(buf, len,
                    utf32_output);
            }

            [[nodiscard]] size_t convert_valid_utf16le_to_utf32(
                const char16_t* buf, size_t len,
                char32_t* utf32_output) const noexcept final {
                return set_best()->convert_valid_utf16le_to_utf32(buf, len, utf32_output);
            }

            [[nodiscard]] size_t convert_valid_utf16be_to_utf32(
                const char16_t* buf, size_t len,
                char32_t* utf32_output) const noexcept final {
                return set_best()->convert_valid_utf16be_to_utf32(buf, len, utf32_output);
            }

            void change_endianness_utf16(const char16_t* buf, size_t len,
                char16_t* output) const noexcept final {
                set_best()->change_endianness_utf16(buf, len, output);
            }

            [[nodiscard]] size_t
            count_utf16le(const char16_t* buf, size_t len) const noexcept final {
                return set_best()->count_utf16le(buf, len);
            }

            [[nodiscard]] size_t
            count_utf16be(const char16_t* buf, size_t len) const noexcept final {
                return set_best()->count_utf16be(buf, len);
            }

            [[nodiscard]] size_t
            count_utf8(const char* buf, size_t len) const noexcept final {
                return set_best()->count_utf8(buf, len);
            }

            [[nodiscard]] size_t
            latin1_length_from_utf8(const char* buf, size_t len) const noexcept override {
                return set_best()->latin1_length_from_utf8(buf, len);
            }

            [[nodiscard]] size_t
            utf8_length_from_latin1(const char* buf, size_t len) const noexcept override {
                return set_best()->utf8_length_from_latin1(buf, len);
            }

            [[nodiscard]] size_t utf8_length_from_utf16le(
                const char16_t* buf, size_t len) const noexcept override {
                return set_best()->utf8_length_from_utf16le(buf, len);
            }

            [[nodiscard]] size_t utf8_length_from_utf16be(
                const char16_t* buf, size_t len) const noexcept override {
                return set_best()->utf8_length_from_utf16be(buf, len);
            }

            [[nodiscard]] size_t utf32_length_from_utf16le(
                const char16_t* buf, size_t len) const noexcept override {
                return set_best()->utf32_length_from_utf16le(buf, len);
            }

            [[nodiscard]] size_t utf32_length_from_utf16be(
                const char16_t* buf, size_t len) const noexcept override {
                return set_best()->utf32_length_from_utf16be(buf, len);
            }

            [[nodiscard]] size_t
            utf16_length_from_utf8(const char* buf, size_t len) const noexcept override {
                return set_best()->utf16_length_from_utf8(buf, len);
            }

            [[nodiscard]] size_t utf8_length_from_utf32(
                const char32_t* buf, size_t len) const noexcept override {
                return set_best()->utf8_length_from_utf32(buf, len);
            }

            [[nodiscard]] size_t utf16_length_from_utf32(
                const char32_t* buf, size_t len) const noexcept override {
                return set_best()->utf16_length_from_utf32(buf, len);
            }

            [[nodiscard]] size_t
            utf32_length_from_utf8(const char* buf, size_t len) const noexcept override {
                return set_best()->utf32_length_from_utf8(buf, len);
            }

            [[nodiscard]] UnicodeResult base64_to_binary(
                const char* input, size_t length, char* output, Base64Options options,
                last_chunk_handling_options last_chunk_handling_options = last_chunk_handling_options::loose) const noexcept override {
                return set_best()->base64_to_binary(input, length, output, options,
                    last_chunk_handling_options);
            }

            [[nodiscard]] full_result base64_to_binary_details(
                const char* input, size_t length, char* output, Base64Options options,
                last_chunk_handling_options last_chunk_handling_options = last_chunk_handling_options::loose) const noexcept override {
                return set_best()->base64_to_binary_details(input, length, output, options,
                    last_chunk_handling_options);
            }

            [[nodiscard]] UnicodeResult base64_to_binary(
                const char16_t* input, size_t length, char* output,
                Base64Options options,
                last_chunk_handling_options last_chunk_handling_options = last_chunk_handling_options::loose) const noexcept override {
                return set_best()->base64_to_binary(input, length, output, options,
                    last_chunk_handling_options);
            }

            [[nodiscard]] full_result base64_to_binary_details(
                const char16_t* input, size_t length, char* output,
                Base64Options options,
                last_chunk_handling_options last_chunk_handling_options = last_chunk_handling_options::loose) const noexcept override {
                return set_best()->base64_to_binary_details(input, length, output, options,
                    last_chunk_handling_options);
            }

            size_t binary_to_base64(const char* input, size_t length, char* output,
                Base64Options options) const noexcept override {
                return set_best()->binary_to_base64(input, length, output, options);
            }

            size_t
            binary_to_base64_with_lines(const char* input, size_t length, char* output,
                size_t line_length,
                Base64Options options) const noexcept override {
                return set_best()->binary_to_base64_with_lines(input, length, output,
                    line_length, options);
            }

            const char* find(const char* start, const char* end,
                char character) const noexcept override {
                return set_best()->find(start, end, character);
            }

            const char16_t* find(const char16_t* start, const char16_t* end,
                char16_t character) const noexcept override {
                return set_best()->find(start, end, character);
            }

            [[nodiscard]] size_t binary_length_from_base64(
                const char* input, size_t length) const noexcept override {
                return set_best()->binary_length_from_base64(input, length);
            }

            [[nodiscard]] size_t binary_length_from_base64(
                const char16_t* input, size_t length) const noexcept override {
                return set_best()->binary_length_from_base64(input, length);
            }

            KUMO_FORCE_INLINE
            detect_best_supported_implementation_on_first_use() noexcept
                : implementation("best_supported_detector",
                      "Detects the best supported implementation and sets it",
                      0) { }

        private:
            const implementation* set_best() const noexcept;
        };

        static_assert(std::is_trivially_destructible<
                          detect_best_supported_implementation_on_first_use>::value,
            "detect_best_supported_implementation_on_first_use should be "
            "trivially destructible");

        static const std::initializer_list<const implementation*>&
        get_available_implementation_pointers() {
            static const std::initializer_list<const implementation*>
                available_implementation_pointers {
#if UNICODE_IMPLEMENTATION_ICELAKE
                    get_icelake_singleton(),
#endif
#if UNICODE_IMPLEMENTATION_HASWELL
                    get_haswell_singleton(),
#endif
#if UNICODE_IMPLEMENTATION_WESTMERE
                    get_westmere_singleton(),
#endif
#if UNICODE_IMPLEMENTATION_ARM64
                    get_arm64_singleton(),
#endif
#if UNICODE_IMPLEMENTATION_PPC64
                    get_ppc64_singleton(),
#endif
#if UNICODE_IMPLEMENTATION_RVV
                    get_rvv_singleton(),
#endif
#if UNICODE_IMPLEMENTATION_LASX
                    get_lasx_singleton(),
#endif
#if UNICODE_IMPLEMENTATION_LSX
                    get_lsx_singleton(),
#endif
#if UNICODE_IMPLEMENTATION_FALLBACK
                    get_fallback_singleton(),
#endif
                };
            return available_implementation_pointers;
        }

        // So we can return UNSUPPORTED_ARCHITECTURE from the parser when there is no
        // support
        class unsupported_implementation : public implementation {
        public:
            [[nodiscard]] int detect_encodings(const char*,
                size_t) const noexcept override {
                return TextEncoding::unspecified;
            }

            [[nodiscard]] bool validate_utf8(const char*,
                size_t) const noexcept final {
                return false; // Just refuse to validate. Given that we have a fallback
                              // implementation
                // it seems unlikely that unsupported_implementation will ever be used. If
                // it is used, then it will flag all strings as invalid. The alternative is
                // to return an UnicodeError from which the user has to figure out whether the
                // string is valid UTF-8... which seems like a lot of work just to handle
                // the very unlikely case that we have an unsupported implementation. And,
                // when it does happen (that we have an unsupported implementation), what
                // are the chances that the programmer has a fallback? Given that *we*
                // provide the fallback, it implies that the programmer would need a
                // fallback for our fallback.
            }

            [[nodiscard]] UnicodeResult validate_utf8_with_errors(
                const char*, size_t) const noexcept final {
                return UnicodeResult{UnicodeError::OTHER, 0};
            }

            [[nodiscard]] bool
            validate_ascii(const char*, size_t) const noexcept final {
                return false;
            }

            [[nodiscard]] UnicodeResult validate_ascii_with_errors(
                const char*, size_t) const noexcept final {
                return UnicodeResult{UnicodeError::OTHER, 0};
            }

            [[nodiscard]] bool
            validate_utf16le_as_ascii(const char16_t*,
                size_t) const noexcept final {
                return false;
            }

            [[nodiscard]] bool
            validate_utf16be_as_ascii(const char16_t*,
                size_t) const noexcept final {
                return false;
            }

            [[nodiscard]] bool
            validate_utf16le(const char16_t*, size_t) const noexcept final {
                return false;
            }

            [[nodiscard]] bool
            validate_utf16be(const char16_t*, size_t) const noexcept final {
                return false;
            }

            [[nodiscard]] UnicodeResult validate_utf16le_with_errors(
                const char16_t*, size_t) const noexcept final {
                return UnicodeResult{UnicodeError::OTHER, 0};
            }

            [[nodiscard]] UnicodeResult validate_utf16be_with_errors(
                const char16_t*, size_t) const noexcept final {
                return UnicodeResult{UnicodeError::OTHER, 0};
            }
            void to_well_formed_utf16be(const char16_t*, size_t,
                char16_t*) const noexcept final { }
            void to_well_formed_utf16le(const char16_t*, size_t,
                char16_t*) const noexcept final { }

            [[nodiscard]] bool
            validate_utf32(const char32_t*, size_t) const noexcept final {
                return false;
            }

            [[nodiscard]] UnicodeResult validate_utf32_with_errors(
                const char32_t*, size_t) const noexcept final {
                return UnicodeResult{UnicodeError::OTHER, 0};
            }

            [[nodiscard]] size_t convert_latin1_to_utf8(
                const char*, size_t, char*) const noexcept final {
                return 0;
            }

            [[nodiscard]] size_t convert_latin1_to_utf16le(
                const char*, size_t, char16_t*) const noexcept final {
                return 0;
            }

            [[nodiscard]] size_t convert_latin1_to_utf16be(
                const char*, size_t, char16_t*) const noexcept final {
                return 0;
            }

            [[nodiscard]] size_t convert_latin1_to_utf32(
                const char*, size_t, char32_t*) const noexcept final {
                return 0;
            }

            [[nodiscard]] size_t convert_utf8_to_latin1(
                const char*, size_t, char*) const noexcept final {
                return 0;
            }

            [[nodiscard]] UnicodeResult convert_utf8_to_latin1_with_errors(
                const char*, size_t, char*) const noexcept final {
                return UnicodeResult{UnicodeError::OTHER, 0};
            }

            [[nodiscard]] size_t convert_valid_utf8_to_latin1(
                const char*, size_t, char*) const noexcept final {
                return 0;
            }

            [[nodiscard]] size_t convert_utf8_to_utf16le(
                const char*, size_t, char16_t*) const noexcept final {
                return 0;
            }

            [[nodiscard]] size_t convert_utf8_to_utf16be(
                const char*, size_t, char16_t*) const noexcept final {
                return 0;
            }

            [[nodiscard]] UnicodeResult convert_utf8_to_utf16le_with_errors(
                const char*, size_t, char16_t*) const noexcept final {
                return UnicodeResult{UnicodeError::OTHER, 0};
            }

            [[nodiscard]] UnicodeResult convert_utf8_to_utf16be_with_errors(
                const char*, size_t, char16_t*) const noexcept final {
                return UnicodeResult{UnicodeError::OTHER, 0};
            }

            [[nodiscard]] size_t convert_valid_utf8_to_utf16le(
                const char*, size_t, char16_t*) const noexcept final {
                return 0;
            }

            [[nodiscard]] size_t convert_valid_utf8_to_utf16be(
                const char*, size_t, char16_t*) const noexcept final {
                return 0;
            }
            [[nodiscard]] UnicodeResult utf8_length_from_utf16le_with_replacement(
                const char16_t*, size_t) const noexcept final {
                return { OTHER, 0 }; // Not supported
            }

            [[nodiscard]] UnicodeResult utf8_length_from_utf16be_with_replacement(
                const char16_t*, size_t) const noexcept final {
                return { OTHER, 0 }; // Not supported
            }

            [[nodiscard]] size_t convert_utf16le_to_utf8_with_replacement(
                const char16_t*, size_t, char*) const noexcept final {
                return 0; // Not supported
            }

            [[nodiscard]] size_t convert_utf16be_to_utf8_with_replacement(
                const char16_t*, size_t, char*) const noexcept final {
                return 0; // Not supported
            }

            [[nodiscard]] size_t convert_utf8_to_utf32(
                const char*, size_t, char32_t*) const noexcept final {
                return 0;
            }

            [[nodiscard]] UnicodeResult convert_utf8_to_utf32_with_errors(
                const char*, size_t, char32_t*) const noexcept final {
                return UnicodeResult{UnicodeError::OTHER, 0};
            }

            [[nodiscard]] size_t convert_valid_utf8_to_utf32(
                const char*, size_t, char32_t*) const noexcept final {
                return 0;
            }

            [[nodiscard]] size_t convert_utf16le_to_latin1(
                const char16_t*, size_t, char*) const noexcept final {
                return 0;
            }

            [[nodiscard]] size_t convert_utf16be_to_latin1(
                const char16_t*, size_t, char*) const noexcept final {
                return 0;
            }

            [[nodiscard]] UnicodeResult convert_utf16le_to_latin1_with_errors(
                const char16_t*, size_t, char*) const noexcept final {
                return UnicodeResult{UnicodeError::OTHER, 0};
            }

            [[nodiscard]] UnicodeResult convert_utf16be_to_latin1_with_errors(
                const char16_t*, size_t, char*) const noexcept final {
                return UnicodeResult{UnicodeError::OTHER, 0};
            }

            [[nodiscard]] size_t convert_valid_utf16le_to_latin1(
                const char16_t*, size_t, char*) const noexcept final {
                return 0;
            }

            [[nodiscard]] size_t convert_valid_utf16be_to_latin1(
                const char16_t*, size_t, char*) const noexcept final {
                return 0;
            }

            [[nodiscard]] size_t convert_utf16le_to_utf8(
                const char16_t*, size_t, char*) const noexcept final {
                return 0;
            }

            [[nodiscard]] size_t convert_utf16be_to_utf8(
                const char16_t*, size_t, char*) const noexcept final {
                return 0;
            }

            [[nodiscard]] UnicodeResult convert_utf16le_to_utf8_with_errors(
                const char16_t*, size_t, char*) const noexcept final {
                return UnicodeResult{UnicodeError::OTHER, 0};
            }

            [[nodiscard]] UnicodeResult convert_utf16be_to_utf8_with_errors(
                const char16_t*, size_t, char*) const noexcept final {
                return UnicodeResult{UnicodeError::OTHER, 0};
            }

            [[nodiscard]] size_t convert_valid_utf16le_to_utf8(
                const char16_t*, size_t, char*) const noexcept final {
                return 0;
            }

            [[nodiscard]] size_t convert_valid_utf16be_to_utf8(
                const char16_t*, size_t, char*) const noexcept final {
                return 0;
            }

            [[nodiscard]] size_t convert_utf32_to_latin1(
                const char32_t*, size_t, char*) const noexcept final {
                return 0;
            }

            [[nodiscard]] UnicodeResult convert_utf32_to_latin1_with_errors(
                const char32_t*, size_t, char*) const noexcept final {
                return UnicodeResult{UnicodeError::OTHER, 0};
            }

            [[nodiscard]] size_t convert_valid_utf32_to_latin1(
                const char32_t*, size_t, char*) const noexcept final {
                return 0;
            }

            [[nodiscard]] size_t convert_utf32_to_utf8(
                const char32_t*, size_t, char*) const noexcept final {
                return 0;
            }

            [[nodiscard]] UnicodeResult convert_utf32_to_utf8_with_errors(
                const char32_t*, size_t, char*) const noexcept final {
                return UnicodeResult{UnicodeError::OTHER, 0};
            }

            [[nodiscard]] size_t convert_valid_utf32_to_utf8(
                const char32_t*, size_t, char*) const noexcept final {
                return 0;
            }

            [[nodiscard]] size_t convert_utf32_to_utf16le(
                const char32_t*, size_t, char16_t*) const noexcept final {
                return 0;
            }

            [[nodiscard]] size_t convert_utf32_to_utf16be(
                const char32_t*, size_t, char16_t*) const noexcept final {
                return 0;
            }

            [[nodiscard]] UnicodeResult convert_utf32_to_utf16le_with_errors(
                const char32_t*, size_t, char16_t*) const noexcept final {
                return UnicodeResult{UnicodeError::OTHER, 0};
            }

            [[nodiscard]] UnicodeResult convert_utf32_to_utf16be_with_errors(
                const char32_t*, size_t, char16_t*) const noexcept final {
                return UnicodeResult{UnicodeError::OTHER, 0};
            }

            [[nodiscard]] size_t convert_valid_utf32_to_utf16le(
                const char32_t*, size_t, char16_t*) const noexcept final {
                return 0;
            }

            [[nodiscard]] size_t convert_valid_utf32_to_utf16be(
                const char32_t*, size_t, char16_t*) const noexcept final {
                return 0;
            }

            [[nodiscard]] size_t convert_utf16le_to_utf32(
                const char16_t*, size_t, char32_t*) const noexcept final {
                return 0;
            }

            [[nodiscard]] size_t convert_utf16be_to_utf32(
                const char16_t*, size_t, char32_t*) const noexcept final {
                return 0;
            }

            [[nodiscard]] UnicodeResult convert_utf16le_to_utf32_with_errors(
                const char16_t*, size_t, char32_t*) const noexcept final {
                return UnicodeResult{UnicodeError::OTHER, 0};
            }

            [[nodiscard]] UnicodeResult convert_utf16be_to_utf32_with_errors(
                const char16_t*, size_t, char32_t*) const noexcept final {
                return UnicodeResult{UnicodeError::OTHER, 0};
            }

            [[nodiscard]] size_t convert_valid_utf16le_to_utf32(
                const char16_t*, size_t, char32_t*) const noexcept final {
                return 0;
            }

            [[nodiscard]] size_t convert_valid_utf16be_to_utf32(
                const char16_t*, size_t, char32_t*) const noexcept final {
                return 0;
            }

            void change_endianness_utf16(const char16_t*, size_t,
                char16_t*) const noexcept final { }

            [[nodiscard]] size_t
            count_utf16le(const char16_t*, size_t) const noexcept final {
                return 0;
            }

            [[nodiscard]] size_t
            count_utf16be(const char16_t*, size_t) const noexcept final {
                return 0;
            }

            [[nodiscard]] size_t count_utf8(const char*,
                size_t) const noexcept final {
                return 0;
            }

            [[nodiscard]] size_t
            latin1_length_from_utf8(const char*, size_t) const noexcept override {
                return 0;
            }

            [[nodiscard]] size_t
            utf8_length_from_latin1(const char*, size_t) const noexcept override {
                return 0;
            }

            [[nodiscard]] size_t
            utf8_length_from_utf16le(const char16_t*, size_t) const noexcept override {
                return 0;
            }

            [[nodiscard]] size_t
            utf8_length_from_utf16be(const char16_t*, size_t) const noexcept override {
                return 0;
            }

            [[nodiscard]] size_t
            utf32_length_from_utf16le(const char16_t*, size_t) const noexcept override {
                return 0;
            }

            [[nodiscard]] size_t
            utf32_length_from_utf16be(const char16_t*, size_t) const noexcept override {
                return 0;
            }

            [[nodiscard]] size_t
            utf16_length_from_utf8(const char*, size_t) const noexcept override {
                return 0;
            }

            [[nodiscard]] size_t
            utf8_length_from_utf32(const char32_t*, size_t) const noexcept override {
                return 0;
            }

            [[nodiscard]] size_t
            utf16_length_from_utf32(const char32_t*, size_t) const noexcept override {
                return 0;
            }

            [[nodiscard]] size_t
            utf32_length_from_utf8(const char*, size_t) const noexcept override {
                return 0;
            }

            [[nodiscard]] UnicodeResult
            base64_to_binary(const char*, size_t, char*, Base64Options,
                last_chunk_handling_options) const noexcept override {
                return UnicodeResult{UnicodeError::OTHER, 0};
            }

            [[nodiscard]] full_result base64_to_binary_details(
                const char*, size_t, char*, Base64Options,
                last_chunk_handling_options) const noexcept override {
                return full_result(UnicodeError::OTHER, 0, 0);
            }

            [[nodiscard]] UnicodeResult
            base64_to_binary(const char16_t*, size_t, char*, Base64Options,
                last_chunk_handling_options) const noexcept override {
                return UnicodeResult{UnicodeError::OTHER, 0};
            }

            [[nodiscard]] full_result base64_to_binary_details(
                const char16_t*, size_t, char*, Base64Options,
                last_chunk_handling_options) const noexcept override {
                return full_result(UnicodeError::OTHER, 0, 0);
            }

            size_t binary_to_base64(const char*, size_t, char*,
                Base64Options) const noexcept override {
                return 0;
            }
            size_t binary_to_base64_with_lines(const char*, size_t, char*, size_t,
                Base64Options) const noexcept override {
                return 0;
            }
            const char* find(const char*, const char*, char) const noexcept override {
                return nullptr;
            }
            const char16_t* find(const char16_t*, const char16_t*,
                char16_t) const noexcept override {
                return nullptr;
            }
            [[nodiscard]] size_t
            binary_length_from_base64(const char*, size_t) const noexcept override {
                return 0;
            }
            [[nodiscard]] size_t
            binary_length_from_base64(const char16_t*, size_t) const noexcept override {
                return 0;
            }

            unsupported_implementation()
                : implementation("unsupported",
                      "Unsupported CPU (no detected SIMD instructions)", 0) { }
        };

        const unsupported_implementation* get_unsupported_singleton() {
            static const unsupported_implementation unsupported_singleton { };
            return &unsupported_singleton;
        }
        static_assert(std::is_trivially_destructible<unsupported_implementation>::value,
            "unsupported_singleton should be trivially destructible");

        size_t AvailableImplementationList::size() const noexcept {
            return internal::get_available_implementation_pointers().size();
        }
        const implementation* const*
        AvailableImplementationList::begin() const noexcept {
            return internal::get_available_implementation_pointers().begin();
        }
        const implementation* const*
        AvailableImplementationList::end() const noexcept {
            return internal::get_available_implementation_pointers().end();
        }
        const implementation*
        AvailableImplementationList::detect_best_supported() const noexcept {
            // They are prelisted in priority order, so we just go down the list
            uint32_t supported_instruction_sets = internal::detect_supported_architectures();
            for (const implementation* impl :
                internal::get_available_implementation_pointers()) {
                uint32_t required_instruction_sets = impl->required_instruction_sets();
                if ((supported_instruction_sets & required_instruction_sets) == required_instruction_sets) {
                    return impl;
                }
            }
            return get_unsupported_singleton(); // this should never happen?
        }

        const implementation*
        detect_best_supported_implementation_on_first_use::set_best() const noexcept {
            KUMO_DISABLE_DEPRECATED_WARNINGS // Disable CRT_SECURE warning on MSVC:
                                             // manually verified this is safe
                char* force_implementation_name
                = getenv("UNICODE_FORCE_IMPLEMENTATION");
            KUMO_RESTORE_DEPRECATED_WARNINGS

            if (force_implementation_name) {
                auto force_implementation = get_available_implementations()[force_implementation_name];
                if (force_implementation) {
                    return get_active_implementation() = force_implementation;
                } else {
                    // Note: abort() and stderr usage within the library is forbidden.
                    return get_active_implementation() = get_unsupported_singleton();
                }
            }
            return get_active_implementation() = get_available_implementations().detect_best_supported();
        }

    } // namespace internal

    /// The list of available implementations compiled into simdutf.
    KUMO_DLL const internal::AvailableImplementationList&
    get_available_implementations() {
        static const internal::AvailableImplementationList
            available_implementations_instance { };
        return available_implementations_instance;
    }

    /// The active implementation.
    KUMO_DLL internal::atomic_ptr<const implementation>&
    get_active_implementation() {
#if !UNICODE_SINGLE_IMPLEMENTATION
        static const internal::detect_best_supported_implementation_on_first_use
            detect_best_supported_implementation_on_first_use_singleton;
#endif
        static internal::atomic_ptr<const implementation>
            active_implementation_instance {
#if UNICODE_SINGLE_IMPLEMENTATION
                internal::get_single_implementation()
#else
                &detect_best_supported_implementation_on_first_use_singleton
#endif
            };
        return active_implementation_instance;
    }

#if UNICODE_SINGLE_IMPLEMENTATION
    const implementation* get_default_implementation() {
        return internal::get_single_implementation();
    }
#else
    internal::atomic_ptr<const implementation>&
    get_default_implementation() {
        return get_active_implementation();
    }
#endif
#define UNICODE_GET_CURRENT_IMPLEMENTATION

    const implementation* builtin_implementation() {
        static const implementation* const builtin_impl_instance = get_available_implementations()[KUMO_STRINGIFY(
            UNICODE_BUILTIN_IMPLEMENTATION)];
        return builtin_impl_instance;
    }

} // namespace turbo
