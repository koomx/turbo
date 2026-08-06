// Copyright 2023 The Abseil Authors.
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
//
// -----------------------------------------------------------------------------
// File: overload.h
// -----------------------------------------------------------------------------
//
// `turbo::Overload` is a functor that provides overloads based on the functors
// with which it is created. This can, for example, be used to locally define an
// anonymous visitor type for `std::visit` inside a function using lambdas.
//
// Before using this function, consider whether named function overloads would
// be a better design.
//
// Example:
//
//     std::variant<std::string, int32_t, int64_t> v(int32_t{1});
//     const size_t result =
//         std::visit(turbo::Overload{
//                        [](const std::string& s) { return s.size(); },
//                        [](const auto& s) { return sizeof(s); },
//                    },
//                    v);
//     assert(result == 4);
//

#ifndef TURBO_FUNCTIONAL_OVERLOAD_H_
#define TURBO_FUNCTIONAL_OVERLOAD_H_

#include <turbo/macros/config.h>
#include <turbo/meta/type_traits.h>

namespace turbo {


template <typename... T>
struct Overload final : T... {
  using T::operator()...;

  // For historical reasons we want to support use that looks like a function
  // call:
  //
  //     turbo::Overload(lambda_1, lambda_2)
  //
  // This works automatically in C++20 because we have support for parenthesized
  // aggregate initialization. Before then we must provide a constructor that
  // makes this work.
  //
  constexpr explicit Overload(T... ts) : T(std::move(ts))... {}
};

// Before C++20, which added support for CTAD for aggregate types, we must also
// teach the compiler how to deduce the template arguments for Overload.
//
template <typename... T>
Overload(T...) -> Overload<T...>;


}  // namespace turbo

#endif  // TURBO_FUNCTIONAL_OVERLOAD_H_
