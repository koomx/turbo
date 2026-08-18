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
#include <turbo/macros/have/sanitizer.h>


// KUMO_ATTRIBUTE_NO_SANITIZE_ADDRESS
//
// Tells the AddressSanitizer (or other memory testing tools) to ignore a given
// function. Useful for cases when a function reads random locations on stack,
// calls _exit from a cloned subprocess, deliberately accesses buffer
// out of bounds or does other scary things with memory.
// NOTE: GCC supports AddressSanitizer(asan) since 4.8.
// https://gcc.gnu.org/gcc-4.8/changes.html
#if KUMO_HAVE_ADDRESS_SANITIZER && \
KUMO_HAVE_ATTRIBUTE(no_sanitize_address)
#define KUMO_ATTRIBUTE_NO_SANITIZE_ADDRESS __attribute__((no_sanitize_address))
#elif KUMO_HAVE_ADDRESS_SANITIZER && defined(_MSC_VER) && \
    _MSC_VER >= 1928
// https://docs.microsoft.com/en-us/cpp/cpp/no-sanitize-address
#define KUMO_ATTRIBUTE_NO_SANITIZE_ADDRESS __declspec(no_sanitize_address)
#elif defined(KUMO_HAVE_HWADDRESS_SANITIZER) && KUMO_HAVE_ATTRIBUTE(no_sanitize)
// HWAddressSanitizer is a sanitizer similar to AddressSanitizer, which uses CPU
// features to detect similar bugs with less CPU and memory overhead.
// NOTE: GCC supports HWAddressSanitizer(hwasan) since 11.
// https://gcc.gnu.org/gcc-11/changes.html
#define KUMO_ATTRIBUTE_NO_SANITIZE_ADDRESS \
  __attribute__((no_sanitize("hwaddress")))
#else
#define KUMO_ATTRIBUTE_NO_SANITIZE_ADDRESS
#endif

// KUMO_ATTRIBUTE_NO_SANITIZE_MEMORY
//
// Tells the MemorySanitizer to relax the handling of a given function. All "Use
// of uninitialized value" warnings from such functions will be suppressed, and
// all values loaded from memory will be considered fully initialized.  This
// attribute is similar to the KUMO_ATTRIBUTE_NO_SANITIZE_ADDRESS attribute
// above, but deals with initialized-ness rather than addressability issues.
// NOTE: MemorySanitizer(msan) is supported by Clang but not GCC.
#if KUMO_HAVE_ATTRIBUTE(no_sanitize_memory)
#define KUMO_ATTRIBUTE_NO_SANITIZE_MEMORY __attribute__((no_sanitize_memory))
#else
#define KUMO_ATTRIBUTE_NO_SANITIZE_MEMORY
#endif

// KUMO_ATTRIBUTE_NO_SANITIZE_THREAD
//
// Tells the ThreadSanitizer to not instrument a given function.
// NOTE: GCC supports ThreadSanitizer(tsan) since 4.8.
// https://gcc.gnu.org/gcc-4.8/changes.html
#if KUMO_HAVE_ATTRIBUTE(no_sanitize_thread)
#define KUMO_ATTRIBUTE_NO_SANITIZE_THREAD __attribute__((no_sanitize_thread))
#else
#define KUMO_ATTRIBUTE_NO_SANITIZE_THREAD
#endif

// KUMO_ATTRIBUTE_NO_SANITIZE_UNDEFINED
//
// Tells the UndefinedSanitizer to ignore a given function. Useful for cases
// where certain behavior (eg. division by zero) is being used intentionally.
// NOTE: GCC supports UndefinedBehaviorSanitizer(ubsan) since 4.9.
// https://gcc.gnu.org/gcc-4.9/changes.html
#if KUMO_HAVE_ATTRIBUTE(no_sanitize_undefined)
#define KUMO_ATTRIBUTE_NO_SANITIZE_UNDEFINED \
  __attribute__((no_sanitize_undefined))
#elif KUMO_HAVE_ATTRIBUTE(no_sanitize)
#define KUMO_ATTRIBUTE_NO_SANITIZE_UNDEFINED \
  __attribute__((no_sanitize("undefined")))
#else
#define KUMO_ATTRIBUTE_NO_SANITIZE_UNDEFINED
#endif

// KUMO_ATTRIBUTE_NO_SANITIZE_CFI
//
// Tells the ControlFlowIntegrity sanitizer to not instrument a given function.
// See https://clang.llvm.org/docs/ControlFlowIntegrity.html for details.
#if KUMO_HAVE_ATTRIBUTE(no_sanitize) && defined(__llvm__)
#define KUMO_ATTRIBUTE_NO_SANITIZE_CFI __attribute__((no_sanitize("cfi")))
#else
#define KUMO_ATTRIBUTE_NO_SANITIZE_CFI
#endif

// KUMO_ATTRIBUTE_NO_SANITIZE_SAFESTACK
//
// Tells the SafeStack to not instrument a given function.
// See https://clang.llvm.org/docs/SafeStack.html for details.
#if KUMO_HAVE_ATTRIBUTE(no_sanitize)
#define KUMO_ATTRIBUTE_NO_SANITIZE_SAFESTACK \
  __attribute__((no_sanitize("safe-stack")))
#else
#define KUMO_ATTRIBUTE_NO_SANITIZE_SAFESTACK
#endif


