// Copyright 2022 The Abseil Authors.
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
// File: log/internal/check_op.h
// -----------------------------------------------------------------------------
//
// This file declares helpers routines and macros used to implement `KCHECK`
// macros.

#ifndef TURBO_LOG_INTERNAL_CHECK_OP_H_
#define TURBO_LOG_INTERNAL_CHECK_OP_H_

#include <stdint.h>

#include <cstddef>
#include <ostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

#include <turbo/macros/config.h>
#include <turbo/bits/casts.h>
#include <turbo/base/nullability.h>
#include <turbo/log/internal/nullguard.h>
#include <turbo/log/internal/nullstream.h>
#include <turbo/log/internal/strip.h>
#include <turbo/format/has_turbo_stringify.h>
#include <turbo/strings/has_ostream_operator.h>
#include <string_view>

// `TURBO_LOG_INTERNAL_STRIP_STRING_LITERAL` wraps string literals that
// should be stripped when `TURBO_MIN_LOG_LEVEL` exceeds `kFatal`.
#ifdef TURBO_MIN_LOG_LEVEL
#define TURBO_LOG_INTERNAL_STRIP_STRING_LITERAL(literal)                \
  (::turbo::LogSeverity::kFatal >=                                      \
           static_cast<::turbo::LogSeverityAtLeast>(TURBO_MIN_LOG_LEVEL) \
       ? (literal)                                                     \
       : "")
#else
#define TURBO_LOG_INTERNAL_STRIP_STRING_LITERAL(literal) (literal)
#endif

#ifdef NDEBUG
// `NDEBUG` is defined, so `DKCHECK_EQ(x, y)` and so on do nothing.  However, we
// still want the compiler to parse `x` and `y`, because we don't want to lose
// potentially useful errors and warnings.
#define TURBO_LOG_INTERNAL_DCHECK_NOP(x, y)   \
  while (false && ((void)(x), (void)(y), 0)) \
  ::turbo::log_internal::NullStream().internal_stream()
#endif

#define TURBO_LOG_INTERNAL_CHECK_OP(name, op, val1, val1_text, val2, val2_text) \
  while (const char* turbo_nullable turbo_log_internal_check_op_result           \
         [[maybe_unused]] = ::turbo::log_internal::name##Impl(                  \
             ::turbo::log_internal::get_referenceable_value(val1),                \
             ::turbo::log_internal::get_referenceable_value(val2),                \
             TURBO_LOG_INTERNAL_STRIP_STRING_LITERAL(val1_text " " #op          \
                                                              " " val2_text))) \
    TURBO_LOG_INTERNAL_CONDITION_FATAL(STATELESS, true)                         \
  TURBO_LOG_INTERNAL_CHECK(::turbo::implicit_cast<const char* turbo_nonnull>(     \
                              turbo_log_internal_check_op_result))              \
      .internal_stream()
#define TURBO_LOG_INTERNAL_QCHECK_OP(name, op, val1, val1_text, val2,        \
    val2_text)                              \
  while (const char* turbo_nullable turbo_log_internal_qcheck_op_result =     \
             ::turbo::log_internal::name##Impl(                              \
                 ::turbo::log_internal::get_referenceable_value(val1),         \
                 ::turbo::log_internal::get_referenceable_value(val2),         \
                 TURBO_LOG_INTERNAL_STRIP_STRING_LITERAL(                    \
                     val1_text " " #op " " val2_text)))                     \
    TURBO_LOG_INTERNAL_CONDITION_QFATAL(STATELESS, true)                     \
  TURBO_LOG_INTERNAL_QCHECK(::turbo::implicit_cast<const char* turbo_nonnull>( \
                               turbo_log_internal_qcheck_op_result))         \
      .internal_stream()
#define TURBO_LOG_INTERNAL_CHECK_STROP(func, op, expected, s1, s1_text, s2,     \
    s2_text)                                 \
  while (const char* turbo_nullable turbo_log_internal_check_strop_result =      \
             ::turbo::log_internal::Check##func##expected##Impl(                \
                 (s1), (s2),                                                   \
                 TURBO_LOG_INTERNAL_STRIP_STRING_LITERAL(s1_text " " #op        \
                                                                " " s2_text))) \
    TURBO_LOG_INTERNAL_CONDITION_FATAL(STATELESS, true)                         \
  TURBO_LOG_INTERNAL_CHECK(::turbo::implicit_cast<const char* turbo_nonnull>(     \
                              turbo_log_internal_check_strop_result))           \
      .internal_stream()
