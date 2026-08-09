// Copyright 2020 The Abseil Authors.
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
#ifndef TURBO_STATUS_INTERNAL_STATUSOR_INTERNAL_H_
#define TURBO_STATUS_INTERNAL_STATUSOR_INTERNAL_H_

#include <cstdint>
#include <type_traits>
#include <utility>

#include <turbo/macros/config.h>
#include <turbo/base/nullability.h>
#include <turbo/meta/type_traits.h>
#include <turbo/status/status.h>
#include <string_view>
#include <turbo/utility/utility.h>

namespace turbo {
    template<typename T>
    class KUMO_MUST_USE_RESULT
    Result;

    namespace internal_statusor {
        // Detects whether `U` has conversion operator to `Result<T>`, i.e. `operator
        // Result<T>()`.
        template<typename T, typename U, typename = void>
        struct HasConversionOperatorToResult : std::false_type {
        };

        template<typename T, typename U>
        void test(char (*turbo_nullable)[sizeof(
            std::declval<U>().operator turbo::Result<T>())]);

        template<typename T, typename U>
        struct HasConversionOperatorToResult<T, U, decltype(test<T, U>(0))>
                : std::true_type {
        };

        // Detects whether `T` is equality-comparable.
        template<typename T, typename = void>
        struct IsEqualityComparable : std::false_type {
        };

        template<typename T>
        struct IsEqualityComparable<
                    T, std::enable_if_t<std::is_convertible_v<
                        decltype(std::declval<T>() == std::declval<T>()), bool> > >
                : std::true_type {
        };

        // Detects whether `T` is constructible or convertible from `Result<U>`.
        template<typename T, typename U>
        using IsConstructibleOrConvertibleFromResult =
        std::disjunction<std::is_constructible<T, Result<U> &>,
            std::is_constructible<T, const Result<U> &>,
            std::is_constructible<T, Result<U> &&>,
            std::is_constructible<T, const Result<U> &&>,
            std::is_convertible<Result<U> &, T>,
            std::is_convertible<const Result<U> &, T>,
            std::is_convertible<Result<U> &&, T>,
            std::is_convertible<const Result<U> &&, T> >;

        // Detects whether `T` is constructible or convertible or assignable from
        // `Result<U>`.
        template<typename T, typename U>
        using IsConstructibleOrConvertibleOrAssignableFromResult =
        std::disjunction<IsConstructibleOrConvertibleFromResult<T, U>,
            std::is_assignable<T &, Result<U> &>,
            std::is_assignable<T &, const Result<U> &>,
            std::is_assignable<T &, Result<U> &&>,
            std::is_assignable<T &, const Result<U> &&> >;

        // Detects whether direct initializing `Result<T>` from `U` is ambiguous, i.e.
        // when `U` is `Result<V>` and `T` is constructible or convertible from `V`.
        template<typename T, typename U>
        struct IsDirectInitializationAmbiguous
                : public std::conditional_t<
                    std::is_same_v<turbo::remove_cvref_t<U>, U>, std::false_type,
                    IsDirectInitializationAmbiguous<T, turbo::remove_cvref_t<U> > > {
        };

        template<typename T, typename V>
        struct IsDirectInitializationAmbiguous<T, turbo::Result<V> >
                : public IsConstructibleOrConvertibleFromResult<T, V> {
        };

        // Checks whether the conversion from U to T can be done without dangling
        // temporaries.
        // REQUIRES: T and U are references.
        template<typename T, typename U>
        using IsReferenceConversionValid = std::conjunction< //
            std::is_reference<T>, std::is_reference<U>,
            // The references are convertible. This checks for
            // lvalue/rvalue compatibility.
            std::is_convertible<U, T>,
            // The pointers are convertible. This checks we don't have
            // a temporary.
            std::is_convertible<std::remove_reference_t<U> *,
                std::remove_reference_t<T> *> >;

