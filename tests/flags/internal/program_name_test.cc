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

#include <turbo/flags/argv.h>

#include <string>
#include <vector>

#include <gtest/gtest.h>
#include <turbo/flags/flag.h>
#include <turbo/flags/reflection.h>

namespace {

TEST(ArgvFlagTest, ShortNameFromArgv) {
  turbo::FlagSaver fs;
  EXPECT_EQ(turbo::flags_internal::ShortProgramInvocationName(), "UNKNOWN");

  turbo::SetFlag(&FLAGS_argv, std::vector<std::string>{"a/b/my_test", "--x"});
  EXPECT_EQ(turbo::flags_internal::ShortProgramInvocationName(), "my_test");
  EXPECT_EQ(turbo::GetFlag(FLAGS_argv).size(), 2u);
  EXPECT_EQ(turbo::GetFlag(FLAGS_argv)[0], "a/b/my_test");

  turbo::SetFlag(&FLAGS_argv, std::vector<std::string>{"urbo/aaa/b"});
  EXPECT_EQ(turbo::flags_internal::ShortProgramInvocationName(), "b");
}

}  // namespace
