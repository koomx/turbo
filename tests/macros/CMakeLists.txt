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

set(MACROS_TESTS
        attributes_test
        optimization_test
        config_test
)

foreach (case IN LISTS MACROS_TESTS)
    kmcmake_cc_test(
            NAME ${case}
            MODULE macros
            SOURCES
            ${case}.cc
            CXXOPTS
            ${KMCMAKE_CXX_OPTIONS}
            LINKS
            GTest::gtest
            GTest::gmock
            GTest::gtest_main
            turbo::base_testing_static
            turbo::turbo_static
            ${KMCMAKE_DEPS_LINK}
    )
endforeach ()
kmcmake_cc_test(
        NAME c_header_test
        MODULE macros
        SOURCES
        c_header_test.c
        CXXOPTS
        ${KMCMAKE_CXX_OPTIONS}
        LINKS
        GTest::gtest
        GTest::gmock
        GTest::gtest_main
        turbo::turbo_static
        ${KMCMAKE_DEPS_LINK}
)
