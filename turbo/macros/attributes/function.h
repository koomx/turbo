// Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// -----------------------------------------------------------------------------
// File: function.h
// -----------------------------------------------------------------------------
//
// Function attributes: format checking, inline control, weak, nonnull, etc.

#pragma once

#include <turbo/macros/compiler/compiler.h>


// -----------------------------------------------------------------------------
// Function Attributes
// -----------------------------------------------------------------------------
//
// GCC: https://gcc.gnu.org/onlinedocs/gcc/Function-Attributes.html
// Clang: https://clang.llvm.org/docs/AttributeReference.html

// KUMO_PRINTF_ATTRIBUTE
// KUMO_SCANF_ATTRIBUTE
//
// Tells the compiler to perform `printf` format string checking if the
// compiler supports it; see the 'format' attribute in
// <https://gcc.gnu.org/onlinedocs/gcc-4.7.0/gcc/Function-Attributes.html>.
//
// Note: As the GCC manual states, "[s]ince non-static C++ methods
// have an implicit 'this' argument, the arguments of such methods
// should be counted from two, not one."
#if KUMO_HAVE_ATTRIBUTE(format) || (defined(__GNUC__))
#define KUMO_PRINTF_ATTRIBUTE(string_index, first_to_check) \
  __attribute__((__format__(__printf__, string_index, first_to_check)))
#define KUMO_SCANF_ATTRIBUTE(string_index, first_to_check) \
  __attribute__((__format__(__scanf__, string_index, first_to_check)))
#else
#define KUMO_PRINTF_ATTRIBUTE(string_index, first_to_check)
#define KUMO_SCANF_ATTRIBUTE(string_index, first_to_check)
#endif


// KUMO_ATTRIBUTE_ALWAYS_INLINE
// KUMO_ATTRIBUTE_NOINLINE
//
// Forces functions to either inline or not inline. Introduced in gcc 3.1.
// MSVC uses __forceinline / __declspec(noinline). clang-cl keeps GNU attributes.
#if defined(_MSC_VER) && !defined(__clang__)
#define KUMO_ATTRIBUTE_ALWAYS_INLINE __forceinline
#define KUMO_FORCE_INLINE inline __forceinline
#define KUMO_HAVE_ATTRIBUTE_ALWAYS_INLINE 1
#elif KUMO_HAVE_ATTRIBUTE(always_inline) || defined(__GNUC__)
#define KUMO_ATTRIBUTE_ALWAYS_INLINE __attribute__((always_inline))
#define KUMO_FORCE_INLINE inline __attribute__((always_inline))

#define KUMO_HAVE_ATTRIBUTE_ALWAYS_INLINE 1
#else
#define KUMO_ATTRIBUTE_ALWAYS_INLINE
#define KUMO_FORCE_INLINE inline
#define KUMO_HAVE_ATTRIBUTE_ALWAYS_INLINE 0
#endif

#if defined(_MSC_VER) && !defined(__clang__)
#define KUMO_ATTRIBUTE_NOINLINE __declspec(noinline)
#define KUMO_HAVE_ATTRIBUTE_NOINLINE 1
#elif KUMO_HAVE_ATTRIBUTE(noinline) || defined(__GNUC__)
#define KUMO_ATTRIBUTE_NOINLINE __attribute__((noinline))
#define KUMO_HAVE_ATTRIBUTE_NOINLINE 1
#else
#define KUMO_ATTRIBUTE_NOINLINE
#define KUMO_HAVE_ATTRIBUTE_NOINLINE 0
#endif


// KUMO_ATTRIBUTE_WEAK
//
// Tags a function as weak for the purposes of compilation and linking.
// Weak attributes did not work properly in LLVM's Windows backend before
// 9.0.0, so disable them there. See https://bugs.llvm.org/show_bug.cgi?id=37598
// for further information. Weak attributes do not work across DLL boundary.
// The MinGW compiler doesn't complain about the weak attribute until the link
// step, presumably because Windows doesn't use ELF binaries.
#if (KUMO_HAVE_ATTRIBUTE(weak) ||                                 \
     (defined(__GNUC__))) &&               \
    (!defined(_WIN32) ||                                          \
     (defined(__clang__) && __clang_major__ >= 9 &&               \
      !defined(KUMO_BUILD_DLL) && !defined(KUMO_CONSUME_DLL))) && \
    !defined(__MINGW32__)
