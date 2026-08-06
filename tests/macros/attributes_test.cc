// Copyright 2025 The Abseil Authors
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

#include <turbo/macros/macros.h>

#include <gtest/gtest.h>
#include <turbo/macros/config.h>

namespace {

TEST(Attributes, RequireExplicitInit) {
  struct Agg {
    int f1;
    int f2 KUMO_REQUIRE_EXPLICIT_INIT;
  };
  Agg good1 KUMO_ATTRIBUTE_UNUSED = {1, 2};
#if KUMO_CPLUSPLUS_LANG >= 202002L
  Agg good2 KUMO_ATTRIBUTE_UNUSED(1, 2);
#endif
  Agg good3 KUMO_ATTRIBUTE_UNUSED{1, 2};
  Agg good4 KUMO_ATTRIBUTE_UNUSED = {1, 2};
  Agg good5 KUMO_ATTRIBUTE_UNUSED = Agg{1, 2};
  Agg good6[1] KUMO_ATTRIBUTE_UNUSED = {{1, 2}};
  Agg good7[1] KUMO_ATTRIBUTE_UNUSED = {Agg{1, 2}};
  union {
    Agg agg;
  } good8 KUMO_ATTRIBUTE_UNUSED = {{1, 2}};
  constexpr Agg good9 KUMO_ATTRIBUTE_UNUSED = {1, 2};
  constexpr Agg good10 KUMO_ATTRIBUTE_UNUSED{1, 2};
}

}  // namespace
