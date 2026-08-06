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
// File: cache_line.h
// -----------------------------------------------------------------------------
//
// Cache line alignment macros.  KUMO_CACHELINE_SIZE is defined by each
// architecture header in <xmacros/arch/>; this file provides the alignment
// annotation macro that uses it.

#pragma once

#include <turbo/macros/compiler/compiler.h>

// KUMO_REFACTOR_INLINE
//
// Marks a function or type for automated refactoring by go/cpp-inliner. It can
// be used on inline function definitions or type aliases in header files and
// should be combined with the `[[deprecated]]` attribute.
//
// Using `KUMO_REFACTOR_INLINE` differs from using the `[[deprecated]]` alone in
// the following ways:
//
// 1. New uses of the function or type will be discouraged via Tricorder
//    warnings.
// 2. If enabled via `METADATA`, automated changes will be sent out inlining the
//    functions's body or replacing the type where it is used.
//
// Examples:
//
// [[deprecated("Use NewFunc() instead")]] KUMO_REFACTOR_INLINE
// inline int OldFunc(int x) {
//   return NewFunc(x, 0);
// }
//
// using OldType [[deprecated("Use NewType instead")]] KUMO_REFACTOR_INLINE =
//     NewType;
//
// will mark `OldFunc` and `OldType` as deprecated, and the go/cpp-inliner
// service will replace calls to `OldFunc(x)` with calls to `NewFunc(x, 0)` and
// `OldType` with `NewType`. Once all replacements have been completed, the old
// function or type can be deleted.
//
// Internal note: Clang also allows `KUMO_REFACTOR_INLINE` to be used on
// using-declarations, but attributes on using-declarations are invalid in C++.
// (NOTE: This note refers to `using a::b KUMO_REFACTOR_INLINE;` and not
// `using b KUMO_REFACTOR_INLINE = a::b;`, which is OK.) Therefore:
//
// 1. In OSS: Do not use this on using-declarations. Such usage is invalid and
//    unsupported usage, and may break at any time.
// 2. In Google: Avoid such usage except as a last resort. Instead, prefer other
//    inlining approaches (such as type aliases or forwarding functions,
//    illustrated above) whenever possible. This is because Clang (currently)
//    does not honor the [[deprecated]] attribute on using-declarations, and
//    therefore cannot surface the deprecation to users in the middle of a
//    migration.
//
// See go/cpp-inliner for more information.
//
// Note: go/cpp-inliner is Google-internal service for automated refactoring.
// While open-source users do not have access to this service, the macro is
// provided for compatibility.
#if KUMO_HAVE_CPP_ATTRIBUTE(clang::annotate)
#define KUMO_REFACTOR_INLINE                                                \
  _Pragma("clang diagnostic push") /* Avoid errors on using-declarations */ \
      _Pragma("clang diagnostic ignored \"-Wcxx-attribute-extension\"")     \
          [[clang::annotate("inline-me")]] _Pragma("clang diagnostic pop")
#else
#define KUMO_REFACTOR_INLINE
#endif

// KUMO_DEPRECATE_AND_INLINE()
//
// This is the original macro used by go/cpp-inliner that combines
// [[deprecated]] and KUMO_REFACTOR_INLINE.
//
// Examples:
//
// KUMO_DEPRECATE_AND_INLINE() inline int OldFunc(int x) {
//   return NewFunc(x, 0);
// }
//
// using OldType KUMO_DEPRECATE_AND_INLINE() = NewType;
//
// The combination of `[[deprecated("Use X instead")]]` and
// `KUMO_REFACTOR_INLINE` is preferred because it provides a more informative
// deprecation message to developers, especially those that do not have access
// to the automated refactoring capabilities of go/cpp-inliner.
#define KUMO_DEPRECATE_AND_INLINE() [[deprecated]] KUMO_REFACTOR_INLINE
