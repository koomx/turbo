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

// This file contains string processing functions related to
// numeric values.

#include <turbo/strings/numbers.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <cfloat>  // for DBL_DIG and FLT_DIG
#include <clocale>  // for localeconv
#include <cmath>   // for HUGE_VAL
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <limits>
#include <system_error>  // NOLINT(build/c++11)
#include <utility>


#include <turbo/macros/config.h>
#include <turbo/bits/endian.h>
#include <turbo/base/internal/raw_logging.h>
#include <turbo/base/nullability.h>
#include <turbo/bits/bits.h>
#include <turbo/numeric/int128.h>
#include <turbo/strings/ascii.h>
#include <turbo/strings/charconv.h>
#include <turbo/strings/match.h>
#include <string_view>

namespace turbo {


bool SimpleAtof(std::string_view str, float* turbo_nonnull out) {
  *out = 0.0;
  str = StripAsciiWhitespace(str);
  if (str.empty()) {
    // turbo::from_chars doesn't accept empty strings.
    return false;
  }
  // std::from_chars doesn't accept an initial +, but SimpleAtof does, so if one
  // is present, skip it, while avoiding accepting "+-0" as valid.
  if (str[0] == '+') {
    str.remove_prefix(1);
    if (str.empty() || str[0] == '-') {
      return false;
    }
  }
  auto result = turbo::from_chars(str.data(), str.data() + str.size(), *out);
  if (result.ec == std::errc::invalid_argument) {
    return false;
  }
  if (result.ptr != str.data() + str.size()) {
    // not all non-whitespace characters consumed
    return false;
  }
  // from_chars() with DR 3081's current wording will return max() on
  // overflow.  SimpleAtof returns infinity instead.
  if (result.ec == std::errc::result_out_of_range) {
    if (*out > 1.0) {
      *out = std::numeric_limits<float>::infinity();
    } else if (*out < -1.0) {
      *out = -std::numeric_limits<float>::infinity();
    }
  }
  return true;
}

bool SimpleAtod(std::string_view str, double* turbo_nonnull out) {
  *out = 0.0;
  str = StripAsciiWhitespace(str);
  if (str.empty()) {
    // turbo::from_chars doesn't accept empty strings.
    return false;
  }
  // std::from_chars doesn't accept an initial +, but SimpleAtod does, so if one
  // is present, skip it, while avoiding accepting "+-0" as valid.
  if (str[0] == '+') {
    str.remove_prefix(1);
    if (str.empty() || str[0] == '-') {
      return false;
    }
  }
  auto result = turbo::from_chars(str.data(), str.data() + str.size(), *out);
  if (result.ec == std::errc::invalid_argument) {
    return false;
  }
  if (result.ptr != str.data() + str.size()) {
    // not all non-whitespace characters consumed
    return false;
  }
  // from_chars() with DR 3081's current wording will return max() on
  // overflow.  SimpleAtod returns infinity instead.
  if (result.ec == std::errc::result_out_of_range) {
    if (*out > 1.0) {
      *out = std::numeric_limits<double>::infinity();
    } else if (*out < -1.0) {
      *out = -std::numeric_limits<double>::infinity();
    }
  }
  return true;
}

bool SimpleAtob(std::string_view str, bool* turbo_nonnull out) {
  TURBO_RAW_CHECK(out != nullptr, "Output pointer must not be nullptr.");
  if (EqualsIgnoreCase(str, "true") || EqualsIgnoreCase(str, "t") ||
      EqualsIgnoreCase(str, "yes") || EqualsIgnoreCase(str, "y") ||
      EqualsIgnoreCase(str, "1")) {
    *out = true;
    return true;
  }
  if (EqualsIgnoreCase(str, "false") || EqualsIgnoreCase(str, "f") ||
      EqualsIgnoreCase(str, "no") || EqualsIgnoreCase(str, "n") ||
      EqualsIgnoreCase(str, "0")) {
    *out = false;
    return true;
  }
  return false;
}

// ----------------------------------------------------------------------
// fast_int_to_buffer() overloads
//
// Like the Fast*ToBuffer() functions above, these are intended for speed.
// Unlike the Fast*ToBuffer() functions, however, these functions write
// their output to the beginning of the buffer.  The caller is responsible
// for ensuring that the buffer has enough space to hold the output.
//
// Returns a pointer to the end of the string (i.e. the null character
// terminating the string).
// ----------------------------------------------------------------------

namespace {
// Represents integer values of digits.
// Uses 36 to indicate an invalid character since we support
// bases up to 36.
static constexpr std::array<int8_t, 256> kAsciiToInt = {
    36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36,  // 16 36s.
    36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36,
    36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 0,  1,  2,  3,  4,  5,
    6,  7,  8,  9,  36, 36, 36, 36, 36, 36, 36, 10, 11, 12, 13, 14, 15, 16, 17,
    18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,
    36, 36, 36, 36, 36, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
    24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 36, 36, 36, 36, 36, 36,
    36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36,
    36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36,
    36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36,
    36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36,
    36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36,
    36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36,
    36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36};

// Parse the sign and optional hex or oct prefix in text.
inline bool safe_parse_sign_and_base(
    std::string_view* turbo_nonnull text /*inout*/,
    int* turbo_nonnull base_ptr /*inout*/,
    bool* turbo_nonnull negative_ptr /*output*/) {
  if (text->data() == nullptr) {
    return false;
  }

  const char* start = text->data();
  const char* end = start + text->size();
  int base = *base_ptr;

  // Consume whitespace.
  while (start < end &&
         turbo::ascii_isspace(static_cast<unsigned char>(start[0]))) {
    ++start;
  }
  while (start < end &&
         turbo::ascii_isspace(static_cast<unsigned char>(end[-1]))) {
    --end;
  }
  if (start >= end) {
    return false;
  }

  // Consume sign.
  *negative_ptr = (start[0] == '-');
  if (*negative_ptr || start[0] == '+') {
    ++start;
    if (start >= end) {
      return false;
    }
  }

  // Consume base-dependent prefix.
  //  base 0: "0x" -> base 16, "0" -> base 8, default -> base 10
  //  base 16: "0x" -> base 16
  // Also validate the base.
  if (base == 0) {
    if (end - start >= 2 && start[0] == '0' &&
        (start[1] == 'x' || start[1] == 'X')) {
      base = 16;
      start += 2;
      if (start >= end) {
        // "0x" with no digits after is invalid.
        return false;
      }
    } else if (end - start >= 1 && start[0] == '0') {
      base = 8;
      start += 1;
    } else {
      base = 10;
    }
  } else if (base == 16) {
    if (end - start >= 2 && start[0] == '0' &&
        (start[1] == 'x' || start[1] == 'X')) {
      start += 2;
      if (start >= end) {
        // "0x" with no digits after is invalid.
        return false;
      }
    }
  } else if (base >= 2 && base <= 36) {
    // okay
  } else {
    return false;
  }
  *text = std::string_view(start, static_cast<size_t>(end - start));
  *base_ptr = base;
  return true;
}

// Consume digits.
//
// The classic loop:
//
//   for each digit
//     value = value * base + digit
//   value *= sign
//
// The classic loop needs overflow checking.  It also fails on the most
// negative integer, -2147483648 in 32-bit two's complement representation.
//
// My improved loop:
//
//  if (!negative)
//    for each digit
//      value = value * base
//      value = value + digit
//  else
//    for each digit
//      value = value * base
//      value = value - digit
//
// Overflow checking becomes simple.

// Lookup tables per IntType:
// vmax/base and vmin/base are precomputed because division costs at least 8ns.
// TODO(junyer): Doing this per base instead (i.e. an array of structs, not a
// struct of arrays) would probably be better in terms of d-cache for the most
// commonly used bases.
template <typename IntType>
struct LookupTables {
  KUMO_CONST_INIT static const IntType kVmaxOverBase[];
  KUMO_CONST_INIT static const IntType kVminOverBase[];
};

// An array initializer macro for X/base where base in [0, 36].
// However, note that lookups for base in [0, 1] should never happen because
// base has been validated to be in [2, 36] by safe_parse_sign_and_base().
#define X_OVER_BASE_INITIALIZER(X)                                        \
  {                                                                       \
    0, 0, X / 2, X / 3, X / 4, X / 5, X / 6, X / 7, X / 8, X / 9, X / 10, \
        X / 11, X / 12, X / 13, X / 14, X / 15, X / 16, X / 17, X / 18,   \
        X / 19, X / 20, X / 21, X / 22, X / 23, X / 24, X / 25, X / 26,   \
        X / 27, X / 28, X / 29, X / 30, X / 31, X / 32, X / 33, X / 34,   \
        X / 35, X / 36,                                                   \
  }

// This kVmaxOverBase is generated with
//  for (int base = 2; base < 37; ++base) {
//    turbo::uint128 max = std::numeric_limits<turbo::uint128>::max();
//    auto result = max / base;
//    std::cout << "    MakeUint128(" << turbo::Uint128High64(result) << "u, "
//              << turbo::Uint128Low64(result) << "u),\n";
//  }
// See https://godbolt.org/z/aneYsb
//
// uint128& operator/=(uint128) is not constexpr, so hardcode the resulting
// array to avoid a static initializer.
template <>
KUMO_CONST_INIT const uint128 LookupTables<uint128>::kVmaxOverBase[] = {
    0,
    0,
    MakeUint128(9223372036854775807u, 18446744073709551615u),
    MakeUint128(6148914691236517205u, 6148914691236517205u),
    MakeUint128(4611686018427387903u, 18446744073709551615u),
    MakeUint128(3689348814741910323u, 3689348814741910323u),
    MakeUint128(3074457345618258602u, 12297829382473034410u),
    MakeUint128(2635249153387078802u, 5270498306774157604u),
    MakeUint128(2305843009213693951u, 18446744073709551615u),
    MakeUint128(2049638230412172401u, 14347467612885206812u),
    MakeUint128(1844674407370955161u, 11068046444225730969u),
    MakeUint128(1676976733973595601u, 8384883669867978007u),
    MakeUint128(1537228672809129301u, 6148914691236517205u),
    MakeUint128(1418980313362273201u, 4256940940086819603u),
    MakeUint128(1317624576693539401u, 2635249153387078802u),
    MakeUint128(1229782938247303441u, 1229782938247303441u),
    MakeUint128(1152921504606846975u, 18446744073709551615u),
    MakeUint128(1085102592571150095u, 1085102592571150095u),
    MakeUint128(1024819115206086200u, 16397105843297379214u),
    MakeUint128(970881267037344821u, 16504981539634861972u),
    MakeUint128(922337203685477580u, 14757395258967641292u),
    MakeUint128(878416384462359600u, 14054662151397753612u),
    MakeUint128(838488366986797800u, 13415813871788764811u),
    MakeUint128(802032351030850070u, 4812194106185100421u),
    MakeUint128(768614336404564650u, 12297829382473034410u),
    MakeUint128(737869762948382064u, 11805916207174113034u),
    MakeUint128(709490156681136600u, 11351842506898185609u),
    MakeUint128(683212743470724133u, 17080318586768103348u),
    MakeUint128(658812288346769700u, 10540996613548315209u),
    MakeUint128(636094623231363848u, 15266270957552732371u),
    MakeUint128(614891469123651720u, 9838263505978427528u),
    MakeUint128(595056260442243600u, 9520900167075897608u),
    MakeUint128(576460752303423487u, 18446744073709551615u),
    MakeUint128(558992244657865200u, 8943875914525843207u),
    MakeUint128(542551296285575047u, 9765923333140350855u),
    MakeUint128(527049830677415760u, 8432797290838652167u),
    MakeUint128(512409557603043100u, 8198552921648689607u),
};

// This kVmaxOverBase generated with
//   for (int base = 2; base < 37; ++base) {
//    turbo::int128 max = std::numeric_limits<turbo::int128>::max();
//    auto result = max / base;
//    std::cout << "\tMakeInt128(" << turbo::Int128High64(result) << ", "
//              << turbo::Int128Low64(result) << "u),\n";
//  }
// See https://godbolt.org/z/7djYWz
//
// int128& operator/=(int128) is not constexpr, so hardcode the resulting array
// to avoid a static initializer.
template <>
KUMO_CONST_INIT const int128 LookupTables<int128>::kVmaxOverBase[] = {
    0,
    0,
    MakeInt128(4611686018427387903, 18446744073709551615u),
    MakeInt128(3074457345618258602, 12297829382473034410u),
    MakeInt128(2305843009213693951, 18446744073709551615u),
    MakeInt128(1844674407370955161, 11068046444225730969u),
    MakeInt128(1537228672809129301, 6148914691236517205u),
    MakeInt128(1317624576693539401, 2635249153387078802u),
    MakeInt128(1152921504606846975, 18446744073709551615u),
    MakeInt128(1024819115206086200, 16397105843297379214u),
    MakeInt128(922337203685477580, 14757395258967641292u),
    MakeInt128(838488366986797800, 13415813871788764811u),
    MakeInt128(768614336404564650, 12297829382473034410u),
    MakeInt128(709490156681136600, 11351842506898185609u),
    MakeInt128(658812288346769700, 10540996613548315209u),
    MakeInt128(614891469123651720, 9838263505978427528u),
    MakeInt128(576460752303423487, 18446744073709551615u),
    MakeInt128(542551296285575047, 9765923333140350855u),
    MakeInt128(512409557603043100, 8198552921648689607u),
    MakeInt128(485440633518672410, 17475862806672206794u),
    MakeInt128(461168601842738790, 7378697629483820646u),
    MakeInt128(439208192231179800, 7027331075698876806u),
    MakeInt128(419244183493398900, 6707906935894382405u),
    MakeInt128(401016175515425035, 2406097053092550210u),
    MakeInt128(384307168202282325, 6148914691236517205u),
    MakeInt128(368934881474191032, 5902958103587056517u),
    MakeInt128(354745078340568300, 5675921253449092804u),
    MakeInt128(341606371735362066, 17763531330238827482u),
    MakeInt128(329406144173384850, 5270498306774157604u),
    MakeInt128(318047311615681924, 7633135478776366185u),
    MakeInt128(307445734561825860, 4919131752989213764u),
    MakeInt128(297528130221121800, 4760450083537948804u),
    MakeInt128(288230376151711743, 18446744073709551615u),
    MakeInt128(279496122328932600, 4471937957262921603u),
    MakeInt128(271275648142787523, 14106333703424951235u),
    MakeInt128(263524915338707880, 4216398645419326083u),
    MakeInt128(256204778801521550, 4099276460824344803u),
};

// This kVminOverBase generated with
//  for (int base = 2; base < 37; ++base) {
//    turbo::int128 min = std::numeric_limits<turbo::int128>::min();
//    auto result = min / base;
//    std::cout << "\tMakeInt128(" << turbo::Int128High64(result) << ", "
//              << turbo::Int128Low64(result) << "u),\n";
//  }
//
// See https://godbolt.org/z/7djYWz
//
// int128& operator/=(int128) is not constexpr, so hardcode the resulting array
// to avoid a static initializer.
template <>
KUMO_CONST_INIT const int128 LookupTables<int128>::kVminOverBase[] = {
    0,
    0,
    MakeInt128(-4611686018427387904, 0u),
    MakeInt128(-3074457345618258603, 6148914691236517206u),
    MakeInt128(-2305843009213693952, 0u),
    MakeInt128(-1844674407370955162, 7378697629483820647u),
    MakeInt128(-1537228672809129302, 12297829382473034411u),
    MakeInt128(-1317624576693539402, 15811494920322472814u),
    MakeInt128(-1152921504606846976, 0u),
    MakeInt128(-1024819115206086201, 2049638230412172402u),
    MakeInt128(-922337203685477581, 3689348814741910324u),
    MakeInt128(-838488366986797801, 5030930201920786805u),
    MakeInt128(-768614336404564651, 6148914691236517206u),
    MakeInt128(-709490156681136601, 7094901566811366007u),
    MakeInt128(-658812288346769701, 7905747460161236407u),
    MakeInt128(-614891469123651721, 8608480567731124088u),
    MakeInt128(-576460752303423488, 0u),
    MakeInt128(-542551296285575048, 8680820740569200761u),
    MakeInt128(-512409557603043101, 10248191152060862009u),
    MakeInt128(-485440633518672411, 970881267037344822u),
    MakeInt128(-461168601842738791, 11068046444225730970u),
    MakeInt128(-439208192231179801, 11419412998010674810u),
    MakeInt128(-419244183493398901, 11738837137815169211u),
    MakeInt128(-401016175515425036, 16040647020617001406u),
    MakeInt128(-384307168202282326, 12297829382473034411u),
    MakeInt128(-368934881474191033, 12543785970122495099u),
    MakeInt128(-354745078340568301, 12770822820260458812u),
    MakeInt128(-341606371735362067, 683212743470724134u),
    MakeInt128(-329406144173384851, 13176245766935394012u),
    MakeInt128(-318047311615681925, 10813608594933185431u),
    MakeInt128(-307445734561825861, 13527612320720337852u),
    MakeInt128(-297528130221121801, 13686293990171602812u),
    MakeInt128(-288230376151711744, 0u),
    MakeInt128(-279496122328932601, 13974806116446630013u),
    MakeInt128(-271275648142787524, 4340410370284600381u),
    MakeInt128(-263524915338707881, 14230345428290225533u),
    MakeInt128(-256204778801521551, 14347467612885206813u),
};

template <typename IntType>
KUMO_CONST_INIT const IntType LookupTables<IntType>::kVmaxOverBase[] =
    X_OVER_BASE_INITIALIZER(std::numeric_limits<IntType>::max());

template <typename IntType>
KUMO_CONST_INIT const IntType LookupTables<IntType>::kVminOverBase[] =
    X_OVER_BASE_INITIALIZER(std::numeric_limits<IntType>::min());

#undef X_OVER_BASE_INITIALIZER

template <typename IntType>
inline bool safe_parse_positive_int(std::string_view text, int base,
                                    IntType* turbo_nonnull value_p) {
  IntType value = 0;
  const IntType vmax = std::numeric_limits<IntType>::max();
  assert(vmax > 0);
  assert(base >= 0);
  const IntType base_inttype = static_cast<IntType>(base);
  assert(vmax >= base_inttype);
  const IntType vmax_over_base = LookupTables<IntType>::kVmaxOverBase[base];
  assert(base < 2 ||
         std::numeric_limits<IntType>::max() / base_inttype == vmax_over_base);
  const char* start = text.data();
  const char* end = start + text.size();
  // loop over digits
  for (; start < end; ++start) {
    unsigned char c = static_cast<unsigned char>(start[0]);
    IntType digit = static_cast<IntType>(kAsciiToInt[c]);
    if (digit >= base_inttype) {
      *value_p = value;
      return false;
    }
    if (value > vmax_over_base) {
      *value_p = vmax;
      return false;
    }
    value *= base_inttype;
    if (value > vmax - digit) {
      *value_p = vmax;
      return false;
    }
    value += digit;
  }
  *value_p = value;
  return true;
}

template <typename IntType>
inline bool safe_parse_negative_int(std::string_view text, int base,
                                    IntType* turbo_nonnull value_p) {
  IntType value = 0;
  const IntType vmin = std::numeric_limits<IntType>::min();
  assert(vmin < 0);
  assert(vmin <= 0 - base);
  IntType vmin_over_base = LookupTables<IntType>::kVminOverBase[base];
  assert(base < 2 ||
         std::numeric_limits<IntType>::min() / base == vmin_over_base);
  // 2003 c++ standard [expr.mul]
  // "... the sign of the remainder is implementation-defined."
  // Although (vmin/base)*base + vmin%base is always vmin.
  // 2011 c++ standard tightens the spec but we cannot rely on it.
  // TODO(junyer): Handle this in the lookup table generation.
  if (vmin % base > 0) {
    vmin_over_base += 1;
  }
  const char* start = text.data();
  const char* end = start + text.size();
  // loop over digits
  for (; start < end; ++start) {
    unsigned char c = static_cast<unsigned char>(start[0]);
    int digit = kAsciiToInt[c];
    if (digit >= base) {
      *value_p = value;
      return false;
    }
    if (value < vmin_over_base) {
      *value_p = vmin;
      return false;
    }
    value *= base;
    if (value < vmin + digit) {
      *value_p = vmin;
      return false;
    }
    value -= digit;
  }
  *value_p = value;
  return true;
}

// Input format based on POSIX.1-2008 strtol
// http://pubs.opengroup.org/onlinepubs/9699919799/functions/strtol.html
template <typename IntType>
inline bool safe_int_internal(std::string_view text,
                              IntType* turbo_nonnull value_p, int base) {
  *value_p = 0;
  bool negative;
  if (!safe_parse_sign_and_base(&text, &base, &negative)) {
    return false;
  }
  if (!negative) {
    return safe_parse_positive_int(text, base, value_p);
  } else {
    return safe_parse_negative_int(text, base, value_p);
  }
}

template <typename IntType>
inline bool safe_uint_internal(std::string_view text,
                               IntType* turbo_nonnull value_p, int base) {
  *value_p = 0;
  bool negative;
  if (!safe_parse_sign_and_base(&text, &base, &negative) || negative) {
    return false;
  }
  return safe_parse_positive_int(text, base, value_p);
}
}  // anonymous namespace

namespace numbers_internal {

bool safe_strto8_base(std::string_view text, int8_t* turbo_nonnull value,
                      int base) {
  return safe_int_internal<int8_t>(text, value, base);
}

bool safe_strto16_base(std::string_view text, int16_t* turbo_nonnull value,
                       int base) {
  return safe_int_internal<int16_t>(text, value, base);
}

bool safe_strto32_base(std::string_view text, int32_t* turbo_nonnull value,
                       int base) {
  return safe_int_internal<int32_t>(text, value, base);
}

bool safe_strto64_base(std::string_view text, int64_t* turbo_nonnull value,
                       int base) {
  return safe_int_internal<int64_t>(text, value, base);
}

bool safe_strto128_base(std::string_view text, int128* turbo_nonnull value,
                        int base) {
  return safe_int_internal<turbo::int128>(text, value, base);
}

bool safe_strtou8_base(std::string_view text, uint8_t* turbo_nonnull value,
                       int base) {
  return safe_uint_internal<uint8_t>(text, value, base);
}

bool safe_strtou16_base(std::string_view text, uint16_t* turbo_nonnull value,
                        int base) {
  return safe_uint_internal<uint16_t>(text, value, base);
}

bool safe_strtou32_base(std::string_view text, uint32_t* turbo_nonnull value,
                        int base) {
  return safe_uint_internal<uint32_t>(text, value, base);
}

bool safe_strtou64_base(std::string_view text, uint64_t* turbo_nonnull value,
                        int base) {
  return safe_uint_internal<uint64_t>(text, value, base);
}

bool safe_strtou128_base(std::string_view text, uint128* turbo_nonnull value,
                         int base) {
  return safe_uint_internal<turbo::uint128>(text, value, base);
}

}  // namespace numbers_internal

}  // namespace turbo
