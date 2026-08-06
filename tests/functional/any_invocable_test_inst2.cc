// Copyright 2022 The Abseil Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// To prevent compiler memory exhaustion (OOM / Killed signal terminates
// cc1plus) during parallel builds with GCC, the test suite instantiations have
// been split into multiple compilation units.

// SKIP_TURBO_INLINE_NAMESPACE_CHECK

#include <tests/functional/any_invocable_test.h>

namespace turbo_any_invocable_test {

// Suites are REGISTER'd in the shared header but only some are INSTANTIATE'd
// in each binary; tell gtest that is intentional.
GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(AnyInvTestBasic);
GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(AnyInvTestCombinatoric);
GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(AnyInvTestMovable);
GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(AnyInvTestNoexceptFalse);
GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(AnyInvTestNoexceptTrue);
GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(AnyInvTestNonRvalue);
GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(AnyInvTestRvalue);

INSTANTIATE_TYPED_TEST_SUITE_P(RemoteMovable, AnyInvTestBasic,
                               TestParameterListRemoteMovable);

INSTANTIATE_TYPED_TEST_SUITE_P(RemoteMovable, AnyInvTestCombinatoric,
                               TestParameterListRemoteMovable);

INSTANTIATE_TYPED_TEST_SUITE_P(RemoteMovable, AnyInvTestMovable,
                               TestParameterListRemoteMovable);

INSTANTIATE_TYPED_TEST_SUITE_P(RemoteMovable, AnyInvTestNoexceptFalse,
                               TestParameterListRemoteMovable);

INSTANTIATE_TYPED_TEST_SUITE_P(RemoteMovable, AnyInvTestNonRvalue,
                               TestParameterListRemoteMovable);

}  // namespace turbo_any_invocable_test
