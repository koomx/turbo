// Copyright 2026 The Abseil Authors
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

#include <turbo/status/status_builder.h>

#include <stddef.h>

#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <string_view>
#include <turbo/log/klog.h>
#include <turbo/log/log_entry.h>
#include <turbo/log/log_sink.h>
#include <turbo/macros/config.h>
#include <turbo/status/result.h>
#include <turbo/status/status.h>
#include <turbo/status/status_matchers.h>
#include <turbo/types/source_location.h>

namespace turbo {
    namespace {
        using ::testing::AllOf;
        using ::testing::AnyOf;
        using ::testing::ElementsAre;
        using ::testing::Eq;
        using ::testing::IsEmpty;
        using ::testing::Pointee;
        using ::testing::Property;
        using ::turbo_testing::StatusIs;

        // Converts a StatusBuilder to a Status.
        turbo::Status ToStatus(const StatusBuilder& s) {
            return s;
        }

        // Converts a StatusBuilder to a Result<T>.
        template <typename T>
        turbo::Result<T> ToResult(const StatusBuilder& s) {
            return s;
        }

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

        class StatusBuilderTest : public ::testing::Test {
        };

        TEST_F(StatusBuilderTest, Size) {
            EXPECT_LE(sizeof(StatusBuilder), 40)
                << "Relax this test with caution and thorough testing. If StatusBuilder "
                   "is too large it can potentially blow stacks, especially in debug "
                   "builds. See the comments for StatusBuilder::Rep.";
        }

        TEST_F(StatusBuilderTest, ExplicitSourceLocation) {
            const turbo::SourceLocation kLocation = turbo::SourceLocation::current();

            {
                const StatusBuilder builder(turbo::ok_status(), kLocation);
                EXPECT_THAT(builder.source_location().file_name(),
                    Eq(kLocation.file_name()));
                EXPECT_THAT(builder.source_location().line(), Eq(kLocation.line()));
            }
        }

        TEST_F(StatusBuilderTest, ImplicitSourceLocation) {
            const StatusBuilder builder(turbo::ok_status());
            auto loc = turbo::SourceLocation::current();
            EXPECT_THAT(builder.source_location().file_name(),
                AnyOf(Eq(std::string_view(loc.file_name())),
                    Eq(std::string_view("<source_location>"))));
            EXPECT_THAT(builder.source_location().line(),
                AnyOf(Eq(1), Eq(loc.line() - 1)));
        }

        testing::Matcher<turbo::SourceLocation> SourceLocationIs(
            turbo::SourceLocation loc) {
            return AnyOf(
                AllOf(Property(&turbo::SourceLocation::file_name, Eq(loc.file_name())),
                    Property(&turbo::SourceLocation::line, Eq(loc.line()))),
                // Fallback for platforms that don't support source locations.
                AllOf(Property(&turbo::SourceLocation::file_name, Eq("<source_location>")),
                    Property(&turbo::SourceLocation::line, Eq(1))));
        }

        TEST_F(StatusBuilderTest, GetPreviousSourceLocations) {
            const turbo::SourceLocation loc0 = turbo::SourceLocation::current();
            turbo::Status status = turbo::invalid_argument_error("hi", loc0);
            const turbo::SourceLocation loc1 = turbo::SourceLocation::current();
            status.add_source_location(loc1);
            const turbo::SourceLocation loc2 = turbo::SourceLocation::current();
            status.add_source_location(loc2);

            // The builder's location is not included.
            const StatusBuilder builder(status);
            EXPECT_THAT(builder.GetPreviousSourceLocations(),
                ElementsAre(SourceLocationIs(loc0), SourceLocationIs(loc1),
                    SourceLocationIs(loc2)));
        }

        TEST_F(StatusBuilderTest, EmptyGetPreviousSourceLocationsForNewFromStatusCode) {
            const StatusBuilder builder(turbo::StatusCode::kInvalidArgument);
            EXPECT_THAT(builder.GetPreviousSourceLocations(), IsEmpty());
        }

        TEST_F(StatusBuilderTest, StatusCode) {
            // OK
            {
                const StatusBuilder builder(turbo::StatusCode::kOk);
                EXPECT_TRUE(builder.ok());
                EXPECT_THAT(builder.code(), Eq(turbo::StatusCode::kOk));
            }
            // Non-OK code
            {
                const StatusBuilder builder(turbo::StatusCode::kInvalidArgument);
                EXPECT_FALSE(builder.ok());
                EXPECT_THAT(builder.code(), Eq(turbo::StatusCode::kInvalidArgument));
            }
        }

        TEST_F(StatusBuilderTest, OkIgnoresStuff) {
            EXPECT_THAT(ToStatus(StatusBuilder(turbo::ok_status(), turbo::SourceLocation())
                            << "booyah"),
                Eq(turbo::ok_status()));
        }

        TEST_F(StatusBuilderTest, Streaming) {
            EXPECT_THAT(
                ToStatus(StatusBuilder(turbo::cancelled_error(), turbo::SourceLocation())
                    << "booyah"),
                Eq(turbo::cancelled_error("booyah")));
            EXPECT_THAT(
                ToStatus(
                    StatusBuilder(turbo::aborted_error("hello"), turbo::SourceLocation())
                    << "world"),
                Eq(turbo::aborted_error("hello; world")));
        }