        // Checks against the constraints of the direction initialization, i.e. when
        // `Result<T>::Result(U&&)` should participate in overload resolution.
        template<typename T, typename U>
        using IsDirectInitializationValid = std::disjunction<
            // Short circuits if T is basically U.
            std::is_same<T, turbo::remove_cvref_t<U> >, //
            std::conditional_t<
                std::is_reference_v<T>, //
                IsReferenceConversionValid<T, U>,
                std::negation<std::disjunction<
                    std::is_same<turbo::Result<T>, turbo::remove_cvref_t<U> >,
                    std::is_same<turbo::Status, turbo::remove_cvref_t<U> >,
                    std::is_same<std::in_place_t, turbo::remove_cvref_t<U> >,
                    IsDirectInitializationAmbiguous<T, U> > > > >;

        // This trait detects whether `Result<T>::operator=(U&&)` is ambiguous, which
        // is equivalent to whether all the following conditions are met:
        // 1. `U` is `Result<V>`.
        // 2. `T` is constructible and assignable from `V`.
        // 3. `T` is constructible and assignable from `U` (i.e. `Result<V>`).
        // For example, the following code is considered ambiguous:
        // (`T` is `bool`, `U` is `Result<bool>`, `V` is `bool`)
        //   Result<bool> s1 = true;  // s1.ok() && s1.ValueOrDie() == true
        //   Result<bool> s2 = false;  // s2.ok() && s2.ValueOrDie() == false
        //   s1 = s2;  // ambiguous, `s1 = s2.ValueOrDie()` or `s1 = bool(s2)`?
        template<typename T, typename U>
        struct IsForwardingAssignmentAmbiguous
                : public std::conditional_t<
                    std::is_same_v<turbo::remove_cvref_t<U>, U>, std::false_type,
                    IsForwardingAssignmentAmbiguous<T, turbo::remove_cvref_t<U> > > {
        };

        template<typename T, typename U>
        struct IsForwardingAssignmentAmbiguous<T, turbo::Result<U> >
                : public IsConstructibleOrConvertibleOrAssignableFromResult<T, U> {
        };

        // Checks against the constraints of the forwarding assignment, i.e. whether
        // `Result<T>::operator(U&&)` should participate in overload resolution.
        template<typename T, typename U>
        using IsForwardingAssignmentValid = std::disjunction<
            // Short circuits if T is basically U.
            std::is_same<T, turbo::remove_cvref_t<U> >,
            std::negation<std::disjunction<
                std::is_same<turbo::Result<T>, turbo::remove_cvref_t<U> >,
                std::is_same<turbo::Status, turbo::remove_cvref_t<U> >,
                std::is_same<std::in_place_t, turbo::remove_cvref_t<U> >,
                IsForwardingAssignmentAmbiguous<T, U> > > >;

        template<bool Value, typename T>
        using Equality = std::conditional_t<Value, T, std::negation<T> >;

        template<bool Explicit, typename T, typename U, bool Lifetimebound>
        using IsConstructionValid = std::conjunction<
            Equality<Lifetimebound,
                std::disjunction<
                    std::is_reference<T>,
                    type_traits_internal::IsLifetimeBoundAssignment<T, U> > >,
            IsDirectInitializationValid<T, U &&>, std::is_constructible<T, U &&>,
            Equality<!Explicit, std::is_convertible<U &&, T> >,
            std::disjunction<
                std::is_same<T, turbo::remove_cvref_t<U> >,
                std::conjunction<
                    std::conditional_t<
                        Explicit,
                        std::negation<std::is_constructible<turbo::Status, U &&> >,
                        std::negation<std::is_convertible<U &&, turbo::Status> > >,
                    std::negation<
                        internal_statusor::HasConversionOperatorToResult<T, U &&> > > > >;

        template<typename T, typename U, bool Lifetimebound>
        using IsAssignmentValid = std::conjunction<
            Equality<Lifetimebound,
                std::disjunction<
                    std::is_reference<T>,
                    type_traits_internal::IsLifetimeBoundAssignment<T, U> > >,
            std::conditional_t<std::is_reference_v<T>,
                IsReferenceConversionValid<T, U &&>,
                std::conjunction<std::is_constructible<T, U &&>,
                    std::is_assignable<T &, U &&> > >,
            std::disjunction<
                std::is_same<T, turbo::remove_cvref_t<U> >,
                std::conjunction<
                    std::negation<std::is_convertible<U &&, turbo::Status> >,
                    std::negation<HasConversionOperatorToResult<T, U &&> > > >,
            IsForwardingAssignmentValid<T, U &&> >;

