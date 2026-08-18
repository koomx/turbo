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

#pragma once

#include <turbo/macros/compiler/compiler.h>


// KUMO_ATTRIBUTE_LIFETIME_BOUND indicates that a resource owned by a function
// parameter or implicit object parameter is retained by the return value of the
// annotated function (or, for a parameter of a constructor, in the value of the
// constructed object). This attribute causes warnings to be produced if a
// temporary object does not live long enough.
//
// When applied to a reference parameter, the referenced object is assumed to be
// retained by the return value of the function. When applied to a non-reference
// parameter (for example, a pointer or a class type), all temporaries
// referenced by the parameter are assumed to be retained by the return value of
// the function.
//
// See also the upstream documentation:
// https://clang.llvm.org/docs/AttributeReference.html#lifetimebound
// https://learn.microsoft.com/en-us/cpp/code-quality/c26816?view=msvc-170
#if KUMO_HAVE_CPP_ATTRIBUTE(clang::lifetimebound)
#define KUMO_ATTRIBUTE_LIFETIME_BOUND [[clang::lifetimebound]]
#elif KUMO_HAVE_CPP_ATTRIBUTE(msvc::lifetimebound)
#define KUMO_ATTRIBUTE_LIFETIME_BOUND [[msvc::lifetimebound]]
#elif KUMO_HAVE_ATTRIBUTE(lifetimebound)
#define KUMO_ATTRIBUTE_LIFETIME_BOUND __attribute__((lifetimebound))
#else
#define KUMO_ATTRIBUTE_LIFETIME_BOUND
#endif


// KUMO_ATTRIBUTE_NO_UNIQUE_ADDRESS
//
// Indicates a data member can be optimized to occupy no space (if it is empty)
// and/or its tail padding can be used for other members.
//
// For code that is assured to only build with C++20 or later, prefer using
// the standard attribute `[[no_unique_address]]` directly instead of this
// macro.
//
// https://devblogs.microsoft.com/cppblog/msvc-cpp20-and-the-std-cpp20-switch/#c20-no_unique_address
// Current versions of MSVC have disabled `[[no_unique_address]]` since it
// breaks ABI compatibility, but offers `[[msvc::no_unique_address]]` for
// situations when it can be assured that it is desired. Since library does not
// claim ABI compatibility in mixed builds, we can offer it unconditionally.
#if defined(_MSC_VER) && _MSC_VER >= 1929
#define KUMO_ATTRIBUTE_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#elif KUMO_HAVE_CPP_ATTRIBUTE(no_unique_address)
#define KUMO_ATTRIBUTE_NO_UNIQUE_ADDRESS [[no_unique_address]]
#else
#define KUMO_ATTRIBUTE_NO_UNIQUE_ADDRESS
#endif


// KUMO_ATTRIBUTE_UNINITIALIZED
//
// GCC and Clang support a flag `-ftrivial-auto-var-init=<option>` (<option>
// can be "zero" or "pattern") that can be used to initialize automatic stack
// variables. Variables with this attribute will be left uninitialized,
// overriding the compiler flag.
//
// See https://clang.llvm.org/docs/AttributeReference.html#uninitialized
// and https://gcc.gnu.org/onlinedocs/gcc/Common-Variable-Attributes.html#index-uninitialized-variable-attribute
#if KUMO_HAVE_CPP_ATTRIBUTE(clang::uninitialized)
#define KUMO_ATTRIBUTE_UNINITIALIZED [[clang::uninitialized]]
#elif KUMO_HAVE_CPP_ATTRIBUTE(gnu::uninitialized)
#define KUMO_ATTRIBUTE_UNINITIALIZED [[gnu::uninitialized]]
#elif KUMO_HAVE_ATTRIBUTE(uninitialized)
#define KUMO_ATTRIBUTE_UNINITIALIZED __attribute__((uninitialized))
#else
#define KUMO_ATTRIBUTE_UNINITIALIZED
#endif

// Requires the compiler to prove that the size of the given object is at least
// the expected amount.
#if KUMO_HAVE_ATTRIBUTE(diagnose_if) && KUMO_HAVE_BUILTIN(__builtin_object_size)
#define KUMO_INTERNAL_NEED_MIN_SIZE(Obj, N)                     \
  __attribute__((diagnose_if(__builtin_object_size(Obj, 0) < N, \
                             "object size provably too small "  \
                             "(this would corrupt memory)",     \
                             "error")))
