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


set(RANDOM_INTERNAL_TESTS
        chi_square_test
        distribution_test_util_test
        entropy_pool_test
        explicit_seed_seq_test
        fast_uniform_bits_test
        fastmath_test
        generate_real_test
        iostream_state_saver_test
        nanobenchmark_test
        nonsecure_base_test
        pcg_engine_test
        randen_engine_test
        randen_hwaes_test
        randen_slow_test
        randen_test
        salted_seed_seq_test
        seed_material_test
        traits_test
        uniform_helper_test
        wide_multiply_test
)

foreach (case IN LISTS RANDOM_INTERNAL_TESTS)
    kmcmake_cc_test(
            NAME ${case}
            MODULE random_internal
            SOURCES
            ${case}.cc
            CXXOPTS
            ${KMCMAKE_CXX_OPTIONS}
            LINKS
            GTest::gtest
            GTest::gmock
            GTest::gtest_main
            turbo::random_testing_static
            turbo::turbo_static
            ${KMCMAKE_DEPS_LINK}
    )
endforeach ()