#define TURBO_LOG_INTERNAL_QCHECK_STROP(func, op, expected, s1, s1_text, s2,    \
    s2_text)                                \
  while (const char* turbo_nullable turbo_log_internal_qcheck_strop_result =     \
             ::turbo::log_internal::Check##func##expected##Impl(                \
                 (s1), (s2),                                                   \
                 TURBO_LOG_INTERNAL_STRIP_STRING_LITERAL(s1_text " " #op        \
                                                                " " s2_text))) \
    TURBO_LOG_INTERNAL_CONDITION_QFATAL(STATELESS, true)                        \
  TURBO_LOG_INTERNAL_QCHECK(::turbo::implicit_cast<const char* turbo_nonnull>(    \
                               turbo_log_internal_qcheck_strop_result))         \
      .internal_stream()

// This one is tricky:
// * We must evaluate `val` exactly once, yet we need to do two things with it:
//   evaluate `.ok()` and (sometimes) `.ToString()`.
// * `val` might be an `turbo::Status` or some `turbo::StatusOr<T>`.
// * `val` might be e.g. `ATemporary().GetStatus()`, which may return a
//   reference to a member of `ATemporary` that is only valid until the end of
//   the full expression.
// * We don't want this file to depend on `turbo::Status` `#include`s or linkage,
//   nor do we want to move the definition to status and introduce a dependency
//   in the other direction.  We can be assured that callers must already have a
//   `Status` and the necessary `#include`s and linkage.
// * Callsites should be small and fast (at least when `val.ok()`): one branch,
//   minimal stack footprint.
//   * In particular, the string concat stuff should be out-of-line and emitted
//     in only one TU to save linker input size
// * We want the `val.ok()` check inline so static analyzers and optimizers can
//   see it.
// * As usual, no braces so we can stream into the expansion with `operator<<`.
// * Also as usual, it must expand to a single (partial) statement with no
//   ambiguous-else problems.
// * When stripped by `TURBO_MIN_LOG_LEVEL`, we must discard the `<expr> is OK`
//   string literal and abort without doing any streaming.  We don't need to
//   strip the call to stringify the non-ok `Status` as long as we don't log it;
//   dropping the `Status`'s message text is out of scope.
#define TURBO_LOG_INTERNAL_CHECK_OK(val, val_text)                         \
  for (::std::pair<const ::turbo::Status* turbo_nonnull,                    \
                   const char* turbo_nonnull>                              \
           turbo_log_internal_check_ok_goo;                                \
       turbo_log_internal_check_ok_goo.first =                             \
           ::turbo::log_internal::AsStatus(val),                           \
       turbo_log_internal_check_ok_goo.second =                            \
           KUMO_LIKELY(turbo_log_internal_check_ok_goo.first->ok())  \
               ? "" /* Don't use nullptr, to keep the annotation happy */ \
               : ::turbo::status_internal::make_check_fail_string(            \
                     turbo_log_internal_check_ok_goo.first,                \
                     TURBO_LOG_INTERNAL_STRIP_STRING_LITERAL(val_text      \
                                                            " is OK")),   \
       !KUMO_LIKELY(turbo_log_internal_check_ok_goo.first->ok());)   \
    TURBO_LOG_INTERNAL_CONDITION_FATAL(STATELESS, true)                    \
  TURBO_LOG_INTERNAL_CHECK(turbo_log_internal_check_ok_goo.second)          \
      .internal_stream()
