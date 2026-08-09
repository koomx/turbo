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

#include <string.h>

#include <cctype>
#include <cstdint>
#include <utility>
#include <turbo/strings/strip.h>
#include <turbo/strings/ascii.h>
#include <turbo/strings/match.h>
#include <string_view>
#include <turbo/cctz/time_zone.h>
#include <turbo/time/time.h>

namespace cctz = turbo::time_internal::cctz;

namespace turbo {


KUMO_DLL extern const char RFC3339_full[] = "%Y-%m-%d%ET%H:%M:%E*S%Ez";
KUMO_DLL extern const char RFC3339_sec[] = "%Y-%m-%d%ET%H:%M:%S%Ez";

KUMO_DLL extern const char RFC1123_full[] = "%a, %d %b %E4Y %H:%M:%S %z";
KUMO_DLL extern const char RFC1123_no_wday[] = "%d %b %E4Y %H:%M:%S %z";

namespace {

constexpr std::string_view kInfiniteFutureStr = "infinite-future";
constexpr std::string_view kInfinitePastStr = "infinite-past";

struct cctz_parts {
  cctz::time_point<cctz::seconds> sec;
  cctz::detail::femtoseconds fem;
};

inline cctz::time_point<cctz::seconds> unix_epoch() {
  return std::chrono::time_point_cast<cctz::seconds>(
      std::chrono::system_clock::from_time_t(0));
}

// Splits a Time into seconds and femtoseconds, which can be used with CCTZ.
// Requires that 't' is finite. See duration.cc for details about rep_hi and
// rep_lo.
cctz_parts Split(turbo::Time t) {
  const auto d = time_internal::ToUnixDuration(t);
  const int64_t rep_hi = time_internal::GetRepHi(d);
  const int64_t rep_lo = time_internal::GetRepLo(d);
  const auto sec = unix_epoch() + cctz::seconds(rep_hi);
  const auto fem = cctz::detail::femtoseconds(rep_lo * (1000 * 1000 / 4));
  return {sec, fem};
}

// Joins the given seconds and femtoseconds into a Time. See duration.cc for
// details about rep_hi and rep_lo.
turbo::Time Join(const cctz_parts& parts) {
  const int64_t rep_hi = (parts.sec - unix_epoch()).count();
  const uint32_t rep_lo =
      static_cast<uint32_t>(parts.fem.count() / (1000 * 1000 / 4));
  const auto d = time_internal::MakeDuration(rep_hi, rep_lo);
  return time_internal::FromUnixDuration(d);
}

}  // namespace

std::string FormatTime(std::string_view format, turbo::Time t,
                       turbo::TimeZone tz) {
  if (t == turbo::InfiniteFuture()) return std::string(kInfiniteFutureStr);
  if (t == turbo::InfinitePast()) return std::string(kInfinitePastStr);
  const auto parts = Split(t);
  return cctz::detail::format(std::string(format), parts.sec, parts.fem,
                              cctz::time_zone(tz));
}

std::string FormatTime(turbo::Time t, turbo::TimeZone tz) {
  return FormatTime(RFC3339_full, t, tz);
}

std::string FormatTime(turbo::Time t) {
  return turbo::FormatTime(RFC3339_full, t, turbo::LocalTimeZone());
}

bool ParseTime(std::string_view format, std::string_view input,
               turbo::Time* time, std::string* err) {
  return turbo::ParseTime(format, input, turbo::UTCTimeZone(), time, err);
}

// If the input string does not contain an explicit UTC offset, interpret
// the fields with respect to the given TimeZone.
bool ParseTime(std::string_view format, std::string_view input,
               turbo::TimeZone tz, turbo::Time* time, std::string* err) {
  static constexpr struct Literal {
    std::string_view name;
    turbo::Time value;
  } kLiterals[] = {
      {kInfiniteFutureStr, InfiniteFuture()},
      {kInfinitePastStr, InfinitePast()},
  };
  input = trim_left(input);
  for (const auto& lit : kLiterals) {
    if (turbo::StartsWith(input, lit.name)) {
      std::string_view tail = input.substr(lit.name.size());
      // The trailing portion must be empty or whitespace.
      if (trim_left(tail).empty()) {
        *time = lit.value;
        return true;
      }
    }
  }

  std::string error;
  cctz_parts parts;
  const bool b =
      cctz::detail::parse(std::string(format), std::string(input),
                          cctz::time_zone(tz), &parts.sec, &parts.fem, &error);
  if (b) {
    *time = Join(parts);
  } else if (err != nullptr) {
    *err = std::move(error);
  }
  return b;
}

// Functions required to support turbo::Time flags.
bool TurboParseFlag(std::string_view text, turbo::Time* t, std::string* error) {
  return turbo::ParseTime(RFC3339_full, text, turbo::UTCTimeZone(), t, error);
}

std::string TurboUnparseFlag(turbo::Time t) {
  return turbo::FormatTime(RFC3339_full, t, turbo::UTCTimeZone());
}


}  // namespace turbo
