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
#include <string_view>
#include <turbo/cord/cord.h>
#include <turbo/format/str_format.h>
#include <turbo/strings/str_cat.h>
#include <turbo/types/source_location.h>

namespace {

    using ::testing::Eq;
    using ::testing::HasSubstr;
    using ::testing::Optional;
    using ::testing::UnorderedElementsAreArray;

    TEST(StatusCode, InsertionOperator) {
        const turbo::StatusCode code = turbo::StatusCode::kUnknown;
        std::ostringstream oss;
        oss << code;
        EXPECT_EQ(oss.str(), turbo::status_code_to_string(code));
        EXPECT_EQ(oss.str(), turbo::status_code_to_string_view(code));
    }

    // This structure holds the details for testing a single error code,
    // its creator, and its classifier.
    struct ErrorTest {
        turbo::StatusCode code;
        using Creator = turbo::Status (*)(std::string_view, turbo::SourceLocation);
        using Classifier = bool (*)(const turbo::Status&);
        Creator creator;
        Classifier classifier;
    };

    constexpr ErrorTest kErrorTests[] {
        { turbo::StatusCode::kCancelled, turbo::cancelled_error, turbo::is_cancelled },
        { turbo::StatusCode::kUnknown, turbo::unknown_error, turbo::is_unknown },
        { turbo::StatusCode::kInvalidArgument, turbo::invalid_argument_error,
            turbo::is_invalid_argument },
        { turbo::StatusCode::kDeadlineExceeded, turbo::deadline_exceeded_error,
            turbo::is_deadline_exceeded },
        { turbo::StatusCode::kNotFound, turbo::not_found_error, turbo::is_not_found },
        { turbo::StatusCode::kAlreadyExists, turbo::already_exists_error,
            turbo::is_already_exists },
        { turbo::StatusCode::kPermissionDenied, turbo::permission_denied_error,
            turbo::is_permission_denied },
        { turbo::StatusCode::kResourceExhausted, turbo::resource_exhausted_error,
            turbo::is_resource_exhausted },
        { turbo::StatusCode::kFailedPrecondition, turbo::failed_precondition_error,
            turbo::is_failed_precondition },
        { turbo::StatusCode::kAborted, turbo::aborted_error, turbo::is_aborted },
        { turbo::StatusCode::kOutOfRange, turbo::outOf_range_error, turbo::is_out_of_range },
        { turbo::StatusCode::kUnimplemented, turbo::unimplemented_error,
            turbo::is_unimplemented },
        { turbo::StatusCode::kInternal, turbo::internal_error, turbo::is_internal },
        { turbo::StatusCode::kUnavailable, turbo::unavailable_error,
            turbo::is_unavailable },
        { turbo::StatusCode::kDataLoss, turbo::data_loss_error, turbo::is_data_loss },
        { turbo::StatusCode::kUnauthenticated, turbo::unauthenticated_error,
            turbo::is_unauthenticated },
    };