#undef KUMO_ATTRIBUTE_WEAK
#define KUMO_ATTRIBUTE_WEAK __attribute__((weak))
#define KUMO_HAVE_ATTRIBUTE_WEAK 1
#else
#define KUMO_ATTRIBUTE_WEAK
#define KUMO_HAVE_ATTRIBUTE_WEAK 0
#endif


// KUMO_ATTRIBUTE_NONNULL
//
// Tells the compiler either (a) that a particular function parameter
// should be a non-null pointer, or (b) that all pointer arguments should
// be non-null.
//
// Note: As the GCC manual states, "[s]ince non-static C++ methods
// have an implicit 'this' argument, the arguments of such methods
// should be counted from two, not one."
//
// Args are indexed starting at 1.
//
// For non-static class member functions, the implicit `this` argument
// is arg 1, and the first explicit argument is arg 2. For static class member
// functions, there is no implicit `this`, and the first explicit argument is
// arg 1.
//
// Example:
//
//   /* arg_a cannot be null, but arg_b can */
//   void Function(void* arg_a, void* arg_b) KUMO_ATTRIBUTE_NONNULL(1);
//
//   class C {
//     /* arg_a cannot be null, but arg_b can */
//     void Method(void* arg_a, void* arg_b) KUMO_ATTRIBUTE_NONNULL(2);
//
//     /* arg_a cannot be null, but arg_b can */
//     static void StaticMethod(void* arg_a, void* arg_b)
//     KUMO_ATTRIBUTE_NONNULL(1);
//   };
//
// If no arguments are provided, then all pointer arguments should be non-null.
//
//  /* No pointer arguments may be null. */
//  void Function(void* arg_a, void* arg_b, int arg_c) KUMO_ATTRIBUTE_NONNULL();
//
// NOTE: The GCC nonnull attribute actually accepts a list of arguments, but
// KUMO_ATTRIBUTE_NONNULL does not.
#if KUMO_HAVE_ATTRIBUTE(nonnull) || (defined(__GNUC__))
#define KUMO_ATTRIBUTE_NONNULL(arg_index) __attribute__((nonnull(arg_index)))
#else
#define KUMO_ATTRIBUTE_NONNULL(...)
#endif


// KUMO_ATTRIBUTE_NORETURN
//
// Tells the compiler that a given function never returns.
//
// Deprecated: Prefer the `[[noreturn]]` attribute standardized by C++11 over
// this macro.
#if KUMO_HAVE_ATTRIBUTE(noreturn) || (defined(__GNUC__))
#define KUMO_ATTRIBUTE_NORETURN __attribute__((noreturn))
#elif defined(_MSC_VER)
#define KUMO_ATTRIBUTE_NORETURN __declspec(noreturn)
#else
#define KUMO_ATTRIBUTE_NORETURN
#endif


// KUMO_MUST_USE_RESULT
//
// Tells the compiler to warn about unused results.
//
// For code or headers that are assured to only build with C++17 and up, prefer
// just using the standard `[[nodiscard]]` directly over this macro.
//
// When annotating a function, it must appear as the first part of the
// declaration or definition. The compiler will warn if the return value from
// such a function is unused:
//
//   KUMO_MUST_USE_RESULT Sprocket* AllocateSprocket();
//   AllocateSprocket();  // Triggers a warning.
//
// When annotating a class, it is equivalent to annotating every function which
// returns an instance.
//
//   class KUMO_MUST_USE_RESULT Sprocket {};
//   Sprocket();  // Triggers a warning.
//
//   Sprocket MakeSprocket();
//   MakeSprocket();  // Triggers a warning.
//
// Note that references and pointers are not instances:
//
//   Sprocket* SprocketPointer();
//   SprocketPointer();  // Does *not* trigger a warning.
//
// KUMO_MUST_USE_RESULT allows using cast-to-void to suppress the unused result
// warning. For that, warn_unused_result is used only for clang but not for gcc.
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66425
//
// Note: past advice was to place the macro after the argument list.
//
// TODO(b/176172494): Use KUMO_HAVE_CPP_ATTRIBUTE(nodiscard) when all code is
// compliant with the stricter [[nodiscard]].
#if defined(__clang__) && KUMO_HAVE_ATTRIBUTE(warn_unused_result)
#define KUMO_MUST_USE_RESULT __attribute__((warn_unused_result))
#else
#define KUMO_MUST_USE_RESULT
#endif


