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
//
// -----------------------------------------------------------------------------
// File: statusor.h
// -----------------------------------------------------------------------------
//
// An `turbo::Result<T>` represents a union of an `turbo::Status` object
// and an object of type `T`. The `turbo::Result<T>` will either contain an
// object of type `T` (indicating a successful operation), or an error (of type
// `turbo::Status`) explaining why such a value is not present.
//
// In general, check the success of an operation returning an
// `turbo::Result<T>` like you would an `turbo::Status` by using the `ok()`
// member function.
//
// Example:
//
//   Result<Foo> result = Calculation();
//   if (result.ok()) {
//     result->DoSomethingCool();
//   } else {
//     KLOG(ERROR) << result.status();
//   }
#ifndef TURBO_STATUS_STATUSOR_H_
#define TURBO_STATUS_STATUSOR_H_

#include <exception>
#include <initializer_list>
#include <new>
#include <ostream>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>

#include <turbo/types/source_location.h>
#include <turbo/base/call_once.h>
#include <turbo/base/nullability.h>
#include <turbo/format/has_turbo_stringify.h>
#include <turbo/format/str_format.h>
#include <turbo/macros/config.h>
#include <turbo/meta/type_traits.h>
#include <turbo/status/internal/result_internal.h>
#include <turbo/status/status.h>
#include <turbo/strings/has_ostream_operator.h>
#include <turbo/types/span.h>
#include <turbo/types/variant.h>
#include <turbo/utility/utility.h>

namespace turbo {
    // BadResultAccess
    //
    // This class defines the type of object to throw (if exceptions are enabled),
    // when accessing the value of an `turbo::Result<T>` object that does not
    // contain a value. This behavior is analogous to that of
    // `std::bad_optional_access` in the case of accessing an invalid
    // `std::optional` value.
    //
    // Example:
    //
    // try {
    //   turbo::Result<int> v = FetchInt();
    //   DoWork(v.value());  // Accessing value() when not "OK" may throw
    // } catch (turbo::BadResultAccess& ex) {
    //   KLOG(ERROR) << ex.status();
    // }
    class BadResultAccess : public std::exception {
    public:
        explicit BadResultAccess(turbo::Status status);

        ~BadResultAccess() override = default;

        BadResultAccess(const BadResultAccess &other);

        BadResultAccess &operator=(const BadResultAccess &other);

        BadResultAccess(BadResultAccess &&other);

        BadResultAccess &operator=(BadResultAccess &&other);

        // BadResultAccess::what()
        //
        // Returns the associated explanatory string of the `turbo::Result<T>`
        // object's error code. This function contains information about the failing
        // status, but its exact formatting may change and should not be depended on.
        //
        // The pointer of this string is guaranteed to be valid until any non-const
        // function is invoked on the exception object.
        const char * turbo_nonnull what() const noexcept override;

        // BadResultAccess::status()
        //
        // Returns the associated `turbo::Status` of the `turbo::Result<T>` object's
        // error.
        const turbo::Status &status() const;

    private:
        void init_what() const;

        turbo::Status status_;
        mutable turbo::once_flag init_what_;
        mutable std::string what_;
    };

    // Returned Result objects may not be ignored.
    template<typename T>
#if KUMO_HAVE_CPP_ATTRIBUTE(nodiscard)
    // TODO(b/176172494): KUMO_MUST_USE_RESULT should expand to the more strict
    // [[nodiscard]]. For now, just use [[nodiscard]] directly when it is available.
    class [[nodiscard]] Result;
#else
    class KUMO_MUST_USE_RESULT Result;
#endif  // KUMO_HAVE_CPP_ATTRIBUTE(nodiscard)

