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

kmcmake_cc_library(
        NAMESPACE ${PROJECT_NAME}
        NAME cctz_testing
        SOURCES
        test_time_zone_names.cc
        CXXOPTS
        ${KMCMAKE_CXX_OPTIONS}
        PLINKS
        GTest::gtest
        GTest::gmock
        turbo::turbo_static
        ${KMCMAKE_DEPS_LINK}
)

set(CCTZ_TESTS
        civil_time_test
        time_zone_format_test
        time_zone_lookup_test
        time_zone_name_win_test
)

foreach (case IN LISTS CCTZ_TESTS)
    kmcmake_cc_test(
            NAME ${case}
            MODULE cctz
            SOURCES
            ${case}.cc
            CXXOPTS
            ${KMCMAKE_CXX_OPTIONS}
            LINKS
            GTest::gtest
            GTest::gmock
            GTest::gtest_main
            turbo::cctz_testing_static
            turbo::turbo_static
            ${KMCMAKE_DEPS_LINK}
    )
endforeach ()

# Windows (and any host without system tzdata) needs bundled zoneinfo.
set(_TURBO_TEST_TZDIR "${CMAKE_CURRENT_SOURCE_DIR}/testdata/zoneinfo")
foreach (case IN LISTS CCTZ_TESTS)
    set_tests_properties(cctz_${case} PROPERTIES
            ENVIRONMENT "TZDIR=${_TURBO_TEST_TZDIR}")
endforeach ()