// KUMO_ATTRIBUTE_HOT, KUMO_ATTRIBUTE_COLD
//
// Tells GCC that a function is hot or cold. GCC can use this information to
// improve static analysis, i.e. a conditional branch to a cold function
// is likely to be not-taken.
// This annotation is used for function declarations.
//
// Example:
//
//   int foo() KUMO_ATTRIBUTE_HOT;
#if KUMO_HAVE_ATTRIBUTE(hot) || (defined(__GNUC__))
#define KUMO_ATTRIBUTE_HOT __attribute__((hot))
#else
#define KUMO_ATTRIBUTE_HOT
#endif

#if KUMO_HAVE_ATTRIBUTE(cold) || (defined(__GNUC__))
#define KUMO_ATTRIBUTE_COLD __attribute__((cold))
#else
#define KUMO_ATTRIBUTE_COLD
#endif


// KUMO_ATTRIBUTE_PURE_FUNCTION
//
// KUMO_ATTRIBUTE_PURE_FUNCTION is used to annotate declarations of "pure"
// functions. A function is pure if its return value is only a function of its
// arguments. The pure attribute prohibits a function from modifying the state
// of the program that is observable by means other than inspecting the
// function's return value. Declaring such functions with the pure attribute
// allows the compiler to avoid emitting some calls in repeated invocations of
// the function with the same argument values.
//
// Example:
//
//  KUMO_ATTRIBUTE_PURE_FUNCTION std::string format_time(Time t);
#if KUMO_HAVE_CPP_ATTRIBUTE(gnu::pure)
#define KUMO_ATTRIBUTE_PURE_FUNCTION [[gnu::pure]]
#elif KUMO_HAVE_ATTRIBUTE(pure)
#define KUMO_ATTRIBUTE_PURE_FUNCTION __attribute__((pure))
#else
// If the attribute isn't defined, we'll fallback to KUMO_MUST_USE_RESULT since
// pure functions are useless if its return is ignored.
#define KUMO_ATTRIBUTE_PURE_FUNCTION KUMO_MUST_USE_RESULT
#endif



// KUMO_ATTRIBUTE_CONST_FUNCTION
//
// KUMO_ATTRIBUTE_CONST_FUNCTION is used to annotate declarations of "const"
// functions. A const function is similar to a pure function, with one
// exception: Pure functions may return value that depend on a non-volatile
// object that isn't provided as a function argument, while the const function
// is guaranteed to return the same result given the same arguments.
//
// Example:
//
//  KUMO_ATTRIBUTE_CONST_FUNCTION int64_t ToInt64Milliseconds(Duration d);
#if defined(_MSC_VER) && !defined(__clang__)
// Put the MSVC case first since MSVC seems to parse const as a C++ keyword.
#define KUMO_ATTRIBUTE_CONST_FUNCTION KUMO_ATTRIBUTE_PURE_FUNCTION
#elif KUMO_HAVE_CPP_ATTRIBUTE(gnu::const)
#define KUMO_ATTRIBUTE_CONST_FUNCTION [[gnu::const]]
#elif KUMO_HAVE_ATTRIBUTE(const)
#define KUMO_ATTRIBUTE_CONST_FUNCTION __attribute__((const))
#else
// Since const functions are more restrictive pure function, we'll fallback to a
// pure function if the const attribute is not handled.
#define KUMO_ATTRIBUTE_CONST_FUNCTION KUMO_ATTRIBUTE_PURE_FUNCTION
#endif


// KUMO_ATTRIBUTE_NO_TAIL_CALL
//
// Prevents the compiler from optimizing away stack frames for functions which
// end in a call to another function.
#if KUMO_HAVE_ATTRIBUTE(disable_tail_calls)
#define KUMO_HAVE_ATTRIBUTE_NO_TAIL_CALL 1
#define KUMO_ATTRIBUTE_NO_TAIL_CALL __attribute__((disable_tail_calls))
#elif defined(__GNUC__) && !defined(__clang__) && !defined(__e2k__)
#define KUMO_HAVE_ATTRIBUTE_NO_TAIL_CALL 1
#define KUMO_ATTRIBUTE_NO_TAIL_CALL \
  __attribute__((optimize("no-optimize-sibling-calls")))
