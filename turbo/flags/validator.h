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

#pragma once

#include <string>
#include <string_view>
#include <type_traits>

#include <turbo/strings/match.h>

namespace turbo {

template <typename T>
struct AllPassValidator {
  static bool validate(const T&, std::string* error) noexcept {
    (void)error;
    return true;
  }
};

template <typename T>
struct EqValidatorComparator {
  static bool validate(T lhs, T rhs) noexcept { return lhs == rhs; }
};

template <typename T>
struct GtValidatorComparator {
  static bool validate(T lhs, T rhs) noexcept { return lhs > rhs; }
};

template <typename T>
struct GEValidatorComparator {
  static bool validate(T lhs, T rhs) noexcept { return lhs >= rhs; }
};

template <typename T>
struct LeValidatorComparator {
  static bool validate(T lhs, T rhs) noexcept { return lhs <= rhs; }
};

template <typename T>
struct LtValidatorComparator {
  static bool validate(T lhs, T rhs) noexcept { return lhs < rhs; }
};

template <typename T, T Min, typename CM,
          std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>,
                           int> = 0>
struct UnaryValidator {
  static bool validate(const T& value, std::string* error) noexcept {
    if (!CM::validate(value, Min)) {
      if (error) {
        *error = "value rejected by unary range validator";
      }
      return false;
    }
    return true;
  }
};

template <typename T, T Min>
using GeValidator = UnaryValidator<T, Min, GEValidatorComparator<T>>;

template <typename T, T Min>
using GtValidator = UnaryValidator<T, Min, GtValidatorComparator<T>>;

template <typename T, T Max>
using LeValidator = UnaryValidator<T, Max, LeValidatorComparator<T>>;

template <typename T, T Max>
using LtValidator = UnaryValidator<T, Max, LtValidatorComparator<T>>;

template <typename T, T Min, T Max, typename LCM, typename RCM,
          std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>,
                           int> = 0>
struct BinaryValidator {
  static bool validate(const T& value, std::string* error) noexcept {
    if (!LCM::validate(value, Min) || !RCM::validate(value, Max)) {
      if (error) {
        *error = "value must be in the range [" + std::to_string(Min) + ", " +
                 std::to_string(Max) + "]";
      }
      return false;
    }
    return true;
  }
};

template <typename T, T Min, T Max>
using ClosedClosedInRangeValidator =
    BinaryValidator<T, Min, Max, GEValidatorComparator<T>,
                    LeValidatorComparator<T>>;

template <typename T, T Min, T Max>
using ClosedOpenInRangeValidator =
    BinaryValidator<T, Min, Max, GEValidatorComparator<T>,
                    LtValidatorComparator<T>>;

template <typename T, T Min, T Max>
using OpenClosedInRangeValidator =
    BinaryValidator<T, Min, Max, GtValidatorComparator<T>,
                    LeValidatorComparator<T>>;

template <typename T, T Min, T Max>
using OpenOpenInRangeValidator =
    BinaryValidator<T, Min, Max, GtValidatorComparator<T>,
                    LtValidatorComparator<T>>;

template <typename T, T Min, T Max>
using ClosedClosedOutRangeValidator =
    BinaryValidator<T, Min, Max, LtValidatorComparator<T>,
                    GtValidatorComparator<T>>;

template <typename T, T Min, T Max>
using ClosedOpenOutRangeValidator =
    BinaryValidator<T, Min, Max, LtValidatorComparator<T>,
                    GEValidatorComparator<T>>;

template <typename T, T Min, T Max>
using OpenClosedOutRangeValidator =
    BinaryValidator<T, Min, Max, LeValidatorComparator<T>,
                    GtValidatorComparator<T>>;

template <typename T, T Min, T Max>
using OpenOpenOutRangeValidator =
    BinaryValidator<T, Min, Max, LeValidatorComparator<T>,
                    GEValidatorComparator<T>>;

template <const std::string_view& prefix>
struct StartsWithValidator {
  static bool validate(const std::string& value, std::string* error) noexcept {
    if (!turbo::starts_with(value, prefix)) {
      if (error) {
        *error = "value must start with " + std::string(prefix);
      }
      return false;
    }
    return true;
  }
};

template <const std::string_view& prefix>
struct StartsWithIgnoreCaseValidator {
  static bool validate(const std::string& value, std::string* error) noexcept {
    if (!turbo::starts_with_ignore_case(value, prefix)) {
      if (error) {
        *error = "value must start with " + std::string(prefix);
      }
      return false;
    }
    return true;
  }
};

template <const std::string_view& suffix>
struct EndsWithValidator {
  static bool validate(const std::string& value, std::string* error) noexcept {
    if (!turbo::ends_with(value, suffix)) {
      if (error) {
        *error = "value must ends with " + std::string(suffix);
      }
      return false;
    }
    return true;
  }
};

template <const std::string_view& suffix>
struct EndsWithIgnoreCaseValidator {
  static bool validate(const std::string& value, std::string* error) noexcept {
    if (!turbo::ends_with_ignore_case(value, suffix)) {
      if (error) {
        *error = "value must ends with " + std::string(suffix);
      }
      return false;
    }
    return true;
  }
};

template <const std::string_view& frag>
struct ContainsValidator {
  static bool validate(const std::string& value, std::string* error) noexcept {
    if (!turbo::str_contains(value, frag)) {
      if (error) {
        *error = "value must contains " + std::string(frag);
      }
      return false;
    }
    return true;
  }
};

template <const std::string_view& frag>
struct ContainsIgnoreCaseValidator {
  static bool validate(const std::string& value, std::string* error) noexcept {
    if (!turbo::str_contains_ignore_case(value, frag)) {
      if (error) {
        *error = "value must contains " + std::string(frag);
      }
      return false;
    }
    return true;
  }
};

}  // namespace turbo
