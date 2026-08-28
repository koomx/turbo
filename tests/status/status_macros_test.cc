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

#include <turbo/status/status_macros.h>

#include <cstddef>
#include <memory>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <string_view>
#include <turbo/status/result.h>
#include <turbo/status/status.h>
#include <turbo/status/status_builder.h>
#include <turbo/types/source_location.h>

namespace {

    using ::testing::AllOf;
    using ::testing::Eq;
    using ::testing::HasSubstr;

    using turbo::status_macro_internal::ReturnIfErrorAdaptor;
    using turbo::status_macro_internal::StatusAdaptorForMacros;
    static_assert(!std::is_default_constructible_v<ReturnIfErrorAdaptor>);
    static_assert(!std::is_default_constructible_v<StatusAdaptorForMacros>);
    static_assert(!std::is_copy_constructible_v<ReturnIfErrorAdaptor>);
    static_assert(!std::is_copy_constructible_v<StatusAdaptorForMacros>);
    static_assert(!std::is_copy_assignable_v<ReturnIfErrorAdaptor>);
    static_assert(!std::is_copy_assignable_v<StatusAdaptorForMacros>);

    turbo::Status ReturnOk() {
        return turbo::ok_status();
    }

    turbo::StatusBuilder ReturnOkBuilder() {
        return turbo::StatusBuilder(turbo::ok_status());
    }

    turbo::Status ReturnError(std::string_view msg) {
        return turbo::Status(turbo::StatusCode::kUnknown, msg);
    }

    turbo::StatusBuilder ReturnErrorBuilder(std::string_view msg) {
        return turbo::StatusBuilder(turbo::Status(turbo::StatusCode::kUnknown, msg));
    }

    turbo::Result<int> ReturnResultValue(int v) {
        return v;
    }

    turbo::Result<int> ReturnResultError(std::string_view msg) {
        return turbo::Status(turbo::StatusCode::kUnknown, msg);
    }

    template <class... Args>
    turbo::Result<std::tuple<Args...>> ReturnResultTupleValue(Args&&... v) {
        return std::tuple<Args...>(std::forward<Args>(v)...);
    }

    template <class... Args>
    turbo::Result<std::tuple<Args...>> ReturnResultTupleError(
        std::string_view msg) {
        return turbo::Status(turbo::StatusCode::kUnknown, msg);
    }

    turbo::Result<int&> ReturnResultRef(int& v) {
        return v;
    }

