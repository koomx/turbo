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
//

#include <gtest/gtest.h>



#define STATUS_ASSERT(cond) if (!(cond)) {throw std::runtime_error(std::string("assertion failure")); }

#include <turbo/types/expected.h>

TEST(expected, Assertions) {
  turbo::expected<int,int> o1 = 42;
  ASSERT_ANY_THROW(o1.error());

  turbo::expected<int,int> o2 {turbo::unexpect, 0};
  ASSERT_ANY_THROW(*o2);

  struct foo { int bar; };
  turbo::expected<struct foo,int> o3 {turbo::unexpect, 0};
  ASSERT_ANY_THROW(o3->bar);
}