    TEST(Status, CreateAndClassify) {
        for (const auto& test : kErrorTests) {
            SCOPED_TRACE(turbo::status_code_to_string(test.code));

            // Ensure that the creator does, in fact, create status objects with the
            // expected error code and message.
            std::string message = turbo::str_cat("error code ", test.code, " test message");
            turbo::Status status = test.creator(message, turbo::SourceLocation::current());
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

    TEST(Status, ok_status) {
        turbo::Status status = turbo::ok_status();
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
            turbo::Status status = turbo::ok_status();
            EXPECT_EQ(status.message(), "");
            EXPECT_STREQ(turbo::status_message_as_cstr(status), "");
            EXPECT_EQ(status.message(), turbo::status_message_as_cstr(status));
            EXPECT_NE(turbo::status_message_as_cstr(status), nullptr);
        }
        {
            turbo::Status status;
            EXPECT_EQ(status.message(), "");
            EXPECT_NE(turbo::status_message_as_cstr(status), nullptr);
            EXPECT_STREQ(turbo::status_message_as_cstr(status), "");
        }
        {
            turbo::Status status(turbo::StatusCode::kInternal, "message");
            EXPECT_FALSE(status.ok());
            EXPECT_EQ(turbo::StatusCode::kInternal, status.code());
            EXPECT_EQ("message", status.message());
            EXPECT_STREQ("message", turbo::status_message_as_cstr(status));
        }
    }

    TEST(Status, ConstructOutOfRangeCode) {
        // StatusCode is uint8_t; values outside 0..255 pack as the low 8 bits.
        const int kRawCode = 9999;
        turbo::Status status(static_cast<turbo::StatusCode>(kRawCode), "");
        EXPECT_EQ(15, status.raw_code());
        EXPECT_EQ(turbo::StatusCode::kDataLoss, status.code());

        // 17 is in range of the byte but not a canonical code → kUnknown.
        turbo::Status unknown(static_cast<turbo::StatusCode>(17), "");
        EXPECT_EQ(17, unknown.raw_code());
        EXPECT_EQ(turbo::StatusCode::kUnknown, unknown.code());
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
        turbo::Status ok_status = turbo::ok_status();
        ok_status.set_payload(kUrl1, std::string(kPayload1));
        ok_status.set_payload(kUrl2, std::string(kPayload2));

        EXPECT_FALSE(ok_status.get_payload(kUrl1));
        EXPECT_FALSE(ok_status.get_payload(kUrl2));

        turbo::Status bad_status(turbo::StatusCode::kInternal, "fail");
        bad_status.set_payload(kUrl1, std::string(kPayload1));
        bad_status.set_payload(kUrl2, std::string(kPayload2));

        EXPECT_THAT(bad_status.get_payload(kUrl1), Optional(Eq(kPayload1)));
        EXPECT_THAT(bad_status.get_payload(kUrl2), Optional(Eq(kPayload2)));

        EXPECT_FALSE(bad_status.get_payload(kUrl3));

        bad_status.set_payload(kUrl1, std::string(kPayload3));
        EXPECT_THAT(bad_status.get_payload(kUrl1), Optional(Eq(kPayload3)));

        // Testing dynamically generated type_url
        bad_status.set_payload(turbo::str_cat(kUrl1, ".1"), std::string(kPayload1));
        EXPECT_THAT(bad_status.get_payload(turbo::str_cat(kUrl1, ".1")),
            Optional(Eq(kPayload1)));
    }

    TEST(Status, TestErasePayload) {
        turbo::Status bad_status(turbo::StatusCode::kInternal, "fail");
        bad_status.set_payload(kUrl1, std::string(kPayload1));
        bad_status.set_payload(kUrl2, std::string(kPayload2));
        bad_status.set_payload(kUrl3, std::string(kPayload3));

        EXPECT_FALSE(bad_status.erase_payload(kUrl4));

        EXPECT_TRUE(bad_status.get_payload(kUrl2));
        EXPECT_TRUE(bad_status.erase_payload(kUrl2));
        EXPECT_FALSE(bad_status.get_payload(kUrl2));
        EXPECT_FALSE(bad_status.erase_payload(kUrl2));

        EXPECT_TRUE(bad_status.erase_payload(kUrl1));
        EXPECT_TRUE(bad_status.erase_payload(kUrl3));

        bad_status.set_payload(kUrl1, std::string(kPayload1));
        EXPECT_TRUE(bad_status.erase_payload(kUrl1));
    }

    TEST(Status, TestComparePayloads) {
        turbo::Status bad_status1(turbo::StatusCode::kInternal, "fail");
        bad_status1.set_payload(kUrl1, std::string(kPayload1));
        bad_status1.set_payload(kUrl2, std::string(kPayload2));
        bad_status1.set_payload(kUrl3, std::string(kPayload3));

        turbo::Status bad_status2(turbo::StatusCode::kInternal, "fail");
        bad_status2.set_payload(kUrl2, std::string(kPayload2));
        bad_status2.set_payload(kUrl3, std::string(kPayload3));
        bad_status2.set_payload(kUrl1, std::string(kPayload1));

        EXPECT_EQ(bad_status1, bad_status2);
    }

    TEST(Status, TestComparePayloadsAfterErase) {
        turbo::Status payload_status(turbo::StatusCode::kInternal, "");
        payload_status.set_payload(kUrl1, std::string(kPayload1));
        payload_status.set_payload(kUrl2, std::string(kPayload2));

        turbo::Status empty_status(turbo::StatusCode::kInternal, "");

        // Different payloads, not equal
        EXPECT_NE(payload_status, empty_status);
        EXPECT_TRUE(payload_status.erase_payload(kUrl1));

        // Still Different payloads, still not equal.
        EXPECT_NE(payload_status, empty_status);
        EXPECT_TRUE(payload_status.erase_payload(kUrl2));

        // Both empty payloads, should be equal
        EXPECT_EQ(payload_status, empty_status);
    }

    PayloadsVec AllVisitedPayloads(const turbo::Status& s) {
        PayloadsVec result;

        s.for_each_payload([&](std::string_view type_url, const std::string& payload) {
            result.push_back(std::make_pair(std::string(type_url), payload));
        });

        return result;
    }

    TEST(Status, TestForEachPayload) {
        turbo::Status bad_status(turbo::StatusCode::kInternal, "fail");
        bad_status.set_payload(kUrl1, std::string(kPayload1));
        bad_status.set_payload(kUrl2, std::string(kPayload2));
        bad_status.set_payload(kUrl3, std::string(kPayload3));

        int count = 0;

        bad_status.for_each_payload(
            [&count](std::string_view, const std::string&) { ++count; });

        EXPECT_EQ(count, 3);

        PayloadsVec expected_payloads = { { kUrl1, std::string(kPayload1) },
            { kUrl2, std::string(kPayload2) },
            { kUrl3, std::string(kPayload3) } };

        // Test that we visit all the payloads in the status.
        PayloadsVec visited_payloads = AllVisitedPayloads(bad_status);
        EXPECT_THAT(visited_payloads, UnorderedElementsAreArray(expected_payloads));

        // Test that visitation order is not consistent between run.
        std::vector<turbo::Status> scratch;
        while (true) {
            scratch.emplace_back(turbo::StatusCode::kInternal, "fail");

            scratch.back().set_payload(kUrl1, std::string(kPayload1));
            scratch.back().set_payload(kUrl2, std::string(kPayload2));
            scratch.back().set_payload(kUrl3, std::string(kPayload3));

            if (AllVisitedPayloads(scratch.back()) != visited_payloads) {
                break;
            }
        }
    }

    TEST(Status, ToString) {
        turbo::Status status(turbo::StatusCode::kInternal, "fail");
        EXPECT_EQ("INTERNAL(s0:0): fail", status.ToString());
        status.set_payload("foo", std::string("bar"));
        EXPECT_EQ("INTERNAL(s0:0): fail [foo='bar']", status.ToString());
        status.set_payload("bar", std::string("\377"));
        EXPECT_THAT(status.ToString(),
            AllOf(HasSubstr("INTERNAL(s0:0): fail"), HasSubstr("[foo='bar']"),
                HasSubstr("[bar='\\xff']")));
    }

    TEST(Status, ToStringMode) {
        turbo::Status status(turbo::StatusCode::kInternal, "fail");
        status.set_payload("foo", std::string("bar"));
        status.set_payload("bar", std::string("\377"));

        EXPECT_EQ("INTERNAL(s0:0): fail",
            status.ToString(turbo::StatusToStringMode::kWithNoExtraData));

        EXPECT_THAT(status.ToString(turbo::StatusToStringMode::kWithPayload),
            AllOf(HasSubstr("INTERNAL(s0:0): fail"), HasSubstr("[foo='bar']"),
                HasSubstr("[bar='\\xff']")));

        EXPECT_THAT(status.ToString(turbo::StatusToStringMode::kWithEverything),
            AllOf(HasSubstr("INTERNAL(s0:0): fail"), HasSubstr("[foo='bar']"),
                HasSubstr("[bar='\\xff']")));

        EXPECT_THAT(status.ToString(~turbo::StatusToStringMode::kWithPayload),
            AllOf(HasSubstr("INTERNAL(s0:0): fail"), Not(HasSubstr("[foo='bar']")),
                Not(HasSubstr("[bar='\\xff']"))));
    }

    TEST(Status, OstreamOperator) {
        turbo::Status status(turbo::StatusCode::kInternal, "fail");
        {
            std::stringstream stream;
            stream << status;
            EXPECT_THAT(stream.str(),
                AllOf(HasSubstr("INTERNAL(s0:0): fail"),
                    HasSubstr("status_test.cc:")));
        }
        status.set_payload("foo", std::string("bar"));
        {
            std::stringstream stream;
            stream << status;
            EXPECT_THAT(stream.str(),
                AllOf(HasSubstr("INTERNAL(s0:0): fail"), HasSubstr("[foo='bar']"),
                    HasSubstr("status_test.cc:")));
        }
        status.set_payload("bar", std::string("\377"));
        {
            std::stringstream stream;
            stream << status;
            EXPECT_THAT(stream.str(),
                AllOf(HasSubstr("INTERNAL(s0:0): fail"), HasSubstr("[foo='bar']"),
                    HasSubstr("[bar='\\xff']"),
                    HasSubstr("status_test.cc:")));
        }
    }

    TEST(Status, turbo_stringify) {
        turbo::Status status(turbo::StatusCode::kInternal, "fail");
        EXPECT_THAT(turbo::str_cat(status),
            AllOf(HasSubstr("INTERNAL(s0:0): fail"),
                HasSubstr("status_test.cc:")));
        EXPECT_THAT(turbo::str_sprintf("%v", status),
            AllOf(HasSubstr("INTERNAL(s0:0): fail"),
                HasSubstr("status_test.cc:")));
        EXPECT_EQ(turbo::str_cat(status), turbo::str_sprintf("%v", status));
        status.set_payload("foo", std::string("bar"));
        EXPECT_THAT(turbo::str_cat(status),
            AllOf(HasSubstr("INTERNAL(s0:0): fail"), HasSubstr("[foo='bar']"),
                HasSubstr("status_test.cc:")));
        status.set_payload("bar", std::string("\377"));
        EXPECT_THAT(turbo::str_cat(status),
            AllOf(HasSubstr("INTERNAL(s0:0): fail"), HasSubstr("[foo='bar']"),
                HasSubstr("[bar='\\xff']"),
                HasSubstr("status_test.cc:")));
    }

    TEST(Status, OstreamEqStringify) {
        turbo::Status status(turbo::StatusCode::kUnknown, "fail");
        status.set_payload("foo", std::string("bar"));
        std::stringstream stream;
        stream << status;
        EXPECT_EQ(stream.str(), turbo::str_cat(status));
    }

    turbo::Status EraseAndReturn(const turbo::Status& base) {
        turbo::Status copy = base;
        EXPECT_TRUE(copy.erase_payload(kUrl1));
        return copy;
    }

    TEST(Status, CopyOnWriteForErasePayload) {
        {
            turbo::Status base(turbo::StatusCode::kInvalidArgument, "fail");
            base.set_payload(kUrl1, std::string(kPayload1));
            EXPECT_TRUE(base.get_payload(kUrl1).has_value());
            turbo::Status copy = EraseAndReturn(base);
            EXPECT_TRUE(base.get_payload(kUrl1).has_value());
            EXPECT_FALSE(copy.get_payload(kUrl1).has_value());
        }
        {
            turbo::Status base(turbo::StatusCode::kInvalidArgument, "fail");
            base.set_payload(kUrl1, std::string(kPayload1));
            turbo::Status copy = base;

            EXPECT_TRUE(base.get_payload(kUrl1).has_value());
            EXPECT_TRUE(copy.get_payload(kUrl1).has_value());

            EXPECT_TRUE(base.erase_payload(kUrl1));

            EXPECT_FALSE(base.get_payload(kUrl1).has_value());
            EXPECT_TRUE(copy.get_payload(kUrl1).has_value());
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
            status.set_payload(kUrl1, std::string(kPayload1));
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
            status.set_payload(kUrl1, std::string(kPayload1));
            assignee = status;
            EXPECT_EQ(assignee, status);
        }
    }

    TEST(Status, CopyAssignmentIsNotRef) {
        const turbo::Status status_orig(turbo::StatusCode::kInvalidArgument, "message");
        turbo::Status status_copy = status_orig;
        EXPECT_EQ(status_orig, status_copy);
        status_copy.set_payload(kUrl1, std::string(kPayload1));
        EXPECT_NE(status_orig, status_copy);
    }

    TEST(Status, MoveConstructor) {
        {
            turbo::Status status;
            turbo::Status copy(turbo::Status { });
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
            status.set_payload(kUrl1, std::string(kPayload1));
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
            status.set_payload(kUrl1, std::string(kPayload1));
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
        s.Update(turbo::ok_status());
        EXPECT_TRUE(s.ok());
        const turbo::Status a(turbo::StatusCode::kCancelled, "message");
        s.Update(a);
        EXPECT_EQ(s, a);
        const turbo::Status b(turbo::StatusCode::kInternal, "other message");
        s.Update(b);
        EXPECT_EQ(s, a);
        s.Update(turbo::ok_status());
        EXPECT_EQ(s, a);
        EXPECT_FALSE(s.ok());
    }

    TEST(Status, Equality) {
        turbo::Status ok;
        turbo::Status no_payload = turbo::cancelled_error("no payload");
        turbo::Status one_payload = turbo::invalid_argument_error("one payload");
        one_payload.set_payload(kUrl1, std::string(kPayload1));
        turbo::Status two_payloads = one_payload;
        two_payloads.set_payload(kUrl2, std::string(kPayload2));
        const std::array<turbo::Status, 4> status_arr = { ok, no_payload, one_payload,
            two_payloads };
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
        with_payload.set_payload(kUrl1, std::string(kPayload1));
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
  EXPECT_EQ(status.sub_type(), turbo::kSubErrno);
  EXPECT_EQ(status.sub_code(), ENOENT);
  EXPECT_EQ(status.message(), "Cannot open 'path': No such file or directory");
}

TEST(StatusErrno, ErrnoToStatusInlined) {
  turbo::Status status = turbo::ErrnoToStatus(EINVAL);
  EXPECT_EQ(status.code(), turbo::StatusCode::kInvalidArgument);
  EXPECT_EQ(status.sub_type(), turbo::kSubErrno);
  EXPECT_EQ(status.sub_code(), EINVAL);
  EXPECT_TRUE(status.message().empty());
  EXPECT_EQ(status.ToString(),
            turbo::str_cat("INVALID_ARGUMENT(s0:", EINVAL, ")"));
  turbo::InlineStatus inlined = status.inline_status();
  EXPECT_TRUE(inlined.is_inlined);
  EXPECT_EQ(inlined.code, turbo::StatusCode::kInvalidArgument);
  EXPECT_EQ(inlined.type, turbo::kSubErrno);
  EXPECT_EQ(inlined.sub_code, EINVAL);
}

#if KUMO_HAVE_BUILTIN_LINE_FILE
#define GET_SOURCE_LOCATION(offset) __builtin_LINE() - offset
#else
#define GET_SOURCE_LOCATION(offset) 1
#endif

    void CheckSourceLocation(
        const turbo::Status& status, std::vector<int> lines = { },
        turbo::SourceLocation loc = turbo::SourceLocation::current()) {
        ASSERT_EQ(status.get_source_locations().size(), lines.size())
            << "Size check failed at " << loc.line();
        for (size_t i = 0; i < lines.size(); ++i) {
            EXPECT_EQ(std::string_view(status.get_source_locations()[i].file_name()),
                std::string_view(loc.file_name()))
                << "File name check failed at " << loc.line();
            EXPECT_EQ(status.get_source_locations()[i].line(), lines[i])
                << "Line check failed at " << loc.line();
        }
    }

    TEST(Status, ConstructorCheckSourceLocation) {
        {
            const turbo::Status a;
            const turbo::Status b = a;
            for (const turbo::Status& status : { a, b }) {
                EXPECT_TRUE(status.ok());
                EXPECT_EQ(turbo::StatusCode::kOk, status.code());
                CheckSourceLocation(status);
            }
        }
        {
            const turbo::Status a(turbo::StatusCode::kInternal, "message",
                turbo::SourceLocation::current());
            int line = GET_SOURCE_LOCATION(1);
            const turbo::Status b = a;
            for (const turbo::Status& status : { a, b }) {
                EXPECT_FALSE(status.ok());
                EXPECT_EQ(turbo::StatusCode::kInternal, status.code());
                CheckSourceLocation(status, { line });
            }
        }
        {
            const turbo::Status a(turbo::StatusCode::kInternal, "message",
                turbo::SourceLocation());
            const turbo::Status b = a;
            for (const turbo::Status& status : { a, b }) {
                EXPECT_FALSE(status.ok());
                EXPECT_EQ(turbo::StatusCode::kInternal, status.code());
                CheckSourceLocation(status);
            }
        }
        {
            const turbo::Status a(turbo::StatusCode::kInternal, "",
                turbo::SourceLocation::current());
            const turbo::Status b = a;
            for (const turbo::Status& status : { a, b }) {
                EXPECT_FALSE(status.ok());
                EXPECT_EQ(turbo::StatusCode::kInternal, status.code());
                CheckSourceLocation(status);
            }
        }
        {
            const turbo::Status a(turbo::StatusCode::kInternal, "",
                turbo::SourceLocation());
            const turbo::Status b = a;
            for (const turbo::Status& status : { a, b }) {
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
            const turbo::Status status(original, turbo::SourceLocation());
            EXPECT_TRUE(status.ok());
            EXPECT_EQ(turbo::StatusCode::kOk, status.code());
            CheckSourceLocation(status);
        }
        {
            // OK status doesn't save source locations.
            const turbo::Status original;
            const turbo::Status status(original, turbo::SourceLocation::current());
            EXPECT_TRUE(status.ok());
            EXPECT_EQ(turbo::StatusCode::kOk, status.code());
            CheckSourceLocation(status);
        }
        {
            // Non-ok Status with non-empty msg can save source locations with
            // non-nullptr filename.
            const turbo::Status original(turbo::StatusCode::kInternal, "message",
                turbo::SourceLocation::current());
            int line = GET_SOURCE_LOCATION(1);
            // Default turbo::SourceLocation cannot be saved into the chain.
            const turbo::Status status(original, turbo::SourceLocation());
            EXPECT_FALSE(status.ok());
            EXPECT_EQ(turbo::StatusCode::kInternal, status.code());
            CheckSourceLocation(status, { line });
        }
        {
            const turbo::Status original(turbo::StatusCode::kInternal, "message",
                turbo::SourceLocation::current());
            int line = GET_SOURCE_LOCATION(1);

            const turbo::Status status(original, turbo::SourceLocation::current());
            int line2 = GET_SOURCE_LOCATION(1);

            EXPECT_FALSE(status.ok());
            EXPECT_EQ(turbo::StatusCode::kInternal, status.code());
            CheckSourceLocation(status, { line, line2 });
        }
        {
            // Non-OK status with empty msg doesn't save source locations.
            const turbo::Status original(turbo::StatusCode::kInternal, "",
                turbo::SourceLocation::current());
            const turbo::Status status(original, turbo::SourceLocation());
            EXPECT_FALSE(status.ok());
            EXPECT_EQ(turbo::StatusCode::kInternal, status.code());
            CheckSourceLocation(status);
        }
        {
            // Non-OK status with empty msg doesn't save source locations.
            const turbo::Status original(turbo::StatusCode::kInternal, "",
                turbo::SourceLocation::current());
            const turbo::Status status(original, turbo::SourceLocation::current());
            EXPECT_FALSE(status.ok());
            EXPECT_EQ(turbo::StatusCode::kInternal, status.code());
            CheckSourceLocation(status);
        }
        {
            // Non-OK status with empty msg doesn't save source locations from default
            // constructor.
            const turbo::Status original(turbo::StatusCode::kInternal, "",
                turbo::SourceLocation());
            const turbo::Status status(original, turbo::SourceLocation());
            EXPECT_FALSE(status.ok());
            EXPECT_EQ(turbo::StatusCode::kInternal, status.code());
            CheckSourceLocation(status);
        }
        {
            // Non-OK status with empty msg doesn't save source locations.
            const turbo::Status original(turbo::StatusCode::kInternal, "",
                turbo::SourceLocation());
            const turbo::Status status(original, turbo::SourceLocation::current());
            EXPECT_FALSE(status.ok());
            EXPECT_EQ(turbo::StatusCode::kInternal, status.code());
            CheckSourceLocation(status);
        }
        {
            const turbo::Status original(turbo::StatusCode::kInternal, "message",
                turbo::SourceLocation());
            const turbo::Status status(original, turbo::SourceLocation());
            EXPECT_FALSE(status.ok());
            EXPECT_EQ(turbo::StatusCode::kInternal, status.code());
            CheckSourceLocation(status);
        }
        {
            const turbo::Status original(turbo::StatusCode::kInternal, "message",
                turbo::SourceLocation());
            const turbo::Status status(original, turbo::SourceLocation::current());
            int line = GET_SOURCE_LOCATION(1);
            EXPECT_FALSE(status.ok());
            EXPECT_EQ(turbo::StatusCode::kInternal, status.code());
            CheckSourceLocation(status, { line });
        }
    }

    TEST(Status, SourceLocationWithMoveConstructor) {
        {
            // OK status doesn't save source locations.
            turbo::Status original;
            const turbo::Status status(std::move(original), turbo::SourceLocation());
            EXPECT_TRUE(status.ok());
            EXPECT_EQ(turbo::StatusCode::kOk, status.code());
            CheckSourceLocation(status);
        }
        {
            // OK status doesn't save source locations.
            turbo::Status original;
            const turbo::Status status(std::move(original),
                turbo::SourceLocation::current());
            EXPECT_TRUE(status.ok());
            EXPECT_EQ(turbo::StatusCode::kOk, status.code());
            CheckSourceLocation(status);
        }
        {
            // Non-ok Status with non-empty msg can save source locations with
            // non-nullptr filename.
            turbo::Status original(turbo::StatusCode::kInternal, "message",
                turbo::SourceLocation::current());
            int line = GET_SOURCE_LOCATION(1);
            // Default turbo::SourceLocation cannot be saved into the chain.
            const turbo::Status status(std::move(original), turbo::SourceLocation());
            EXPECT_FALSE(status.ok());
            EXPECT_EQ(turbo::StatusCode::kInternal, status.code());
            CheckSourceLocation(status, { line });
        }
        {
            turbo::Status original(turbo::StatusCode::kInternal, "message",
                turbo::SourceLocation::current());
            int line = GET_SOURCE_LOCATION(1);

            const turbo::Status status(std::move(original),
                turbo::SourceLocation::current());
            int line2 = GET_SOURCE_LOCATION(1);

            EXPECT_FALSE(status.ok());
            EXPECT_EQ(turbo::StatusCode::kInternal, status.code());
            CheckSourceLocation(status, { line, line2 });
        }
        {
            // Non-OK status with empty msg doesn't save source locations.
            turbo::Status original(turbo::StatusCode::kInternal, "",
                turbo::SourceLocation::current());
            const turbo::Status status(std::move(original), turbo::SourceLocation());
            EXPECT_FALSE(status.ok());
            EXPECT_EQ(turbo::StatusCode::kInternal, status.code());
            CheckSourceLocation(status);
        }
        {
            // Non-OK status with empty msg doesn't save source locations.
            turbo::Status original(turbo::StatusCode::kInternal, "",
                turbo::SourceLocation::current());
            const turbo::Status status(std::move(original),
                turbo::SourceLocation::current());
            EXPECT_FALSE(status.ok());
            EXPECT_EQ(turbo::StatusCode::kInternal, status.code());
            CheckSourceLocation(status);
        }
        {
            // Non-OK status with empty msg doesn't save source locations from default
            // constructor.
            turbo::Status original(turbo::StatusCode::kInternal, "",
                turbo::SourceLocation());
            const turbo::Status status(std::move(original), turbo::SourceLocation());
            EXPECT_FALSE(status.ok());
            EXPECT_EQ(turbo::StatusCode::kInternal, status.code());
            CheckSourceLocation(status);
        }
        {
            // Non-OK status with empty msg doesn't save source locations.
            turbo::Status original(turbo::StatusCode::kInternal, "",
                turbo::SourceLocation());
            const turbo::Status status(std::move(original),
                turbo::SourceLocation::current());
            EXPECT_FALSE(status.ok());
            EXPECT_EQ(turbo::StatusCode::kInternal, status.code());
            CheckSourceLocation(status);
        }
        {
            turbo::Status original(turbo::StatusCode::kInternal, "message",
                turbo::SourceLocation());
            const turbo::Status status(std::move(original), turbo::SourceLocation());
            EXPECT_FALSE(status.ok());
            EXPECT_EQ(turbo::StatusCode::kInternal, status.code());
            CheckSourceLocation(status);
        }
        {
            turbo::Status original(turbo::StatusCode::kInternal, "message",
                turbo::SourceLocation());
            const turbo::Status status(std::move(original),
                turbo::SourceLocation::current());
            int line = GET_SOURCE_LOCATION(1);
            EXPECT_FALSE(status.ok());
            EXPECT_EQ(turbo::StatusCode::kInternal, status.code());
            CheckSourceLocation(status, { line });
        }
    }

    TEST(Status, add_source_location) {
        int max_iter = 10;
        {
            // Status that ignores source location.
            turbo::Status status_ignores_source_location[] = {
                turbo::Status(),
                turbo::Status(turbo::StatusCode::kInternal, "")
            };
            for (turbo::Status& s : status_ignores_source_location) {
                for (int i = 0; i < max_iter; ++i) {
                    s.add_source_location(turbo::SourceLocation::current());
                    s.add_source_location(turbo::SourceLocation());
                }
                CheckSourceLocation(s);
            }
        }
        {
            // Default turbo::SourceLocation is not added.
            turbo::Status status(turbo::StatusCode::kInternal, "foo",
                turbo::SourceLocation::current());
            int line = GET_SOURCE_LOCATION(1);
            for (int i = 0; i < max_iter; ++i) {
                status.add_source_location(turbo::SourceLocation());
            }
            CheckSourceLocation(status, { line });
        }
        {
            // Default turbo::SourceLocation is not added.
            turbo::Status status(turbo::StatusCode::kInternal, "foo",
                turbo::SourceLocation::current());
            int line = GET_SOURCE_LOCATION(1);
            std::vector<int> lines = { line };
            lines.reserve(1 + max_iter);
            for (int i = 0; i < max_iter; ++i) {
                status.add_source_location(turbo::SourceLocation::current());
                lines.push_back(GET_SOURCE_LOCATION(1));
            }
            CheckSourceLocation(status, lines);
        }
    }

    TEST(Status, WithSourceLocationCopy) {
        turbo::Status original(turbo::StatusCode::kInternal, "message",
            turbo::SourceLocation::current());
        int line = GET_SOURCE_LOCATION(1);

        const turbo::Status status = original.with_source_location(turbo::SourceLocation::current());
        int line2 = GET_SOURCE_LOCATION(1);

        CheckSourceLocation(original, { line });
        CheckSourceLocation(status, { line, line2 });
        EXPECT_EQ(original, status);
    }

    turbo::Status&& IsRvalueStatus(turbo::Status&& s) {
        return std::move(s);
    }

    TEST(Status, WithSourceLocationMove) {
        turbo::Status original(turbo::StatusCode::kInternal, "message",
            turbo::SourceLocation::current());
        int line = GET_SOURCE_LOCATION(1);

        const turbo::Status status = IsRvalueStatus(
            std::move(original).with_source_location(turbo::SourceLocation::current()));
        int line2 = GET_SOURCE_LOCATION(1);

        CheckSourceLocation(status, { line, line2 });
        EXPECT_FALSE(status.ok());
        EXPECT_EQ(turbo::StatusCode::kInternal, status.code());
    }

    TEST(Status, CopyOnWriteSourceLocations) {
        turbo::Status source(turbo::StatusCode::kInvalidArgument, "fail",
            turbo::SourceLocation::current());
        EXPECT_EQ(source.get_source_locations().size(), 1);
        turbo::Status copy = source;
        EXPECT_EQ(copy.get_source_locations().size(), 1);
        copy.add_source_location(turbo::SourceLocation::current()); // Copy rep.
        EXPECT_EQ(copy.get_source_locations().size(), 2);
        EXPECT_EQ(source.get_source_locations().size(), 1);
    }

    TEST(Status, SourceLocationToStringMode) {
        turbo::Status s(turbo::StatusCode::kInternal, "fail",
            turbo::SourceLocation::current());
        int line = GET_SOURCE_LOCATION(1);
        std::string source_location_string = "\n=== Source Location Trace: ===";
        std::string source_location_stack = turbo::str_cat(
            turbo::SourceLocation::current().file_name(), ":", line, "\n");

        s.set_payload("foo", std::string("bar"));

        EXPECT_EQ("INTERNAL(s0:0): fail",
            s.ToString(turbo::StatusToStringMode::kWithNoExtraData));

        EXPECT_EQ("INTERNAL(s0:0): fail",
            s.ToString(~turbo::StatusToStringMode::kWithSourceLocation & ~turbo::StatusToStringMode::kWithPayload));
        EXPECT_THAT(s.ToString(turbo::StatusToStringMode::kWithSourceLocation | turbo::StatusToStringMode::kWithPayload),
            AllOf(HasSubstr("INTERNAL(s0:0): fail [foo='bar']"),
                HasSubstr(source_location_string),
                HasSubstr(source_location_stack)));

        s.set_payload("bar", std::string("\377"));

        EXPECT_THAT(s.ToString(turbo::StatusToStringMode::kWithEverything),
            AllOf(HasSubstr("INTERNAL(s0:0): fail"), HasSubstr("[foo='bar']"),
                HasSubstr("[bar='\\xff']"),
                HasSubstr(source_location_string),
                HasSubstr(source_location_stack)));
        EXPECT_THAT(s.ToString(turbo::StatusToStringMode::kWithPayload | turbo::StatusToStringMode::kWithSourceLocation),
            AllOf(HasSubstr("INTERNAL(s0:0): fail"), HasSubstr("[foo='bar']"),
                HasSubstr("[bar='\\xff']"),
                HasSubstr(source_location_string),
                HasSubstr(source_location_stack)));
        EXPECT_THAT(s.ToString(turbo::StatusToStringMode::kWithSourceLocation),
            AllOf(HasSubstr("INTERNAL(s0:0): fail"), Not(HasSubstr("[foo='bar']")),
                Not(HasSubstr("[bar='\\xff']")),
                HasSubstr(source_location_string),
                HasSubstr(source_location_stack)));
        EXPECT_THAT(s.ToString(turbo::StatusToStringMode::kWithPayload),
            AllOf(HasSubstr("INTERNAL(s0:0): fail"), HasSubstr("[foo='bar']"),
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
        stack.size = 200; // Overflows frames.

        turbo::Status status = turbo::cancelled_error();
        status.set_payload("TurboStatusStackTracePayload",
            std::string(std::string_view(
                reinterpret_cast<const char*>(&stack), sizeof(stack))));

        // An unchecked overflow should be detected by ASAN/MSAN on the next line.
        static_cast<void>(status.ToString(turbo::StatusToStringMode::kWithEverything));
    }

    TEST(Status, DefaultHasZeroSub) {
        turbo::Status ok;
        EXPECT_TRUE(ok.ok());
        EXPECT_EQ(ok.sub_type(), 0);
        EXPECT_EQ(ok.sub_code(), 0);
        EXPECT_EQ(ok.ToString(), "OK");
        turbo::InlineStatus inlined = ok.inline_status();
        EXPECT_TRUE(inlined.is_inlined);
        EXPECT_EQ(inlined.code, turbo::StatusCode::kOk);
        EXPECT_EQ(inlined.type, 0);
        EXPECT_EQ(inlined.sub_code, 0);
    }

    TEST(Status, OkWithSubCodeStaysOk) {
        turbo::Status ok(turbo::StatusCode::kOk, turbo::kSubUser, 11);
        EXPECT_TRUE(ok.ok());
        EXPECT_EQ(ok.code(), turbo::StatusCode::kOk);
        EXPECT_EQ(ok.sub_type(), turbo::kSubUser);
        EXPECT_EQ(ok.sub_code(), 11);
        EXPECT_EQ(ok.ToString(), "OK(s2:11)");
        EXPECT_TRUE(ok.inline_status().is_inlined);
    }

    TEST(Status, InlinedSubCodeToString) {
        turbo::Status s(turbo::StatusCode::kUnimplemented, turbo::kSubErrno, 11);
        EXPECT_FALSE(s.ok());
        EXPECT_TRUE(s.message().empty());
        EXPECT_EQ(s.ToString(), "UNIMPLEMENTED(s0:11)");
        EXPECT_TRUE(s.inline_status().is_inlined);
    }

    TEST(Status, HeapSubCodeToString) {
        turbo::Status s(turbo::StatusCode::kUnimplemented, turbo::kSubUser, 11, "xxxx");
        EXPECT_FALSE(s.ok());
        EXPECT_EQ(s.message(), "xxxx");
        EXPECT_EQ(s.sub_type(), turbo::kSubUser);
        EXPECT_EQ(s.sub_code(), 11);
        EXPECT_EQ(s.ToString(), "UNIMPLEMENTED(s2:11): xxxx");
        turbo::InlineStatus packed = s.inline_status();
        EXPECT_FALSE(packed.is_inlined);
        EXPECT_EQ(packed.code, turbo::StatusCode::kUnimplemented);
        EXPECT_EQ(packed.type, turbo::kSubUser);
        EXPECT_EQ(packed.sub_code, 11);
    }

    TEST(Status, CustomSubType) {
        constexpr turbo::SubStatusType kCustom = 9;
        turbo::Status s(turbo::StatusCode::kInternal, kCustom, -7);
        EXPECT_EQ(s.sub_type(), kCustom);
        EXPECT_EQ(s.sub_code(), -7);
        EXPECT_EQ(s.ToString(), "INTERNAL(s9:-7)");
        EXPECT_EQ(turbo::to_string(kCustom), "9");
        EXPECT_EQ(turbo::to_string(turbo::kSubErrno), "errno");
        EXPECT_EQ(turbo::to_string(turbo::kSubSignal), "signal");
        EXPECT_EQ(turbo::to_string(turbo::kSubUser), "user");
    }

    TEST(Status, WithSubCodeCopyDoesNotMutate) {
        turbo::Status original(turbo::StatusCode::kInternal, "fail");
        EXPECT_EQ(original.sub_type(), 0);
        EXPECT_EQ(original.sub_code(), 0);
        turbo::Status copy = original.with_sub_code(42);
        EXPECT_EQ(original.sub_type(), 0);
        EXPECT_EQ(original.sub_code(), 0);
        EXPECT_EQ(copy.sub_type(), turbo::kSubUser);
        EXPECT_EQ(copy.sub_code(), 42);
        EXPECT_EQ(copy.code(), turbo::StatusCode::kInternal);
        EXPECT_EQ(copy.message(), "fail");
        EXPECT_EQ(copy.ToString(), "INTERNAL(s2:42): fail");
    }

    TEST(Status, WithSubCodeMove) {
        turbo::Status original(turbo::StatusCode::kInternal, "fail");
        turbo::Status moved =
            IsRvalueStatus(std::move(original).with_sub_code(7, turbo::kSubUser));
        EXPECT_EQ(moved.sub_type(), turbo::kSubUser);
        EXPECT_EQ(moved.sub_code(), 7);
        EXPECT_EQ(moved.message(), "fail");
    }

    TEST(Status, WithErrnoAndSignal) {
        turbo::Status base(turbo::StatusCode::kUnknown, "fail");
        turbo::Status errno_copy = base.with_errno(EINVAL);
        EXPECT_EQ(base.sub_type(), 0);
        EXPECT_EQ(errno_copy.sub_type(), turbo::kSubErrno);
        EXPECT_EQ(errno_copy.sub_code(), EINVAL);

        turbo::Status signal_copy = base.with_signal(9);
        EXPECT_EQ(signal_copy.sub_type(), turbo::kSubSignal);
        EXPECT_EQ(signal_copy.sub_code(), 9);

        turbo::Status in_place(turbo::StatusCode::kAborted, "x");
        in_place.add_errno(EIO);
        EXPECT_EQ(in_place.sub_type(), turbo::kSubErrno);
        EXPECT_EQ(in_place.sub_code(), EIO);
        in_place.add_signal(11);
        EXPECT_EQ(in_place.sub_type(), turbo::kSubSignal);
        EXPECT_EQ(in_place.sub_code(), 11);
    }

    TEST(Status, WithoutSubCode) {
        turbo::Status original(turbo::StatusCode::kInternal, turbo::kSubUser, 11,
            "fail");
        turbo::Status cleared = original.without_sub_code();
        EXPECT_EQ(original.sub_type(), turbo::kSubUser);
        EXPECT_EQ(original.sub_code(), 11);
        EXPECT_EQ(cleared.sub_type(), 0);
        EXPECT_EQ(cleared.sub_code(), 0);
        EXPECT_EQ(cleared.code(), turbo::StatusCode::kInternal);
        EXPECT_EQ(cleared.message(), "fail");
        EXPECT_EQ(cleared.ToString(), "INTERNAL(s0:0): fail");

        turbo::Status moved = IsRvalueStatus(std::move(original).without_sub_code());
        EXPECT_EQ(moved.sub_type(), 0);
        EXPECT_EQ(moved.sub_code(), 0);
        EXPECT_EQ(moved.message(), "fail");
    }

    TEST(Status, ClearSubCodeInPlace) {
        turbo::Status s(turbo::StatusCode::kCancelled, turbo::kSubErrno, 4);
        s.clear_sub_code();
        EXPECT_EQ(s.sub_type(), 0);
        EXPECT_EQ(s.sub_code(), 0);
        EXPECT_EQ(s.code(), turbo::StatusCode::kCancelled);
    }

    TEST(Status, SubCodeEquality) {
        turbo::Status a(turbo::StatusCode::kInternal, turbo::kSubUser, 1);
        turbo::Status b(turbo::StatusCode::kInternal, turbo::kSubUser, 1);
        turbo::Status c(turbo::StatusCode::kInternal, turbo::kSubUser, 2);
        turbo::Status d(turbo::StatusCode::kInternal, turbo::kSubErrno, 1);
        EXPECT_EQ(a, b);
        EXPECT_NE(a, c);
        EXPECT_NE(a, d);

        turbo::Status heap_a(turbo::StatusCode::kInternal, turbo::kSubUser, 1, "m");
        turbo::Status heap_b(turbo::StatusCode::kInternal, turbo::kSubUser, 1, "m");
        turbo::Status heap_c(turbo::StatusCode::kInternal, turbo::kSubUser, 2, "m");
        EXPECT_EQ(heap_a, heap_b);
        EXPECT_NE(heap_a, heap_c);
    }

    TEST(Status, SubCodeCopyOnWrite) {
        turbo::Status source(turbo::StatusCode::kInvalidArgument, "fail");
        source.add_sub_code(3);
        turbo::Status copy = source;
        EXPECT_EQ(copy.sub_code(), 3);
        copy.add_sub_code(9);
        EXPECT_EQ(source.sub_code(), 3);
        EXPECT_EQ(copy.sub_code(), 9);
        EXPECT_EQ(source.message(), "fail");
        EXPECT_EQ(copy.message(), "fail");
    }

} // namespace
