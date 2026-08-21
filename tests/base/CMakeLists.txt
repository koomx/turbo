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
        NAME base_testing
        SOURCES
        internal/atomic_hook_test_helper.cc
        internal/exception_safety_testing.cc
        CXXOPTS
        ${KMCMAKE_CXX_OPTIONS}
        PLINKS
        GTest::gtest
        GTest::gmock
        turbo::turbo_static
        ${KMCMAKE_DEPS_LINK}
)

set(BASE_TESTS
        spinlock_test_common
        call_once_test
        fast_type_id_test
        throw_delegate_test
        log_severity_test
        raw_logging_test
        resize_and_overwrite_test
        nullability_default_nonnull_test
        exception_safety_testing_test
        nullability_test
        no_destructor_test
)

foreach (case IN LISTS BASE_TESTS)
    kmcmake_cc_test(
            NAME ${case}
            MODULE base
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

add_subdirectory(internal)
