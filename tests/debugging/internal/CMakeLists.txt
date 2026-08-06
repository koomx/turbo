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

set(DBG_INTERNAL_TESTS
        bounded_utf8_length_sequence_test
        decode_rust_punycode_test
        demangle_rust_test
        demangle_test
        stack_consumption_test
        utf8_for_code_point_test
)

foreach (case IN LISTS DBG_INTERNAL_TESTS)
    kmcmake_cc_test(
            NAME ${case}
            MODULE dbg_internal
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