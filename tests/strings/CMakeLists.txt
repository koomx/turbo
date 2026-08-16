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

set(STRINGS_TESTS
        ascii_test
        atod_manual_test
        char_formatting_test
        charconv_test
        charset_test
        escaping_test
        has_absl_stringify_test
        has_ostream_operator_test
        match_test
        numbers_test
        str_cat_test
        str_join_test
        str_replace_test
        str_split_test
        string_view_test
        strip_test
        substitute_test
)

foreach (case IN LISTS STRINGS_TESTS)
    kmcmake_cc_test(
            NAME ${case}
            MODULE strings
            SOURCES
            ${case}.cc
            CXXOPTS
            ${KMCMAKE_CXX_OPTIONS}
            LINKS
            GTest::gtest
            GTest::gmock
            GTest::gtest_main
            turbo::turbo_static
            ${KMCMAKE_DEPS_LINK}
    )
endforeach ()

add_subdirectory(internal)