#define TURBO_LOG_INTERNAL_QCHECK_OK(val, val_text)                        \
  for (::std::pair<const ::turbo::Status* turbo_nonnull,                    \
                   const char* turbo_nonnull>                              \
           turbo_log_internal_qcheck_ok_goo;                               \
       turbo_log_internal_qcheck_ok_goo.first =                            \
           ::turbo::log_internal::AsStatus(val),                           \
       turbo_log_internal_qcheck_ok_goo.second =                           \
           KUMO_LIKELY(turbo_log_internal_qcheck_ok_goo.first->ok()) \
               ? "" /* Don't use nullptr, to keep the annotation happy */ \
               : ::turbo::status_internal::make_check_fail_string(            \
                     turbo_log_internal_qcheck_ok_goo.first,               \
                     TURBO_LOG_INTERNAL_STRIP_STRING_LITERAL(val_text      \
                                                            " is OK")),   \
       !KUMO_LIKELY(turbo_log_internal_qcheck_ok_goo.first->ok());)  \
    TURBO_LOG_INTERNAL_CONDITION_QFATAL(STATELESS, true)                   \
  TURBO_LOG_INTERNAL_QCHECK(turbo_log_internal_qcheck_ok_goo.second)        \
      .internal_stream()

namespace turbo {
    class Status;
    template<typename T>
    class StatusOr;

    namespace status_internal {
KUMO_ATTRIBUTE_PURE_FUNCTION const char * turbo_nonnull make_check_fail_string(
            const turbo::Status * turbo_nonnull status, const char * turbo_nonnull prefix);
    } // namespace status_internal

    namespace log_internal {
        // Convert a Status or a StatusOr to its underlying status value.
        //
        // (This implementation does not require a dep on turbo::Status to work.)
        inline const turbo::Status * turbo_nonnull AsStatus(const turbo::Status &s) {
            return &s;
        }

        template<typename T>
        const turbo::Status * turbo_nonnull AsStatus(const turbo::StatusOr<T> &s) {
            return &s.status();
        }

        // A helper class for formatting `expr (V1 vs. V2)` in a `CHECK_XX` statement.
        // See `make_check_op_string` for sample usage.
        class CheckOpMessageBuilder final {
        public:
            // Inserts `exprtext` and ` (` to the stream.
            explicit CheckOpMessageBuilder(const char * turbo_nonnull exprtext);

            ~CheckOpMessageBuilder() = default;

            // For inserting the first variable.
            std::ostream &ForVar1() { return stream_; }

            // For inserting the second variable (adds an intermediate ` vs. `).
            std::ostream &ForVar2();

            // Get the result (inserts the closing `)`).
            const char * turbo_nonnull NewString();

        private:
            std::ostringstream stream_;
        };

        // This formats a value for a failing `CHECK_XX` statement.  Ordinarily, it uses
        // the definition for `operator<<`, with a few special cases below.
        template<typename T>
        inline void make_check_op_value_string(std::ostream &os, const T &v) {
            os << log_internal::NullGuard<T>::Guard(v);
        }

        // Overloads for char types provide readable values for unprintable characters.
        void make_check_op_value_string(std::ostream &os, char v);

        void make_check_op_value_string(std::ostream &os, signed char v);

        void make_check_op_value_string(std::ostream &os, unsigned char v);

        void make_check_op_value_string(std::ostream &os, const void * turbo_nullable p);

        // A wrapper for types that have no operator<<.
        struct UnprintableWrapper {
            template<typename T>
            explicit UnprintableWrapper(const T &) {
            }

            friend std::ostream &operator<<(std::ostream &os, UnprintableWrapper);
        };

