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

set(BASE_INT_TESTS
        atomic_hook_test
        cmake_thread_test
        cpu_detect_test
        errno_saver_test
        hardening_test
        low_level_alloc_test
        nullability_traits_test
        poison_test
        scoped_set_env_test
        strerror_test
        sysinfo_test
        thread_identity_test
        tracing_strong_test
        tracing_weak_test
        unique_small_name_test
)

foreach (case IN LISTS BASE_INT_TESTS)
    kmcmake_cc_test(
            NAME ${case}
            MODULE base_internal
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