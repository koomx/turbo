// Copyright 2017 The Abseil Authors.
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

#ifndef TURBO_UTILITY_UTILITY_H_
#define TURBO_UTILITY_UTILITY_H_

#include <cstddef>
#include <cstdlib>
#include <tuple>
#include <type_traits>
#include <utility>
#include <turbo/macros/macros.h>

namespace turbo {

#if KUMO_CPLUSPLUS_LANG >= 202002L
// Backfill for std::nontype_t. An instance of this class can be provided as a
// disambiguation tag to `turbo::function_ref` to pass the address of a known
// callable at compile time.
// Requires C++20 due to `auto` template parameter.
template <auto>
struct nontype_t {
  explicit nontype_t() = default;
};
template <auto V>
constexpr nontype_t<V> nontype{};
#endif


}  // namespace turbo

#endif  // TURBO_UTILITY_UTILITY_H_