        namespace detect_specialization {
            // make_check_op_string is being specialized for every T and U pair that is being
            // passed to the CHECK_op macros. However, there is a lot of redundancy in these
            // specializations that creates unnecessary library and binary bloat.
            // The number of instantiations tends to be O(n^2) because we have two
            // independent inputs. This technique works by reducing `n`.
            //
            // Most user-defined types being passed to CHECK_op end up being printed as a
            // builtin type. For example, enums tend to be implicitly converted to its
            // underlying type when calling operator<<, and pointers are printed with the
            // `const void*` overload.
            // To reduce the number of instantiations we coerce these values before calling
            // make_check_op_string instead of inside it.
            //
            // To detect if this coercion is needed, we duplicate all the relevant
            // operator<< overloads as specified in the standard, just in a different
            // namespace. If the call to `stream << value` becomes ambiguous, it means that
            // one of these overloads is the one selected by overload resolution. We then
            // do overload resolution again just with our overload set to see which one gets
            // selected. That tells us which type to coerce to.
            // If the augmented call was not ambiguous, it means that none of these were
            // selected and we can't coerce the input.
            //
            // As a secondary step to reduce code duplication, we promote integral types to
            // their 64-bit variant. This does not change the printed value, but reduces the
            // number of instantiations even further. Promoting an integer is very cheap at
            // the call site.
            int64_t operator<<(std::ostream &, short value); // NOLINT
            int64_t operator<<(std::ostream &, unsigned short value); // NOLINT
            int64_t operator<<(std::ostream &, int value);

            int64_t operator<<(std::ostream &, unsigned int value);

            int64_t operator<<(std::ostream &, long value); // NOLINT
            uint64_t operator<<(std::ostream &, unsigned long value); // NOLINT
            int64_t operator<<(std::ostream &, long long value); // NOLINT
            uint64_t operator<<(std::ostream &, unsigned long long value); // NOLINT
            float operator<<(std::ostream &, float value);

            double operator<<(std::ostream &, double value);

            long double operator<<(std::ostream &, long double value);

            bool operator<<(std::ostream &, bool value);

            const void * turbo_nullable operator<<(std::ostream &,
                                                   const void * turbo_nullable value);

            const void * turbo_nullable operator<<(std::ostream &, std::nullptr_t);

            // These `char` overloads are specified like this in the standard, so we have to
            // write them exactly the same to ensure the call is ambiguous.
            // If we wrote it in a different way (eg taking std::ostream instead of the
            // template) then one call might have a higher rank than the other and it would
            // not be ambiguous.
            template<typename Traits>
            char operator<<(std::basic_ostream<char, Traits> &, char);

            template<typename Traits>
            signed char operator<<(std::basic_ostream<char, Traits> &, signed char);

            template<typename Traits>
            unsigned char operator<<(std::basic_ostream<char, Traits> &, unsigned char);

            template<typename Traits>
            const char * turbo_nonnull operator<<(std::basic_ostream<char, Traits> &,
                                                  const char *turbo_nonnull);

            template<typename Traits>
            const signed char * turbo_nonnull operator<<(std::basic_ostream<char, Traits> &,
                                                         const signed char *turbo_nonnull);

            template<typename Traits>
            const unsigned char * turbo_nonnull operator<<(std::basic_ostream<char, Traits> &,
                                                           const unsigned char *turbo_nonnull);

            // This overload triggers when the call is not ambiguous.
            // It means that T is being printed with some overload not on this list.
            // We keep the value as `const T&`.
            template<typename T, typename = decltype(std::declval<std::ostream &>()
                                                     << std::declval<const T &>())>
            const T &Detect(int);

            // This overload triggers when the call is ambiguous.
            // It means that T is either one from this list or printed as one from this
            // list. Eg an unscoped enum that decays to `int` for printing.
            // We ask the overload set to give us the type we want to convert it to.
            template<typename T>
            decltype(detect_specialization::operator<<(
                std::declval<std::ostream &>(), std::declval<const T &>())) Detect(char);

            // A sink for turbo_stringify which redirects everything to a std::ostream.
            class StringifySink {
            public:
                explicit StringifySink(std::ostream & os KUMO_ATTRIBUTE_LIFETIME_BOUND);

                void Append(std::string_view text);

                void Append(size_t length, char ch);

                friend void TurboFormatFlush(StringifySink * turbo_nonnull sink,
                                             std::string_view text);

            private:
                std::ostream &os_;
            };

            // Wraps a type implementing turbo_stringify, and implements operator<<.
            template<typename T>
            class StringifyToStreamWrapper {
            public:
                explicit StringifyToStreamWrapper(const T & v KUMO_ATTRIBUTE_LIFETIME_BOUND)
                    : v_(v) {
                }