    // turbo::Result<T>
    //
    // The `turbo::Result<T>` class template is a union of an `turbo::Status` object
    // and an object of type `T`. The `turbo::Result<T>` models an object that is
    // either a usable object, or an error (of type `turbo::Status`) explaining why
    // such an object is not present. An `turbo::Result<T>` is typically the return
    // value of a function which may fail.
    //
    // An `turbo::Result<T>` can never hold an "OK" status (an
    // `turbo::StatusCode::kOk` value); instead, the presence of an object of type
    // `T` indicates success. Instead of checking for a `kOk` value, use the
    // `turbo::Result<T>::ok()` member function. (It is for this reason, and code
    // readability, that using the `ok()` function is preferred for `turbo::Status`
    // as well.)
    //
    // Example:
    //
    //   Result<Foo> result = DoBigCalculationThatCouldFail();
    //   if (result.ok()) {
    //     result->DoSomethingCool();
    //   } else {
    //     KLOG(ERROR) << result.status();
    //   }
    //
    // Accessing the object held by an `turbo::Result<T>` should be performed via
    // `operator*` or `operator->`, after a call to `ok()` confirms that the
    // `turbo::Result<T>` holds an object of type `T`:
    //
    // Example:
    //
    //   turbo::Result<int> i = GetCount();
    //   if (i.ok()) {
    //     updated_total += *i;
    //   }
    //
    // NOTE: using `turbo::Result<T>::value()` when no valid value is present will
    // throw an exception if exceptions are enabled or terminate the process when
    // exceptions are not enabled.
    //
    // Example:
    //
    //   Result<Foo> result = DoBigCalculationThatCouldFail();
    //   const Foo& foo = result.value();    // Crash/exception if no value present
    //   foo.DoSomethingCool();
    //
    // A `turbo::Result<T*>` can be constructed from a null pointer like any other
    // pointer value, and the result will be that `ok()` returns `true` and
    // `value()` returns `nullptr`. Checking the value of pointer in an
    // `turbo::Result<T*>` generally requires a bit more care, to ensure both that
    // a value is present and that value is not null:
    //
    //  Result<std::unique_ptr<Foo>> result = FooFactory::MakeNewFoo(arg);
    //  if (!result.ok()) {
    //    KLOG(ERROR) << result.status();
    //  } else if (*result == nullptr) {
    //    KLOG(ERROR) << "Unexpected null pointer";
    //  } else {
    //    (*result)->DoSomethingCool();
    //  }
    //
    // Example factory implementation returning Result<T>:
    //
    //  Result<Foo> FooFactory::MakeFoo(int arg) {
    //    if (arg <= 0) {
    //      return turbo::Status(turbo::StatusCode::kInvalidArgument,
    //                          "Arg must be positive");
    //    }
    //    return Foo(arg);
    //  }
    template<typename T>
    class Result : private internal_statusor::OperatorBase<T>,
                     private internal_statusor::ResultData<T>,
                     private internal_statusor::CopyCtorBase<T>,
                     private internal_statusor::MoveCtorBase<T>,
                     private internal_statusor::CopyAssignBase<T>,
                     private internal_statusor::MoveAssignBase<T> {
#ifndef SWIG
        static_assert(!std::is_rvalue_reference_v<T>,
                      "rvalue references are not yet supported.");
#endif  // SWIG

        template<typename U>
        friend class Result;

        friend internal_statusor::OperatorBase<T>;

        typedef internal_statusor::ResultData<T> Base;

    public:
        // Result<T>::value_type
        //
        // This instance data provides a generic `value_type` member for use within
        // generic programming. This usage is analogous to that of
        // `optional::value_type` in the case of `std::optional`.
        typedef T value_type;

        // Constructors

        // Constructs a new `turbo::Result` with an `turbo::StatusCode::kUnknown`
        // status. This constructor is marked 'explicit' to prevent usages in return
        // values such as 'return {};', under the misconception that
        // `turbo::Result<std::vector<int>>` will be initialized with an empty
        // vector, instead of an `turbo::StatusCode::kUnknown` error code.
        explicit Result();

        // `Result<T>` is copy constructible if `T` is copy constructible.
        Result(const Result &) = default;

        // `Result<T>` is copy assignable if `T` is copy constructible and copy
        // assignable.
        Result &operator=(const Result &) = default;

        // `Result<T>` is move constructible if `T` is move constructible.
        Result(Result &&) = default;

        // `Result<T>` is moveAssignable if `T` is move constructible and move
        // assignable.
        Result &operator=(Result &&) = default;

        // Converting Constructors

        // Constructs a new `turbo::Result<T>` from an `turbo::Result<U>`, when `T`
        // is constructible from `U`. To avoid ambiguity, these constructors are
        // disabled if `T` is also constructible from `Result<U>.`. This constructor
        // is explicit if and only if the corresponding construction of `T` from `U`
        // is explicit. (This constructor inherits its explicitness from the
        // underlying constructor.)
        template<typename U, std::enable_if_t<
            internal_statusor::IsConstructionFromResultValid<
                false, T, U, false, const U &>::value,
            int> = 0>
        Result(const Result<U> &other) // NOLINT
            : Base(static_cast<const typename Result<U>::Base &>(other)) {
        }

        template<typename U, std::enable_if_t<
            internal_statusor::IsConstructionFromResultValid<
                false, T, U, true, const U &>::value,
            int> = 0>
        Result(const Result<U> &other KUMO_ATTRIBUTE_LIFETIME_BOUND) // NOLINT
            : Base(static_cast<const typename Result<U>::Base &>(other)) {
        }

        template<typename U, std::enable_if_t<
            internal_statusor::IsConstructionFromResultValid<
                true, T, U, false, const U &>::value,
            int> = 0>
        explicit Result(const Result<U> &other)
            : Base(static_cast<const typename Result<U>::Base &>(other)) {
        }

        template<typename U, std::enable_if_t<
            internal_statusor::IsConstructionFromResultValid<
                true, T, U, true, const U &>::value,
            int> = 0>
        explicit Result(const Result<U> &other KUMO_ATTRIBUTE_LIFETIME_BOUND)
            : Base(static_cast<const typename Result<U>::Base &>(other)) {
        }

        template<typename U, std::enable_if_t<
            internal_statusor::IsConstructionFromResultValid<
                false, T, U, false, U &&>::value,
            int> = 0>
        Result(Result<U> &&other) // NOLINT
            : Base(static_cast<typename Result<U>::Base &&>(other)) {
        }

        template<typename U, std::enable_if_t<
            internal_statusor::IsConstructionFromResultValid<
                false, T, U, true, U &&>::value,
            int> = 0>
        Result(Result<U> &&other KUMO_ATTRIBUTE_LIFETIME_BOUND) // NOLINT
            : Base(static_cast<typename Result<U>::Base &&>(other)) {
        }

        template<typename U, std::enable_if_t<
            internal_statusor::IsConstructionFromResultValid<
                true, T, U, false, U &&>::value,
            int> = 0>
        explicit Result(Result<U> &&other)
            : Base(static_cast<typename Result<U>::Base &&>(other)) {
        }

        template<typename U, std::enable_if_t<
            internal_statusor::IsConstructionFromResultValid<
                true, T, U, true, U &&>::value,
            int> = 0>
        explicit Result(Result<U> &&other KUMO_ATTRIBUTE_LIFETIME_BOUND)
            : Base(static_cast<typename Result<U>::Base &&>(other)) {
        }

        // Converting Assignment Operators

        // Creates an `turbo::Result<T>` through assignment from an
        // `turbo::Result<U>` when:
        //
        //   * Both `turbo::Result<T>` and `turbo::Result<U>` are OK by assigning
        //     `U` to `T` directly.
        //   * `turbo::Result<T>` is OK and `turbo::Result<U>` contains an error
        //      code by destroying `turbo::Result<T>`'s value and assigning from
        //      `turbo::Result<U>'
        //   * `turbo::Result<T>` contains an error code and `turbo::Result<U>` is
        //      OK by directly initializing `T` from `U`.
        //   * Both `turbo::Result<T>` and `turbo::Result<U>` contain an error
        //     code by assigning the `Status` in `turbo::Result<U>` to
        //     `turbo::Result<T>`
        //
        // These overloads only apply if `turbo::Result<T>` is constructible and
        // assignable from `turbo::Result<U>` and `Result<T>` cannot be directly
        // assigned from `Result<U>`.
        template<typename U,
            std::enable_if_t<internal_statusor::IsResultAssignmentValid<
                    T, const U &, false>::value,
                int> = 0>
        Result &operator=(const Result<U> &other) {
            this->Assign(other);
            return *this;
        }

        template<typename U,
            std::enable_if_t<internal_statusor::IsResultAssignmentValid<
                    T, const U &, true>::value,
                int> = 0>
        Result &operator=(const Result<U> &other KUMO_ATTRIBUTE_LIFETIME_BOUND) {
            this->Assign(other);
            return *this;
        }

        template<typename U,
            std::enable_if_t<internal_statusor::IsResultAssignmentValid<
                    T, U &&, false>::value,
                int> = 0>
        Result &operator=(Result<U> &&other) {
            this->Assign(std::move(other));
            return *this;
        }

        template<typename U,
            std::enable_if_t<internal_statusor::IsResultAssignmentValid<
                    T, U &&, true>::value,
                int> = 0>
        Result &operator=(Result<U> &&other KUMO_ATTRIBUTE_LIFETIME_BOUND) {
            this->Assign(std::move(other));
            return *this;
        }

        // Constructs a new `turbo::Result<T>` with a non-ok status. After calling
        // this constructor, `this->ok()` will be `false` and calls to `value()` will
        // crash, or produce an exception if exceptions are enabled.
        //
        // The constructor also takes any type `U` that is convertible to
        // `turbo::Status`. This constructor is explicit if an only if `U` is not of
        // type `turbo::Status` and the conversion from `U` to `Status` is explicit.
        //
        // REQUIRES: !Status(std::forward<U>(v)).ok(). This requirement is DCHECKed.
        // In optimized builds, passing turbo::ok_status() here will have the effect
        // of passing turbo::StatusCode::kInternal as a fallback.
        template<typename U = turbo::Status,
            std::enable_if_t<internal_statusor::IsConstructionFromStatusValid<
                    false, T, U>::value,
                int> = 0>
        Result(U &&v) : Base(std::forward<U>(v)) {
        }

        template<typename U = turbo::Status,
            std::enable_if_t<internal_statusor::IsConstructionFromStatusValid<
                    true, T, U>::value,
                int> = 0>
        explicit Result(U &&v) : Base(std::forward<U>(v)) {
        }

        template<typename U = turbo::Status,
            std::enable_if_t<internal_statusor::IsConstructionFromStatusValid<
                    false, T, U>::value,
                int> = 0>
        Result &operator=(U &&v) {
            this->AssignStatus(std::forward<U>(v));
            return *this;
        }

        // Perfect-forwarding value assignment operator.

        // If `*this` contains a `T` value before the call, the contained value is
        // assigned from `std::forward<U>(v)`; Otherwise, it is directly-initialized
        // from `std::forward<U>(v)`.
        // This function does not participate in overload unless:
        // 1. `std::is_constructible_v<T, U>` is true,
        // 2. `std::is_assignable_v<T&, U>` is true.
        // 3. `std::is_same_v<Result<T>, std::remove_cvref_t<U>>` is false.
        // 4. Assigning `U` to `T` is not ambiguous:
        //  If `U` is `Result<V>` and `T` is constructible and assignable from
        //  both `Result<V>` and `V`, the assignment is considered bug-prone and
        //  ambiguous thus will fail to compile. For example:
        //    Result<bool> s1 = true;  // s1.ok() && *s1 == true
        //    Result<bool> s2 = false;  // s2.ok() && *s2 == false
        //    s1 = s2;  // ambiguous, `s1 = *s2` or `s1 = bool(s2)`?
        template<
            typename U = T,
            std::enable_if_t<internal_statusor::IsAssignmentValid<T, U, false>::value,
                int> = 0>
        Result &operator=(U &&v) {
            this->Assign(std::forward<U>(v));
            return *this;
        }

        template<
            typename U = T,
            std::enable_if_t<internal_statusor::IsAssignmentValid<T, U, true>::value,
                int> = 0>
        Result &operator=(U &&v KUMO_INTERNAL_ATTRIBUTE_CAPTURED_BY_THIS) {
            this->Assign(std::forward<U>(v));
            return *this;
        }

        // Constructs the inner value `T` in-place using the provided args, using the
        // `T(args...)` constructor.
        template<typename... Args>
        explicit Result(std::in_place_t, Args &&... args);

        template<typename U, typename... Args>
        explicit Result(std::in_place_t, std::initializer_list<U> ilist,
                          Args &&... args);

        // Constructs the inner value `T` in-place using the provided args, using the
        // `T(U)` (direct-initialization) constructor. This constructor is only valid
        // if `T` can be constructed from a `U`. Can accept move or copy constructors.
        //
        // This constructor is explicit if `U` is not convertible to `T`. To avoid
        // ambiguity, this constructor is disabled if `U` is a `Result<J>`, where
        // `J` is convertible to `T`.
        template<typename U = T,
            std::enable_if_t<internal_statusor::IsConstructionValid<
                    false, T, U, false>::value,
                int> = 0>
        Result(U &&u) // NOLINT
            : Result(std::in_place, std::forward<U>(u)) {
        }

        template<typename U = T,
            std::enable_if_t<internal_statusor::IsConstructionValid<
                    false, T, U, true>::value,
                int> = 0>
        Result(U &&u KUMO_ATTRIBUTE_LIFETIME_BOUND) // NOLINT
            : Result(std::in_place, std::forward<U>(u)) {
        }

        template<typename U = T,
            std::enable_if_t<internal_statusor::IsConstructionValid<
                    true, T, U, false>::value,
                int> = 0>
        explicit Result(U &&u) // NOLINT
            : Result(std::in_place, std::forward<U>(u)) {
        }

        template<typename U = T,
            std::enable_if_t<
                internal_statusor::IsConstructionValid<true, T, U, true>::value,
                int> = 0>
        explicit Result(U &&u KUMO_ATTRIBUTE_LIFETIME_BOUND) // NOLINT
            : Result(std::in_place, std::forward<U>(u)) {
        }

        // Result<T>::ok()
        //
        // Returns whether or not this `turbo::Result<T>` holds a `T` value. This
        // member function is analogous to `turbo::Status::ok()` and should be used
        // similarly to check the status of return values.
        //
        // Example:
        //
        // Result<Foo> result = DoBigCalculationThatCouldFail();
        // if (result.ok()) {
        //    // Handle result
        // else {
        //    // Handle error
        // }
        KUMO_MUST_USE_RESULT bool ok() const { return this->status_.ok(); }

        // Result<T>::status()
        //
        // Returns a reference to the current `turbo::Status` contained within the
        // `turbo::Result<T>`. If `turbo::Result<T>` contains a `T`, then this
        // function returns `turbo::ok_status()`.
        KUMO_MUST_USE_RESULT const Status &status() const &;

        Status status() &&;

        turbo::Span<const turbo::SourceLocation> get_source_locations() const {
            return this->status_.get_source_locations();
        }

        // Appends the `loc` to the current location chain inside the status, iff the
        // status-or is non-ok and contains a non-empty message.
        void add_source_location(
            turbo::SourceLocation loc = turbo::SourceLocation::current()) {
            this->status_.add_source_location(loc);
        }

        // Result<T>::with_source_location()
        //
        // Appends the `loc` to the current location chain inside the status iff the
        // status-or is non-ok and contains a non-empty message, and returns an rvalue
        // reference to `*this`.
        //
        // Example:
        //
        //   Result<int> Finalize(...);
        //
        //   Result<int> DoSomething(...) {
        //     ...
        //     return Finalize().with_source_location();
        //   }
        KUMO_MUST_USE_RESULT Result<T> &&with_source_location(
            turbo::SourceLocation loc = turbo::SourceLocation::current()) && {
            add_source_location(loc);
            return std::move(*this);
        }

        SubStatusType sub_type() const { return this->status_.sub_type(); }
        int32_t sub_code() const { return this->status_.sub_code(); }

        void add_sub_code(int32_t code, int8_t type = kSubUser) {
            this->status_.add_sub_code(code, type);
        }

        KUMO_MUST_USE_RESULT Result<T> &&with_sub_code(int32_t code,
            int8_t type = kSubUser) && {
            add_sub_code(code, type);
            return std::move(*this);
        }

        void add_errno(int32_t code) { this->status_.add_errno(code); }

        KUMO_MUST_USE_RESULT Result<T> &&with_errno(int32_t code) && {
            add_errno(code);
            return std::move(*this);
        }

        void add_signal(int32_t code) { this->status_.add_signal(code); }

        KUMO_MUST_USE_RESULT Result<T> &&with_signal(int32_t code) && {
            add_signal(code);
            return std::move(*this);
        }

        void clear_sub_code() { this->status_.clear_sub_code(); }

        KUMO_MUST_USE_RESULT Result<T> &&without_sub_code() && {
            clear_sub_code();
            return std::move(*this);
        }

        // Result<T>::value()
        //
        // Returns a reference to the held value if `this->ok()`. Otherwise, throws
        // `turbo::BadResultAccess` if exceptions are enabled, or is guaranteed to
        // terminate the process if exceptions are disabled.
        //
        // If you have already checked the status using `this->ok()`, you probably
        // want to use `operator*()` or `operator->()` to access the value instead of
        // `value`.
        //
        // Note: for value types that are cheap to copy, prefer simple code:
        //
        //   T value = statusor.value();
        //
        // Otherwise, if the value type is expensive to copy, but can be left
        // in the Result, simply assign to a reference:
        //
        //   T& value = statusor.value();  // or `const T&`
        //
        // Otherwise, if the value type supports an efficient move, it can be
        // used as follows:
        //
        //   T value = std::move(statusor).value();
        //
        // The `std::move` on statusor instead of on the whole expression enables
        // warnings about possible uses of the statusor object after the move.
        using Result::OperatorBase::value;

        // Result<T>:: operator*()
        //
        // Returns a reference to the current value.
        //
        // REQUIRES: `this->ok() == true`, otherwise the behavior is undefined.
        //
        // Use `this->ok()` to verify that there is a current value within the
        // `turbo::Result<T>`. Alternatively, see the `value()` member function for a
        // similar API that guarantees crashing or throwing an exception if there is
        // no current value.
        using Result::OperatorBase::operator*;

        // Result<T>::operator->()
        //
        // Returns a pointer to the current value.
        //
        // REQUIRES: `this->ok() == true`, otherwise the behavior is undefined.
        //
        // Use `this->ok()` to verify that there is a current value.
        using Result::OperatorBase::operator->;

        // Result<T>::value_or()
        //
        // Returns the current value if `this->ok() == true`. Otherwise constructs a
        // value using the provided `default_value`.
        //
        // Unlike `value`, this function returns by value, copying the current value
        // if necessary. If the value type supports an efficient move, it can be used
        // as follows:
        //
        //   T value = std::move(statusor).value_or(def);
        //
        // Unlike with `value`, calling `std::move()` on the result of `value_or` will
        // still trigger a copy.
        template<
            typename U,
            std::enable_if_t<internal_statusor::IsValueOrValid<T, U &&, false>::value,
                int> = 0>
        T value_or(U &&default_value) const & {
            return this->ValueOrImpl(std::forward<U>(default_value));
        }

        template<
            typename U,
            std::enable_if_t<internal_statusor::IsValueOrValid<T, U &&, false>::value,
                int> = 0>
        T value_or(U &&default_value) && {
            return std::move(*this).ValueOrImpl(std::forward<U>(default_value));
        }

        template<
            typename U,
            std::enable_if_t<internal_statusor::IsValueOrValid<T, U &&, true>::value,
                int> = 0>
        T value_or(U &&default_value KUMO_ATTRIBUTE_LIFETIME_BOUND) const & {
            return this->ValueOrImpl(std::forward<U>(default_value));
        }

        template<
            typename U,
            std::enable_if_t<internal_statusor::IsValueOrValid<T, U &&, true>::value,
                int> = 0>
        T value_or(U &&default_value KUMO_ATTRIBUTE_LIFETIME_BOUND) && {
            return std::move(*this).ValueOrImpl(std::forward<U>(default_value));
        }

        // Result<T>::ignore_error()
        //
        // Ignores any errors. This method does nothing except potentially suppress
        // complaints from any tools that are checking that errors are not dropped on
        // the floor.
        void ignore_error() const;

        // Result<T>::emplace()
        //
        // Reconstructs the inner value T in-place using the provided args, using the
        // T(args...) constructor. Returns reference to the reconstructed `T`.
        template<typename... Args>
        T &emplace(Args &&... args) KUMO_ATTRIBUTE_LIFETIME_BOUND {
            if (ok()) {
                this->Clear();
                // Temporarily transition to a non-ok status (using the zero-allocation
                // inlined representation) so that if MakeValue() throws an exception,
                // ok() returns false during stack unwinding and ~ResultData() does not
                // attempt to destroy uninitialized memory.
                this->status_ = turbo::Status(turbo::StatusCode::kInternal);
            }
            this->MakeValue(std::forward<Args>(args)...);
            this->status_ = turbo::ok_status();
            return this->data_;
        }

        template<typename U, typename... Args,
            std::enable_if_t<std::is_constructible_v<
                    T, std::initializer_list<U> &, Args &&...>,
                int> = 0>
        T &emplace(std::initializer_list<U> ilist,
                   Args &&... args) KUMO_ATTRIBUTE_LIFETIME_BOUND {
            if (ok()) {
                this->Clear();
                // Temporarily transition to a non-ok status (using the zero-allocation
                // inlined representation) so that if MakeValue() throws an exception,
                // ok() returns false during stack unwinding and ~ResultData() does not
                // attempt to destroy uninitialized memory.
                this->status_ = turbo::Status(turbo::StatusCode::kInternal);
            }
            this->MakeValue(ilist, std::forward<Args>(args)...);
            this->status_ = turbo::ok_status();
            return this->data_;
        }

        // Result<T>::AssignStatus()
        //
        // Sets the status of `turbo::Result<T>` to the given non-ok status value.
        //
        // NOTE: We recommend using the constructor and `operator=` where possible.
        // This method is intended for use in generic programming, to enable setting
        // the status of a `Result<T>` when `T` may be `Status`. In that case, the
        // constructor and `operator=` would assign into the inner value of type
        // `Status`, rather than status of the `Result` (b/280392796).
        //
        // REQUIRES: !Status(std::forward<U>(v)).ok(). This requirement is DCHECKed.
        // In optimized builds, passing turbo::ok_status() here will have the effect
        // of passing turbo::StatusCode::kInternal as a fallback.
        using internal_statusor::ResultData<T>::AssignStatus;

    private:
        using internal_statusor::ResultData<T>::Assign;

        template<typename U>
        void Assign(const turbo::Result<U> &other);

        template<typename U>
        void Assign(turbo::Result<U> &&other);
    };