        TEST_F(StatusBuilderTest, PrependLvalue) {
            {
                StatusBuilder builder(turbo::cancelled_error(), turbo::SourceLocation());
                EXPECT_THAT(ToStatus(builder.SetPrepend() << "booyah"),
                    Eq(turbo::cancelled_error("booyah")));
            }
            {
                StatusBuilder builder(turbo::aborted_error(" hello"), turbo::SourceLocation());
                EXPECT_THAT(ToStatus(builder.SetPrepend() << "world"),
                    Eq(turbo::aborted_error("world hello")));
            }
        }

        TEST_F(StatusBuilderTest, PrependRvalue) {
            EXPECT_THAT(
                ToStatus(StatusBuilder(turbo::cancelled_error(), turbo::SourceLocation())
                             .SetPrepend()
                    << "booyah"),
                Eq(turbo::cancelled_error("booyah")));
            EXPECT_THAT(ToStatus(StatusBuilder(turbo::aborted_error(" hello"),
                                     turbo::SourceLocation())
                                     .SetPrepend()
                            << "world"),
                Eq(turbo::aborted_error("world hello")));
        }

        TEST_F(StatusBuilderTest, AppendLvalue) {
            {
                StatusBuilder builder(turbo::cancelled_error(), turbo::SourceLocation());
                EXPECT_THAT(ToStatus(builder.SetAppend() << "booyah"),
                    Eq(turbo::cancelled_error("booyah")));
            }
            {
                StatusBuilder builder(turbo::aborted_error("hello"), turbo::SourceLocation());
                EXPECT_THAT(ToStatus(builder.SetAppend() << " world"),
                    Eq(turbo::aborted_error("hello world")));
            }
        }

        TEST_F(StatusBuilderTest, AppendRvalue) {
            EXPECT_THAT(
                ToStatus(StatusBuilder(turbo::cancelled_error(), turbo::SourceLocation())
                             .SetAppend()
                    << "booyah"),
                Eq(turbo::cancelled_error("booyah")));
            EXPECT_THAT(ToStatus(StatusBuilder(turbo::aborted_error("hello"),
                                     turbo::SourceLocation())
                                     .SetAppend()
                            << " world"),
                Eq(turbo::aborted_error("hello world")));
        }

        TEST_F(StatusBuilderTest, WithRvalueRef) {
            auto policy = [](StatusBuilder sb) { return sb << "policy"; };
            EXPECT_THAT(ToStatus(StatusBuilder(turbo::aborted_error("hello"),
                            turbo::SourceLocation())
                                .With(policy)),
                Eq(turbo::aborted_error("hello; policy")));
        }

        TEST_F(StatusBuilderTest, WithRef) {
            auto policy = [](StatusBuilder sb) { return sb << "policy"; };
            StatusBuilder sb(turbo::aborted_error("zomg"), turbo::SourceLocation());
            EXPECT_THAT(ToStatus(sb.With(policy)),
                Eq(turbo::aborted_error("zomg; policy")));
        }

        TEST_F(StatusBuilderTest, WithTypeChange) {
            auto policy = [](StatusBuilder sb) -> std::string {
                return sb.ok() ? "true" : "false";
            };
            EXPECT_EQ(StatusBuilder(turbo::cancelled_error(), turbo::SourceLocation())
                          .With(policy),
                "false");
            EXPECT_EQ(
                StatusBuilder(turbo::ok_status(), turbo::SourceLocation()).With(policy),
                "true");
        }

        struct MoveOnlyAdaptor {
            std::unique_ptr<int> value;

            std::unique_ptr<int> operator()(const turbo::Status&) && {
                return std::move(value);
            }
        };

        TEST_F(StatusBuilderTest, WithMoveOnlyAdaptor) {
            StatusBuilder sb(turbo::aborted_error("zomg"), turbo::SourceLocation());
            EXPECT_THAT(sb.With(MoveOnlyAdaptor { std::make_unique<int>(100) }),
                Pointee(100));
            EXPECT_THAT(StatusBuilder(turbo::aborted_error("zomg"), turbo::SourceLocation())
                            .With(MoveOnlyAdaptor { std::make_unique<int>(100) }),
                Pointee(100));
        }

        struct StringifiableType {
            std::string_view message;

            template <typename Sink>
            friend void turbo_stringify(Sink& sink, const StringifiableType& o) {
                sink.append(o.message);
            }
        };

        class MockLogSink : public turbo::LogSink {
        public:
            MOCK_METHOD(void, send, (const turbo::LogEntry&), (override));
        };

