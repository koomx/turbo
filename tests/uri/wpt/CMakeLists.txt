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

set(URI_WPT_TESTS
        url_components_test
        url_search_params_test
        basic_test
        from_file_test
        wpt_test
)

foreach (case IN LISTS URI_WPT_TESTS)
    kmcmake_cc_test(
            NAME ${case}
            MODULE uri_wpt
            SOURCES
            ${case}.cc
            CXXOPTS
            ${KMCMAKE_CXX_OPTIONS}
            DEFINES
            WPT_DATA_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/wpt/\"
            LINKS
            GTest::gtest
            GTest::gmock
            GTest::gtest_main
            simdjson::simdjson
            turbo::test_utils_static
            turbo::base_testing_static
            turbo::turbo_static
            ${KMCMAKE_DEPS_LINK}
    )
endforeach ()

kmcmake_cc_test(
        NAME basic_fuzzer
        MODULE uri_wpt
        SOURCES
        basic_fuzzer.cc
        CXXOPTS
        ${KMCMAKE_CXX_OPTIONS}
        LINKS
        turbo::turbo_static
        ${KMCMAKE_DEPS_LINK}
)