    // operator==()
    //
    // This operator checks the equality of two `turbo::Result<T>` objects.
    template<typename T,
        std::enable_if_t<internal_statusor::IsEqualityComparable<T>::value,
            int> = 0>
    bool operator==(const Result<T> &lhs, const Result<T> &rhs) {
        if (lhs.ok() && rhs.ok()) return *lhs == *rhs;
        return lhs.status() == rhs.status();
    }

    // operator!=()
    //
    // This operator checks the inequality of two `turbo::Result<T>` objects.
    template<typename T,
        std::enable_if_t<internal_statusor::IsEqualityComparable<T>::value,
            int> = 0>
    bool operator!=(const Result<T> &lhs, const Result<T> &rhs) {
        return !(lhs == rhs);
    }

    // Prints the `value` or the status in brackets to `os`.
    //
    // Requires `T` supports `operator<<`.  Do not rely on the output format which
    // may change without notice.
    template<typename T,
        std::enable_if_t<turbo::HasOstreamOperator<T>::value, int> = 0>
    std::ostream &operator<<(std::ostream &os, const Result<T> &status_or) {
        if (status_or.ok()) {
            os << status_or.value();
        } else {
            os << internal_statusor::StringifyRandom::OpenBrackets()
                    << status_or.status()
                    << internal_statusor::StringifyRandom::CloseBrackets();
        }
        return os;
    }

