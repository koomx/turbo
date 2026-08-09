// Copyright 2026 The Abseil Authors.
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
// File: status_macros.h
// -----------------------------------------------------------------------------
//
// Helper macros and methods to return and propagate errors with `turbo::Status`.

#ifndef TURBO_STATUS_STATUS_MACROS_H_
#define TURBO_STATUS_STATUS_MACROS_H_

#include <cstddef>
#include <type_traits>
#include <utility>

#include <source_location>
#include <turbo/macros/config.h>
#include <turbo/status/status.h>
#include <turbo/status/status_builder.h> // IWYU pragma: export
#include <turbo/status/statusor.h>

// Evaluates an expression that produces a `turbo::Status`. If the status is not
// ok, returns it from the current function.
//
// For example:
//   turbo::Status MultiStepFunction() {
//     TURBO_RETURN_IF_ERROR(Function(args...));
//     TURBO_RETURN_IF_ERROR(foo.Method(args...));
//     return turbo::OkStatus();
//   }
//
// The macro ends with a `turbo::StatusBuilder` which allows the returned status
// to be extended with more details.  Any chained expressions after the macro
// will not be evaluated unless there is an error.
//
// For example:
//   turbo::Status MultiStepFunction() {
//     TURBO_RETURN_IF_ERROR(Function(args...)) << "in MultiStepFunction";
//     TURBO_RETURN_IF_ERROR(foo.Method(args...)).Log(turbo::LogSeverity::kError)
//         << "while processing query: " << query.DebugString();
//     return turbo::OkStatus();
//   }
//
// `turbo::StatusBuilder` supports adapting the builder chain using a `With`
// method and a functor.  This allows for powerful extensions to the macro.
//
// For example, teams can define local policies to use across their code:
//
//   StatusBuilder TeamPolicy(StatusBuilder builder) {
//     return std::move(builder.Log(turbo::LogSeverity::kWarning).Attach(...));
//   }
//
//   TURBO_RETURN_IF_ERROR(foo()).With(TeamPolicy);
//   TURBO_RETURN_IF_ERROR(bar()).With(TeamPolicy);
//
// Changing the return type allows the macro to be used with Task and Rpc
// interfaces.  See `util::TaskReturn` and `rpc::RpcSetStatus` for details.
//
//   void Read(StringPiece name, util::Task* task) {
//     int64 id;
//     TURBO_RETURN_IF_ERROR(GetIdForName(name, &id)).With(TaskReturn(task));
//     TURBO_RETURN_IF_ERROR(ReadForId(id)).With(TaskReturn(task));
//     task->Return();
//   }
//
// If using this macro inside a lambda, you need to annotate the return type
// to avoid confusion between a `turbo::StatusBuilder` and a `turbo::Status` type.
// E.g.
//
//   []() -> turbo::Status {
//     TURBO_RETURN_IF_ERROR(Function(args...));
//     TURBO_RETURN_IF_ERROR(foo.Method(args...));
//     return turbo::OkStatus();
//   }
#define TURBO_RETURN_IF_ERROR(expr) \
    TURBO_INTERNAL_STATUS_MACROS_RETURN_IF_ERROR_IMPL_(return, expr)