        TEST(WithExtraMessagePolicyTest, AppendsToExtraMessage) {
            // The policy simply calls operator<< on the builder; the following examples
            // demonstrate that, without duplicating all of the above tests.
            EXPECT_THAT(ToStatus(StatusBuilder(turbo::aborted_error("hello"),
                            turbo::SourceLocation())
                                .With(ExtraMessage("world"))),
                Eq(turbo::aborted_error("hello; world")));
            EXPECT_THAT(ToStatus(StatusBuilder(turbo::aborted_error("hello"),
                            turbo::SourceLocation())
                                .With(ExtraMessage() << "world")),
                Eq(turbo::aborted_error("hello; world")));
            EXPECT_THAT(ToStatus(StatusBuilder(turbo::aborted_error("hello"),
                            turbo::SourceLocation())
                                .With(ExtraMessage("world"))
                                .With(ExtraMessage("!"))),
                Eq(turbo::aborted_error("hello; world!")));
            EXPECT_THAT(ToStatus(StatusBuilder(turbo::aborted_error("hello"),
                            turbo::SourceLocation())
                                .With(ExtraMessage("world, "))
                                .SetPrepend()),
                Eq(turbo::aborted_error("world, hello")));
            EXPECT_THAT(ToStatus(StatusBuilder(turbo::aborted_error("hello"),
                            turbo::SourceLocation())
                                .With(ExtraMessage() << StringifiableType { "world" })),
                Eq(turbo::aborted_error("hello; world")));

            // The above examples use temporary StatusBuilder rvalues; verify things also
            // work fine when StatusBuilder is an lvalue.
            StatusBuilder builder(turbo::aborted_error("hello"), turbo::SourceLocation());
            EXPECT_THAT(
                ToStatus(builder.With(ExtraMessage("world")).With(ExtraMessage("!"))),
                Eq(turbo::aborted_error("hello; world!")));
        }

        TEST(WithExtraMessagePolicyTest,
            ExtraMessageStreamOperatorPreservesRvalueness) {
            static_assert(
                std::is_same_v<ExtraMessage&&, decltype(ExtraMessage() << "foo")>);
        }

        TEST_F(StatusBuilderTest, StatusSourceLocationChaining) {
            {
                turbo::Status src = turbo::ok_status();
                CheckSourceLocation(src);
                CheckSourceLocation(ToStatus(StatusBuilder(src, turbo::SourceLocation())));
                CheckSourceLocation(
                    ToStatus(StatusBuilder(src, turbo::SourceLocation::current())));
                CheckSourceLocation(
                    ToStatus(StatusBuilder(src, turbo::SourceLocation::current()) << "hmm"));
            }
            {
                turbo::Status src = turbo::Status(turbo::StatusCode::kCancelled, "");
                CheckSourceLocation(src);
                CheckSourceLocation(ToStatus(StatusBuilder(src, turbo::SourceLocation())));
                CheckSourceLocation(
                    ToStatus(StatusBuilder(src, turbo::SourceLocation::current())));
                CheckSourceLocation(
                    ToStatus(StatusBuilder(src, turbo::SourceLocation::current()) << ""));
                CheckSourceLocation(
                    ToStatus(StatusBuilder(src, turbo::SourceLocation::current()) << "hmm"),
                    { __builtin_LINE() - 1 });
            }
            {
                turbo::Status src = turbo::Status(turbo::StatusCode::kCancelled, "msg",
                    turbo::SourceLocation());
                CheckSourceLocation(src);
                CheckSourceLocation(ToStatus(StatusBuilder(src, turbo::SourceLocation())));
                CheckSourceLocation(
                    ToStatus(StatusBuilder(src, turbo::SourceLocation::current())),
                    { __builtin_LINE() - 1 });
                CheckSourceLocation(
                    ToStatus(StatusBuilder(src, turbo::SourceLocation::current()) << "hmm"),
                    { __builtin_LINE() - 1 });
            }
            {
                turbo::Status src = turbo::Status(turbo::StatusCode::kCancelled, "msg");
                int src_line = __builtin_LINE() - 1;
                CheckSourceLocation(src, { src_line });
                CheckSourceLocation(ToStatus(StatusBuilder(src, turbo::SourceLocation())),
                    { src_line });
                CheckSourceLocation(
                    ToStatus(StatusBuilder(src, turbo::SourceLocation::current())),
                    { src_line, __builtin_LINE() - 1 });
                CheckSourceLocation(
                    ToStatus(StatusBuilder(src, turbo::SourceLocation::current()) << "hmm"),
                    { src_line, __builtin_LINE() - 1 });
            }
        }

        TEST_F(StatusBuilderTest, SetErrorCode) {
            StatusBuilder builder;
            builder.SetCode(turbo::StatusCode::kResourceExhausted);
            KLOG(INFO) << "Builder code: " << builder;
            EXPECT_FALSE(builder.ok());
            EXPECT_EQ(builder.code(), turbo::StatusCode::kResourceExhausted);
        }

        TEST_F(StatusBuilderTest, BuilderToResultStatusShouldGiveErrorResult) {
            turbo::Result<turbo::Status> value = StatusBuilder(turbo::cancelled_error());
            ASSERT_FALSE(value.ok());
            EXPECT_THAT(value.status(), StatusIs(turbo::StatusCode::kCancelled));
        }
    } // namespace
} // namespace turbo
