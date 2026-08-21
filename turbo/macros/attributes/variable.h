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
// File: variable.h
// -----------------------------------------------------------------------------
//
// Variable attributes: unused, packed, TLS model, constinit, and similar.

#pragma once

#include <turbo/macros/compiler/compiler.h>


// KUMO_CONST_INIT
//
// A variable declaration annotated with the `KUMO_CONST_INIT` attribute will
// not compile (on supported platforms) unless the variable has a constant
// initializer. This is useful for variables with static and thread storage
// duration, because it guarantees that they will not suffer from the so-called
// "static init order fiasco".
//
// This attribute must be placed on the initializing declaration of the
// variable. Some compilers will give a -Wmissing-constinit warning when this
// attribute is placed on some other declaration but missing from the
// initializing declaration.
//
// In some cases (notably with thread_local variables), `KUMO_CONST_INIT` can
// also be used in a non-initializing declaration to tell the compiler that a
// variable is already initialized, reducing overhead that would otherwise be
// incurred by a hidden guard variable. Thus annotating all declarations with
// this attribute is recommended to potentially enhance optimization.
//
// Example:
//
//   class MyClass {
//    public:
//     KUMO_CONST_INIT static MyType my_var;
//   };
//
//   KUMO_CONST_INIT MyType MyClass::my_var = MakeMyType(...);
//
// For code or headers that are assured to only build with C++20 and up, prefer
// just using the standard `constinit` keyword directly over this macro.
//
// Note that this attribute is redundant if the variable is declared constexpr.
#if defined(__cpp_constinit) && __cpp_constinit >= 201907L
#define KUMO_CONST_INIT constinit
#elif KUMO_HAVE_CPP_ATTRIBUTE(clang::require_constant_initialization)
#define KUMO_CONST_INIT [[clang::require_constant_initialization]]
#else
#define KUMO_CONST_INIT
#endif


// KUMO_ATTRIBUTE_NULL_AFTER_MOVE
//
// Indicates that a user-defined smart-pointer-like type makes guarantees on the
// state of a moved-from object, leaving it in a null state, where it can be
// used as long as it is not dereferenced. In other words, these are the same
// semantics that smart pointers from the standard library provide.
//
// The clang-tidy check bugprone-use-after-move allows member functions of types
// marked with this attribute to be called on objects that have been moved from;
// without the attribute, this would result in a use-after-move warning.
#if KUMO_HAVE_CPP_ATTRIBUTE(clang::annotate) && defined(__clang__) && \
    __clang_major__ >= 12
#define KUMO_ATTRIBUTE_NULL_AFTER_MOVE                       \
  [[clang::annotate("clang-tidy", "bugprone-use-after-move", \
                    "null_after_move")]]
#else
#define KUMO_ATTRIBUTE_NULL_AFTER_MOVE
#endif


// KUMO_ATTRIBUTE_UNUSED
//
// Prevents the compiler from complaining about variables that appear unused.
//
// Deprecated: Use the standard C++17 `[[maybe_unused]]` instead.
//
// Due to differences in positioning requirements between the old, compiler
// specific __attribute__ syntax and the now standard `[[maybe_unused]]`, this
// macro does not attempt to take advantage of `[[maybe_unused]]`.
#if KUMO_HAVE_ATTRIBUTE(unused) || (defined(__GNUC__))
#undef KUMO_ATTRIBUTE_UNUSED
#define KUMO_ATTRIBUTE_UNUSED __attribute__((__unused__))
#else
#define KUMO_ATTRIBUTE_UNUSED
#endif


// KUMO_ATTRIBUTE_INITIAL_EXEC
//
// Tells the compiler to use "initial-exec" mode for a thread-local variable.
// See http://people.redhat.com/drepper/tls.pdf for the gory details.
#if KUMO_HAVE_ATTRIBUTE(tls_model) || (defined(__GNUC__))
#define KUMO_ATTRIBUTE_INITIAL_EXEC __attribute__((tls_model("initial-exec")))
#else
#define KUMO_ATTRIBUTE_INITIAL_EXEC
#endif


// When deprecating library code, it is sometimes necessary to turn off the
// warning within library, until the deprecated code is actually removed. The
// deprecated code can be surrounded with these directives to achieve that
// result.
//
// class KUMO_DEPRECATED("Use Bar instead") Foo;
//
// KUMO_DISABLE_DEPRECATED_DECLARATION_WARNING
// Baz ComputeBazFromFoo(Foo f);
// KUMO_RESTORE_DEPRECATED_DECLARATION_WARNING
#if defined(__GNUC__) || defined(__clang__)
// Clang also supports these GCC pragmas.
#define KUMO_DISABLE_DEPRECATED_DECLARATION_WARNING \
  _Pragma("GCC diagnostic push")             \
  _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"")
#define KUMO_RESTORE_DEPRECATED_DECLARATION_WARNING \
  _Pragma("GCC diagnostic pop")
#elif defined(_MSC_VER)
#define KUMO_DISABLE_DEPRECATED_DECLARATION_WARNING \
  _Pragma("warning(push)") _Pragma("warning(disable: 4996)")
#define KUMO_RESTORE_DEPRECATED_DECLARATION_WARNING \
  _Pragma("warning(pop)")
#else
#define KUMO_DISABLE_DEPRECATED_DECLARATION_WARNING
#define KUMO_RESTORE_DEPRECATED_DECLARATION_WARNING
#endif  // defined(__GNUC__) || defined(__clang__)


// KUMO_ATTRIBUTE_STACK_ALIGN_FOR_OLD_LIBC
//
// Support for aligning the stack on 32-bit x86.
#if KUMO_HAVE_ATTRIBUTE(force_align_arg_pointer) || \
    (defined(__GNUC__))
#if defined(__i386__)
#define KUMO_ATTRIBUTE_STACK_ALIGN_FOR_OLD_LIBC \
  __attribute__((force_align_arg_pointer))
#define KUMO_REQUIRE_STACK_ALIGN_TRAMPOLINE (0)
#elif defined(__x86_64__)
#define KUMO_REQUIRE_STACK_ALIGN_TRAMPOLINE (1)
#define KUMO_ATTRIBUTE_STACK_ALIGN_FOR_OLD_LIBC
#else  // !__i386__ && !__x86_64
#define KUMO_REQUIRE_STACK_ALIGN_TRAMPOLINE (0)
#define KUMO_ATTRIBUTE_STACK_ALIGN_FOR_OLD_LIBC
#endif  // __i386__
#else
#define KUMO_ATTRIBUTE_STACK_ALIGN_FOR_OLD_LIBC
#define KUMO_REQUIRE_STACK_ALIGN_TRAMPOLINE (0)
#endif

