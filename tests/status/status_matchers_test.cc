// Copyright 2024 The Abseil Authors
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
//
// -----------------------------------------------------------------------------
// File: status_matchers_test.cc
// -----------------------------------------------------------------------------
#include <turbo/status/status_matchers.h>

#include <string>
#include <vector>

#include <gmock/gmock.h>
#include "gtest/gtest-spi.h"
#include <gtest/gtest.h>
#include <turbo/status/status.h>
#include <turbo/status/statusor.h>
#include <string_view>

namespace {

using ::turbo_testing::IsOk;
using ::turbo_testing::IsOkAndHolds;
using ::turbo_testing::StatusIs;
using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::Gt;
using ::testing::MatchesRegex;
using ::testing::Not;
using ::testing::Ref;

TEST(StatusMatcherTest, TurboExpectAssertOk) {
  TURBO_EXPECT_OK(turbo::OkStatus());
  TURBO_ASSERT_OK(turbo::OkStatus());
  EXPECT_NONFATAL_FAILURE(TURBO_EXPECT_OK(turbo::InternalError("Smigla error")),
                          "Smigla error");
  EXPECT_FATAL_FAILURE(TURBO_ASSERT_OK(turbo::InternalError("Smigla error")),
                       "Smigla error");
}

TEST(StatusMatcherTest, ExpectAssertOk) {
#ifdef TURBO_DEFINE_UNQUALIFIED_STATUS_TESTING_MACROS
  EXPECT_OK(turbo::OkStatus());
  ASSERT_OK(turbo::OkStatus());
  EXPECT_NONFATAL_FAILURE(EXPECT_OK(turbo::InternalError("Smigla error")),
                          "Smigla error");
  EXPECT_FATAL_FAILURE(ASSERT_OK(turbo::InternalError("Smigla error")),
                       "Smigla error");
#else
#ifdef EXPECT_OK
  static_assert(false, "EXPECT_OK defined despite being turned off.");
#endif  // EXPECT_OK
#ifdef ASSERT_OK
  static_assert(false, "ASSERT_OK defined despite being turned off.");
#endif  // ASSERT_OK
#endif  // TURBO_DEFINE_UNQUALIFIED_STATUS_TESTING_MACROS
}

TEST(StatusMatcherTest, StatusIsOk) { EXPECT_THAT(turbo::OkStatus(), IsOk()); }

TEST(StatusMatcherTest, StatusOrIsOk) {
  turbo::StatusOr<int> ok_int = {0};
  EXPECT_THAT(ok_int, IsOk());
}

TEST(StatusMatcherTest, StatusIsNotOk) {
  turbo::Status error = turbo::UnknownError("Smigla");
  EXPECT_NONFATAL_FAILURE(EXPECT_THAT(error, IsOk()), "Smigla");
}

TEST(StatusMatcherTest, StatusOrIsNotOk) {
  turbo::StatusOr<int> error = turbo::UnknownError("Smigla");
  EXPECT_NONFATAL_FAILURE(EXPECT_THAT(error, IsOk()), "Smigla");
}

TEST(StatusMatcherTest, IsOkAndHolds) {
  turbo::StatusOr<int> ok_int = {4};
  turbo::StatusOr<std::string_view> ok_str = {"text"};
  EXPECT_THAT(ok_int, IsOkAndHolds(4));
  EXPECT_THAT(ok_int, IsOkAndHolds(Gt(0)));
  EXPECT_THAT(ok_str, IsOkAndHolds("text"));
}

TEST(StatusMatcherTest, IsOkAndHoldsFailure) {
  turbo::StatusOr<int> ok_int = {502};
  turbo::StatusOr<int> error = turbo::UnknownError("Smigla");
  turbo::StatusOr<std::string_view> ok_str = {"actual"};
  EXPECT_NONFATAL_FAILURE(EXPECT_THAT(ok_int, IsOkAndHolds(0)), "502");
  EXPECT_NONFATAL_FAILURE(EXPECT_THAT(error, IsOkAndHolds(0)), "Smigla");
  EXPECT_NONFATAL_FAILURE(EXPECT_THAT(ok_str, IsOkAndHolds("expected")),
                          "actual");
}

template <typename MatcherType, typename Value>
std::string Explain(const MatcherType& m, const Value& x) {
  ::testing::StringMatchResultListener listener;
  ExplainMatchResult(m, x, &listener);
  return listener.str();
}

TEST(StatusMatcherTest, StatusIs) {
  turbo::Status unknown = turbo::UnknownError("unbekannt");
  turbo::Status invalid = turbo::InvalidArgumentError("ungueltig");
  EXPECT_THAT(turbo::OkStatus(), StatusIs(turbo::StatusCode::kOk));
  EXPECT_THAT(turbo::OkStatus(), StatusIs(0));
  EXPECT_THAT(unknown, StatusIs(turbo::StatusCode::kUnknown));
  EXPECT_THAT(unknown, StatusIs(2));
  EXPECT_THAT(unknown, StatusIs(turbo::StatusCode::kUnknown, "unbekannt"));
  EXPECT_THAT(invalid, StatusIs(turbo::StatusCode::kInvalidArgument));
  EXPECT_THAT(invalid, StatusIs(3));
  EXPECT_THAT(invalid,
              StatusIs(turbo::StatusCode::kInvalidArgument, "ungueltig"));

  auto m = StatusIs(turbo::StatusCode::kInternal, "internal error");
  EXPECT_THAT(::testing::DescribeMatcher<turbo::Status>(m),
              MatchesRegex("has a status code that is equal to INTERNAL, and "
                           "has an error message that .*"));
  EXPECT_THAT(
      ::testing::DescribeMatcher<turbo::Status>(m, /*negation=*/true),
      MatchesRegex(
          "either has a status code that .*, or has an error message that .*"));
  EXPECT_THAT(Explain(m, turbo::InvalidArgumentError("internal error")),
              Eq("whose status code is wrong"));
  EXPECT_THAT(Explain(m, turbo::InternalError("unexpected error")),
              Eq("whose error message is wrong"));
}

TEST(StatusMatcherTest, StatusOrIs) {
  turbo::StatusOr<int> ok = {42};
  turbo::StatusOr<int> unknown = turbo::UnknownError("unbekannt");
  turbo::StatusOr<std::string_view> invalid =
      turbo::InvalidArgumentError("ungueltig");
  EXPECT_THAT(ok, StatusIs(turbo::StatusCode::kOk));
  EXPECT_THAT(ok, StatusIs(0));
  EXPECT_THAT(unknown, StatusIs(turbo::StatusCode::kUnknown));
  EXPECT_THAT(unknown, StatusIs(2));
  EXPECT_THAT(unknown, StatusIs(turbo::StatusCode::kUnknown, "unbekannt"));
  EXPECT_THAT(invalid, StatusIs(turbo::StatusCode::kInvalidArgument));
  EXPECT_THAT(invalid, StatusIs(3));
  EXPECT_THAT(invalid,
              StatusIs(turbo::StatusCode::kInvalidArgument, "ungueltig"));

  auto m = StatusIs(turbo::StatusCode::kInternal, "internal error");
  EXPECT_THAT(
      ::testing::DescribeMatcher<turbo::StatusOr<int>>(m),
      MatchesRegex(
          "has a status code that .*, and has an error message that .*"));
  EXPECT_THAT(
      ::testing::DescribeMatcher<turbo::StatusOr<int>>(m, /*negation=*/true),
      MatchesRegex(
          "either has a status code that .*, or has an error message that .*"));
  EXPECT_THAT(Explain(m, turbo::StatusOr<int>(57)), Eq("which is OK"));
  EXPECT_THAT(Explain(m, turbo::StatusOr<int>(
                             turbo::InvalidArgumentError("internal error"))),
              Eq("whose status code is wrong"));
  EXPECT_THAT(
      Explain(m, turbo::StatusOr<int>(turbo::InternalError("unexpected error"))),
      Eq("whose error message is wrong"));
}

TEST(StatusMatcherTest, StatusIsFailure) {
  turbo::Status unknown = turbo::UnknownError("unbekannt");
  turbo::Status invalid = turbo::InvalidArgumentError("ungueltig");
  EXPECT_NONFATAL_FAILURE(
      EXPECT_THAT(turbo::OkStatus(),
                  StatusIs(turbo::StatusCode::kInvalidArgument)),
      "OK");
  EXPECT_NONFATAL_FAILURE(
      EXPECT_THAT(unknown, StatusIs(turbo::StatusCode::kCancelled)), "UNKNOWN");
  EXPECT_NONFATAL_FAILURE(
      EXPECT_THAT(unknown, StatusIs(turbo::StatusCode::kUnknown, "inconnu")),
      "unbekannt");
  EXPECT_NONFATAL_FAILURE(
      EXPECT_THAT(invalid, StatusIs(turbo::StatusCode::kOutOfRange)), "INVALID");
  EXPECT_NONFATAL_FAILURE(
      EXPECT_THAT(invalid,
                  StatusIs(turbo::StatusCode::kInvalidArgument, "invalide")),
      "ungueltig");
}

TEST(StatusMatcherTest, ReferencesWork) {
  int i = 17;
  int j = 19;
  EXPECT_THAT(turbo::StatusOr<int&>(i), IsOkAndHolds(17));
  EXPECT_THAT(turbo::StatusOr<int&>(i), Not(IsOkAndHolds(19)));
  EXPECT_THAT(turbo::StatusOr<const int&>(i), IsOkAndHolds(17));

  // Reference testing works as expected.
  EXPECT_THAT(turbo::StatusOr<int&>(i), IsOkAndHolds(Ref(i)));
  EXPECT_THAT(turbo::StatusOr<int&>(i), Not(IsOkAndHolds(Ref(j))));

  // Try a more complex one.
  std::vector<std::string> vec = {"A", "B", "C"};
  EXPECT_THAT(turbo::StatusOr<std::vector<std::string>&>(vec),
              IsOkAndHolds(ElementsAre("A", "B", "C")));
  EXPECT_THAT(turbo::StatusOr<std::vector<std::string>&>(vec),
              Not(IsOkAndHolds(ElementsAre("A", "X", "C"))));
}

}  // namespace