                friend std::ostream &operator<<(std::ostream &os,
                                                const StringifyToStreamWrapper &wrapper) {
                    StringifySink sink(os);
                    turbo_stringify(sink, wrapper.v_);
                    return os;
                }

            private:
                const T &v_;
            };

            // This overload triggers when T implements turbo_stringify.
            // StringifyToStreamWrapper is used to allow make_check_op_string to use
            // operator<<.
            template<typename T>
            std::enable_if_t<HasTurboStringify<T>::value,
                StringifyToStreamWrapper<T> >
            Detect(...); // Ellipsis has lowest preference when int passed.

            // This overload triggers when T is neither possible to print nor an enum.
            template<typename T>
            std::enable_if_t<std::negation_v<std::disjunction<
                    std::is_convertible<T, int>, std::is_enum<T>,
                    std::is_pointer<T>, std::is_same<T, std::nullptr_t>,
                    HasOstreamOperator<T>, HasTurboStringify<T> > >,
                UnprintableWrapper>
            Detect(...);

            // Equivalent to the updated std::underlying_type from C++20, which is no
            // longer undefined behavior for non-enum types.
            template<typename T, typename EnableT = void>
            struct UnderlyingType {
            };

            template<typename T>
            struct UnderlyingType<T, std::enable_if_t<std::is_enum_v<T> > > {
                using type = std::underlying_type_t<T>;
            };

            template<typename T>
            using UnderlyingTypeT = typename UnderlyingType<T>::type;

            // This overload triggers when T is a scoped enum that has not defined an output
            // stream operator (operator<<) or turbo_stringify. It causes the enum value to be
            // converted to a type that can be streamed. For consistency with other enums, a
            // scoped enum backed by a bool or char is converted to its underlying type, and
            // one backed by another integer is converted to (u)int64_t.
            template<typename T>
            std::enable_if_t<
                std::conjunction_v<std::is_enum<T>,
                    std::negation<std::is_convertible<T, int> >,
                    std::negation<HasOstreamOperator<T> >,
                    std::negation<HasTurboStringify<T> > >,
                std::conditional_t < std::is_same_v<UnderlyingTypeT<T>, bool> ||
                std::is_same_v<UnderlyingTypeT<T>, char> ||
                std::is_same_v<UnderlyingTypeT<T>, signed char> ||
                std::is_same_v<UnderlyingTypeT<T>, unsigned char>,
                UnderlyingTypeT<T>,
                std::conditional_t<std::is_signed_v<UnderlyingTypeT<T> >,
                    int64_t, uint64_t> >
            >
            Detect(...);

            template<typename T>
            using Detected = decltype(Detect<T>(0));
        } // namespace detect_specialization

        // If the comparison will happen as pointers, decay `char*` arguments to `void*`
        // when printing them. There is no evidence that they are a NULL terminated
        // C-String so printing them as such could lead to UB, and more importantly we
        // compared pointers so showing the pointers is a better result.
        template<typename T>
        constexpr bool IsCharStarOrVoidStar() {
            if constexpr (std::is_reference_v<T>) {
                return IsCharStarOrVoidStar<std::remove_reference_t<T> >();
            } else if constexpr (std::is_array_v<T>) {
                return IsCharStarOrVoidStar<std::decay_t<T> >();
            } else {
                using U = std::remove_const_t<std::remove_pointer_t<T> >;
                return std::is_pointer_v<T> &&
                       (std::is_same_v<char, U> || std::is_same_v<unsigned char, U> ||
                        std::is_same_v<signed char, U> || std::is_void_v<U>);
            }
        }

        template<typename T1, typename T2,
            typename U1 = detect_specialization::Detected<T1>,
            typename U2 = detect_specialization::Detected<T2> >
        using CheckOpStreamType =
        std::conditional_t<IsCharStarOrVoidStar<U1>() && IsCharStarOrVoidStar<U2>(),
            const void *, U1>;

        // Build the error message string.  Specify no inlining for code size.
        template<typename T1, typename T2>
KUMO_ATTRIBUTE_RETURNS_NONNULL const char * turbo_nonnull make_check_op_string(
            T1 v1, T2 v2, const char * turbo_nonnull exprtext) KUMO_ATTRIBUTE_NOINLINE;

