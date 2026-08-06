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
// File: basic.h
// -----------------------------------------------------------------------------
//
// Fundamental utility macros — token manipulation, type safety, class
// disallowing, array sizing, structure packing, and portable compiler hints.

#pragma once

#include <stddef.h>

// ---------------------------------------------------------------------------
// Token manipulation
// ---------------------------------------------------------------------------

#define KUMO_DO_CONCAT(a, b)    a ## b
#define KUMO_CONCAT(a, b)     KUMO_DO_CONCAT(a, b)

#define KUMO_DO_STRINGIFY(x)    #x
#define KUMO_STRINGIFY(x)     KUMO_DO_STRINGIFY(x)
#define KUMO_COUNTER          __COUNTER__

// ---------------------------------------------------------------------------
// KUMO_UNUSED
//
// Suppresses "unused variable/parameter" warnings.
//   int KUMO_UNUSED(ret) = fn();
// ---------------------------------------------------------------------------

#define KUMO_UNUSED(x)        ((void)(x))

// ---------------------------------------------------------------------------
// KUMO_RESTRICT
//
// Portable restrict qualifier for pointers.
//   void copy(KUMO_RESTRICT char *dst, KUMO_RESTRICT const char *src);
// ---------------------------------------------------------------------------

#if defined(__clang__) || defined(__GNUC__)
#define KUMO_RESTRICT         __restrict__
#elif defined(_MSC_VER)
#define KUMO_RESTRICT         __restrict
#else
#define KUMO_RESTRICT
#endif

// ---------------------------------------------------------------------------
// KUMO_DISABLE_UBSAN
//
// Disables a specific Undefined-Behaviour Sanitizer check for a function.
//
//   KUMO_DISABLE_UBSAN("alignment")
//   static void unaligned_load(void) { ... }
// ---------------------------------------------------------------------------

#if defined(__clang__) || defined(__GNUC__)
#define KUMO_DISABLE_UBSAN(feature)  __attribute__((no_sanitize(feature)))
#else
#define KUMO_DISABLE_UBSAN(feature)
#endif

// ---------------------------------------------------------------------------
// KUMO_ARRAYSIZE
//
// Returns the number of elements in a C/C++ array as a compile-time constant.
// Passing a pointer triggers a compile error (C++ template version).
//
//   int buf[16];
//   for (size_t i = 0; i < KUMO_ARRAYSIZE(buf); ++i) { ... }
// ---------------------------------------------------------------------------

#if defined(__cplusplus)
template <typename T, size_t N>
constexpr auto KUMO_INTERNAL_ARRAYSIZE_HELPER(const T (&)[N]) -> char (&)[N];
#define KUMO_ARRAYSIZE(arr)  (sizeof(KUMO_INTERNAL_ARRAYSIZE_HELPER(arr)))
#else
#define KUMO_ARRAYSIZE(arr)  (sizeof(arr) / sizeof((arr)[0]))
#endif

// ---------------------------------------------------------------------------
// KUMO_CONTAINER_OF
//
// Returns a pointer to the enclosing struct/class given a pointer to one of
// its members.
//
//   struct Foo { int a; int b; };
//   struct Foo *f = KUMO_CONTAINER_OF(&ptr->b, struct Foo, b);
// ---------------------------------------------------------------------------

#define KUMO_CONTAINER_OF(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

// ---------------------------------------------------------------------------
// C++ specific macros
// ---------------------------------------------------------------------------

#if defined(__cplusplus)

// KUMO_DELETE_FUNCTION
#define KUMO_DELETE_FUNCTION(decl)  decl = delete

// KUMO_DISALLOW_COPY / KUMO_DISALLOW_ASSIGN / ...
//
// Put these in the private: section of a class to prevent copying / assigning.
//
//   class NonCopyable {
//    public:
//     NonCopyable() = default;
//    private:
//     KUMO_DISALLOW_COPY_AND_ASSIGN(NonCopyable);
//   };

#define KUMO_DISALLOW_COPY(TypeName)                \
    KUMO_DELETE_FUNCTION(TypeName(const TypeName &))

#define KUMO_DISALLOW_ASSIGN(TypeName)              \
    KUMO_DELETE_FUNCTION(TypeName &operator=(const TypeName &))

#define KUMO_DISALLOW_COPY_AND_ASSIGN(TypeName)     \
    KUMO_DISALLOW_COPY(TypeName);                    \
    KUMO_DISALLOW_ASSIGN(TypeName)

#define KUMO_DISALLOW_IMPLICIT_CONSTRUCTORS(TypeName) \
    KUMO_DELETE_FUNCTION(TypeName());                  \
    KUMO_DISALLOW_COPY_AND_ASSIGN(TypeName)

// KUMO_DEFAULT_MOVE_AND_ASSIGN
//
//   class Movable {
//    public:
//     Movable() = default;
//     KUMO_DEFAULT_MOVE_AND_ASSIGN(Movable);
//   };

#define KUMO_DEFAULT_MOVE_AND_ASSIGN(TypeName)      \
    TypeName(TypeName &&) = default;                 \
    TypeName &operator=(TypeName &&) = default

