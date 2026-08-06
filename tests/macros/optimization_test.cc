// Copyright 2020 The Abseil Authors.
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

#include <turbo/macros/macros.h>
#include <optional>
#include <gtest/gtest.h>

namespace {

// Tests for the KUMO_LIKELY and KUMO_UNLIKELY macros.
// The tests only verify that the macros are functionally correct - i.e. code
// behaves as if they weren't used. They don't try to check their impact on
// optimization.

TEST(PredictTest, PredictTrue) {
  EXPECT_TRUE(KUMO_LIKELY(true));
  EXPECT_FALSE(KUMO_LIKELY(false));
  EXPECT_TRUE(KUMO_LIKELY(1 == 1));
  EXPECT_FALSE(KUMO_LIKELY(1 == 2));

  if (KUMO_LIKELY(false)) ADD_FAILURE();
  if (!KUMO_LIKELY(true)) ADD_FAILURE();

  EXPECT_TRUE(KUMO_LIKELY(true) && true);
  EXPECT_TRUE(KUMO_LIKELY(true) || false);
}

TEST(PredictTest, PredictFalse) {
  EXPECT_TRUE(KUMO_UNLIKELY(true));
  EXPECT_FALSE(KUMO_UNLIKELY(false));
  EXPECT_TRUE(KUMO_UNLIKELY(1 == 1));
  EXPECT_FALSE(KUMO_UNLIKELY(1 == 2));

  if (KUMO_UNLIKELY(false)) ADD_FAILURE();
  if (!KUMO_UNLIKELY(true)) ADD_FAILURE();

  EXPECT_TRUE(KUMO_UNLIKELY(true) && true);
  EXPECT_TRUE(KUMO_UNLIKELY(true) || false);
}

TEST(PredictTest, OneEvaluation) {
  // Verify that the expression is only evaluated once.
  int x = 0;
  if (KUMO_LIKELY((++x) == 0)) ADD_FAILURE();
  EXPECT_EQ(x, 1);
  if (KUMO_UNLIKELY((++x) == 0)) ADD_FAILURE();
  EXPECT_EQ(x, 2);
}

TEST(PredictTest, OperatorOrder) {
  // Verify that operator order inside and outside the macro behaves well.
  // These would fail for a naive '#define KUMO_LIKELY(x) x'
  EXPECT_TRUE(KUMO_LIKELY(1 && 2) == true);
  EXPECT_TRUE(KUMO_UNLIKELY(1 && 2) == true);
  EXPECT_TRUE(!KUMO_LIKELY(1 == 2));
  EXPECT_TRUE(!KUMO_UNLIKELY(1 == 2));
}

TEST(PredictTest, Pointer) {
  const int x = 3;
  const int *good_intptr = &x;
  const int *null_intptr = nullptr;
  EXPECT_TRUE(KUMO_LIKELY(good_intptr));
  EXPECT_FALSE(KUMO_LIKELY(null_intptr));
  EXPECT_TRUE(KUMO_UNLIKELY(good_intptr));
  EXPECT_FALSE(KUMO_UNLIKELY(null_intptr));
}

TEST(PredictTest, Optional) {
  // Note: An optional's truth value is the value's existence, not its truth.
  std::optional<bool> has_value(false);
  std::optional<bool> no_value;
  EXPECT_TRUE(KUMO_LIKELY(has_value));
  EXPECT_FALSE(KUMO_LIKELY(no_value));
  EXPECT_TRUE(KUMO_UNLIKELY(has_value));
  EXPECT_FALSE(KUMO_UNLIKELY(no_value));
}

class ImplicitlyConvertibleToBool {
 public:
  explicit ImplicitlyConvertibleToBool(bool value) : value_(value) {}
  operator bool() const {  // NOLINT(google-explicit-constructor)
    return value_;
  }

 private:
  bool value_;
};

TEST(PredictTest, ImplicitBoolConversion) {
  const ImplicitlyConvertibleToBool is_true(true);
  const ImplicitlyConvertibleToBool is_false(false);
  if (!KUMO_LIKELY(is_true)) ADD_FAILURE();
  if (KUMO_LIKELY(is_false)) ADD_FAILURE();
  if (!KUMO_UNLIKELY(is_true)) ADD_FAILURE();
  if (KUMO_UNLIKELY(is_false)) ADD_FAILURE();
}

class ExplicitlyConvertibleToBool {
 public:
  explicit ExplicitlyConvertibleToBool(bool value) : value_(value) {}
  explicit operator bool() const { return value_; }

 private:
  bool value_;
};

TEST(PredictTest, ExplicitBoolConversion) {
  const ExplicitlyConvertibleToBool is_true(true);
  const ExplicitlyConvertibleToBool is_false(false);
  if (!KUMO_LIKELY(is_true)) ADD_FAILURE();
  if (KUMO_LIKELY(is_false)) ADD_FAILURE();
  if (!KUMO_UNLIKELY(is_true)) ADD_FAILURE();
  if (KUMO_UNLIKELY(is_false)) ADD_FAILURE();
}

// This verifies that KUMO_ASSUME compiles in a variety of contexts.
// It does not test optimization.
TEST(TurboAssume, Compiles) {
  int x = 0;
  KUMO_ASSUME(x >= 0);
  EXPECT_EQ(x, 0);

  // https://github.com/abseil/abseil-cpp/issues/1814
  KUMO_ASSUME(x >= 0), (x >= 0) ? ++x : --x;
  EXPECT_EQ(x, 1);
}

}  // namespace