// Executes an expression `rexpr` that returns an `turbo::StatusOr<T>`. On OK,
// moves its value into the variable defined by `lhs`, otherwise returns
// from the current function. By default the error status is returned
// unchanged, but it may be modified by an `error_expression`. If there is an
// error, `lhs` is not evaluated; thus any side effects that `lhs` may have
// only occur in the success case.
//
// Interface:
//
//   TURBO_ASSIGN_OR_RETURN(lhs, rexpr)
//   TURBO_ASSIGN_OR_RETURN(lhs, rexpr, error_expression);
//
// WARNING: if lhs is parenthesized, the parentheses are removed. See examples
// for more details.
//
// WARNING: expands into multiple statements; it cannot be used in a single
// statement (e.g. as the body of an if statement without {})!
//
// Example: Declaring and initializing a new variable (ValueType can be anything
//          that can be initialized with assignment--including a const
//          reference, although that's discouraged by go/totw/107):
//   TURBO_ASSIGN_OR_RETURN(ValueType value, MaybeGetValue(arg));
//
// Example: Assigning to an existing variable:
//   ValueType value;
//   TURBO_ASSIGN_OR_RETURN(value, MaybeGetValue(arg));
//
// Example: Assigning to an expression with side effects:
//   MyProto data;
//   TURBO_ASSIGN_OR_RETURN(*data.mutable_str(), MaybeGetValue(arg));
//   // No field "str" is added on error.
//
// Example: Initializing a `std::unique_ptr`.
//   TURBO_ASSIGN_OR_RETURN(std::unique_ptr<T> ptr, MaybeGetPtr(arg));
//
// Example: Initializing a map. Because of C++ preprocessor limitations,
// the type used in TURBO_ASSIGN_OR_RETURN cannot contain commas, so wrap the
// lhs in parentheses:
//   TURBO_ASSIGN_OR_RETURN((std::unordered_map<Foo, Bar> my_map), GetMap());
// Or use `auto` if the type is obvious enough:
//   TURBO_ASSIGN_OR_RETURN(auto my_map, GetMap());
//
// Example: Assigning to structured bindings (go/totw/169). The same situation
// with comma as in map, so wrap the statement in parentheses.
//   TURBO_ASSIGN_OR_RETURN((auto [first, second]), GetPair());
//
// If passed, the `error_expression` is evaluated to produce the return
// value. The expression may reference any variable visible in scope, as
// well as a `turbo::StatusBuilder` object populated with the error and
// named by a single underscore `_`. The expression typically uses the
// builder to modify the status and is returned directly in manner similar
// to TURBO_RETURN_IF_ERROR. The expression may, however, evaluate to any type
// returnable by the function, including (void). For example:
//
// Example: Adjusting the error message.
//   TURBO_ASSIGN_OR_RETURN(ValueType value, MaybeGetValue(query),
//                    _ << "while processing query " << query.DebugString());
//
// Example: Logging the error on failure.
//   TURBO_ASSIGN_OR_RETURN(ValueType value, MaybeGetValue(query), _.LogError());
//
#define TURBO_ASSIGN_OR_RETURN(...) \
    TURBO_INTERNAL_STATUS_MACROS_ASSIGN_OR_RETURN_IMPL_(return, __VA_ARGS__)

// =================================================================
// == Implementation details, do not rely on anything below here. ==
// =================================================================

#define TURBO_INTERNAL_STATUS_MACROS_RETURN_IF_ERROR_IMPL_(return_keyword,               \
    expr)                                                                                \
    TURBO_INTERNAL_STATUS_MACROS_IMPL_ELSE_BLOCKER_                                      \
    if (auto status_macro_internal_adaptor = turbo::status_macro_internal::MacroAdaptor( \
            (expr), std::source_location::current())) {                                  \
    } else /* NOLINT */                                                                  \
        return_keyword status_macro_internal_adaptor.Consume()

#define TURBO_INTERNAL_STATUS_MACROS_ASSIGN_OR_RETURN_IMPL_(return_keyword, \
    ...)                                                                    \
    TURBO_INTERNAL_STATUS_MACROS_IMPL_GET_VARIADIC_(                        \
        (return_keyword, __VA_ARGS__,                                       \
            TURBO_INTERNAL_STATUS_MACROS_IMPL_ASSIGN_OR_RETURN_3_,          \
            TURBO_INTERNAL_STATUS_MACROS_IMPL_ASSIGN_OR_RETURN_2_))         \
    (return_keyword, __VA_ARGS__)

constexpr bool HasPotentialConditionalOperator(const char* lhs, int size) {
    for (int i = 0; i < size; ++i) {
        if (lhs[i] == '?') {
            return true;
        }
    }
    return false;
}

