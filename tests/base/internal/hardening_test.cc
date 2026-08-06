// Copyright 2026 The Abseil Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <turbo/base/internal/hardening.h>

#include <vector>

#include <gtest/gtest.h>
#include <turbo/macros/config.h>

namespace {

bool IsHardened() {
  bool hardened = false;
  KUMO_HARDENING_ASSERT([&hardened]() {
      hardened = true;
      return true;
    }()
  );
  return hardened;
}

bool IsHardenedSlow() {
  bool hardened = false;
  KUMO_HARDENING_ASSERT_SLOW([&hardened]() {
      hardened = true;
      return true;
    }()
  );
  return hardened;
}

class HardeningTest : public testing::Test {
 public:
  ~HardeningTest() override {
    turbo::base_internal::SetTurboHardeningEnabled(true);
  }
};

class HardeningDeathTest : public testing::Test {
 public:
  ~HardeningDeathTest() override {
    turbo::base_internal::SetTurboHardeningEnabled(true);
  }
};

TEST_F(HardeningTest, HardeningAssertSlow) {
  turbo::base_internal::HardeningAssertSlow(true);
  if (!IsHardenedSlow()) {
    turbo::base_internal::HardeningAssertSlow(false);
  }
}

TEST_F(HardeningDeathTest, HardeningAssertSlow) {
#if GTEST_HAS_DEATH_TEST
  if (IsHardenedSlow()) {
    // The underlying mechanism of termination varies, and may include SIGILL
    // or SIGABRT.
    EXPECT_DEATH(turbo::base_internal::HardeningAssertSlow(false), "");
  }
#endif
}

TEST_F(HardeningTest, HardeningAssertGT) {
  turbo::base_internal::HardeningAssertGT(1, 0);
}

TEST_F(HardeningDeathTest, HardeningAssertGT) {
#if GTEST_HAS_DEATH_TEST
  if (IsHardened()) {
    // The underlying mechanism of termination varies, and may include SIGILL
    // or SIGABRT.
    turbo::base_internal::SetTurboHardeningEnabled(true);
    EXPECT_DEATH(turbo::base_internal::HardeningAssertGT(1, 1), "");
    EXPECT_DEATH(turbo::base_internal::HardeningAssertGT(0, 1), "");
  }
#endif
}

TEST_F(HardeningTest, HardeningAssertGE) {
  turbo::base_internal::HardeningAssertGE(1, 0);
  turbo::base_internal::HardeningAssertGE(1, 1);
}

TEST_F(HardeningDeathTest, HardeningAssertGE) {
#if GTEST_HAS_DEATH_TEST
  if (IsHardened()) {
    // The underlying mechanism of termination varies, and may include SIGILL
    // or SIGABRT.
    turbo::base_internal::SetTurboHardeningEnabled(true);
    EXPECT_DEATH(turbo::base_internal::HardeningAssertGE(0, 1), "");
  }
#endif
}

TEST_F(HardeningTest, HardeningAssertLT) {
  turbo::base_internal::HardeningAssertLT(0, 1);
}

TEST_F(HardeningDeathTest, HardeningAssertLT) {
#if GTEST_HAS_DEATH_TEST
  if (IsHardened()) {
    // The underlying mechanism of termination varies, and may include SIGILL
    // or SIGABRT.
    turbo::base_internal::SetTurboHardeningEnabled(true);
    EXPECT_DEATH(turbo::base_internal::HardeningAssertLT(1, 1), "");
    EXPECT_DEATH(turbo::base_internal::HardeningAssertLT(1, 0), "");
  }
#endif
}

TEST_F(HardeningTest, HardeningAssertLE) {
  turbo::base_internal::HardeningAssertLE(0, 1);
  turbo::base_internal::HardeningAssertLE(1, 1);
}

TEST_F(HardeningDeathTest, HardeningAssertLE) {
#if GTEST_HAS_DEATH_TEST
  if (IsHardened()) {
    // The underlying mechanism of termination varies, and may include SIGILL
    // or SIGABRT.
    turbo::base_internal::SetTurboHardeningEnabled(true);
    EXPECT_DEATH(turbo::base_internal::HardeningAssertLE(1, 0), "");
  }
#endif
}

TEST_F(HardeningTest, HardeningAssertInBounds) {
  turbo::base_internal::HardeningAssertInBounds(0, 10);
}

TEST_F(HardeningDeathTest, HardeningAssertInBounds) {
#if GTEST_HAS_DEATH_TEST
  if (IsHardened()) {
    // The underlying mechanism of termination varies, and may include SIGILL
    // or SIGABRT.
    turbo::base_internal::SetTurboHardeningEnabled(true);
    EXPECT_DEATH(turbo::base_internal::HardeningAssertInBounds(10, 10), "");
  }
#endif
}

TEST_F(HardeningTest, HardeningAssertNonEmpty) {
  std::vector<int> v = {1};
  turbo::base_internal::HardeningAssertNonEmpty(v);
}

TEST_F(HardeningDeathTest, HardeningAssertNonEmpty) {
#if GTEST_HAS_DEATH_TEST
  if (IsHardened()) {
    // The underlying mechanism of termination varies, and may include SIGILL
    // or SIGABRT.
    std::vector<int> v = {};
    turbo::base_internal::SetTurboHardeningEnabled(true);
    EXPECT_DEATH(turbo::base_internal::HardeningAssertNonEmpty(v), "");
  }
#endif
}

TEST_F(HardeningTest, HardeningAssertNonNull) {
  int x = 1;
  turbo::base_internal::HardeningAssertNonNull(&x);
}

TEST_F(HardeningDeathTest, HardeningAssertNonNull) {
#if GTEST_HAS_DEATH_TEST
  if (IsHardened()) {
    // The underlying mechanism of termination varies, and may include SIGILL
    // or SIGABRT.
    int *x = nullptr;
    turbo::base_internal::SetTurboHardeningEnabled(true);
    EXPECT_DEATH(turbo::base_internal::HardeningAssertNonNull(x), "");
  }
#endif
}

}  // namespace