        template<bool Explicit, typename T, typename U>
        using IsConstructionFromStatusValid = std::conjunction<
            std::negation<std::is_same<turbo::Result<T>, turbo::remove_cvref_t<U> > >,
            std::negation<std::is_same<T, turbo::remove_cvref_t<U> > >,
            std::negation<std::is_same<std::in_place_t, turbo::remove_cvref_t<U> > >,
            Equality<!Explicit, std::is_convertible<U, turbo::Status> >,
            std::is_constructible<turbo::Status, U>,
            std::negation<HasConversionOperatorToResult<T, U> > >;

        template<bool Explicit, typename T, typename U, bool Lifetimebound,
            typename UQ>
        using IsConstructionFromResultValid = std::conjunction<
            std::negation<std::is_same<T, U> >,
            // If `T` is a reference, then U must be a compatible one.
            std::disjunction<std::negation<std::is_reference<T> >,
                IsReferenceConversionValid<T, U> >,
            Equality<Lifetimebound,
                type_traits_internal::IsLifetimeBoundAssignment<T, U> >,
            std::is_constructible<T, UQ>,
            Equality<!Explicit, std::is_convertible<UQ, T> >,
            std::negation<IsConstructibleOrConvertibleFromResult<T, U> > >;

        template<typename T, typename U, bool Lifetimebound>
        using IsResultAssignmentValid = std::conjunction<
            std::negation<std::is_same<T, turbo::remove_cvref_t<U> > >,
            Equality<Lifetimebound,
                type_traits_internal::IsLifetimeBoundAssignment<T, U> >,
            std::is_constructible<T, U>, std::is_assignable<T, U>,
            std::negation<IsConstructibleOrConvertibleOrAssignableFromResult<
                T, turbo::remove_cvref_t<U> > > >;

        template<typename T, typename U, bool Lifetimebound>
        using IsValueOrValid = std::conjunction<
            // If `T` is a reference, then U must be a compatible one.
            std::disjunction<std::negation<std::is_reference<T> >,
                IsReferenceConversionValid<T, U> >,
            Equality<Lifetimebound,
                std::disjunction<
                    std::is_reference<T>,
                    type_traits_internal::IsLifetimeBoundAssignment<T, U> > > >;

        class Helper {
        public:
            // Move type-agnostic error handling to the .cc.
            static void HandleInvalidStatusCtorArg(Status * turbo_nonnull);

            [[noreturn]] static void Crash(const turbo::Status &status);
        };

        // Construct an instance of T in `p` through placement new, passing Args... to
        // the constructor.
        // This abstraction is here mostly for the gcc performance fix.
        template<typename T, typename... Args>
        KUMO_ATTRIBUTE_NONNULL(1)
        void PlacementNew(void * turbo_nonnull p, Args &&... args) {
            new(p) T(std::forward<Args>(args)...);
        }

        template<typename T>
        class Reference {
        public:
            constexpr explicit Reference(T ref KUMO_ATTRIBUTE_LIFETIME_BOUND)
                : payload_(std::addressof(ref)) {
            }

            Reference(const Reference &) = default;

            Reference &operator=(const Reference &) = default;

            Reference &operator=(T value) {
                payload_ = std::addressof(value);
                return *this;
            }

            operator T() const { return static_cast<T>(*payload_); } // NOLINT
            T get() const { return *this; }

        private:
            std::remove_reference_t<T> * turbo_nonnull payload_;
        };

        // Helper base class to hold the data and all operations.
        // We move all this to a base class to allow mixing with the appropriate
        // TraitsBase specialization.
        template<typename T>
        class ResultData {
            template<typename U>
            friend class ResultData;