        template<typename T1, typename T2>
        const char * turbo_nonnull make_check_op_string(T1 v1, T2 v2,
                                                     const char * turbo_nonnull exprtext) {
            if constexpr (std::is_same_v<CheckOpStreamType<T1, T2>, UnprintableWrapper> &&
                std::is_same_v<CheckOpStreamType<T2, T1>, UnprintableWrapper>) {
                // No sense printing " (UNPRINTABLE vs. UNPRINTABLE)"
                return exprtext;
            } else {
                CheckOpMessageBuilder comb(exprtext);
                make_check_op_value_string(comb.ForVar1(), v1);
                make_check_op_value_string(comb.ForVar2(), v2);
                return comb.NewString();
            }
        }

        // Add a few commonly used instantiations as extern to reduce size of objects
        // files.
#define TURBO_LOG_INTERNAL_DEFINE_MAKE_CHECK_OP_STRING_EXTERN(x) \
  extern template const char* turbo_nonnull make_check_op_string(   \
      x, x, const char* turbo_nonnull)
        TURBO_LOG_INTERNAL_DEFINE_MAKE_CHECK_OP_STRING_EXTERN(bool);

        TURBO_LOG_INTERNAL_DEFINE_MAKE_CHECK_OP_STRING_EXTERN(int64_t);

        TURBO_LOG_INTERNAL_DEFINE_MAKE_CHECK_OP_STRING_EXTERN(uint64_t);

        TURBO_LOG_INTERNAL_DEFINE_MAKE_CHECK_OP_STRING_EXTERN(float);

        TURBO_LOG_INTERNAL_DEFINE_MAKE_CHECK_OP_STRING_EXTERN(double);

        TURBO_LOG_INTERNAL_DEFINE_MAKE_CHECK_OP_STRING_EXTERN(char);

        TURBO_LOG_INTERNAL_DEFINE_MAKE_CHECK_OP_STRING_EXTERN(unsigned char);

        TURBO_LOG_INTERNAL_DEFINE_MAKE_CHECK_OP_STRING_EXTERN(const std::string&);

        TURBO_LOG_INTERNAL_DEFINE_MAKE_CHECK_OP_STRING_EXTERN(const std::string_view&);

        TURBO_LOG_INTERNAL_DEFINE_MAKE_CHECK_OP_STRING_EXTERN(const char* turbo_nonnull);

        TURBO_LOG_INTERNAL_DEFINE_MAKE_CHECK_OP_STRING_EXTERN(
            const signed char* turbo_nonnull);

        TURBO_LOG_INTERNAL_DEFINE_MAKE_CHECK_OP_STRING_EXTERN(
            const unsigned char* turbo_nonnull);

        TURBO_LOG_INTERNAL_DEFINE_MAKE_CHECK_OP_STRING_EXTERN(const void* turbo_nonnull);
#undef TURBO_LOG_INTERNAL_DEFINE_MAKE_CHECK_OP_STRING_EXTERN

        // `TURBO_LOG_INTERNAL_CHECK_OP_IMPL_RESULT` skips formatting the Check_OP result
        // string iff `TURBO_MIN_LOG_LEVEL` exceeds `kFatal`, instead returning an empty
        // string.
#ifdef TURBO_MIN_LOG_LEVEL
#define TURBO_LOG_INTERNAL_CHECK_OP_IMPL_RESULT(U1, U2, v1, v2, exprtext) \
  ((::turbo::LogSeverity::kFatal >=                                       \
    static_cast<::turbo::LogSeverityAtLeast>(TURBO_MIN_LOG_LEVEL))         \
       ? make_check_op_string<U1, U2>(v1, v2, exprtext)                     \
       : "")
#else
#define TURBO_LOG_INTERNAL_CHECK_OP_IMPL_RESULT(U1, U2, v1, v2, exprtext) \
  make_check_op_string<U1, U2>(v1, v2, exprtext)
#endif

