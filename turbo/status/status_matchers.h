// Copyright 2024 The Abseil Authors.
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
// File: status_matchers.h
// -----------------------------------------------------------------------------
//
// Testing utilities for working with `turbo::Status` and `turbo::StatusOr`.
//
// Defines the following utilities:
//
//   =================
//   TURBO_EXPECT_OK(s)
//
//   TURBO_ASSERT_OK(s)
//   =================
//   Convenience macros for `EXPECT_THAT(s, IsOk())`, where `s` is either
//   a `Status` or a `StatusOr<T>`.
//
//   There are no EXPECT_NOT_OK/ASSERT_NOT_OK macros.
//   Prefer to check for the specific expected error:
//
//   EXPECT_THAT(s, StatusIs(expected_error));
//   EXPECT_THAT(s, StatusIs(_, _, HasSubstr("expected error")));
//
//
//   ===============
//   `IsOkAndHolds(m)`
//   ===============
//
//   This gMock matcher matches a StatusOr<T> value whose status is OK
//   and whose inner value matches matcher m.  Example:
//
//   ```
//   using ::testing::MatchesRegex;
//   using ::turbo_testing::IsOkAndHolds;
//   ...
//   turbo::StatusOr<string> maybe_name = ...;
//   EXPECT_THAT(maybe_name, IsOkAndHolds(MatchesRegex("John .*")));
//   ```
//
//   ===============================
//   `StatusIs(status_code_matcher)`
//   ===============================
//
//   This is a shorthand for
//     `StatusIs(status_code_matcher, ::testing::_)`
//   In other words, it's like the two-argument `StatusIs()`, except that it
//   ignores error message.
//
//   ===============
//   `IsOk()`
//   ===============
//
//   Matches an `turbo::Status` or `turbo::StatusOr<T>` value whose status value
//   is `turbo::StatusCode::kOk.`
//
//   Equivalent to 'StatusIs(turbo::StatusCode::kOk)'.
//   Example:
//   ```
//   using ::turbo_testing::IsOk;
//   ...
//   turbo::StatusOr<string> maybe_name = ...;
//   EXPECT_THAT(maybe_name, IsOk());
//   Status s = ...;
//   EXPECT_THAT(s, IsOk());
//   ```
//
//   There is no NotOk() matcher. Prefer to check for the specific expected
//   error.

#ifndef TURBO_STATUS_STATUS_MATCHERS_H_
#define TURBO_STATUS_STATUS_MATCHERS_H_

#include <ostream>  // NOLINT
#include <type_traits>
#include <utility>

#include <gmock/gmock.h>  // gmock_for_status_matchers.h
#include <turbo/macros/config.h>
#include <tests/status/internal/status_matchers.h>

namespace turbo_testing {


// Macros for testing the results of functions that return turbo::Status or
// turbo::StatusOr<T> (for any type T).
#define TURBO_EXPECT_OK(expression) \
  EXPECT_THAT(expression, ::turbo_testing::IsOk())
#define TURBO_ASSERT_OK(expression) \
  ASSERT_THAT(expression, ::turbo_testing::IsOk())

// Returns a gMock matcher that matches a StatusOr<> whose status is
// OK and whose value matches the inner matcher.
template <typename InnerMatcherT>
status_internal::IsOkAndHoldsMatcher<std::decay_t<InnerMatcherT>> IsOkAndHolds(
    InnerMatcherT&& inner_matcher) {
  return status_internal::IsOkAndHoldsMatcher<std::decay_t<InnerMatcherT>>(
      std::forward<InnerMatcherT>(inner_matcher));
}

// Returns a gMock matcher that matches a Status or StatusOr<> whose status code
// matches code_matcher and whose error message matches message_matcher.
// Typically, code_matcher will be an turbo::StatusCode, e.g.
//
// StatusIs(turbo::StatusCode::kInvalidArgument, "...")
template <typename StatusCodeMatcherT, typename StatusMessageMatcherT>
status_internal::StatusIsMatcher StatusIs(
    StatusCodeMatcherT&& code_matcher,
    StatusMessageMatcherT&& message_matcher) {
  return status_internal::StatusIsMatcher(
      std::forward<StatusCodeMatcherT>(code_matcher),
      std::forward<StatusMessageMatcherT>(message_matcher));
}

// Returns a gMock matcher that matches a Status or StatusOr<> and whose status
// code matches code_matcher.  See above for details.
template <typename StatusCodeMatcherT>
status_internal::StatusIsMatcher StatusIs(StatusCodeMatcherT&& code_matcher) {
  return turbo_testing::StatusIs(std::forward<StatusCodeMatcherT>(code_matcher),
                                ::testing::_);
}

// Returns a gMock matcher that matches a Status or StatusOr<> which is OK.
inline status_internal::IsOkMatcher IsOk() {
  return status_internal::IsOkMatcher();
}

// By defining TURBO_DEFINE_UNQUALIFIED_STATUS_TESTING_MACROS, this library also
// provides unqualified versions of macros
//
// Unqualified macro names are likely to collide with those other projects, and
// so are not recommended.  Further, this is true of any transitive dependency
// of Abseil; it is impossible to be confident no downstream library will not
// also define these macros itself nor depend on a different library that also
// defines them.
//
// To enable this, define `TURBO_DEFINE_UNQUALIFIED_STATUS_TESTING_MACROS`
// preferably at the command line, e.g.
// `-DTURBO_DEFINE_UNQUALIFIED_STATUS_TESTING_MACROS` or
// `local_defines = ["TURBO_DEFINE_UNQUALIFIED_STATUS_TESTING_MACROS"]` if using
// Bazel.
//
// These are turned on by default inside Google's internal codebase where their
// use is historically ubiquitous.  Other OSS Google projects should use the
// qualified versions or the `EXPECT_THAT(..., IsOk())` form.
#ifdef TURBO_DEFINE_UNQUALIFIED_STATUS_TESTING_MACROS
#define EXPECT_OK(expression) TURBO_EXPECT_OK(expression)
#define ASSERT_OK(expression) TURBO_ASSERT_OK(expression)
#endif  // TURBO_DEFINE_UNQUALIFIED_STATUS_TESTING_MACROS


}  // namespace turbo_testing

#endif  // TURBO_STATUS_STATUS_MATCHERS_H_