#else
#define KUMO_ATTRIBUTE_NO_TAIL_CALL
#define KUMO_HAVE_ATTRIBUTE_NO_TAIL_CALL 0
#endif

// KUMO_ATTRIBUTE_RETURNS_NONNULL
//
// Tells the compiler that a particular function never returns a null pointer.
#if KUMO_HAVE_ATTRIBUTE(returns_nonnull)
#define KUMO_ATTRIBUTE_RETURNS_NONNULL __attribute__((returns_nonnull))
#else
#define KUMO_ATTRIBUTE_RETURNS_NONNULL
#endif


// KUMO_ATTRIBUTE_REINITIALIZES
//
// Indicates that a member function reinitializes the entire object to a known
// state, independent of the previous state of the object.
//
// The clang-tidy check bugprone-use-after-move allows member functions marked
// with this attribute to be called on objects that have been moved from;
// without the attribute, this would result in a use-after-move warning.
#if KUMO_HAVE_CPP_ATTRIBUTE(clang::reinitializes)
#define KUMO_ATTRIBUTE_REINITIALIZES [[clang::reinitializes]]
#else
#define KUMO_ATTRIBUTE_REINITIALIZES
#endif


// KUMO_ATTRIBUTE_FUNC_ALIGN
//
// Tells the compiler to align the function start at least to certain
// alignment boundary
#if KUMO_HAVE_ATTRIBUTE(aligned) || (defined(__GNUC__))
#define KUMO_ATTRIBUTE_FUNC_ALIGN(bytes) __attribute__((aligned(bytes)))
#else
#define KUMO_ATTRIBUTE_FUNC_ALIGN(bytes)
#endif


// KUMO_XRAY_ALWAYS_INSTRUMENT, KUMO_XRAY_NEVER_INSTRUMENT, KUMO_XRAY_LOG_ARGS
//
// We define the KUMO_XRAY_ALWAYS_INSTRUMENT and KUMO_XRAY_NEVER_INSTRUMENT
// macro used as an attribute to mark functions that must always or never be
// instrumented by XRay. Currently, this is only supported in Clang/LLVM.
//
// For reference on the LLVM XRay instrumentation, see
// http://llvm.org/docs/XRay.html.
//
// A function with the XRAY_ALWAYS_INSTRUMENT macro attribute in its declaration
// will always get the XRay instrumentation sleds. These sleds may introduce
// some binary size and runtime overhead and must be used sparingly.
//
// These attributes only take effect when the following conditions are met:
//
//   * The file/target is built with a Clang compiler that supports XRay
//     attributes.
//   * The file/target is built with the -fxray-instrument flag set for the
//     Clang/LLVM compiler.
//   * The function is defined in the translation unit (the compiler honors the
//     attribute in either the definition or the declaration, and must match).
//
// There are cases when, even when building with XRay instrumentation, users
// might want to control specifically which functions are instrumented for a
// particular build using special-case lists provided to the compiler. These
// special case lists are provided to Clang via the
// -fxray-always-instrument=... and -fxray-never-instrument=... flags. The
// attributes in source take precedence over these special-case lists.
//
// To disable the XRay attributes at build-time, users may define
// KUMO_NO_XRAY_ATTRIBUTES. Do NOT define KUMO_NO_XRAY_ATTRIBUTES on specific
// packages/targets, as this may lead to conflicting definitions of functions at
// link-time.
//
// XRay isn't currently supported on Android:
// https://github.com/android/ndk/issues/368
#if KUMO_HAVE_CPP_ATTRIBUTE(clang::xray_always_instrument) && \
    !defined(KUMO_NO_XRAY_ATTRIBUTES) && !defined(__ANDROID__)
#define KUMO_XRAY_ALWAYS_INSTRUMENT [[clang::xray_always_instrument]]
#define KUMO_XRAY_NEVER_INSTRUMENT [[clang::xray_never_instrument]]
#if KUMO_HAVE_CPP_ATTRIBUTE(clang::xray_log_args)
#define KUMO_XRAY_LOG_ARGS(N) \
  [[clang::xray_always_instrument, clang::xray_log_args(N)]]
#else
#define KUMO_XRAY_LOG_ARGS(N) [[clang::xray_always_instrument]]
#endif
#else
#define KUMO_XRAY_ALWAYS_INSTRUMENT
#define KUMO_XRAY_NEVER_INSTRUMENT
#define KUMO_XRAY_LOG_ARGS(N)
#endif

