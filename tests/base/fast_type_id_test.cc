// Copyright 2020 The Abseil Authors.
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

#include <turbo/base/fast_type_id.h>

#include <cstddef>
#include <cstdint>
#include <map>
#include <vector>

#include <gtest/gtest.h>
#include <turbo/macros/config.h>

namespace {

// Ensure that turbo::bit_cast works between FastTypeIdType and void*.
static_assert(sizeof(turbo::FastTypeIdType) == sizeof(void*));

// NOLINTBEGIN(runtime/int)
#define PRIM_TYPES(A)   \
  A(bool)               \
  A(short)              \
  A(unsigned short)     \
  A(int)                \
  A(unsigned int)       \
  A(long)               \
  A(unsigned long)      \
  A(long long)          \
  A(unsigned long long) \
  A(float)              \
  A(double)             \
  A(long double)
// NOLINTEND(runtime/int)

TEST(FastTypeIdTest, PrimitiveTypes) {
  // clang-format off
  constexpr turbo::FastTypeIdType kTypeIds[] = {
#define A(T) turbo::FastTypeId<T>(),
    PRIM_TYPES(A)
#undef A
#define A(T) turbo::FastTypeId<const T>(),
    PRIM_TYPES(A)
#undef A
#define A(T) turbo::FastTypeId<volatile T>(),
    PRIM_TYPES(A)
#undef A
#define A(T) turbo::FastTypeId<const volatile T>(),
    PRIM_TYPES(A)
#undef A
  };
  // clang-format on

  for (size_t i = 0; i < KUMO_ARRAYSIZE(kTypeIds); ++i) {
    EXPECT_EQ(kTypeIds[i], kTypeIds[i]);
    for (size_t j = 0; j < i; ++j) {
      EXPECT_NE(kTypeIds[i], kTypeIds[j]);
    }
  }
}

#define FIXED_WIDTH_TYPES(A) \
  A(int8_t)                  \
  A(uint8_t)                 \
  A(int16_t)                 \
  A(uint16_t)                \
  A(int32_t)                 \
  A(uint32_t)                \
  A(int64_t)                 \
  A(uint64_t)

TEST(FastTypeIdTest, FixedWidthTypes) {
  // clang-format off
  constexpr turbo::FastTypeIdType kTypeIds[] = {
#define A(T) turbo::FastTypeId<T>(),
    FIXED_WIDTH_TYPES(A)
#undef A
#define A(T) turbo::FastTypeId<const T>(),
    FIXED_WIDTH_TYPES(A)
#undef A
#define A(T) turbo::FastTypeId<volatile T>(),
    FIXED_WIDTH_TYPES(A)
#undef A
#define A(T) turbo::FastTypeId<const volatile T>(),
    FIXED_WIDTH_TYPES(A)
#undef A
  };
  // clang-format on

  for (size_t i = 0; i < KUMO_ARRAYSIZE(kTypeIds); ++i) {
    EXPECT_EQ(kTypeIds[i], kTypeIds[i]);
    for (size_t j = 0; j < i; ++j) {
      EXPECT_NE(kTypeIds[i], kTypeIds[j]);
    }
  }
}

TEST(FastTypeIdTest, AliasTypes) {
  using int_alias = int;
  EXPECT_EQ(turbo::FastTypeId<int_alias>(), turbo::FastTypeId<int>());
}

TEST(FastTypeIdTest, TemplateSpecializations) {
  EXPECT_NE(turbo::FastTypeId<std::vector<int>>(),
            turbo::FastTypeId<std::vector<long>>());  // NOLINT(runtime/int)

  EXPECT_NE((turbo::FastTypeId<std::map<int, float>>()),
            (turbo::FastTypeId<std::map<int, double>>()));
}

struct Base {};
struct Derived : Base {};
struct PDerived : private Base {};

TEST(FastTypeIdTest, Inheritance) {
  EXPECT_NE(turbo::FastTypeId<Base>(), turbo::FastTypeId<Derived>());
  EXPECT_NE(turbo::FastTypeId<Base>(), turbo::FastTypeId<PDerived>());
}

}  // namespace