            decltype(auto) MaybeMoveData() {
                if constexpr (std::is_reference_v<T>) {
                    return data_.get();
                } else {
                    return std::move(data_);
                }
            }

        public:
            ResultData() = delete;

            ResultData(const ResultData &other) {
                if (other.ok()) {
                    MakeValue(other.data_);
                    MakeStatus();
                } else {
                    MakeStatus(other.status_);
                }
            }

            ResultData(ResultData &&other) noexcept {
                if (other.ok()) {
                    MakeValue(other.MaybeMoveData());
                    MakeStatus();
                } else {
                    MakeStatus(std::move(other.status_));
                }
            }

            template<typename U>
            explicit ResultData(const ResultData<U> &other) {
                if (other.ok()) {
                    MakeValue(other.data_);
                    MakeStatus();
                } else {
                    MakeStatus(other.status_);
                }
            }

            template<typename U>
            explicit ResultData(ResultData<U> &&other) {
                if (other.ok()) {
                    MakeValue(other.MaybeMoveData());
                    MakeStatus();
                } else {
                    MakeStatus(std::move(other.status_));
                }
            }

            template<typename... Args>
            explicit ResultData(std::in_place_t, Args &&... args)
                : data_(std::forward<Args>(args)...) {
                MakeStatus();
            }

            template<
                typename U,
                std::enable_if_t<std::is_constructible_v<turbo::Status, U &&>, int> = 0>
            explicit ResultData(U &&v) : status_(std::forward<U>(v)) {
                EnsureNotOk();
            }

            ResultData &operator=(const ResultData &other) {
                if (this == &other) return *this;
                if (other.ok())
                    Assign(other.data_);
                else
                    AssignStatus(other.status_);
                return *this;
            }

            ResultData &operator=(ResultData &&other) {
                if (this == &other) return *this;
                if (other.ok())
                    Assign(other.MaybeMoveData());
                else
                    AssignStatus(std::move(other.status_));
                return *this;
            }

            ~ResultData() {
                if (ok()) {
                    status_.~Status();
                    if constexpr (!std::is_trivially_destructible_v<T>) {
                        data_.~T();
                    }
                } else {
                    status_.~Status();
                }
            }

            template<typename U>
            void Assign(U &&value) {
                if (ok()) {
                    data_ = std::forward<U>(value);
                } else {
                    MakeValue(std::forward<U>(value));
                    status_ = ok_status();
                }
            }

            template<typename U>
            void AssignStatus(U &&v) {
                Clear();
                status_ = static_cast<turbo::Status>(std::forward<U>(v));
                EnsureNotOk();
            }

            bool ok() const { return status_.ok(); }

        protected:
            // status_ will always be active after the constructor.
            // We make it a union to be able to initialize exactly how we need without
            // waste.
            // Eg. in the copy constructor we use the default constructor of Status in
            // the ok() path to avoid an extra Ref call.
            union {
                Status status_;
            };

            // data_ is active iff status_.ok()==true
            struct Dummy {
            };

            union {
                // When T is const, we need some non-const object we can cast to void* for
                // the placement new. dummy_ is that object.
                Dummy dummy_;
                std::conditional_t<std::is_reference_v<T>, Reference<T>, T> data_;
            };

            void Clear() {
                if constexpr (!std::is_trivially_destructible_v<T>) {
                    if (ok()) data_.~T();
                }
            }

            void EnsureOk() const {
                if (KUMO_UNLIKELY(!ok())) Helper::Crash(status_);
            }

            void EnsureNotOk() {
                if (KUMO_UNLIKELY(ok())) Helper::HandleInvalidStatusCtorArg(&status_);
            }

            // Construct the value (ie. data_) through placement new with the passed
            // argument.
            template<typename... Arg>
            void MakeValue(Arg &&... arg) {
                internal_statusor::PlacementNew<decltype(data_)>(&dummy_,
                                                                 std::forward<Arg>(arg)...);
            }

