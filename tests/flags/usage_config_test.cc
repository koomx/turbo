//
//  Copyright 2019 The Abseil Authors.
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

#include <turbo/flags/usage_config.h>

#include <string>
#include <string_view>

#include <gtest/gtest.h>

namespace {

class FlagsUsageConfigTest : public testing::Test {
 protected:
  void SetUp() override {
    turbo::FlagsUsageConfig default_config;
    turbo::SetFlagsUsageConfig(default_config);
  }
};

namespace flags = turbo::flags_internal;

std::string TstNormalizeFilename(std::string_view filename) {
  return std::string(filename.substr(2));
}

TEST_F(FlagsUsageConfigTest, TestGetSetFlagsUsageConfig) {
  EXPECT_TRUE(flags::GetUsageConfig().normalize_filename);

  turbo::FlagsUsageConfig empty_config;
  empty_config.normalize_filename = &TstNormalizeFilename;
  turbo::SetFlagsUsageConfig(empty_config);

  EXPECT_TRUE(flags::GetUsageConfig().normalize_filename);
  EXPECT_EQ(flags::GetUsageConfig().normalize_filename("a/a.cc"), "a.cc");
}

TEST_F(FlagsUsageConfigTest, TestNormalizeFilename) {
  EXPECT_EQ(flags::GetUsageConfig().normalize_filename("a/a.cc"), "a/a.cc");
  EXPECT_EQ(flags::GetUsageConfig().normalize_filename("/a/a.cc"), "a/a.cc");
  EXPECT_EQ(flags::GetUsageConfig().normalize_filename("///a/a.cc"), "a/a.cc");
  EXPECT_EQ(flags::GetUsageConfig().normalize_filename("/"), "");

  turbo::FlagsUsageConfig empty_config;
  empty_config.normalize_filename = &TstNormalizeFilename;
  turbo::SetFlagsUsageConfig(empty_config);

  EXPECT_EQ(flags::GetUsageConfig().normalize_filename("a/a.cc"), "a.cc");
  EXPECT_EQ(flags::GetUsageConfig().normalize_filename("aaa/a.cc"), "a/a.cc");

  empty_config.normalize_filename = nullptr;
  turbo::SetFlagsUsageConfig(empty_config);

  EXPECT_EQ(flags::GetUsageConfig().normalize_filename("a/a.cc"), "a/a.cc");
  EXPECT_EQ(flags::GetUsageConfig().normalize_filename("/a/a.cc"), "a/a.cc");
  EXPECT_EQ(flags::GetUsageConfig().normalize_filename("///a/a.cc"), "a/a.cc");
  EXPECT_EQ(flags::GetUsageConfig().normalize_filename("\\a\\a.cc"), "a\\a.cc");
  EXPECT_EQ(flags::GetUsageConfig().normalize_filename("//"), "");
  EXPECT_EQ(flags::GetUsageConfig().normalize_filename("\\\\"), "");
}

}  // namespace
