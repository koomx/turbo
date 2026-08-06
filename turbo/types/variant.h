// Copyright 2018 The Abseil Authors.
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
// variant.h
// -----------------------------------------------------------------------------
//
// Historical note: Abseil once provided an implementation of `turbo::variant`
// as a polyfill for `std::variant` prior to C++17. Now that C++17 is required,
// `turbo::variant` is an alias for `std::variant`.

#ifndef TURBO_TYPES_VARIANT_H_
#define TURBO_TYPES_VARIANT_H_

#include <stddef.h>

#include <variant>

#include <turbo/macros/config.h>
#include <turbo/utility/utility.h>

namespace turbo {

namespace variant_internal {
// Helper visitor for converting a variant<Ts...>` into another type (mostly
// variant) that can be constructed from any type.
template <typename To>
struct ConversionVisitor {
  template <typename T>
  To operator()(T&& v) const {
    return To(std::forward<T>(v));
  }
};
}  // namespace variant_internal

// convert_variant_to()
//
// Helper functions to convert an `std::variant` to a variant of another set of
// types, provided that the alternative type of the new variant type can be
// converted from any type in the source variant.
//
// Example:
//
//   std::variant<name1, name2, float> InternalReq(const Req&);
//
//   // name1 and name2 are convertible to name
//   std::variant<name, float> ExternalReq(const Req& req) {
//     return turbo::convert_variant_to<std::variant<name, float>>(
//              InternalReq(req));
//   }
template <typename To, typename Variant>
To convert_variant_to(Variant&& variant) {
  return std::visit(variant_internal::ConversionVisitor<To>{},
                    std::forward<Variant>(variant));
}


}  // namespace turbo

#endif  // TURBO_TYPES_VARIANT_H_
