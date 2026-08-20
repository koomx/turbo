# Copyright (C) Kumo inc. and its affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

add_subdirectory(reference)
add_subdirectory(helpers)

set(UNICODE_TESTS
        null_safety_tests
        readme_tests
        random_fuzzer
        special_tests
        validate_ascii_basic_tests
        validate_ascii_with_errors_tests
        bele_tests
        validate_utf8_basic_tests
        select_implementation
        validate_utf8_brute_force_tests
        validate_utf8_puzzler_tests
        validate_utf8_with_errors_tests
        validate_utf16le_basic_tests
        validate_utf16be_basic_tests
        validate_utf16le_with_errors_tests
        validate_utf16be_with_errors_tests
        validate_utf32_basic_tests
        validate_utf32_with_errors_tests
        find_tests
        convert_latin1_to_utf8_tests
        convert_latin1_to_utf16le_tests
        convert_latin1_to_utf16be_tests
        convert_latin1_to_utf32_tests
        convert_utf8_to_latin1_tests
        convert_utf8_to_latin1_with_errors_tests
        convert_valid_utf8_to_latin1_tests
        convert_valid_utf8_to_utf16le_tests
        convert_valid_utf8_to_utf16be_tests
        convert_valid_utf8_to_utf32_tests
        convert_utf8_to_utf16le_tests
        convert_utf8_to_utf16be_tests
        convert_utf8_to_utf16le_with_errors_tests
        convert_utf8_to_utf16be_with_errors_tests
        convert_utf8_to_utf32_tests
        convert_utf8_to_utf32_with_errors_tests
        convert_utf16le_to_latin1_tests
        convert_utf16be_to_latin1_tests
        convert_utf16le_to_latin1_tests_with_errors
        convert_utf16be_to_latin1_tests_with_errors
        convert_valid_utf16le_to_latin1_tests
        convert_valid_utf16be_to_latin1_tests
        convert_utf16_to_utf8_safe_tests
        convert_utf16le_to_utf8_tests
        convert_utf16be_to_utf8_tests
        convert_utf16le_to_utf8_with_errors_tests
        convert_utf16be_to_utf8_with_errors_tests
        convert_utf32_to_latin1_tests
        convert_valid_utf32_to_latin1_tests
        convert_utf32_to_latin1_with_errors_tests
        convert_utf32_to_utf8_tests
        convert_utf32_to_utf8_with_errors_tests
        convert_utf32_to_utf16le_tests
        convert_utf32_to_utf16be_tests
        convert_utf32_to_utf16le_with_errors_tests
        convert_utf32_to_utf16be_with_errors_tests
        convert_valid_utf16le_to_utf8_tests
        convert_valid_utf16be_to_utf8_tests
        convert_valid_utf32_to_utf8_tests
        convert_valid_utf32_to_utf16le_tests
        convert_valid_utf32_to_utf16be_tests
        convert_utf16le_to_utf32_tests
        convert_utf16be_to_utf32_tests
        convert_utf16le_to_utf32_with_errors_tests
        convert_utf16be_to_utf32_with_errors_tests
        convert_valid_utf16le_to_utf32_tests
        convert_valid_utf16be_to_utf32_tests
        count_utf8
        count_utf16le
        count_utf16be
        detect_encodings_tests
        text_encoding_tests
        basic_fuzzer
        base64_tests
        constexpr_base64_tests
        c_tests
        internal_tests
        utf8_length_from_utf16_tests
        to_well_formed_utf16_tests
        convert_utf16_to_utf8_with_replacement_tests
        fixed_string_tests
)

foreach (case IN LISTS UNICODE_TESTS)
    if (case STREQUAL "random_fuzzer")
        set(_extra_defines UNICODE_TEST_FUZZER_TRIALS=10000)
    elseif (case STREQUAL "count_utf8")
        set(_extra_defines UNICODE_FAST_TESTS=0)
    elseif (case STREQUAL "basic_fuzzer")
        set(_extra_defines UNICODE_TEST_FUZZER_TRIALS=100000)
    elseif (case STREQUAL "base64_tests")
        set(_extra_defines UNICODE_BASE64_TEST_MAXLEN=2048)
    else ()
        set(_extra_defines)
    endif ()

    if (_extra_defines)
        kmcmake_cc_test(
                NAME ${case}
                MODULE unicode
                SOURCES
                ${case}.cpp
                CXXOPTS
                ${KMCMAKE_CXX_OPTIONS}
                DEFINES
                ${_extra_defines}
                LINKS
                turbo::turbo_static
                turbo::unicode_test_helpers_static
                turbo::unicode_test_reference_static
                ${KMCMAKE_DEPS_LINK}
        )
    else ()
        kmcmake_cc_test(
                NAME ${case}
                MODULE unicode
                SOURCES
                ${case}.cpp
                CXXOPTS
                ${KMCMAKE_CXX_OPTIONS}
                LINKS
                turbo::turbo_static
                turbo::unicode_test_helpers_static
                turbo::unicode_test_reference_static
                ${KMCMAKE_DEPS_LINK}
        )
    endif ()
endforeach ()

kmcmake_cc_test(
        NAME straight_c_test
        MODULE unicode
        SOURCES
        straight_c_test.c
        CXXOPTS
        ${KMCMAKE_CXX_OPTIONS}
        LINKS
        turbo::turbo_static
        ${KMCMAKE_DEPS_LINK}
)
