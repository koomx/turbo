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
        NAME log_testing
        SOURCES
        internal/test_actions.cc
        internal/test_helpers.cc
        internal/test_matchers.cc
        scoped_mock_log.cc
        CXXOPTS
        ${KMCMAKE_CXX_OPTIONS}
        PLINKS
        GTest::gtest
        GTest::gmock
        turbo::turbo_static
        ${KMCMAKE_DEPS_LINK}
)

set(LOG_TESTS
        structured_test
        turbo_log_basic_test
        vlog_is_on_test
        log_format_test
        log_modifier_methods_test
        stripping_test
        log_streamer_test
        scoped_mock_log_test
        turbo_check_test
        log_macro_hygiene_test
        check_test
        die_if_null_test
        globals_test
        scoped_mock_log
        log_basic_test
        log_sink_test
        log_entry_test
        min_log_level_severity_test
        zlog_test
)

foreach (case IN LISTS LOG_TESTS)
    kmcmake_cc_test(
            NAME ${case}
            MODULE log
            SOURCES
            ${case}.cc
            CXXOPTS
            ${KMCMAKE_CXX_OPTIONS}
            LINKS
            GTest::gtest
            GTest::gmock
            GTest::gtest_main
            turbo::log_testing_static
            turbo::turbo_static
            ${KMCMAKE_DEPS_LINK}
    )
endforeach ()
add_subdirectory(internal)
add_subdirectory(sinks)