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

set(HASH_XX_TESTS
        xxhash32_test
        xxhash64_test
        xxh3_64_test
        xxh3_64_secret_test
        xxh3_128_test
        xxh3_128_secret_test
        xxhash_generate_secret_test
)

foreach (case IN LISTS HASH_XX_TESTS)
    kmcmake_cc_test(
            NAME ${case}
            MODULE hash_xx
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
