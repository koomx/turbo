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

set(LOG_SINKS_TESTS
        null_sink_test
        log_filename_test
        ansi_color_sink_test
        daily_file_sink_test
        hourly_file_sink_test
        rotating_file_sink_test
        sink_factory_test
        auto_log_sink_test
)

foreach (case IN LISTS LOG_SINKS_TESTS)
    kmcmake_cc_test(
            NAME ${case}
            MODULE log_sinks
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
