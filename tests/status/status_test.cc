// Copyright 2019 The Abseil Authors.
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

#include <turbo/status/status.h>

#include <errno.h>

#include <array>
#include <cstddef>
#include <sstream>
#include <utility>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <turbo/cord/cord.h>
#include <turbo/strings/str_cat.h>
#include <turbo/format/str_format.h>
#include <string_view>
#include <source_location>

namespace {

using ::testing::Eq;
using ::testing::HasSubstr;
using ::testing::Optional;
using ::testing::UnorderedElementsAreArray;

TEST(StatusCode, InsertionOperator) {
  const turbo::StatusCode code = turbo::StatusCode::kUnknown;
  std::ostringstream oss;
  oss << code;
  EXPECT_EQ(oss.str(), turbo::StatusCodeToString(code));
  EXPECT_EQ(oss.str(), turbo::StatusCodeToStringView(code));
}

// This structure holds the details for testing a single error code,
// its creator, and its classifier.
struct ErrorTest {
  turbo::StatusCode code;
  using Creator = turbo::Status (*)(std::string_view, std::source_location);
  using Classifier = bool (*)(const turbo::Status&);
  Creator creator;
  Classifier classifier;
};

constexpr ErrorTest kErrorTests[]{
    {turbo::StatusCode::kCancelled, turbo::CancelledError, turbo::IsCancelled},
    {turbo::StatusCode::kUnknown, turbo::UnknownError, turbo::IsUnknown},
    {turbo::StatusCode::kInvalidArgument, turbo::InvalidArgumentError,
     turbo::IsInvalidArgument},
    {turbo::StatusCode::kDeadlineExceeded, turbo::DeadlineExceededError,
     turbo::IsDeadlineExceeded},
    {turbo::StatusCode::kNotFound, turbo::NotFoundError, turbo::IsNotFound},
    {turbo::StatusCode::kAlreadyExists, turbo::AlreadyExistsError,
     turbo::IsAlreadyExists},
    {turbo::StatusCode::kPermissionDenied, turbo::PermissionDeniedError,
     turbo::IsPermissionDenied},
    {turbo::StatusCode::kResourceExhausted, turbo::ResourceExhaustedError,
     turbo::IsResourceExhausted},
    {turbo::StatusCode::kFailedPrecondition, turbo::FailedPreconditionError,
     turbo::IsFailedPrecondition},
    {turbo::StatusCode::kAborted, turbo::AbortedError, turbo::IsAborted},
    {turbo::StatusCode::kOutOfRange, turbo::OutOfRangeError, turbo::IsOutOfRange},
    {turbo::StatusCode::kUnimplemented, turbo::UnimplementedError,
     turbo::IsUnimplemented},
    {turbo::StatusCode::kInternal, turbo::InternalError, turbo::IsInternal},
    {turbo::StatusCode::kUnavailable, turbo::UnavailableError,
     turbo::IsUnavailable},
    {turbo::StatusCode::kDataLoss, turbo::DataLossError, turbo::IsDataLoss},
    {turbo::StatusCode::kUnauthenticated, turbo::UnauthenticatedError,
     turbo::IsUnauthenticated},
};

TEST(Status, CreateAndClassify) {
  for (const auto& test : kErrorTests) {
    SCOPED_TRACE(turbo::StatusCodeToString(test.code));

    // Ensure that the creator does, in fact, create status objects with the
    // expected error code and message.
    std::string message =
        turbo::StrCat("error code ", test.code, " test message");
    turbo::Status status =
        test.creator(message, std::source_location::current());
    EXPECT_EQ(test.code, status.code());
    EXPECT_EQ(message, status.message());

    // Ensure that the classifier returns true for a status produced by the
    // creator.
    EXPECT_TRUE(test.classifier(status));

    // Ensure that the classifier returns false for status with a different
    // code.
    for (const auto& other : kErrorTests) {
      if (other.code != test.code) {
        EXPECT_FALSE(test.classifier(turbo::Status(other.code, "")))
            << " other.code = " << other.code;
      }
    }
  }
}

TEST(Status, DefaultConstructor) {
  turbo::Status status;
  EXPECT_TRUE(status.ok());
  EXPECT_EQ(turbo::StatusCode::kOk, status.code());
  EXPECT_EQ("", status.message());
}

TEST(Status, OkStatus) {
  turbo::Status status = turbo::OkStatus();
  EXPECT_TRUE(status.ok());
  EXPECT_EQ(turbo::StatusCode::kOk, status.code());
  EXPECT_EQ("", status.message());
}

TEST(Status, ConstructorWithCodeMessage) {
  {
    turbo::Status status(turbo::StatusCode::kCancelled, "");
    EXPECT_FALSE(status.ok());
    EXPECT_EQ(turbo::StatusCode::kCancelled, status.code());
    EXPECT_EQ("", status.message());
  }
  {
    turbo::Status status(turbo::StatusCode::kInternal, "message");
    EXPECT_FALSE(status.ok());
    EXPECT_EQ(turbo::StatusCode::kInternal, status.code());
    EXPECT_EQ("message", status.message());
  }
}

TEST(Status, StatusMessageCStringTest) {
  {
    turbo::Status status = turbo::OkStatus();
    EXPECT_EQ(status.message(), "");
    EXPECT_STREQ(turbo::StatusMessageAsCStr(status), "");
    EXPECT_EQ(status.message(), turbo::StatusMessageAsCStr(status));
    EXPECT_NE(turbo::StatusMessageAsCStr(status), nullptr);
  }
  {
    turbo::Status status;
    EXPECT_EQ(status.message(), "");
    EXPECT_NE(turbo::StatusMessageAsCStr(status), nullptr);
    EXPECT_STREQ(turbo::StatusMessageAsCStr(status), "");
  }
  {
    turbo::Status status(turbo::StatusCode::kInternal, "message");
    EXPECT_FALSE(status.ok());
    EXPECT_EQ(turbo::StatusCode::kInternal, status.code());
    EXPECT_EQ("message", status.message());
    EXPECT_STREQ("message", turbo::StatusMessageAsCStr(status));
  }
}

TEST(Status, ConstructOutOfRangeCode) {
  const int kRawCode = 9999;
  turbo::Status status(static_cast<turbo::StatusCode>(kRawCode), "");
  EXPECT_EQ(turbo::StatusCode::kUnknown, status.code());
  EXPECT_EQ(kRawCode, status.raw_code());
}

constexpr char kUrl1[] = "url.payload.1";
constexpr char kUrl2[] = "url.payload.2";
constexpr char kUrl3[] = "url.payload.3";
constexpr char kUrl4[] = "url.payload.xx";

constexpr char kPayload1[] = "aaaaa";
constexpr char kPayload2[] = "bbbbb";
constexpr char kPayload3[] = "ccccc";

using PayloadsVec = std::vector<std::pair<std::string, std::string>>;

TEST(Status, TestGetSetPayload) {
  turbo::Status ok_status = turbo::OkStatus();
  ok_status.SetPayload(kUrl1, std::string(kPayload1));
  ok_status.SetPayload(kUrl2, std::string(kPayload2));

  EXPECT_FALSE(ok_status.GetPayload(kUrl1));
  EXPECT_FALSE(ok_status.GetPayload(kUrl2));

  turbo::Status bad_status(turbo::StatusCode::kInternal, "fail");
  bad_status.SetPayload(kUrl1, std::string(kPayload1));
  bad_status.SetPayload(kUrl2, std::string(kPayload2));

  EXPECT_THAT(bad_status.GetPayload(kUrl1), Optional(Eq(kPayload1)));
  EXPECT_THAT(bad_status.GetPayload(kUrl2), Optional(Eq(kPayload2)));

  EXPECT_FALSE(bad_status.GetPayload(kUrl3));

  bad_status.SetPayload(kUrl1, std::string(kPayload3));
  EXPECT_THAT(bad_status.GetPayload(kUrl1), Optional(Eq(kPayload3)));

  // Testing dynamically generated type_url
  bad_status.SetPayload(turbo::StrCat(kUrl1, ".1"), std::string(kPayload1));
  EXPECT_THAT(bad_status.GetPayload(turbo::StrCat(kUrl1, ".1")),
              Optional(Eq(kPayload1)));
}

TEST(Status, TestErasePayload) {
  turbo::Status bad_status(turbo::StatusCode::kInternal, "fail");
  bad_status.SetPayload(kUrl1, std::string(kPayload1));
  bad_status.SetPayload(kUrl2, std::string(kPayload2));
  bad_status.SetPayload(kUrl3, std::string(kPayload3));

  EXPECT_FALSE(bad_status.ErasePayload(kUrl4));

  EXPECT_TRUE(bad_status.GetPayload(kUrl2));
  EXPECT_TRUE(bad_status.ErasePayload(kUrl2));
  EXPECT_FALSE(bad_status.GetPayload(kUrl2));
  EXPECT_FALSE(bad_status.ErasePayload(kUrl2));

  EXPECT_TRUE(bad_status.ErasePayload(kUrl1));
  EXPECT_TRUE(bad_status.ErasePayload(kUrl3));

  bad_status.SetPayload(kUrl1, std::string(kPayload1));
  EXPECT_TRUE(bad_status.ErasePayload(kUrl1));
}

TEST(Status, TestComparePayloads) {
  turbo::Status bad_status1(turbo::StatusCode::kInternal, "fail");
  bad_status1.SetPayload(kUrl1, std::string(kPayload1));
  bad_status1.SetPayload(kUrl2, std::string(kPayload2));
  bad_status1.SetPayload(kUrl3, std::string(kPayload3));

  turbo::Status bad_status2(turbo::StatusCode::kInternal, "fail");
  bad_status2.SetPayload(kUrl2, std::string(kPayload2));
  bad_status2.SetPayload(kUrl3, std::string(kPayload3));
  bad_status2.SetPayload(kUrl1, std::string(kPayload1));

  EXPECT_EQ(bad_status1, bad_status2);
}

TEST(Status, TestComparePayloadsAfterErase) {
  turbo::Status payload_status(turbo::StatusCode::kInternal, "");
  payload_status.SetPayload(kUrl1, std::string(kPayload1));
  payload_status.SetPayload(kUrl2, std::string(kPayload2));

  turbo::Status empty_status(turbo::StatusCode::kInternal, "");

  // Different payloads, not equal
  EXPECT_NE(payload_status, empty_status);
  EXPECT_TRUE(payload_status.ErasePayload(kUrl1));

  // Still Different payloads, still not equal.
  EXPECT_NE(payload_status, empty_status);
  EXPECT_TRUE(payload_status.ErasePayload(kUrl2));

  // Both empty payloads, should be equal
  EXPECT_EQ(payload_status, empty_status);
}

PayloadsVec AllVisitedPayloads(const turbo::Status& s) {
  PayloadsVec result;

  s.ForEachPayload([&](std::string_view type_url, const std::string& payload) {
    result.push_back(std::make_pair(std::string(type_url), payload));
  });

  return result;
}

TEST(Status, TestForEachPayload) {
  turbo::Status bad_status(turbo::StatusCode::kInternal, "fail");
  bad_status.SetPayload(kUrl1, std::string(kPayload1));
  bad_status.SetPayload(kUrl2, std::string(kPayload2));
  bad_status.SetPayload(kUrl3, std::string(kPayload3));

  int count = 0;

  bad_status.ForEachPayload(
      [&count](std::string_view, const std::string&) { ++count; });

  EXPECT_EQ(count, 3);

  PayloadsVec expected_payloads = {{kUrl1, std::string(kPayload1)},
                                   {kUrl2, std::string(kPayload2)},
                                   {kUrl3, std::string(kPayload3)}};

  // Test that we visit all the payloads in the status.
  PayloadsVec visited_payloads = AllVisitedPayloads(bad_status);
  EXPECT_THAT(visited_payloads, UnorderedElementsAreArray(expected_payloads));

  // Test that visitation order is not consistent between run.
  std::vector<turbo::Status> scratch;
  while (true) {
    scratch.emplace_back(turbo::StatusCode::kInternal, "fail");

    scratch.back().SetPayload(kUrl1, std::string(kPayload1));
    scratch.back().SetPayload(kUrl2, std::string(kPayload2));
    scratch.back().SetPayload(kUrl3, std::string(kPayload3));

    if (AllVisitedPayloads(scratch.back()) != visited_payloads) {
      break;
    }
  }
}

TEST(Status, ToString) {
  turbo::Status status(turbo::StatusCode::kInternal, "fail");
  EXPECT_EQ("INTERNAL: fail", status.ToString());
  status.SetPayload("foo", std::string("bar"));
  EXPECT_EQ("INTERNAL: fail [foo='bar']", status.ToString());
  status.SetPayload("bar", std::string("\377"));
  EXPECT_THAT(status.ToString(),
              AllOf(HasSubstr("INTERNAL: fail"), HasSubstr("[foo='bar']"),
                    HasSubstr("[bar='\\xff']")));
}

TEST(Status, ToStringMode) {
  turbo::Status status(turbo::StatusCode::kInternal, "fail");
  status.SetPayload("foo", std::string("bar"));
  status.SetPayload("bar", std::string("\377"));

  EXPECT_EQ("INTERNAL: fail",
            status.ToString(turbo::StatusToStringMode::kWithNoExtraData));

  EXPECT_THAT(status.ToString(turbo::StatusToStringMode::kWithPayload),
              AllOf(HasSubstr("INTERNAL: fail"), HasSubstr("[foo='bar']"),
                    HasSubstr("[bar='\\xff']")));

  EXPECT_THAT(status.ToString(turbo::StatusToStringMode::kWithEverything),
              AllOf(HasSubstr("INTERNAL: fail"), HasSubstr("[foo='bar']"),
                    HasSubstr("[bar='\\xff']")));

  EXPECT_THAT(status.ToString(~turbo::StatusToStringMode::kWithPayload),
              AllOf(HasSubstr("INTERNAL: fail"), Not(HasSubstr("[foo='bar']")),
                    Not(HasSubstr("[bar='\\xff']"))));
}

TEST(Status, OstreamOperator) {
  turbo::Status status(turbo::StatusCode::kInternal, "fail");
  { std::stringstream stream;
    stream << status;
    EXPECT_THAT(stream.str(),
                AllOf(HasSubstr("INTERNAL: fail"),
                      HasSubstr("status_test.cc:")));
  }
  status.SetPayload("foo", std::string("bar"));
  { std::stringstream stream;
    stream << status;
    EXPECT_THAT(stream.str(),
                AllOf(HasSubstr("INTERNAL: fail"), HasSubstr("[foo='bar']"),
                      HasSubstr("status_test.cc:")));
  }
  status.SetPayload("bar", std::string("\377"));
  { std::stringstream stream;
    stream << status;
    EXPECT_THAT(stream.str(),
                AllOf(HasSubstr("INTERNAL: fail"), HasSubstr("[foo='bar']"),
                      HasSubstr("[bar='\\xff']"),
                      HasSubstr("status_test.cc:")));
  }
}

TEST(Status, turbo_stringify) {
  turbo::Status status(turbo::StatusCode::kInternal, "fail");
  EXPECT_THAT(turbo::StrCat(status),
              AllOf(HasSubstr("INTERNAL: fail"),
                    HasSubstr("status_test.cc:")));
  EXPECT_THAT(turbo::str_sprintf("%v", status),
              AllOf(HasSubstr("INTERNAL: fail"),
                    HasSubstr("status_test.cc:")));
  EXPECT_EQ(turbo::StrCat(status), turbo::str_sprintf("%v", status));
  status.SetPayload("foo", std::string("bar"));
  EXPECT_THAT(turbo::StrCat(status),
              AllOf(HasSubstr("INTERNAL: fail"), HasSubstr("[foo='bar']"),
                    HasSubstr("status_test.cc:")));
  status.SetPayload("bar", std::string("\377"));
  EXPECT_THAT(turbo::StrCat(status),
              AllOf(HasSubstr("INTERNAL: fail"), HasSubstr("[foo='bar']"),
                    HasSubstr("[bar='\\xff']"),
                    HasSubstr("status_test.cc:")));
}

TEST(Status, OstreamEqStringify) {
  turbo::Status status(turbo::StatusCode::kUnknown, "fail");
  status.SetPayload("foo", std::string("bar"));
  std::stringstream stream;
  stream << status;
  EXPECT_EQ(stream.str(), turbo::StrCat(status));
}

turbo::Status EraseAndReturn(const turbo::Status& base) {
  turbo::Status copy = base;
  EXPECT_TRUE(copy.ErasePayload(kUrl1));
  return copy;
}

TEST(Status, CopyOnWriteForErasePayload) {
  {
    turbo::Status base(turbo::StatusCode::kInvalidArgument, "fail");
    base.SetPayload(kUrl1, std::string(kPayload1));
    EXPECT_TRUE(base.GetPayload(kUrl1).has_value());
    turbo::Status copy = EraseAndReturn(base);
    EXPECT_TRUE(base.GetPayload(kUrl1).has_value());
    EXPECT_FALSE(copy.GetPayload(kUrl1).has_value());
  }
  {
    turbo::Status base(turbo::StatusCode::kInvalidArgument, "fail");
    base.SetPayload(kUrl1, std::string(kPayload1));
    turbo::Status copy = base;

    EXPECT_TRUE(base.GetPayload(kUrl1).has_value());
    EXPECT_TRUE(copy.GetPayload(kUrl1).has_value());

    EXPECT_TRUE(base.ErasePayload(kUrl1));

    EXPECT_FALSE(base.GetPayload(kUrl1).has_value());
    EXPECT_TRUE(copy.GetPayload(kUrl1).has_value());
  }
}

TEST(Status, CopyConstructor) {
  {
    turbo::Status status;
    turbo::Status copy(status);
    EXPECT_EQ(copy, status);
  }
  {
    turbo::Status status(turbo::StatusCode::kInvalidArgument, "message");
    turbo::Status copy(status);
    EXPECT_EQ(copy, status);
  }
  {
    turbo::Status status(turbo::StatusCode::kInvalidArgument, "message");
    status.SetPayload(kUrl1, std::string(kPayload1));
    turbo::Status copy(status);
    EXPECT_EQ(copy, status);
  }
}

TEST(Status, CopyAssignment) {
  turbo::Status assignee;
  {
    turbo::Status status;
    assignee = status;
    EXPECT_EQ(assignee, status);
  }
  {
    turbo::Status status(turbo::StatusCode::kInvalidArgument, "message");
    assignee = status;
    EXPECT_EQ(assignee, status);
  }
  {
    turbo::Status status(turbo::StatusCode::kInvalidArgument, "message");
    status.SetPayload(kUrl1, std::string(kPayload1));
    assignee = status;
    EXPECT_EQ(assignee, status);
  }
}

TEST(Status, CopyAssignmentIsNotRef) {
  const turbo::Status status_orig(turbo::StatusCode::kInvalidArgument, "message");
  turbo::Status status_copy = status_orig;
  EXPECT_EQ(status_orig, status_copy);
  status_copy.SetPayload(kUrl1, std::string(kPayload1));
  EXPECT_NE(status_orig, status_copy);
}

TEST(Status, MoveConstructor) {
  {
    turbo::Status status;
    turbo::Status copy(turbo::Status{});
    EXPECT_EQ(copy, status);
  }
  {
    turbo::Status status(turbo::StatusCode::kInvalidArgument, "message");
    turbo::Status copy(
        turbo::Status(turbo::StatusCode::kInvalidArgument, "message"));
    EXPECT_EQ(copy, status);
  }
  {
    turbo::Status status(turbo::StatusCode::kInvalidArgument, "message");
    status.SetPayload(kUrl1, std::string(kPayload1));
    turbo::Status copy1(status);
    turbo::Status copy2(std::move(status));
    EXPECT_EQ(copy1, copy2);
  }
}

TEST(Status, MoveAssignment) {
  turbo::Status assignee;
  {
    turbo::Status status;
    assignee = turbo::Status();
    EXPECT_EQ(assignee, status);
  }
  {
    turbo::Status status(turbo::StatusCode::kInvalidArgument, "message");
    assignee = turbo::Status(turbo::StatusCode::kInvalidArgument, "message");
    EXPECT_EQ(assignee, status);
  }
  {
    turbo::Status status(turbo::StatusCode::kInvalidArgument, "message");
    status.SetPayload(kUrl1, std::string(kPayload1));
    turbo::Status copy(status);
    assignee = std::move(status);
    EXPECT_EQ(assignee, copy);
  }
  {
    turbo::Status status(turbo::StatusCode::kInvalidArgument, "message");
    turbo::Status copy(status);
    assignee = static_cast<turbo::Status&&>(status);
    EXPECT_EQ(assignee, copy);
  }
}

TEST(Status, Update) {
  turbo::Status s;
  s.Update(turbo::OkStatus());
  EXPECT_TRUE(s.ok());
  const turbo::Status a(turbo::StatusCode::kCancelled, "message");
  s.Update(a);
  EXPECT_EQ(s, a);
  const turbo::Status b(turbo::StatusCode::kInternal, "other message");
  s.Update(b);
  EXPECT_EQ(s, a);
  s.Update(turbo::OkStatus());
  EXPECT_EQ(s, a);
  EXPECT_FALSE(s.ok());
}

TEST(Status, Equality) {
  turbo::Status ok;
  turbo::Status no_payload = turbo::CancelledError("no payload");
  turbo::Status one_payload = turbo::InvalidArgumentError("one payload");
  one_payload.SetPayload(kUrl1, std::string(kPayload1));
  turbo::Status two_payloads = one_payload;
  two_payloads.SetPayload(kUrl2, std::string(kPayload2));
  const std::array<turbo::Status, 4> status_arr = {ok, no_payload, one_payload,
                                                  two_payloads};
  for (int i = 0; i < status_arr.size(); i++) {
    for (int j = 0; j < status_arr.size(); j++) {
      if (i == j) {
        EXPECT_TRUE(status_arr[i] == status_arr[j]);
        EXPECT_FALSE(status_arr[i] != status_arr[j]);
      } else {
        EXPECT_TRUE(status_arr[i] != status_arr[j]);
        EXPECT_FALSE(status_arr[i] == status_arr[j]);
      }
    }
  }
}

TEST(Status, Swap) {
  auto test_swap = [](const turbo::Status& s1, const turbo::Status& s2) {
    turbo::Status copy1 = s1, copy2 = s2;
    swap(copy1, copy2);
    EXPECT_EQ(copy1, s2);
    EXPECT_EQ(copy2, s1);
  };
  const turbo::Status ok;
  const turbo::Status no_payload(turbo::StatusCode::kAlreadyExists, "no payload");
  turbo::Status with_payload(turbo::StatusCode::kInternal, "with payload");
  with_payload.SetPayload(kUrl1, std::string(kPayload1));
  test_swap(ok, no_payload);
  test_swap(no_payload, ok);
  test_swap(ok, with_payload);
  test_swap(with_payload, ok);
  test_swap(no_payload, with_payload);
  test_swap(with_payload, no_payload);
}

TEST(StatusErrno, ErrnoToStatusCode) {
  EXPECT_EQ(turbo::ErrnoToStatusCode(0), turbo::StatusCode::kOk);

  // Spot-check a few errno values.
  EXPECT_EQ(turbo::ErrnoToStatusCode(EINVAL),
            turbo::StatusCode::kInvalidArgument);
  EXPECT_EQ(turbo::ErrnoToStatusCode(ENOENT), turbo::StatusCode::kNotFound);

  // We'll pick a very large number so it hopefully doesn't collide to errno.
  EXPECT_EQ(turbo::ErrnoToStatusCode(19980927), turbo::StatusCode::kUnknown);
}

TEST(StatusErrno, ErrnoToStatus) {
  turbo::Status status = turbo::ErrnoToStatus(ENOENT, "Cannot open 'path'");
  EXPECT_EQ(status.code(), turbo::StatusCode::kNotFound);
  EXPECT_EQ(status.message(), "Cannot open 'path': No such file or directory");
}

#if KUMO_HAVE_BUILTIN_LINE_FILE
#define GET_SOURCE_LOCATION(offset) __builtin_LINE() - offset
#else
#define GET_SOURCE_LOCATION(offset) 1
#endif

void CheckSourceLocation(
    const turbo::Status& status, std::vector<int> lines = {},
    std::source_location loc = std::source_location::current()) {
  ASSERT_EQ(status.GetSourceLocations().size(), lines.size())
      << "Size check failed at " << loc.line();
  for (size_t i = 0; i < lines.size(); ++i) {
    EXPECT_EQ(std::string_view(status.GetSourceLocations()[i].file_name()),
              std::string_view(loc.file_name()))
        << "File name check failed at " << loc.line();
    EXPECT_EQ(status.GetSourceLocations()[i].line(), lines[i])
        << "Line check failed at " << loc.line();
  }
}

TEST(Status, ConstructorCheckSourceLocation) {
  {
    const turbo::Status a;
    const turbo::Status b = a;
    for (const turbo::Status& status : {a, b}) {
      EXPECT_TRUE(status.ok());
      EXPECT_EQ(turbo::StatusCode::kOk, status.code());
      CheckSourceLocation(status);
    }
  }
  {
    const turbo::Status a(turbo::StatusCode::kInternal, "message",
                         std::source_location::current());
    int line = GET_SOURCE_LOCATION(1);
    const turbo::Status b = a;
    for (const turbo::Status& status : {a, b}) {
      EXPECT_FALSE(status.ok());
      EXPECT_EQ(turbo::StatusCode::kInternal, status.code());
      CheckSourceLocation(status, {line});
    }
  }
  {
    const turbo::Status a(turbo::StatusCode::kInternal, "message",
                         std::source_location());
    const turbo::Status b = a;
    for (const turbo::Status& status : {a, b}) {
      EXPECT_FALSE(status.ok());
      EXPECT_EQ(turbo::StatusCode::kInternal, status.code());
      CheckSourceLocation(status);
    }
  }
  {
    const turbo::Status a(turbo::StatusCode::kInternal, "",
                         std::source_location::current());
    const turbo::Status b = a;
    for (const turbo::Status& status : {a, b}) {
      EXPECT_FALSE(status.ok());
      EXPECT_EQ(turbo::StatusCode::kInternal, status.code());
      CheckSourceLocation(status);
    }
  }
  {
    const turbo::Status a(turbo::StatusCode::kInternal, "",
                         std::source_location());
    const turbo::Status b = a;
    for (const turbo::Status& status : {a, b}) {
      EXPECT_FALSE(status.ok());
      EXPECT_EQ(turbo::StatusCode::kInternal, status.code());
      CheckSourceLocation(status);
    }
  }
}

TEST(Status, SourceLocationConstructor) {
  {
    // OK status doesn't save source locations.
    const turbo::Status original;
    const turbo::Status status(original, std::source_location());
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(turbo::StatusCode::kOk, status.code());
    CheckSourceLocation(status);
  }
  {
    // OK status doesn't save source locations.
    const turbo::Status original;
    const turbo::Status status(original, std::source_location::current());
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(turbo::StatusCode::kOk, status.code());
    CheckSourceLocation(status);
  }
  {
    // Non-ok Status with non-empty msg can save source locations with
    // non-nullptr filename.
    const turbo::Status original(turbo::StatusCode::kInternal, "message",
                                std::source_location::current());
    int line = GET_SOURCE_LOCATION(1);
    // Default std::source_location cannot be saved into the chain.
    const turbo::Status status(original, std::source_location());
    EXPECT_FALSE(status.ok());
    EXPECT_EQ(turbo::StatusCode::kInternal, status.code());
    CheckSourceLocation(status, {line});
  }
  {
    const turbo::Status original(turbo::StatusCode::kInternal, "message",
                                std::source_location::current());
    int line = GET_SOURCE_LOCATION(1);

    const turbo::Status status(original, std::source_location::current());
    int line2 = GET_SOURCE_LOCATION(1);

    EXPECT_FALSE(status.ok());
    EXPECT_EQ(turbo::StatusCode::kInternal, status.code());
    CheckSourceLocation(status, {line, line2});
  }
  {
    // Non-OK status with empty msg doesn't save source locations.
    const turbo::Status original(turbo::StatusCode::kInternal, "",
                                std::source_location::current());
    const turbo::Status status(original, std::source_location());
    EXPECT_FALSE(status.ok());
    EXPECT_EQ(turbo::StatusCode::kInternal, status.code());
    CheckSourceLocation(status);
  }
  {
    // Non-OK status with empty msg doesn't save source locations.
    const turbo::Status original(turbo::StatusCode::kInternal, "",
                                std::source_location::current());
    const turbo::Status status(original, std::source_location::current());
    EXPECT_FALSE(status.ok());
    EXPECT_EQ(turbo::StatusCode::kInternal, status.code());
    CheckSourceLocation(status);
  }
  {
    // Non-OK status with empty msg doesn't save source locations from default
    // constructor.
    const turbo::Status original(turbo::StatusCode::kInternal, "",
                                std::source_location());
    const turbo::Status status(original, std::source_location());
    EXPECT_FALSE(status.ok());
    EXPECT_EQ(turbo::StatusCode::kInternal, status.code());
    CheckSourceLocation(status);
  }
  {
    // Non-OK status with empty msg doesn't save source locations.
    const turbo::Status original(turbo::StatusCode::kInternal, "",
                                std::source_location());
    const turbo::Status status(original, std::source_location::current());
    EXPECT_FALSE(status.ok());
    EXPECT_EQ(turbo::StatusCode::kInternal, status.code());
    CheckSourceLocation(status);
  }
  {
    const turbo::Status original(turbo::StatusCode::kInternal, "message",
                                std::source_location());
    const turbo::Status status(original, std::source_location());
    EXPECT_FALSE(status.ok());
    EXPECT_EQ(turbo::StatusCode::kInternal, status.code());
    CheckSourceLocation(status);
  }
  {
    const turbo::Status original(turbo::StatusCode::kInternal, "message",
                                std::source_location());
    const turbo::Status status(original, std::source_location::current());
    int line = GET_SOURCE_LOCATION(1);
    EXPECT_FALSE(status.ok());
    EXPECT_EQ(turbo::StatusCode::kInternal, status.code());
    CheckSourceLocation(status, {line});
  }
}

TEST(Status, SourceLocationWithMoveConstructor) {
  {
    // OK status doesn't save source locations.
    turbo::Status original;
    const turbo::Status status(std::move(original), std::source_location());
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(turbo::StatusCode::kOk, status.code());
    CheckSourceLocation(status);
  }
  {
    // OK status doesn't save source locations.
    turbo::Status original;
    const turbo::Status status(std::move(original),
                              std::source_location::current());
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(turbo::StatusCode::kOk, status.code());
    CheckSourceLocation(status);
  }
  {
    // Non-ok Status with non-empty msg can save source locations with
    // non-nullptr filename.
    turbo::Status original(turbo::StatusCode::kInternal, "message",
                          std::source_location::current());
    int line = GET_SOURCE_LOCATION(1);
    // Default std::source_location cannot be saved into the chain.
    const turbo::Status status(std::move(original), std::source_location());
    EXPECT_FALSE(status.ok());
    EXPECT_EQ(turbo::StatusCode::kInternal, status.code());
    CheckSourceLocation(status, {line});
  }
  {
    turbo::Status original(turbo::StatusCode::kInternal, "message",
                          std::source_location::current());
    int line = GET_SOURCE_LOCATION(1);

    const turbo::Status status(std::move(original),
                              std::source_location::current());
    int line2 = GET_SOURCE_LOCATION(1);

    EXPECT_FALSE(status.ok());
    EXPECT_EQ(turbo::StatusCode::kInternal, status.code());
    CheckSourceLocation(status, {line, line2});
  }
  {
    // Non-OK status with empty msg doesn't save source locations.
    turbo::Status original(turbo::StatusCode::kInternal, "",
                          std::source_location::current());
    const turbo::Status status(std::move(original), std::source_location());
    EXPECT_FALSE(status.ok());
    EXPECT_EQ(turbo::StatusCode::kInternal, status.code());
    CheckSourceLocation(status);
  }
  {
    // Non-OK status with empty msg doesn't save source locations.
    turbo::Status original(turbo::StatusCode::kInternal, "",
                          std::source_location::current());
    const turbo::Status status(std::move(original),
                              std::source_location::current());
    EXPECT_FALSE(status.ok());
    EXPECT_EQ(turbo::StatusCode::kInternal, status.code());
    CheckSourceLocation(status);
  }
  {
    // Non-OK status with empty msg doesn't save source locations from default
    // constructor.
    turbo::Status original(turbo::StatusCode::kInternal, "",
                          std::source_location());
    const turbo::Status status(std::move(original), std::source_location());
    EXPECT_FALSE(status.ok());
    EXPECT_EQ(turbo::StatusCode::kInternal, status.code());
    CheckSourceLocation(status);
  }
  {
    // Non-OK status with empty msg doesn't save source locations.
    turbo::Status original(turbo::StatusCode::kInternal, "",
                          std::source_location());
    const turbo::Status status(std::move(original),
                              std::source_location::current());
    EXPECT_FALSE(status.ok());
    EXPECT_EQ(turbo::StatusCode::kInternal, status.code());
    CheckSourceLocation(status);
  }
  {
    turbo::Status original(turbo::StatusCode::kInternal, "message",
                          std::source_location());
    const turbo::Status status(std::move(original), std::source_location());
    EXPECT_FALSE(status.ok());
    EXPECT_EQ(turbo::StatusCode::kInternal, status.code());
    CheckSourceLocation(status);
  }
  {
    turbo::Status original(turbo::StatusCode::kInternal, "message",
                          std::source_location());
    const turbo::Status status(std::move(original),
                              std::source_location::current());
    int line = GET_SOURCE_LOCATION(1);
    EXPECT_FALSE(status.ok());
    EXPECT_EQ(turbo::StatusCode::kInternal, status.code());
    CheckSourceLocation(status, {line});
  }
}

TEST(Status, AddSourceLocation) {
  int max_iter = 10;
  {
    // Status that ignores source location.
    turbo::Status status_ignores_source_location[] = {
        turbo::Status(),
        turbo::Status(turbo::StatusCode::kInternal, "")};
    for (turbo::Status& s : status_ignores_source_location) {
      for (int i = 0; i < max_iter; ++i) {
        s.AddSourceLocation(std::source_location::current());
        s.AddSourceLocation(std::source_location());
      }
      CheckSourceLocation(s);
    }
  }
  {
    // Default std::source_location is not added.
    turbo::Status status(turbo::StatusCode::kInternal, "foo",
                        std::source_location::current());
    int line = GET_SOURCE_LOCATION(1);
    for (int i = 0; i < max_iter; ++i) {
      status.AddSourceLocation(std::source_location());
    }
    CheckSourceLocation(status, {line});
  }
  {
    // Default std::source_location is not added.
    turbo::Status status(turbo::StatusCode::kInternal, "foo",
                        std::source_location::current());
    int line = GET_SOURCE_LOCATION(1);
    std::vector<int> lines = {line};
    lines.reserve(1 + max_iter);
    for (int i = 0; i < max_iter; ++i) {
      status.AddSourceLocation(std::source_location::current());
      lines.push_back(GET_SOURCE_LOCATION(1));
    }
    CheckSourceLocation(status, lines);
  }
}

TEST(Status, WithSourceLocationCopy) {
  turbo::Status original(turbo::StatusCode::kInternal, "message",
                        std::source_location::current());
  int line = GET_SOURCE_LOCATION(1);

  const turbo::Status status =
      original.WithSourceLocation(std::source_location::current());
  int line2 = GET_SOURCE_LOCATION(1);

  CheckSourceLocation(original, {line});
  CheckSourceLocation(status, {line, line2});
  EXPECT_EQ(original, status);
}

turbo::Status&& IsRvalueStatus(turbo::Status&& s) { return std::move(s); }

TEST(Status, WithSourceLocationMove) {
  turbo::Status original(turbo::StatusCode::kInternal, "message",
                        std::source_location::current());
  int line = GET_SOURCE_LOCATION(1);

  const turbo::Status status = IsRvalueStatus(
      std::move(original).WithSourceLocation(std::source_location::current()));
  int line2 = GET_SOURCE_LOCATION(1);

  CheckSourceLocation(status, {line, line2});
  EXPECT_FALSE(status.ok());
  EXPECT_EQ(turbo::StatusCode::kInternal, status.code());
}

TEST(Status, CopyOnWriteSourceLocations) {
  turbo::Status source(turbo::StatusCode::kInvalidArgument, "fail",
                      std::source_location::current());
  EXPECT_EQ(source.GetSourceLocations().size(), 1);
  turbo::Status copy = source;
  EXPECT_EQ(copy.GetSourceLocations().size(), 1);
  copy.AddSourceLocation(std::source_location::current());  // Copy rep.
  EXPECT_EQ(copy.GetSourceLocations().size(), 2);
  EXPECT_EQ(source.GetSourceLocations().size(), 1);
}

TEST(Status, SourceLocationToStringMode) {
  turbo::Status s(turbo::StatusCode::kInternal, "fail",
                 std::source_location::current());
  int line = GET_SOURCE_LOCATION(1);
  std::string source_location_string = "\n=== Source Location Trace: ===";
  std::string source_location_stack = turbo::StrCat(
      std::source_location::current().file_name(), ":", line, "\n");

  s.SetPayload("foo", std::string("bar"));

  EXPECT_EQ("INTERNAL: fail",
            s.ToString(turbo::StatusToStringMode::kWithNoExtraData));

  EXPECT_EQ("INTERNAL: fail",
            s.ToString(~turbo::StatusToStringMode::kWithSourceLocation &
                       ~turbo::StatusToStringMode::kWithPayload));
  EXPECT_THAT(s.ToString(turbo::StatusToStringMode::kWithSourceLocation |
                         turbo::StatusToStringMode::kWithPayload),
              AllOf(HasSubstr("INTERNAL: fail [foo='bar']"),
                    HasSubstr(source_location_string),
                    HasSubstr(source_location_stack)));

  s.SetPayload("bar", std::string("\377"));

  EXPECT_THAT(s.ToString(turbo::StatusToStringMode::kWithEverything),
              AllOf(HasSubstr("INTERNAL: fail"), HasSubstr("[foo='bar']"),
                    HasSubstr("[bar='\\xff']"),
                    HasSubstr(source_location_string),
                    HasSubstr(source_location_stack)));
  EXPECT_THAT(s.ToString(turbo::StatusToStringMode::kWithPayload |
                         turbo::StatusToStringMode::kWithSourceLocation),
              AllOf(HasSubstr("INTERNAL: fail"), HasSubstr("[foo='bar']"),
                    HasSubstr("[bar='\\xff']"),
                    HasSubstr(source_location_string),
                    HasSubstr(source_location_stack)));
  EXPECT_THAT(s.ToString(turbo::StatusToStringMode::kWithSourceLocation),
              AllOf(HasSubstr("INTERNAL: fail"), Not(HasSubstr("[foo='bar']")),
                    Not(HasSubstr("[bar='\\xff']")),
                    HasSubstr(source_location_string),
                    HasSubstr(source_location_stack)));
  EXPECT_THAT(s.ToString(turbo::StatusToStringMode::kWithPayload),
              AllOf(HasSubstr("INTERNAL: fail"), HasSubstr("[foo='bar']"),
                    HasSubstr("[bar='\\xff']"),
                    Not(HasSubstr(source_location_string)),
                    Not(HasSubstr(source_location_stack))));
}

TEST(Status, StackTracePayloadOverflow) {
  // Stack must have the same layout as status_internal::StackTracePayload.
  struct Stack {
    size_t size;
    void* frames[20];
  } stack;
  stack.size = 200;  // Overflows frames.

  turbo::Status status = turbo::CancelledError();
  status.SetPayload("TurboStatusStackTracePayload",
                    std::string(std::string_view(
                        reinterpret_cast<const char*>(&stack), sizeof(stack))));

  // An unchecked overflow should be detected by ASAN/MSAN on the next line.
  static_cast<void>(status.ToString(turbo::StatusToStringMode::kWithEverything));
}

}  // namespace