// KUMO_GLOBAL_INIT (C++)
//
// Registers a function to run at program startup (before main()).
//
//   KUMO_GLOBAL_INIT()
//   {
//       register_factory("my_type", create_my_type);
//   }

#define KUMO_GLOBAL_INIT                                               \
namespace {                                                            \
    struct KUMO_CONCAT(KumoGlobalInit, __LINE__) {                    \
        KUMO_CONCAT(KumoGlobalInit, __LINE__)() { init(); }           \
        void init();                                                   \
    } KUMO_CONCAT(kumo_global_init_dummy_, __LINE__);                 \
}                                                                      \
void KUMO_CONCAT(KumoGlobalInit, __LINE__)::init

#else  // !__cplusplus (C only)

// KUMO_GLOBAL_INIT (C)
//
// Uses __attribute__((constructor)) on GCC/Clang.  MSVC in C mode has no
// portable equivalent; a manual init call is required.

#if defined(__GNUC__) || defined(__clang__)
#define KUMO_GLOBAL_INIT                                               \
    static void KUMO_CONCAT(kumo_global_init_, __LINE__)(void)         \
    __attribute__((constructor))
#else
#define KUMO_GLOBAL_INIT                                               \
    static void KUMO_CONCAT(kumo_global_init_, __LINE__)(void)
#endif

#endif  // __cplusplus

// ---------------------------------------------------------------------------
// KUMO_MANUALLY_ALIGNED_STRUCT / KUMO_STRUCT_END
//
// Portable packed + aligned struct for wire-format / file-format data.
//
//   KUMO_MANUALLY_ALIGNED_STRUCT(4) MyHeader {
//       uint32_t magic;
//       uint16_t flags;
//   };
//   KUMO_STRUCT_END(MyHeader, 6);
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Internal compile-time assertion for C < C11
// ---------------------------------------------------------------------------

#if !defined(__cplusplus) && !defined(static_assert)
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define KUMO_INTERNAL_STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)
#else
// C99 fallback: negative-size typedef → compile error if cond is false
#define KUMO_INTERNAL_STATIC_ASSERT(cond, msg) \
  typedef char KUMO_CONCAT(kumo_assert_, KUMO_COUNTER)[(cond) ? 1 : -1]
#endif
#else
#define KUMO_INTERNAL_STATIC_ASSERT(cond, msg) static_assert(cond, msg)
#endif

// ---------------------------------------------------------------------------
// KUMO_MANUALLY_ALIGNED_STRUCT / KUMO_STRUCT_END
//
// Portable packed + aligned struct for wire-format / file-format data.
//
//   KUMO_MANUALLY_ALIGNED_STRUCT(4) MyHeader {
//       uint32_t magic;
//       uint16_t flags;
//   };
//   KUMO_STRUCT_END(MyHeader, 8);
// ---------------------------------------------------------------------------

#if !defined(KUMO_MANUALLY_ALIGNED_STRUCT)

#if defined(_MSC_VER)
#define KUMO_MANUALLY_ALIGNED_STRUCT(alignment) \
  __pragma(pack(1)) struct __declspec(align(alignment))
#define KUMO_STRUCT_END(name, size) \
  __pragma(pack()) KUMO_INTERNAL_STATIC_ASSERT(sizeof(name) == size, "compiler breaks packing rules")

#elif defined(__GNUC__) || defined(__clang__)
#define KUMO_MANUALLY_ALIGNED_STRUCT(alignment) \
  _Pragma("pack(1)") struct __attribute__((aligned(alignment)))
#define KUMO_STRUCT_END(name, size) \
  _Pragma("pack()") KUMO_INTERNAL_STATIC_ASSERT(sizeof(name) == size, "compiler breaks packing rules")

#else
#error KUMO_MANUALLY_ALIGNED_STRUCT is not implemented for this compiler
#endif

#endif  // !defined(KUMO_MANUALLY_ALIGNED_STRUCT)

// KUMO_EMSCRIPTEN_VERSION combines Emscripten's three version macros
// into an integer that can be compared against.
#ifdef KUMO_EMSCRIPTEN_VERSION
#error KUMO_EMSCRIPTEN_VERSION cannot be directly set
#endif
#ifdef __EMSCRIPTEN__
#include <emscripten/version.h>
#ifdef __EMSCRIPTEN_MAJOR__
#if __EMSCRIPTEN_MINOR__ >= 1000
#error __EMSCRIPTEN_MINOR__ is too big to fit in KUMO_EMSCRIPTEN_VERSION
#endif
#if __EMSCRIPTEN_TINY__ >= 1000
#error __EMSCRIPTEN_TINY__ is too big to fit in KUMO_EMSCRIPTEN_VERSION
#endif
#define KUMO_EMSCRIPTEN_VERSION                              \
  ((__EMSCRIPTEN_MAJOR__) * 1000000 + (__EMSCRIPTEN_MINOR__) * 1000 + \
   (__EMSCRIPTEN_TINY__))
#endif
#endif