    turbo::Result<std::unique_ptr<int>> ReturnResultPtrValue(int v) {
        return std::make_unique<int>(v);
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
    TEST(AssignOrReturn, Works) {
        auto func = []() -> turbo::Status {
            TURBO_ASSIGN_OR_RETURN(int value1, ReturnResultValue(1));
            EXPECT_EQ(1, value1);
            TURBO_ASSIGN_OR_RETURN(const int value2, ReturnResultValue(2));
            EXPECT_EQ(2, value2);
            TURBO_ASSIGN_OR_RETURN(const int& value3, ReturnResultValue(3));
            EXPECT_EQ(3, value3);
            TURBO_ASSIGN_OR_RETURN(int value4 [[maybe_unused]],
                ReturnResultError("EXPECTED"));
            return ReturnError("ERROR");
        };

        EXPECT_THAT(func().message(), Eq("EXPECTED"));
    }

    TEST(AssignOrReturn, WorksWithReferences) {
        int value = 17;
        auto func = [&]() -> turbo::Status {
            TURBO_ASSIGN_OR_RETURN(int& value1, ReturnResultRef(value));
            EXPECT_EQ(&value1, &value);

            TURBO_ASSIGN_OR_RETURN(const int& value2, ReturnResultRef(value));
            EXPECT_EQ(&value2, &value);

            TURBO_ASSIGN_OR_RETURN(int value3, ReturnResultRef(value));
            EXPECT_EQ(value3, value);

            value = 11;
            EXPECT_NE(value3, value);

            TURBO_ASSIGN_OR_RETURN(int value4 [[maybe_unused]],
                ReturnResultError("EXPECTED"));
            return ReturnError("ERROR");
        };

        EXPECT_THAT(func().message(), Eq("EXPECTED"));
    }

    TEST(AssignOrReturn, WorksWithCommasInType) {
        auto func = []() -> turbo::Status {
            TURBO_ASSIGN_OR_RETURN((std::tuple<int, int> t1),
                ReturnResultTupleValue(1, 1));
            EXPECT_EQ((std::tuple { 1, 1 }), t1);
            TURBO_ASSIGN_OR_RETURN((const std::tuple<int, std::tuple<int, int>, int> t2),
                ReturnResultTupleValue(1, std::tuple { 1, 1 }, 1));
            EXPECT_EQ((std::tuple { 1, std::tuple { 1, 1 }, 1 }), t2);
            TURBO_ASSIGN_OR_RETURN(
                (std::tuple<int, std::tuple<int, int>, int> t3),
                (ReturnResultTupleError<int, std::tuple<int, int>, int>("EXPECTED")));
            t3 = { }; // fix unused error
            return ReturnError("ERROR");
        };

        EXPECT_THAT(func().message(), Eq("EXPECTED"));
    }

    TEST(AssignOrReturn, WorksWithStructureBindings) {
        auto func = []() -> turbo::Status {
            TURBO_ASSIGN_OR_RETURN(
                (const auto& [t1, t2, t3, t4, t5]),
                ReturnResultTupleValue(std::tuple { 1, 1 }, 1, 2, 3, 4));
            EXPECT_EQ((std::tuple { 1, 1 }), t1);
            EXPECT_EQ(1, t2);
            EXPECT_EQ(2, t3);
            EXPECT_EQ(3, t4);
            EXPECT_EQ(4, t5);
            TURBO_ASSIGN_OR_RETURN(int t6 [[maybe_unused]],
                ReturnResultError("EXPECTED"));
            return ReturnError("ERROR");
        };

        EXPECT_THAT(func().message(), Eq("EXPECTED"));
    }

    TEST(AssignOrReturn, WorksWithParenthesesAndDereference) {
        auto func = []() -> turbo::Status {
            int integer;
            int* pointer_to_integer = &integer;
            TURBO_ASSIGN_OR_RETURN((*pointer_to_integer), ReturnResultValue(1));
            EXPECT_EQ(1, integer);
            TURBO_ASSIGN_OR_RETURN(*pointer_to_integer, ReturnResultValue(2));
            EXPECT_EQ(2, integer);
            // Make the test where the order of dereference matters and treat the
            // parentheses.
            pointer_to_integer--;
            int** pointer_to_pointer_to_integer = &pointer_to_integer;
            TURBO_ASSIGN_OR_RETURN((*pointer_to_pointer_to_integer)[1],
                ReturnResultValue(3));
            EXPECT_EQ(3, integer);
            TURBO_ASSIGN_OR_RETURN(int t1 [[maybe_unused]],
                ReturnResultError("EXPECTED"));
            return ReturnError("ERROR");
        };

        EXPECT_THAT(func().message(), Eq("EXPECTED"));
    }

    TEST(AssignOrReturn, WorksWithAppend) {
        auto fail_test_if_called = []() -> std::string {
            ADD_FAILURE();
            return "FAILURE";
        };
        auto func = [&]() -> turbo::Status {
            int value [[maybe_unused]];
            TURBO_ASSIGN_OR_RETURN(value, ReturnResultValue(1),
                _ << fail_test_if_called());
            TURBO_ASSIGN_OR_RETURN(value, ReturnResultError("EXPECTED A"),
                _ << "EXPECTED B");
            return ReturnOk();
        };

        EXPECT_THAT(func().message(),
            AllOf(HasSubstr("EXPECTED A"), HasSubstr("EXPECTED B")));
    }

    TEST(AssignOrReturn, WorksWithAdaptorFunc) {
        auto fail_test_if_called = [](turbo::StatusBuilder builder) {
            ADD_FAILURE();
            return builder;
        };
        auto adaptor = [](turbo::StatusBuilder builder) {
            return builder << "EXPECTED B";
        };
        auto func = [&]() -> turbo::Status {
            int value [[maybe_unused]];
            TURBO_ASSIGN_OR_RETURN(value, ReturnResultValue(1),
                fail_test_if_called(_));
            TURBO_ASSIGN_OR_RETURN(value, ReturnResultError("EXPECTED A"), adaptor(_));
            return ReturnOk();
        };

        EXPECT_THAT(func().message(),
            AllOf(HasSubstr("EXPECTED A"), HasSubstr("EXPECTED B")));
    }

    TEST(AssignOrReturn, WorksWithThirdArgumentAndCommas) {
        auto fail_test_if_called = [](turbo::StatusBuilder builder) {
            ADD_FAILURE();
            return builder;
        };
        auto adaptor = [](turbo::StatusBuilder builder) {
            return builder << "EXPECTED B";
        };
        auto func = [&]() -> turbo::Status {
            TURBO_ASSIGN_OR_RETURN((const auto& [t1, t2, t3]),
                ReturnResultTupleValue(1, 2, 3),
                fail_test_if_called(_));
            EXPECT_EQ(t1, 1);
            EXPECT_EQ(t2, 2);
            EXPECT_EQ(t3, 3);
            TURBO_ASSIGN_OR_RETURN(
                (const auto& [t4, t5, t6]),
                (ReturnResultTupleError<int, int, int>("EXPECTED A")), adaptor(_));
            // Silence errors about the unused values.
            static_cast<void>(t4);
            static_cast<void>(t5);
            static_cast<void>(t6);
            return ReturnOk();
        };

        EXPECT_THAT(func().message(),
            AllOf(HasSubstr("EXPECTED A"), HasSubstr("EXPECTED B")));
    }

    TEST(AssignOrReturn, WorksWithAppendIncludingLocals) {
        auto func = [&](std::string_view str) -> turbo::Status {
            int value [[maybe_unused]];
            TURBO_ASSIGN_OR_RETURN(value, ReturnResultError("EXPECTED A"), _ << str);
            return ReturnOk();
        };

        EXPECT_THAT(func("EXPECTED B").message(),
            AllOf(HasSubstr("EXPECTED A"), HasSubstr("EXPECTED B")));
    }

    TEST(AssignOrReturn, WorksForExistingVariable) {
        auto func = []() -> turbo::Status {
            int value = 1;
            TURBO_ASSIGN_OR_RETURN(value, ReturnResultValue(2));
            EXPECT_EQ(2, value);
            TURBO_ASSIGN_OR_RETURN(value, ReturnResultValue(3));
            EXPECT_EQ(3, value);
            TURBO_ASSIGN_OR_RETURN(value, ReturnResultError("EXPECTED"));
            return ReturnError("ERROR");
        };

        EXPECT_THAT(func().message(), Eq("EXPECTED"));
    }

    TEST(AssignOrReturn, UniquePtrWorks) {
        auto func = []() -> turbo::Status {
            TURBO_ASSIGN_OR_RETURN(std::unique_ptr<int> ptr, ReturnResultPtrValue(1));
            EXPECT_EQ(*ptr, 1);
            return ReturnError("EXPECTED");
        };

        EXPECT_THAT(func().message(), Eq("EXPECTED"));
    }

    TEST(AssignOrReturn, UniquePtrWorksForExistingVariable) {
        auto func = []() -> turbo::Status {
            std::unique_ptr<int> ptr;
            TURBO_ASSIGN_OR_RETURN(ptr, ReturnResultPtrValue(1));
            EXPECT_EQ(*ptr, 1);

            TURBO_ASSIGN_OR_RETURN(ptr, ReturnResultPtrValue(2));
            EXPECT_EQ(*ptr, 2);
            return ReturnError("EXPECTED");
        };

        EXPECT_THAT(func().message(), Eq("EXPECTED"));
    }

    TEST(AssignOrReturn, ChainSourceLocation) {
        auto func1 = []() -> turbo::Result<std::unique_ptr<int>> {
            std::unique_ptr<int> ptr;
            TURBO_ASSIGN_OR_RETURN(ptr, ReturnResultPtrValue(1));
            return ptr;
        };
        auto func2 = []() -> turbo::Result<std::unique_ptr<int>> {
            return turbo::Status(turbo::StatusCode::kInternal, "msg");
        };
        int func2_line = __builtin_LINE() - 2;

        auto func3 = [=]() -> turbo::Result<std::unique_ptr<int>> {
            std::unique_ptr<int> ptr;
            TURBO_ASSIGN_OR_RETURN(ptr, func1());
            TURBO_ASSIGN_OR_RETURN(ptr, func2());
            return ptr;
        };
        int func3_line = __builtin_LINE() - 3;

        auto func4 = [=]() -> turbo::Result<std::unique_ptr<int>> {
            std::unique_ptr<int> ptr;
            TURBO_ASSIGN_OR_RETURN(ptr, func3());
            TURBO_ASSIGN_OR_RETURN(ptr, func2());
            return ptr;
        };
        int func4_line = __builtin_LINE() - 4;

        turbo::Result<std::unique_ptr<int>> result = func4();
        EXPECT_EQ(turbo::StatusCode::kInternal, result.status().code());
        CheckSourceLocation(result.status(), { func2_line, func3_line, func4_line });
    }

    TEST(AssignOrReturn, NotChainSourceLocationWithEmptyMsg) {
        auto func1 = []() -> turbo::Result<std::unique_ptr<int>> {
            std::unique_ptr<int> ptr;
            TURBO_ASSIGN_OR_RETURN(ptr, ReturnResultPtrValue(1));
            return ptr;
        };
        auto func2 = []() -> turbo::Result<std::unique_ptr<int>> {
            return turbo::Status(turbo::StatusCode::kInternal, "");
        };

        auto func3 = [=]() -> turbo::Result<std::unique_ptr<int>> {
            std::unique_ptr<int> ptr;
            TURBO_ASSIGN_OR_RETURN(ptr, func1());
            TURBO_ASSIGN_OR_RETURN(ptr, func2());
            return ptr;
        };

        auto func4 = [=]() -> turbo::Result<std::unique_ptr<int>> {
            std::unique_ptr<int> ptr;
            TURBO_ASSIGN_OR_RETURN(ptr, func3());
            TURBO_ASSIGN_OR_RETURN(ptr, func2());
            return ptr;
        };

        turbo::Result<std::unique_ptr<int>> result = func4();
        EXPECT_EQ(turbo::StatusCode::kInternal, result.status().code());
        CheckSourceLocation(result.status());
    }

    TEST(AssignOrReturn, ChainSourceLocationWith3ArgStatusMacro) {
        auto func1 = []() -> turbo::Result<std::unique_ptr<int>> {
            std::unique_ptr<int> ptr;
            TURBO_ASSIGN_OR_RETURN(ptr, ReturnResultPtrValue(1));
            return ptr;
        };
        auto func2 = []() -> turbo::Result<std::unique_ptr<int>> {
            return turbo::Status(turbo::StatusCode::kInternal, "");
        };

        auto func3 = [=]() -> turbo::Result<std::unique_ptr<int>> {
            std::unique_ptr<int> ptr;
            TURBO_ASSIGN_OR_RETURN(ptr, func1());
            TURBO_ASSIGN_OR_RETURN(ptr, func2(), _ << "hmm");
            return ptr;
        };
        int func3_line = __builtin_LINE() - 3;

        auto func4 = [=]() -> turbo::Result<std::unique_ptr<int>> {
            std::unique_ptr<int> ptr;
            TURBO_ASSIGN_OR_RETURN(ptr, func3());
            TURBO_ASSIGN_OR_RETURN(ptr, func2());
            return ptr;
        };
        int func4_line = __builtin_LINE() - 4;

        turbo::Result<std::unique_ptr<int>> result = func4();
        EXPECT_EQ(turbo::StatusCode::kInternal, result.status().code());
        CheckSourceLocation(result.status(), { func3_line, func4_line });
    }

    TEST(ReturnIfError, Works) {
        auto func = []() -> turbo::Status {
            TURBO_RETURN_IF_ERROR(ReturnOk());
            TURBO_RETURN_IF_ERROR(ReturnOk());
            TURBO_RETURN_IF_ERROR(ReturnError("EXPECTED"));
            return ReturnError("ERROR");
        };

        EXPECT_THAT(func().message(), Eq("EXPECTED"));
    }

    TEST(ReturnIfError, WorksWithSourceLocation) {
        using StatusCode = turbo::StatusCode;
        {
            auto func = []() -> turbo::Status {
                TURBO_RETURN_IF_ERROR(ReturnOk());
                TURBO_RETURN_IF_ERROR(ReturnOk());
                TURBO_RETURN_IF_ERROR(turbo::Status(StatusCode::kUnknown, "EXPECTED"));
                return ReturnError("ERROR");
            };
            int error_line = __builtin_LINE() - 3;
            int func_line = error_line;

            EXPECT_THAT(func().message(), Eq("EXPECTED"));
            CheckSourceLocation(func(), { error_line, func_line });
        }
        {
            auto func1 = []() -> turbo::Status {
                TURBO_RETURN_IF_ERROR(ReturnOk());
                TURBO_RETURN_IF_ERROR(ReturnOk());
                TURBO_RETURN_IF_ERROR(turbo::Status(StatusCode::kUnknown, "EXPECTED"));
                return ReturnError("ERROR");
            };
            int error_line = __builtin_LINE() - 3;
            int func_line = error_line;

            auto func3 = [=]() -> turbo::Status {
                TURBO_RETURN_IF_ERROR(func1());
                return ReturnError("ERROR_Func3");
            };
            int func3_line = __builtin_LINE() - 3;

            auto func4 = [=]() -> turbo::Status {
                TURBO_RETURN_IF_ERROR(func3());
                return ReturnError("ERROR_Func4");
            };
            int func4_line = __builtin_LINE() - 3;
            CheckSourceLocation(func4(),
                { error_line, func_line, func3_line, func4_line });
        }
        {
            auto func1 = []() -> turbo::Status {
                TURBO_RETURN_IF_ERROR(ReturnOk());
                TURBO_RETURN_IF_ERROR(ReturnOk());
                TURBO_RETURN_IF_ERROR(turbo::Status(StatusCode::kUnknown, ""));
                return ReturnError("ERROR");
            };

            auto func3 = [=]() -> turbo::Status {
                TURBO_RETURN_IF_ERROR(func1());
                return ReturnError("ERROR_Func3");
            };

            auto func4 = [=]() -> turbo::Status {
                TURBO_RETURN_IF_ERROR(func3());
                return ReturnError("ERROR_Func4");
            };
            CheckSourceLocation(func4());
        }
    }

    TEST(ReturnIfError, WorksWithSourceLocationOn2Arg) {
        {
            auto func = []() -> turbo::Status {
                TURBO_RETURN_IF_ERROR(ReturnOk());
                TURBO_RETURN_IF_ERROR(ReturnOk());
                TURBO_RETURN_IF_ERROR(turbo::Status(turbo::StatusCode::kUnknown, "EXPECTED"))
                    << "foo";
                return ReturnError("ERROR");
            };
            int error_line = __builtin_LINE() - 4;
            int func_line = error_line;

            EXPECT_THAT(func().message(), Eq("EXPECTED; foo"));
            CheckSourceLocation(func(), { error_line, func_line });
        }
        {
            auto func1 = []() -> turbo::Status {
                TURBO_RETURN_IF_ERROR(ReturnOk());
                TURBO_RETURN_IF_ERROR(ReturnOk());
                TURBO_RETURN_IF_ERROR(turbo::Status(turbo::StatusCode::kUnknown, "EXPECTED"))
                    << "foo";
                return ReturnError("ERROR");
            };
            int error_line = __builtin_LINE() - 4;
            int func_line = error_line;

            auto func3 = [=]() -> turbo::Status {
                TURBO_RETURN_IF_ERROR(func1());
                return ReturnError("ERROR_Func3");
            };
            int func3_line = __builtin_LINE() - 3;

            auto func4 = [=]() -> turbo::Status {
                TURBO_RETURN_IF_ERROR(func3());
                return ReturnError("ERROR_Func4");
            };
            int func4_line = __builtin_LINE() - 3;
            CheckSourceLocation(func4(),
                { error_line, func_line, func3_line, func4_line });
        }
        {
            auto func1 = []() -> turbo::Status {
                TURBO_RETURN_IF_ERROR(ReturnOk());
                TURBO_RETURN_IF_ERROR(ReturnOk());
                TURBO_RETURN_IF_ERROR(turbo::Status(turbo::StatusCode::kUnknown, ""));
                return ReturnError("ERROR");
            };

            auto func3 = [=]() -> turbo::Status {
                TURBO_RETURN_IF_ERROR(func1()) << "foo";
                return ReturnError("ERROR_Func3");
            };
            int func3_line = __builtin_LINE() - 3;

            auto func4 = [=]() -> turbo::Status {
                TURBO_RETURN_IF_ERROR(func3());
                return ReturnError("ERROR_Func4");
            };
            int func4_line = __builtin_LINE() - 3;
            EXPECT_THAT(func4().message(), Eq("foo"));
            CheckSourceLocation(func4(), { func3_line, func4_line });
        }
        {
            auto func1 = []() -> turbo::Status {
                TURBO_RETURN_IF_ERROR(ReturnOk());
                TURBO_RETURN_IF_ERROR(ReturnOk());
                TURBO_RETURN_IF_ERROR(turbo::Status(turbo::StatusCode::kUnknown, "")) << "";
                return ReturnError("ERROR");
            };

            auto func3 = [=]() -> turbo::Status {
                TURBO_RETURN_IF_ERROR(func1());
                return ReturnError("ERROR_Func3");
            };

            auto func4 = [=]() -> turbo::Status {
                TURBO_RETURN_IF_ERROR(func3());
                return ReturnError("ERROR_Func4");
            };
            CheckSourceLocation(func4());
        }
    }

    TEST(ReturnIfError, WorksWithBuilder) {
        auto func = []() -> turbo::Status {
            TURBO_RETURN_IF_ERROR(ReturnOkBuilder());
            TURBO_RETURN_IF_ERROR(ReturnOkBuilder());
            TURBO_RETURN_IF_ERROR(ReturnErrorBuilder("EXPECTED"));
            return ReturnErrorBuilder("ERROR");
        };

        EXPECT_THAT(func().message(), Eq("EXPECTED"));

        // Test that TURBO_RETURN_IF_ERROR can also return the StatusBuilder directly.
        auto func2 = []() -> turbo::StatusBuilder {
            TURBO_RETURN_IF_ERROR(ReturnOkBuilder());
            TURBO_RETURN_IF_ERROR(ReturnOkBuilder());
            TURBO_RETURN_IF_ERROR(ReturnErrorBuilder("EXPECTED"));
            return ReturnErrorBuilder("ERROR");
        };
        EXPECT_THAT(static_cast<turbo::Status>(func2()).message(), Eq("EXPECTED"));
    }

    TEST(ReturnIfError, IfInputIsBuilderDoesNotEagerlyConvertToStatus) {
        auto func = []() -> turbo::Status {
            auto builder = ReturnErrorBuilder("FIRST");
            builder.SetPrepend();
            // If this call decays the builder to Status first and then makes another
            // builder it will forget about the SetPrepend and the streaming will happen
            // in the wrong order.
            TURBO_RETURN_IF_ERROR(builder) << "SECOND ";
            return turbo::ok_status();
        };
        EXPECT_THAT(func().message(), Eq("SECOND FIRST"));
    }

    TEST(ReturnIfError, WorksWithLambda) {
        auto func = []() -> turbo::Status {
            TURBO_RETURN_IF_ERROR([] { return ReturnOk(); }());
            TURBO_RETURN_IF_ERROR([] { return ReturnError("EXPECTED"); }());
            return ReturnError("ERROR");
        };

        EXPECT_THAT(func().message(), Eq("EXPECTED"));
    }

    TEST(ReturnIfError, WorksWithAppend) {
        auto fail_test_if_called = []() -> std::string {
            ADD_FAILURE();
            return "FAILURE";
        };
        auto func = [&]() -> turbo::Status {
            TURBO_RETURN_IF_ERROR(ReturnOk()) << fail_test_if_called();
            TURBO_RETURN_IF_ERROR(ReturnError("EXPECTED A")) << "EXPECTED B";
            return turbo::ok_status();
        };

        EXPECT_THAT(func().message(),
            AllOf(HasSubstr("EXPECTED A"), HasSubstr("EXPECTED B")));
    }

    TEST(ReturnIfError, WorksWithVoidReturnAdaptor) {
        int code = 0;
        int phase = 0;
        auto adaptor = [&](turbo::Status status) -> void { code = phase; };
        auto func = [&]() -> void {
            phase = 1;
            TURBO_RETURN_IF_ERROR(ReturnOk()).With(adaptor);
            phase = 2;
            TURBO_RETURN_IF_ERROR(ReturnError("EXPECTED A")).With(adaptor);
            phase = 3;
        };

        func();
        EXPECT_EQ(phase, 2);
        EXPECT_EQ(code, 2);
    }

} // namespace
