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
        NAME random_testing
        SOURCES
        internal/chi_square.cc
        internal/distribution_test_util.cc
        CXXOPTS
        ${KMCMAKE_CXX_OPTIONS}
        PLINKS
        GTest::gtest
        GTest::gmock
        turbo::turbo_static
        ${KMCMAKE_DEPS_LINK}
)

set(RANDOM_TESTS
        bernoulli_distribution_test
        beta_distribution_test
        bit_gen_ref_test
        discrete_distribution_test
        distributions_test
        examples_test
        exponential_distribution_test
        gaussian_distribution_test
        generators_test
        log_uniform_int_distribution_test
        mock_distributions_test
        mocking_bit_gen_test
        poisson_distribution_test
        seed_sequences_test
        uniform_int_distribution_test
        uniform_real_distribution_test
        zipf_distribution_test
)

foreach (case IN LISTS RANDOM_TESTS)
    kmcmake_cc_test(
            NAME ${case}
            MODULE random
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


add_subdirectory(internal)