            // Construct the status (ie. status_) through placement new with the passed
            // argument.
            template<typename... Args>
            void MakeStatus(Args &&... args) {
                internal_statusor::PlacementNew<Status>(&status_,
                                                        std::forward<Args>(args)...);
            }

            template<typename U>
            T ValueOrImpl(U &&default_value) const & {
                if (ok()) {
                    return data_;
                }
                return std::forward<U>(default_value);
            }

            template<typename U>
            T ValueOrImpl(U &&default_value) && {
                if (ok()) {
                    return std::move(data_);
                }
                return std::forward<U>(default_value);
            }
        };

        [[noreturn]] void ThrowBadResultAccess(turbo::Status status);

        template<typename T>
        struct OperatorBase {
            auto &self() const { return static_cast<const Result<T> &>(*this); }
            auto &self() { return static_cast<Result<T> &>(*this); }

            const T &operator*() const & KUMO_ATTRIBUTE_LIFETIME_BOUND {
                self().EnsureOk();
                return self().data_;
            }

            T &operator*() & KUMO_ATTRIBUTE_LIFETIME_BOUND {
                self().EnsureOk();
                return self().data_;
            }

            const T &&operator*() const && KUMO_ATTRIBUTE_LIFETIME_BOUND {
                self().EnsureOk();
                return std::move(self().data_);
            }

            T &&operator*() && KUMO_ATTRIBUTE_LIFETIME_BOUND {
                self().EnsureOk();
                return std::move(self().data_);
            }

            const T &value() const & KUMO_ATTRIBUTE_LIFETIME_BOUND {
                if (!self().ok()) internal_statusor::ThrowBadResultAccess(self().status_);
                return self().data_;
            }

            T &value() & KUMO_ATTRIBUTE_LIFETIME_BOUND {
                if (!self().ok()) internal_statusor::ThrowBadResultAccess(self().status_);
                return self().data_;
            }

            const T &&value() const && KUMO_ATTRIBUTE_LIFETIME_BOUND {
                if (!self().ok()) {
                    internal_statusor::ThrowBadResultAccess(std::move(self().status_));
                }
                return std::move(self().data_);
            }

            T &&value() && KUMO_ATTRIBUTE_LIFETIME_BOUND {
                if (!self().ok()) {
                    internal_statusor::ThrowBadResultAccess(std::move(self().status_));
                }
                return std::move(self().data_);
            }

            const T * turbo_nonnull operator->() const KUMO_ATTRIBUTE_LIFETIME_BOUND {
                return std::addressof(**this);
            }

            T * turbo_nonnull operator->() KUMO_ATTRIBUTE_LIFETIME_BOUND {
                return std::addressof(**this);
            }
        };

        template<typename T>
        struct OperatorBase<T &> {
            auto &self() const { return static_cast<const Result<T &> &>(*this); }

            T &operator*() const {
                self().EnsureOk();
                return self().data_;
            }

            T &value() const {
                if (!self().ok()) internal_statusor::ThrowBadResultAccess(self().status_);
                return self().data_;
            }

            T * turbo_nonnull operator->() const {
                return std::addressof(**this);
            }
        };

        // Helper base classes to allow implicitly deleted constructors and assignment
        // operators in `Result`. For example, `CopyCtorBase` will explicitly delete
        // the copy constructor when T is not copy constructible and `Result` will
        // inherit that behavior implicitly.
        template<typename T, bool = std::is_copy_constructible_v<T> >
        struct CopyCtorBase {
            CopyCtorBase() = default;

            CopyCtorBase(const CopyCtorBase &) = default;

            CopyCtorBase(CopyCtorBase &&) = default;

            CopyCtorBase &operator=(const CopyCtorBase &) = default;

            CopyCtorBase &operator=(CopyCtorBase &&) = default;
        };

        template<typename T>
        struct CopyCtorBase<T, false> {
            CopyCtorBase() = default;

            CopyCtorBase(const CopyCtorBase &) = delete;

            CopyCtorBase(CopyCtorBase &&) = default;

            CopyCtorBase &operator=(const CopyCtorBase &) = default;