#else
#define KUMO_INTERNAL_NEED_MIN_SIZE(Obj, N)
#endif


// Internal attribute; name and documentation TBD.
//
// See the upstream documentation:
// https://clang.llvm.org/docs/AttributeReference.html#lifetime_capture_by_this
//
// Note: KUMO_INTERNAL_ATTRIBUTE_CAPTURED_BY(this) is deprecated. Use
// KUMO_INTERNAL_ATTRIBUTE_CAPTURED_BY_THIS instead.
#if KUMO_HAVE_CPP_ATTRIBUTE(clang::lifetime_capture_by_this)
#define KUMO_INTERNAL_ATTRIBUTE_CAPTURED_BY_THIS \
  [[clang::lifetime_capture_by_this]]
#elif KUMO_HAVE_CPP_ATTRIBUTE(clang::lifetime_capture_by)
#define KUMO_INTERNAL_ATTRIBUTE_CAPTURED_BY_THIS \
  KUMO_INTERNAL_ATTRIBUTE_CAPTURED_BY(this)
#else
#define KUMO_INTERNAL_ATTRIBUTE_CAPTURED_BY_THIS
#endif

// Internal attribute; name and documentation TBD.
//
// See the upstream documentation:
// https://clang.llvm.org/docs/AttributeReference.html#lifetime_capture_by
#if KUMO_HAVE_CPP_ATTRIBUTE(clang::lifetime_capture_by)
#define KUMO_INTERNAL_ATTRIBUTE_CAPTURED_BY(Owner) \
  [[clang::lifetime_capture_by(Owner)]]
#else
#define KUMO_INTERNAL_ATTRIBUTE_CAPTURED_BY(Owner)
#endif


#ifdef __cplusplus
struct KumoInternal_YouForgotToExplicitlyInitializeAField {
    // A portable version of [[clang::require_explicit_initialization]] that
    // never builds, as a last resort for all toolchains.
    // The error messages are poor, so we don't rely on this unless we have to.
    template<class T>
#if !defined(SWIG)
    constexpr
#endif
    operator T() const /* NOLINT */ {
        const void *volatile deliberately_volatile_ptr = nullptr;
        // Infinite loop to prevent constexpr compilation
        for (;;) {
            // This assignment ensures the 'this' pointer is not optimized away, so
            // that linking always fails.
            deliberately_volatile_ptr = this; // Deliberately not constexpr
            (void) deliberately_volatile_ptr;
        }
    }

    // This is deliberately left undefined to prevent linking
    static KumoInternal_YouForgotToExplicitlyInitializeAField v;
};
#endif


// KUMO_REQUIRE_EXPLICIT_INIT
//
// KUMO_REQUIRE_EXPLICIT_INIT is placed *after* the data members of an aggregate
// type to indicate that the annotated member must be explicitly initialized by
// the user whenever the aggregate is constructed. For example:
//
//   struct Coord {
//     int x KUMO_REQUIRE_EXPLICIT_INIT;
//     int y KUMO_REQUIRE_EXPLICIT_INIT;
//   };
//   Coord coord = {1};  // warning: field 'y' is not explicitly initialized
//
// Note that usage on C arrays is not supported in C++.
// Use a struct (such as std::array) to wrap the array member instead.
//
// Avoid applying this attribute to the members of non-aggregate types.
// The behavior within non-aggregates is unspecified and subject to change.
//
// Do NOT attempt to suppress or demote the error generated by this attribute.
// Just like with a missing function argument, it is a hard error by design.
//
// See the upstream documentation for more details:
// https://clang.llvm.org/docs/AttributeReference.html#require-explicit-initialization
#ifdef __cplusplus
#if KUMO_HAVE_CPP_ATTRIBUTE(clang::require_explicit_initialization)
// clang-format off
#define KUMO_REQUIRE_EXPLICIT_INIT \
  [[clang::require_explicit_initialization]] = \
    KumoInternal_YouForgotToExplicitlyInitializeAField::v
#else
#define KUMO_REQUIRE_EXPLICIT_INIT \
  = KumoInternal_YouForgotToExplicitlyInitializeAField::v
#endif
// clang-format on
#else
// clang-format off
#if KUMO_HAVE_ATTRIBUTE(require_explicit_initialization)
#define KUMO_REQUIRE_EXPLICIT_INIT \
  __attribute__((require_explicit_initialization))
#else
#define KUMO_REQUIRE_EXPLICIT_INIT \
  /* No portable fallback for C is available */
#endif
// clang-format on
#endif

