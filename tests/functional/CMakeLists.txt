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

set(FUNCTIONAL_TESTS
        any_invocable_test_inst1
        function_ref_test
        any_invocable_test_inst2
        bind_front_test
        bind_back_test
        any_invocable_test_inst3
        any_invocable_test_inst4
        overload_test
)

foreach (case IN LISTS FUNCTIONAL_TESTS)
    kmcmake_cc_test(
            NAME ${case}
            MODULE functional
            SOURCES
            ${case}.cc
            CXXOPTS
            ${KMCMAKE_CXX_OPTIONS}
            LINKS
            GTest::gtest
            GTest::gmock
            GTest::gtest_main
            turbo::test_utils_static
            turbo::turbo_static
            ${KMCMAKE_DEPS_LINK}
    )
endforeach ()