template <std::size_t N>
constexpr bool IsEnclosedByParentheses(const char (&lhs)[N]) {
    if (N < 2) {
        return false;
    }
    return lhs[0] == '(' && lhs[N - 2] == ')';
}

namespace turbo {
    namespace status_macro_internal {
        template <typename T, typename EnableIf = void>
        struct IsAllowedStatusOrMacroType : std::false_type {
        };

        template <typename T>
        struct IsAllowedStatusOrMacroType<
            T, std::enable_if_t<std::is_convertible_v<T*, typename turbo::StatusOr<typename T::value_type>*>>>
            : std::true_type {
        };
    } // namespace status_macro_internal
} // namespace turbo

// MSVC incorrectly expands variadic macros, splice together a macro call to
// work around the bug.
#define TURBO_INTERNAL_STATUS_MACROS_IMPL_GET_VARIADIC_HELPER_(_1, _2, _3, _4, \
    NAME, ...)                                                                 \
    NAME
#define TURBO_INTERNAL_STATUS_MACROS_IMPL_GET_VARIADIC_(args) \
    /* NOLINTNEXTLINE(clang-diagnostic-pre-c++20-compat) */   \
    TURBO_INTERNAL_STATUS_MACROS_IMPL_GET_VARIADIC_HELPER_ args

#define TURBO_INTERNAL_STATUS_MACROS_IMPL_ASSIGN_OR_RETURN_2_(return_keyword,     \
    lhs, rexpr)                                                                   \
    TURBO_INTERNAL_STATUS_MACROS_IMPL_ASSIGN_OR_RETURN_(                          \
        TURBO_INTERNAL_STATUS_MACROS_IMPL_CONCAT_(_status_or_value, __LINE__),    \
        lhs, rexpr,                                                               \
        return_keyword turbo::Status(                                             \
            std::move(TURBO_INTERNAL_STATUS_MACROS_IMPL_CONCAT_(_status_or_value, \
                          __LINE__))                                              \
                .status(),                                                        \
            std::source_location::current()))

#define TURBO_INTERNAL_STATUS_MACROS_IMPL_ASSIGN_OR_RETURN_3_(                    \
    return_keyword, lhs, rexpr, error_expression)                                 \
    TURBO_INTERNAL_STATUS_MACROS_IMPL_ASSIGN_OR_RETURN_(                          \
        TURBO_INTERNAL_STATUS_MACROS_IMPL_CONCAT_(_status_or_value, __LINE__),    \
        lhs, rexpr, /* NOLINTNEXTLINE(misc-const-correctness) */                  \
        turbo::StatusBuilder _(                                                   \
            std::move(TURBO_INTERNAL_STATUS_MACROS_IMPL_CONCAT_(_status_or_value, \
                          __LINE__))                                              \
                .status(),                                                        \
            std::source_location::current());                                     \
        (void)_; /* error_expression is allowed to not use this variable */       \
        return_keyword(error_expression))

