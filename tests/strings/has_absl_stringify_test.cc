// Copyright 2023 The Abseil Authors
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

#include <turbo/format/has_turbo_stringify.h>

#include <optional>
#include <string>

#include <gtest/gtest.h>

namespace {

struct TypeWithoutTurboStringify {};

struct TypeWithTurboStringify {
  template <typename Sink>
  friend void turbo_stringify(Sink&, const TypeWithTurboStringify&) {}
};

TEST(HasTurboStringifyTest, Works) {
  EXPECT_FALSE(turbo::HasTurboStringify<int>::value);
  EXPECT_FALSE(turbo::HasTurboStringify<std::string>::value);
  EXPECT_FALSE(turbo::HasTurboStringify<TypeWithoutTurboStringify>::value);
  EXPECT_TRUE(turbo::HasTurboStringify<TypeWithTurboStringify>::value);
  EXPECT_FALSE(
      turbo::HasTurboStringify<std::optional<TypeWithTurboStringify>>::value);
}

}  // namespace