            CopyCtorBase &operator=(CopyCtorBase &&) = default;
        };

        template<typename T, bool = std::is_move_constructible_v<T> >
        struct MoveCtorBase {
            MoveCtorBase() = default;

            MoveCtorBase(const MoveCtorBase &) = default;

            MoveCtorBase(MoveCtorBase &&) = default;

            MoveCtorBase &operator=(const MoveCtorBase &) = default;

            MoveCtorBase &operator=(MoveCtorBase &&) = default;
        };

        template<typename T>
        struct MoveCtorBase<T, false> {
            MoveCtorBase() = default;

            MoveCtorBase(const MoveCtorBase &) = default;

            MoveCtorBase(MoveCtorBase &&) = delete;

            MoveCtorBase &operator=(const MoveCtorBase &) = default;

            MoveCtorBase &operator=(MoveCtorBase &&) = default;
        };

        template<typename T, bool = (std::is_copy_constructible_v<T> &&
                                     std::is_copy_assignable_v<T>) ||
                                    std::is_reference_v<T> >
        struct CopyAssignBase {
            CopyAssignBase() = default;

            CopyAssignBase(const CopyAssignBase &) = default;

            CopyAssignBase(CopyAssignBase &&) = default;

            CopyAssignBase &operator=(const CopyAssignBase &) = default;

            CopyAssignBase &operator=(CopyAssignBase &&) = default;
        };

        template<typename T>
        struct CopyAssignBase<T, false> {
            CopyAssignBase() = default;

            CopyAssignBase(const CopyAssignBase &) = default;

            CopyAssignBase(CopyAssignBase &&) = default;

            CopyAssignBase &operator=(const CopyAssignBase &) = delete;

            CopyAssignBase &operator=(CopyAssignBase &&) = default;
        };

        template<typename T, bool = (std::is_move_constructible_v<T> &&
                                     std::is_move_assignable_v<T>) ||
                                    std::is_reference_v<T> >
        struct MoveAssignBase {
            MoveAssignBase() = default;

            MoveAssignBase(const MoveAssignBase &) = default;

            MoveAssignBase(MoveAssignBase &&) = default;

            MoveAssignBase &operator=(const MoveAssignBase &) = default;

            MoveAssignBase &operator=(MoveAssignBase &&) = default;
        };

        template<typename T>
        struct MoveAssignBase<T, false> {
            MoveAssignBase() = default;

            MoveAssignBase(const MoveAssignBase &) = default;

            MoveAssignBase(MoveAssignBase &&) = default;

            MoveAssignBase &operator=(const MoveAssignBase &) = default;

            MoveAssignBase &operator=(MoveAssignBase &&) = delete;
        };

        // Used to introduce jitter into the output of printing functions for
        // `Result` (i.e. `turbo_stringify` and `operator<<`).
        class StringifyRandom {
            enum BracesType {
                kBareParens = 0,
                kSpaceParens,
                kBareBrackets,
                kSpaceBrackets,
            };

            // Returns a random `BracesType` determined once per binary load.
            static BracesType RandomBraces() {
                static const BracesType kRandomBraces = static_cast<BracesType>(
                    (reinterpret_cast<uintptr_t>(&kRandomBraces) >> 4) % 4);
                return kRandomBraces;
            }

        public:
            static std::string_view OpenBrackets() {
                switch (RandomBraces()) {
                    case kBareParens:
                        return "(";
                    case kSpaceParens:
                        return "( ";
                    case kBareBrackets:
                        return "[";
                    case kSpaceBrackets:
                        return "[ ";
                }
                return "(";
            }

            static std::string_view CloseBrackets() {
                switch (RandomBraces()) {
                    case kBareParens:
                        return ")";
                    case kSpaceParens:
                        return " )";
                    case kBareBrackets:
                        return "]";
                    case kSpaceBrackets:
                        return " ]";
                }
                return ")";
            }
        };
    } // namespace internal_statusor
} // namespace turbo

#endif  // TURBO_STATUS_INTERNAL_STATUSOR_INTERNAL_H_
