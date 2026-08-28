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

#ifndef TURBO_STATUS_INTERNAL_STATUS_MATCHERS_H_
#define TURBO_STATUS_INTERNAL_STATUS_MATCHERS_H_

#include <ostream> // NOLINT
#include <type_traits>
#include <utility>

#include <gmock/gmock.h> // gmock_for_status_matchers.h
#include <string_view>
#include <turbo/macros/config.h>
#include <turbo/status/result.h>
#include <turbo/status/status.h>

namespace turbo_testing {

    namespace status_internal {

        inline const turbo::Status& GetStatus(const turbo::Status& status) {
            return status;
        }

        template <typename T>
        const turbo::Status& GetStatus(const turbo::Result<T>& status) {
            return status.status();
        }

        ////////////////////////////////////////////////////////////
        // Implementation of IsOkAndHolds().

        // Monomorphic implementation of matcher IsOkAndHolds(m).  ResultType is a
        // reference to Result<T>.
        template <typename ResultType>
        class IsOkAndHoldsMatcherImpl
            : public ::testing::MatcherInterface<ResultType> {
        public:
            typedef typename std::remove_reference_t<ResultType>::value_type value_type;

            template <typename InnerMatcher>
            explicit IsOkAndHoldsMatcherImpl(InnerMatcher&& inner_matcher)
                : inner_matcher_(::testing::SafeMatcherCast<const value_type&>(
                      std::forward<InnerMatcher>(inner_matcher))) { }

            void DescribeTo(std::ostream* os) const override {
                *os << "is OK and has a value that ";
                inner_matcher_.DescribeTo(os);
            }

            void DescribeNegationTo(std::ostream* os) const override {
                *os << "isn't OK or has a value that ";
                inner_matcher_.DescribeNegationTo(os);
            }

            bool MatchAndExplain(
                ResultType actual_value,
                ::testing::MatchResultListener* result_listener) const override {
                if (!actual_value.ok()) {
                    *result_listener << "which has status " << actual_value.status();
                    return false;
                }

                // Call through to the inner matcher.
                return inner_matcher_.MatchAndExplain(*actual_value, result_listener);
            }

        private:
            const ::testing::Matcher<const value_type&> inner_matcher_;
        };

        // Implements IsOkAndHolds(m) as a polymorphic matcher.
        template <typename InnerMatcher>
        class IsOkAndHoldsMatcher {
        public:
            explicit IsOkAndHoldsMatcher(InnerMatcher inner_matcher)
                : inner_matcher_(std::forward<InnerMatcher>(inner_matcher)) { }

            // Converts this polymorphic matcher to a monomorphic matcher of the
            // given type.  ResultType can be either Result<T> or a
            // reference to Result<T>.
            template <typename ResultType>
            operator ::testing::Matcher<ResultType>() const { // NOLINT
                return ::testing::Matcher<ResultType>(
                    new IsOkAndHoldsMatcherImpl<const ResultType&>(inner_matcher_));
            }

        private:
            const InnerMatcher inner_matcher_;
        };

        ////////////////////////////////////////////////////////////
        // Implementation of StatusIs().

        // `StatusCode` is implicitly convertible from `int`, `turbo::StatusCode`, and
        //  is explicitly convertible to these types as well.
        //
        // We need this class because `turbo::StatusCode` (as a scoped enum) is not
        // implicitly convertible to `int`. In order to handle use cases like
        // ```
        // StatusIs(Anyof(turbo::StatusCode::kUnknown, turbo::StatusCode::kCancelled))
        // ```
        // which uses polymorphic matchers, we need to unify the interfaces into
        // `Matcher<StatusCode>`.
        class StatusCode {
        public:
            /*implicit*/ StatusCode(int code) // NOLINT
                : code_(code) { }
            /*implicit*/ StatusCode(::turbo::StatusCode code) // NOLINT
                : code_(static_cast<int>(code)) { }

            explicit operator int() const { return static_cast<int>(code_); }

            friend void PrintTo(const StatusCode& code, std::ostream* os) {
                std::string_view text = turbo::status_code_to_string_view(static_cast<turbo::StatusCode>(code.code_));
                if (!text.empty()) {
                    *os << text;
                } else {
                    *os << code.code_;
                }
            }