    // As above, but supports `str_cat`, `str_sprintf`, etc.
    //
    // Requires `T` has `turbo_stringify`.  Do not rely on the output format which
    // may change without notice.
    template<typename Sink, typename T,
        std::enable_if_t<turbo::HasTurboStringify<T>::value, int> = 0>
    void turbo_stringify(Sink &sink, const Result<T> &status_or) {
        if (status_or.ok()) {
            turbo::str_printf_to(&sink, "%v", status_or.value());
        } else {
            turbo::str_printf_to(&sink, "%s%v%s",
                          internal_statusor::StringifyRandom::OpenBrackets(),
                          status_or.status(),
                          internal_statusor::StringifyRandom::CloseBrackets());
        }
    }

    //------------------------------------------------------------------------------
    // Implementation details for Result<T>
    //------------------------------------------------------------------------------

    // TODO(sbenza): avoid the string here completely.
    template<typename T>
    Result<T>::Result() : Base(Status(turbo::StatusCode::kUnknown, "")) {
    }

    template<typename T>
    template<typename U>
    inline void Result<T>::Assign(const Result<U> &other) {
        if (other.ok()) {
            this->Assign(*other);
        } else {
            this->AssignStatus(other.status());
        }
    }

    template<typename T>
    template<typename U>
    inline void Result<T>::Assign(Result<U> &&other) {
        if (other.ok()) {
            this->Assign(*std::move(other));
        } else {
            this->AssignStatus(std::move(other).status());
        }
    }

    template<typename T>
    template<typename... Args>
    Result<T>::Result(std::in_place_t, Args &&... args)
        : Base(std::in_place, std::forward<Args>(args)...) {
    }

    template<typename T>
    template<typename U, typename... Args>
    Result<T>::Result(std::in_place_t, std::initializer_list<U> ilist,
                          Args &&... args)
        : Base(std::in_place, ilist, std::forward<Args>(args)...) {
    }

    template<typename T>
    const Status &Result<T>::status() const & {
        return this->status_;
    }

    template<typename T>
    Status Result<T>::status() && {
        return ok() ? ok_status() : std::move(this->status_);
    }

    template<typename T>
    void Result<T>::ignore_error() const {
        // no-op
    }
} // namespace turbo

#endif  // TURBO_STATUS_STATUSOR_H_