        // Helper functions for `TURBO_LOG_INTERNAL_CHECK_OP` macro family.  The
        // `(int, int)` override works around the issue that the compiler will not
        // instantiate the template version of the function on values of unnamed enum
        // type.
#define TURBO_LOG_INTERNAL_CHECK_OP_IMPL(name, op)                          \
  template <typename T1, typename T2>                                      \
  inline constexpr const char* turbo_nullable name##Impl(                   \
      const T1& v1, const T2& v2, const char* turbo_nonnull exprtext) {     \
    using U1 = CheckOpStreamType<T1, T2>;                                  \
    using U2 = CheckOpStreamType<T2, T1>;                                  \
    return KUMO_LIKELY(v1 op v2)                                     \
               ? nullptr                                                   \
               : TURBO_LOG_INTERNAL_CHECK_OP_IMPL_RESULT(U1, U2, U1(v1),    \
                                                        U2(v2), exprtext); \
  }                                                                        \
  inline constexpr const char* turbo_nullable name##Impl(                   \
      int v1, int v2, const char* turbo_nonnull exprtext) {                 \
    return name##Impl<int, int>(v1, v2, exprtext);                         \
  }

        TURBO_LOG_INTERNAL_CHECK_OP_IMPL(Check_EQ, ==)
        TURBO_LOG_INTERNAL_CHECK_OP_IMPL(Check_NE, !=)
        TURBO_LOG_INTERNAL_CHECK_OP_IMPL(Check_LE, <=)
        TURBO_LOG_INTERNAL_CHECK_OP_IMPL(Check_LT, <)
        TURBO_LOG_INTERNAL_CHECK_OP_IMPL(Check_GE, >=)
        TURBO_LOG_INTERNAL_CHECK_OP_IMPL(Check_GT, >)
#undef TURBO_LOG_INTERNAL_CHECK_OP_IMPL_RESULT
#undef TURBO_LOG_INTERNAL_CHECK_OP_IMPL

        const char * turbo_nullable CheckstrcmptrueImpl(
            const char * turbo_nullable s1, const char * turbo_nullable s2,
            const char * turbo_nonnull exprtext);

        const char * turbo_nullable CheckstrcmpfalseImpl(
            const char * turbo_nullable s1, const char * turbo_nullable s2,
            const char * turbo_nonnull exprtext);

        const char * turbo_nullable CheckstrcasecmptrueImpl(
            const char * turbo_nullable s1, const char * turbo_nullable s2,
            const char * turbo_nonnull exprtext);

        const char * turbo_nullable CheckstrcasecmpfalseImpl(
            const char * turbo_nullable s1, const char * turbo_nullable s2,
            const char * turbo_nonnull exprtext);

        // `KCHECK_EQ` and friends want to pass their arguments by reference, however
        // this winds up exposing lots of cases where people have defined and
        // initialized static const data members but never declared them (i.e. in a .cc
        // file), meaning they are not referenceable.  This function avoids that problem
        // for integers (the most common cases) by overloading for every primitive
        // integer type, even the ones we discourage, and returning them by value.
        // NOLINTBEGIN(runtime/int)
        // NOLINTBEGIN(google-runtime-int)
        template<typename T>
        constexpr const T &get_referenceable_value(const T &t) {
            return t;
        }

        constexpr char get_referenceable_value(char t) { return t; }
        constexpr unsigned char get_referenceable_value(unsigned char t) { return t; }
        constexpr signed char get_referenceable_value(signed char t) { return t; }
        constexpr short get_referenceable_value(short t) { return t; }
        constexpr unsigned short get_referenceable_value(unsigned short t) { return t; }
        constexpr int get_referenceable_value(int t) { return t; }
        constexpr unsigned int get_referenceable_value(unsigned int t) { return t; }
        constexpr long get_referenceable_value(long t) { return t; }
        constexpr unsigned long get_referenceable_value(unsigned long t) { return t; }
        constexpr long long get_referenceable_value(long long t) { return t; }

        constexpr unsigned long long get_referenceable_value(unsigned long long t) {
            return t;
        }

        // NOLINTEND(google-runtime-int)
        // NOLINTEND(runtime/int)
    } // namespace log_internal
} // namespace turbo

#endif  // TURBO_LOG_INTERNAL_CHECK_OP_H_
