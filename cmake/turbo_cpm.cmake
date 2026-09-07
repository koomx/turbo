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
# ------------------------------------------------------------------------------
# CPM dependency list (loaded only when KMCMAKE_USE_CPM=ON)
# ------------------------------------------------------------------------------
# CPM.cmake is vendored at kmcmake/tools/CPM.cmake (official release, unmodified).
# Enable with: -DKMCMAKE_USE_CPM=ON  or set(KMCMAKE_USE_CPM ON) in *_user_option.cmake
#
# Test/benchmark-only packages — do not list(APPEND KMCMAKE_DEPS_LINK ...).
#
# See docs/AI.md (CPM section) and https://github.com/cpm-cmake/CPM.cmake
# ------------------------------------------------------------------------------

include(CPM)

if (KMCMAKE_BUILD_TEST)
    CPMAddPackage(
            NAME googletest
            GITHUB_REPOSITORY google/googletest
            VERSION 1.17.0
            GIT_TAG v1.17.0
            OPTIONS
            "INSTALL_GTEST OFF"
            "BUILD_GMOCK ON"
            "gtest_force_shared_crt ON"
    )
    CPMAddPackage(
            NAME simdjson
            GITHUB_REPOSITORY simdjson/simdjson
            VERSION 3.12.3
            GIT_TAG v3.12.3
            OPTIONS
            "SIMDJSON_JUST_LIBRARY ON"
            "SIMDJSON_BUILD_STATIC ON"
            "SIMDJSON_ENABLE_THREADS OFF"
    )
endif ()

if (KMCMAKE_BUILD_BENCHMARK)
    CPMAddPackage(
            NAME benchmark
            GITHUB_REPOSITORY google/benchmark
            VERSION 1.9.5
            GIT_TAG v1.9.5
            OPTIONS
            "BENCHMARK_ENABLE_TESTING OFF"
            "BENCHMARK_ENABLE_GTEST_TESTS OFF"
            "BENCHMARK_ENABLE_INSTALL OFF"
    )
endif ()
