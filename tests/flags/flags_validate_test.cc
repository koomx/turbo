// Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <atomic>
#include <string>
#include <string_view>

#include <gtest/gtest.h>
#include <turbo/flags/commandlineflag.h>
#include <turbo/flags/flag.h>
#include <turbo/flags/internal/private_handle_accessor.h>
#include <turbo/flags/reflection.h>
#include <turbo/flags/validator.h>

namespace {

std::atomic<int> validate_update_count{0};

}  // namespace

TURBO_FLAG(int, test_validate_flag, 0, "test validate flag")
    .on_validate([](const int& val, std::string* error) noexcept {
      if (val < 0 || val > 100) {
        if (error) {
          *error = "value must be in [0, 100]";
        }
        return false;
      }
      return true;
    })
    .on_update([]() { validate_update_count.fetch_add(1); });

TURBO_FLAG(int, test_ge_flag, 10, "ge validator flag")
    .on_validate(turbo::GeValidator<int, 5>::validate);

TEST(FlagsValidateTest, HasUserValidator) {
  auto* cl = turbo::FindCommandLineFlag("test_validate_flag");
  ASSERT_NE(cl, nullptr);
  EXPECT_TRUE(cl->has_user_validator());

  auto* plain = turbo::FindCommandLineFlag("test_ge_flag");
  ASSERT_NE(plain, nullptr);
  EXPECT_TRUE(plain->has_user_validator());
}

TEST(FlagsValidateTest, ValidateInputValueBounds) {
  auto* cl = turbo::FindCommandLineFlag("test_validate_flag");
  ASSERT_NE(cl, nullptr);

  EXPECT_TRUE(turbo::flags_internal::PrivateHandleAccessor::ValidateInputValue(
      *cl, "0"));
  EXPECT_TRUE(turbo::flags_internal::PrivateHandleAccessor::ValidateInputValue(
      *cl, "100"));
  EXPECT_FALSE(turbo::flags_internal::PrivateHandleAccessor::ValidateInputValue(
      *cl, "-1"));
  EXPECT_FALSE(turbo::flags_internal::PrivateHandleAccessor::ValidateInputValue(
      *cl, "101"));
}

TEST(FlagsValidateTest, ParseFromRejectsInvalid) {
  ASSERT_TRUE(turbo::SetFlag(&FLAGS_test_validate_flag, 42));
  validate_update_count.store(0);

  auto* cl = turbo::FindCommandLineFlag("test_validate_flag");
  ASSERT_NE(cl, nullptr);

  std::string error;
  EXPECT_FALSE(cl->ParseFrom("101", &error));
  EXPECT_EQ(turbo::GetFlag(FLAGS_test_validate_flag), 42);
  EXPECT_EQ(validate_update_count.load(), 0);

  EXPECT_TRUE(cl->ParseFrom("50", &error)) << error;
  EXPECT_EQ(turbo::GetFlag(FLAGS_test_validate_flag), 50);
  EXPECT_GE(validate_update_count.load(), 1);
}

TEST(FlagsValidateTest, SetFlagRejectsInvalid) {
  ASSERT_TRUE(turbo::SetFlag(&FLAGS_test_validate_flag, 42));
  validate_update_count.store(0);

  std::string error;
  EXPECT_FALSE(turbo::SetFlag(&FLAGS_test_validate_flag, 101, &error));
  EXPECT_EQ(turbo::GetFlag(FLAGS_test_validate_flag), 42);
  EXPECT_FALSE(error.empty());
  EXPECT_EQ(validate_update_count.load(), 0);

  // No error out-param still rejects.
  EXPECT_FALSE(turbo::SetFlag(&FLAGS_test_validate_flag, -1));
  EXPECT_EQ(turbo::GetFlag(FLAGS_test_validate_flag), 42);

  EXPECT_TRUE(turbo::SetFlag(&FLAGS_test_validate_flag, 7));
  EXPECT_EQ(turbo::GetFlag(FLAGS_test_validate_flag), 7);
  EXPECT_GE(validate_update_count.load(), 1);
}

TEST(FlagsValidateTest, GeValidatorHelper) {
  auto* cl = turbo::FindCommandLineFlag("test_ge_flag");
  ASSERT_NE(cl, nullptr);

  EXPECT_FALSE(turbo::flags_internal::PrivateHandleAccessor::ValidateInputValue(
      *cl, "4"));
  EXPECT_TRUE(turbo::flags_internal::PrivateHandleAccessor::ValidateInputValue(
      *cl, "5"));
  EXPECT_TRUE(turbo::flags_internal::PrivateHandleAccessor::ValidateInputValue(
      *cl, "6"));

  ASSERT_TRUE(turbo::SetFlag(&FLAGS_test_ge_flag, 10));
  std::string error;
  EXPECT_FALSE(cl->ParseFrom("4", &error));
  EXPECT_EQ(turbo::GetFlag(FLAGS_test_ge_flag), 10);
  EXPECT_TRUE(cl->ParseFrom("7", &error)) << error;
  EXPECT_EQ(turbo::GetFlag(FLAGS_test_ge_flag), 7);

  EXPECT_FALSE(turbo::SetFlag(&FLAGS_test_ge_flag, 4, &error));
  EXPECT_EQ(turbo::GetFlag(FLAGS_test_ge_flag), 7);
}