#define TURBO_INTERNAL_STATUS_MACROS_IMPL_ASSIGN_OR_RETURN_(                                            \
    statusor, lhs, rexpr, error_expression)                                                             \
    auto statusor = (rexpr);                                                                            \
    if (KUMO_UNLIKELY(!statusor.ok())) {                                                                \
        error_expression;                                                                               \
    }                                                                                                   \
    {                                                                                                   \
        static_assert(                                                                                  \
            !IsEnclosedByParentheses(#lhs) || !HasPotentialConditionalOperator(#lhs, sizeof(#lhs) - 2), \
            "Identified potential conditional operator, consider not "                                  \
            "using TURBO_ASSIGN_OR_RETURN");                                                            \
    }                                                                                                   \
    {                                                                                                   \
        static_assert(                                                                                  \
            turbo::status_macro_internal::IsAllowedStatusOrMacroType<                                   \
                std::remove_const_t<decltype(statusor)>>(),                                             \
            "TURBO_ASSIGN_OR_RETURN should only be used with turbo::StatusOr<>");                       \
    }                                                                                                   \
    TURBO_INTERNAL_STATUS_MACROS_IMPL_UNPARENTHESIZE_IF_PARENTHESIZED(lhs) = (*std::move(statusor))

// Internal helpers for macro expansion.
#define TURBO_INTERNAL_STATUS_MACROS_IMPL_EAT(...)
#define TURBO_INTERNAL_STATUS_MACROS_IMPL_REM(...) __VA_ARGS__
#define TURBO_INTERNAL_STATUS_MACROS_IMPL_COMMA(...) ,
#define TURBO_INTERNAL_STATUS_MACROS_IMPL_ARG_3(a, b, c, ...) c

#define TURBO_INTERNAL_STATUS_MACROS_IMPL_HAS_COMMA(...) \
    TURBO_INTERNAL_STATUS_MACROS_IMPL_REM(               \
        TURBO_INTERNAL_STATUS_MACROS_IMPL_ARG_3(__VA_ARGS__, 1, 10))

// Concatenates five macro arguments.
#define TURBO_INTERNAL_STATUS_MACROS_IMPL_CONCAT_5(a, b, c, d, e) a##b##c##d##e

// Identifies (below) the case where all arguments are empty, except the last.
#define TURBO_INTERNAL_STATUS_MACROS_IMPL_IS_EMPTY_CASE_1010101 ,

// Evaluates to 1 if the arguments concatenate to 1010101, and 10 otherwise.
#define TURBO_INTERNAL_STATUS_MACROS_1_IF_1010101_ELSE_10(a, b, c, d) \
    TURBO_INTERNAL_STATUS_MACROS_IMPL_HAS_COMMA(                      \
        TURBO_INTERNAL_STATUS_MACROS_IMPL_CONCAT_5(                   \
            TURBO_INTERNAL_STATUS_MACROS_IMPL_IS_EMPTY_CASE_, a, b, c, d))

// Evaluates to 1 if __VA_OPT__ is supported and the argument list is non-empty,
// and 0 otherwise.
#define TURBO_INTERNAL_STATUS_MACROS_IMPL_HAVE_VA_OPT(...) \
    TURBO_INTERNAL_STATUS_MACROS_IMPL_ARG_3(__VA_OPT__(, ), 1, 0, )

#if TURBO_INTERNAL_STATUS_MACROS_IMPL_HAVE_VA_OPT(.)
// Evaluates to 1 if the argument list is empty, and 10 otherwise.
#define TURBO_INTERNAL_STATUS_MACROS_1_IF_EMPTY_ELSE_10(...) 1##__VA_OPT__(0)
#else
#define TURBO_INTERNAL_STATUS_MACROS_1_IF_EMPTY_ELSE_10(...)        \
    TURBO_INTERNAL_STATUS_MACROS_1_IF_1010101_ELSE_10(              \
        TURBO_INTERNAL_STATUS_MACROS_IMPL_HAS_COMMA(__VA_ARGS__),   \
        TURBO_INTERNAL_STATUS_MACROS_IMPL_HAS_COMMA(                \
            TURBO_INTERNAL_STATUS_MACROS_IMPL_COMMA __VA_ARGS__),   \
        TURBO_INTERNAL_STATUS_MACROS_IMPL_HAS_COMMA(__VA_ARGS__()), \
        TURBO_INTERNAL_STATUS_MACROS_IMPL_HAS_COMMA(                \
            TURBO_INTERNAL_STATUS_MACROS_IMPL_COMMA __VA_ARGS__()))

#endif

// Parenthesized case: strips the initial pair of parentheses from the input.
//
// Note that this may not result in an equivalent expression. For example,
// (*a)[b] would evaluate to a[b]. The caller below is required to ensure
// this is only called when the input is fully parenthesized to avoid this
// issue.
#define TURBO_INTERNAL_STATUS_MACROS_IMPL_UNPARENTHESIZE_IF_PARENTHESIZED_1(x) \
    TURBO_INTERNAL_STATUS_MACROS_IMPL_REM x

// Unparenthesized case: evaluates to the argument list as-is.
#define TURBO_INTERNAL_STATUS_MACROS_IMPL_UNPARENTHESIZE_IF_PARENTHESIZED_10(x) \
    TURBO_INTERNAL_STATUS_MACROS_IMPL_REM(x)

// If the input is parenthesized, removes the parentheses. Otherwise expands to
// the input unchanged.
#define TURBO_INTERNAL_STATUS_MACROS_IMPL_UNPARENTHESIZE_IF_PARENTHESIZED(...)  \
    TURBO_INTERNAL_STATUS_MACROS_IMPL_REM(                                      \
        TURBO_INTERNAL_STATUS_MACROS_IMPL_CONCAT_(                              \
            TURBO_INTERNAL_STATUS_MACROS_IMPL_UNPARENTHESIZE_IF_PARENTHESIZED_, \
            TURBO_INTERNAL_STATUS_MACROS_1_IF_EMPTY_ELSE_10(                    \
                TURBO_INTERNAL_STATUS_MACROS_IMPL_EAT __VA_ARGS__))(            \
            TURBO_INTERNAL_STATUS_MACROS_IMPL_REM(__VA_ARGS__)))

// Internal helper for concatenating macro values.
#define TURBO_INTERNAL_STATUS_MACROS_IMPL_CONCAT_INNER_(x, y) x##y

#define TURBO_INTERNAL_STATUS_MACROS_IMPL_CONCAT_(x, y) \
    TURBO_INTERNAL_STATUS_MACROS_IMPL_CONCAT_INNER_(x, y)

// The GNU compiler emits a warning for code like:
//
//   if (foo)
//     if (bar) { } else baz;
//
// because it thinks you might want the else to bind to the first if.  This
// leads to problems with code like:
//
//   if (do_expr) TURBO_RETURN_IF_ERROR(expr) << "Some message";
//
// The "switch (0) case 0:" idiom is used to suppress this.
#define TURBO_INTERNAL_STATUS_MACROS_IMPL_ELSE_BLOCKER_ \
    switch (0)                                          \
    case 0:                                             \
    default: // NOLINT

namespace turbo {
    namespace status_macro_internal {
        // Provides a conversion to bool so that it can be used inside an if statement
        // that declares a variable.
        class StatusAdaptorForMacros {
        public:
            StatusAdaptorForMacros(
                const turbo::Status& status,
                std::source_location loc = std::source_location::current())
                : builder_(status, loc) {
            }

            StatusAdaptorForMacros(
                turbo::Status&& status,
                std::source_location loc = std::source_location::current())
                : builder_(std::move(status), loc) {
            }

            StatusAdaptorForMacros(const StatusBuilder& builder,
                std::source_location = std::source_location())
                : builder_(builder) {
            }

            StatusAdaptorForMacros(StatusBuilder&& builder,
                std::source_location = std::source_location())
                : builder_(std::move(builder)) {
            }

            StatusAdaptorForMacros(const StatusAdaptorForMacros&) = delete;

            StatusAdaptorForMacros& operator=(const StatusAdaptorForMacros&) = delete;

            explicit operator bool() const { return KUMO_LIKELY(builder_.ok()); }

            StatusBuilder&& Consume() { return std::move(builder_); }

        private:
            StatusBuilder builder_;
        };

        // Special adaptor for use by TURBO_RETURN_IF_ERROR for turbo::Status arguments.
        // This one avoids constructing a StatusBuilder on the fast path.
        //
        // REQUIRES: Only used by TURBO_RETURN_IF_ERROR implementation.
        class ReturnIfErrorAdaptor {
        public:
            explicit ReturnIfErrorAdaptor(
                const turbo::Status& status,
                std::source_location loc = std::source_location::current())
                : status_(status)
                , loc_(loc) {
            }

            explicit ReturnIfErrorAdaptor(
                turbo::Status&& status,
                std::source_location loc = std::source_location::current())
                : status_(std::move(status))
                , loc_(loc) {
            }

            ~ReturnIfErrorAdaptor() {
                // WARNING! WARNING! WARNING!
                //
                // We play fast and loose here and avoid destroying status_. This should be
                // safe because status_ will never own memory at destruction time. The two
                // cases to consider are:
                //  (1) OK: OkStatus() representation needs no cleanup
                //  (2) Not-OK: we take the else branch in TURBO_RETURN_IF_ERROR and move
                //      status_ into StatusBuilder which leaves status_ with a MovedFromRep
                //      that needs no cleanup.
                // If the turbo::Status implementation changes, leaks should be caught by
                // the various tests we have when run under lsan or debug leak checker.
            }

            explicit operator bool() const { return KUMO_LIKELY(status_.ok()); }

            StatusBuilder Consume() { return StatusBuilder(std::move(status_), loc_); }

        private:
            // Place the status inside a union so we can avoid generating unnecessary code
            // to call the Status destructor.
            union {
                turbo::Status status_;
                char nothing_[1];
            };

            std::source_location loc_;
        };

        // Overloads of MacroAdaptor that pick the right adaptor class
        // for each argument type.
        //
        // TODO(b/501387693): Replace these with deduction guides of template
        // specializations once the bug is fixed. (See prior file revision.)
        inline ReturnIfErrorAdaptor MacroAdaptor(const turbo::Status& s,
            std::source_location loc) {
            return ReturnIfErrorAdaptor(s, loc);
        }

        inline ReturnIfErrorAdaptor MacroAdaptor(turbo::Status&& s,
            std::source_location loc) {
            return ReturnIfErrorAdaptor(std::move(s), loc);
        }

        inline StatusAdaptorForMacros MacroAdaptor(const StatusBuilder& s,
            std::source_location loc) {
            return StatusAdaptorForMacros(s, loc);
        }

        inline StatusAdaptorForMacros MacroAdaptor(StatusBuilder&& s,
            std::source_location loc) {
            return StatusAdaptorForMacros(std::move(s), loc);
        }
    } // namespace status_macro_internal

    // By defining TURBO_DEFINE_UNQUALIFIED_STATUS_MACROS, this library also provides
    // unqualified versions of its macros.
    //
    // Unqualified macro names are likely to collide with those in other projects,
    // and so are not recommended.  Furthermore, this is true for any transitive
    // dependencies of Abseil: it is impossible to be confident that downstream
    // libraries will neither define these macros themselves nor depend on a
    // different library that also defines them.
    //
    // To enable unqualified names despite the caveats, define
    // `TURBO_DEFINE_UNQUALIFIED_STATUS_MACROS` preferably at the command line, e.g.
    // `-DTURBO_DEFINE_UNQUALIFIED_STATUS_MACROS`, or `local_defines =
    // ["TURBO_DEFINE_UNQUALIFIED_STATUS_MACROS"]` if using Bazel.
    //
    // These are turned on by default inside Google's internal codebase where their
    // use is historically ubiquitous.  Other OSS Google projects should use the
    // qualified versions.
    //
#ifdef TURBO_DEFINE_UNQUALIFIED_STATUS_MACROS
#define ASSIGN_OR_RETURN(...) TURBO_ASSIGN_OR_RETURN(__VA_ARGS__)
#define RETURN_IF_ERROR(...) TURBO_RETURN_IF_ERROR(__VA_ARGS__)
#endif // TURBO_DEFINE_UNQUALIFIED_STATUS_MACROS
} // namespace turbo

#endif // TURBO_STATUS_STATUS_MACROS_H_