        private:
            int code_;
        };

        // Relational operators to handle matchers like Eq, Lt, etc..
        inline bool operator==(const StatusCode& lhs, const StatusCode& rhs) {
            return static_cast<int>(lhs) == static_cast<int>(rhs);
        }
        inline bool operator!=(const StatusCode& lhs, const StatusCode& rhs) {
            return static_cast<int>(lhs) != static_cast<int>(rhs);
        }

        // StatusIs() is a polymorphic matcher.  This class is the common
        // implementation of it shared by all types T where StatusIs() can be
        // used as a Matcher<T>.
        class StatusIsMatcherCommonImpl {
        public:
            StatusIsMatcherCommonImpl(
                ::testing::Matcher<StatusCode> code_matcher,
                ::testing::Matcher<std::string_view> message_matcher)
                : code_matcher_(std::move(code_matcher))
                , message_matcher_(std::move(message_matcher)) { }

            void DescribeTo(std::ostream* os) const;

            void DescribeNegationTo(std::ostream* os) const;

            bool MatchAndExplain(const turbo::Status& status,
                ::testing::MatchResultListener* result_listener) const;

        private:
            const ::testing::Matcher<StatusCode> code_matcher_;
            const ::testing::Matcher<std::string_view> message_matcher_;
        };

        // Monomorphic implementation of matcher StatusIs() for a given type
        // T.  T can be Status, Result<>, or a reference to either of them.
        template <typename T>
        class MonoStatusIsMatcherImpl : public ::testing::MatcherInterface<T> {
        public:
            explicit MonoStatusIsMatcherImpl(StatusIsMatcherCommonImpl common_impl)
                : common_impl_(std::move(common_impl)) { }

            void DescribeTo(std::ostream* os) const override {
                common_impl_.DescribeTo(os);
            }

            void DescribeNegationTo(std::ostream* os) const override {
                common_impl_.DescribeNegationTo(os);
            }

            bool MatchAndExplain(
                T actual_value,
                ::testing::MatchResultListener* result_listener) const override {
                return common_impl_.MatchAndExplain(GetStatus(actual_value),
                    result_listener);
            }

        private:
            StatusIsMatcherCommonImpl common_impl_;
        };

        // Implements StatusIs() as a polymorphic matcher.
        class StatusIsMatcher {
        public:
            template <typename StatusCodeMatcher, typename StatusMessageMatcher>
            StatusIsMatcher(StatusCodeMatcher&& code_matcher,
                StatusMessageMatcher&& message_matcher)
                : common_impl_(::testing::MatcherCast<StatusCode>(
                                   std::forward<StatusCodeMatcher>(code_matcher)),
                      ::testing::MatcherCast<std::string_view>(
                          std::forward<StatusMessageMatcher>(message_matcher))) {
            }

            // Converts this polymorphic matcher to a monomorphic matcher of the
            // given type.  T can be Result<>, Status, or a reference to
            // either of them.
            template <typename T>
            /*implicit*/ operator ::testing::Matcher<T>() const { // NOLINT
                return ::testing::Matcher<T>(
                    new MonoStatusIsMatcherImpl<const T&>(common_impl_));
            }

        private:
            const StatusIsMatcherCommonImpl common_impl_;
        };

        // Monomorphic implementation of matcher IsOk() for a given type T.
        // T can be Status, Result<>, or a reference to either of them.
        template <typename T>
        class MonoIsOkMatcherImpl : public ::testing::MatcherInterface<T> {
        public:
            void DescribeTo(std::ostream* os) const override { *os << "is OK"; }
            void DescribeNegationTo(std::ostream* os) const override {
                *os << "is not OK";
            }
            bool MatchAndExplain(T actual_value,
                ::testing::MatchResultListener*) const override {
                return GetStatus(actual_value).ok();
            }
        };

        // Implements IsOk() as a polymorphic matcher.
        class IsOkMatcher {
        public:
            template <typename T>
            /*implicit*/ operator ::testing::Matcher<T>() const { // NOLINT
                return ::testing::Matcher<T>(new MonoIsOkMatcherImpl<const T&>());
            }
        };

    } // namespace status_internal

} // namespace turbo_testing

#endif // TURBO_STATUS_INTERNAL_STATUS_MATCHERS_H_
