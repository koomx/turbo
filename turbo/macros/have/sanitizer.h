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
// File: sanitizer.h
// -----------------------------------------------------------------------------
//
// Sanitizer detection macros.  Every macro is defined to 1 (enabled) or 0
// (disabled).  They reflect the compiler flags at build time
// (e.g. -fsanitize=address).

#pragma once

#include <turbo/macros/compiler/compiler.h>
#include <turbo/macros/have/base.h>


// KUMO_HAVE_MEMORY_SANITIZER
//
// MemorySanitizer (MSan) is a detector of uninitialized reads. It consists of
// a compiler instrumentation module and a run-time library.
#ifdef KUMO_HAVE_MEMORY_SANITIZER
#error "KUMO_HAVE_MEMORY_SANITIZER cannot be directly set."
#elif !defined(__native_client__) && KUMO_HAVE_FEATURE(memory_sanitizer)
#define KUMO_HAVE_MEMORY_SANITIZER 1
#else
#define KUMO_HAVE_MEMORY_SANITIZER 0
#endif

// KUMO_HAVE_THREAD_SANITIZER
//
// ThreadSanitizer (TSan) is a fast data race detector.
#ifdef KUMO_HAVE_THREAD_SANITIZER
#error "KUMO_HAVE_THREAD_SANITIZER cannot be directly set."
#elif defined(__SANITIZE_THREAD__)
#define KUMO_HAVE_THREAD_SANITIZER 1
#elif KUMO_HAVE_FEATURE(thread_sanitizer)
#define KUMO_HAVE_THREAD_SANITIZER 1
#else
#define KUMO_HAVE_THREAD_SANITIZER 0
#endif


// KUMO_HAVE_ADDRESS_SANITIZER
//
// AddressSanitizer (ASan) is a fast memory error detector.
#ifdef KUMO_HAVE_ADDRESS_SANITIZER
#error "KUMO_HAVE_ADDRESS_SANITIZER cannot be directly set."
#elif defined(__SANITIZE_ADDRESS__)
#define KUMO_HAVE_ADDRESS_SANITIZER 1
#elif KUMO_HAVE_FEATURE(address_sanitizer)
#define KUMO_HAVE_ADDRESS_SANITIZER 1
#else
#define KUMO_HAVE_ADDRESS_SANITIZER 0
#endif


// KUMO_HAVE_HWADDRESS_SANITIZER
//
// Hardware-Assisted AddressSanitizer (or HWASAN) is even faster than asan
// memory error detector which can use CPU features like ARM TBI, Intel LAM or
// AMD UAI.
#ifdef KUMO_HAVE_HWADDRESS_SANITIZER
#error "KUMO_HAVE_HWADDRESS_SANITIZER cannot be directly set."
#elif defined(__SANITIZE_HWADDRESS__)
#define KUMO_HAVE_HWADDRESS_SANITIZER 1
#elif KUMO_HAVE_FEATURE(hwaddress_sanitizer)
#define KUMO_HAVE_HWADDRESS_SANITIZER 1
#else
#define KUMO_HAVE_HWADDRESS_SANITIZER 0
#endif



// KUMO_HAVE_DATAFLOW_SANITIZER
//
// Dataflow Sanitizer (or DFSAN) is a generalised dynamic data flow analysis.
#ifdef KUMO_HAVE_DATAFLOW_SANITIZER
#error "KUMO_HAVE_DATAFLOW_SANITIZER cannot be directly set."
#elif defined(DATAFLOW_SANITIZER)
// GCC provides no method for detecting the presence of the standalone
// DataFlowSanitizer (-fsanitize=dataflow), so GCC users of -fsanitize=dataflow
// should also use -DDATAFLOW_SANITIZER.
#define KUMO_HAVE_DATAFLOW_SANITIZER 1
#elif KUMO_HAVE_FEATURE(dataflow_sanitizer)
#define KUMO_HAVE_DATAFLOW_SANITIZER 1
#else
#define KUMO_HAVE_DATAFLOW_SANITIZER 0
#endif

// KUMO_HAVE_UNDEFINED_SANITIZER
//
// UndefinedBehaviorSanitizer (UBSan) is a fast undefined behavior detector.
#ifdef KUMO_HAVE_UNDEFINED_SANITIZER
#error "KUMO_HAVE_UNDEFINED_SANITIZER cannot be directly set."
#elif defined(__SANITIZE_UNDEFINED__)
#define KUMO_HAVE_UNDEFINED_SANITIZER 1
#elif defined(UNDEFINED_SANITIZER)
// GCC provides no method for detecting the presence of the standalone
// UndefinedBehaviorSanitizer (-fsanitize=undefined), so GCC users of
// -fsanitize=undefined should also use -DUNDEFINED_SANITIZER.
#define KUMO_HAVE_UNDEFINED_SANITIZER 1
#elif KUMO_HAVE_FEATURE(undefined_behavior_sanitizer)
#define KUMO_HAVE_UNDEFINED_SANITIZER 1
#else
#define KUMO_HAVE_UNDEFINED_SANITIZER 0
#endif

// KUMO_HAVE_LEAK_SANITIZER
//
// LeakSanitizer (or lsan) is a detector of memory leaks.
// https://clang.llvm.org/docs/LeakSanitizer.html
// https://github.com/google/sanitizers/wiki/AddressSanitizerLeakSanitizer
//
// The macro KUMO_HAVE_LEAK_SANITIZER can be used to detect at compile-time
// whether the LeakSanitizer is potentially available. However, just because the
// LeakSanitizer is available does not mean it is active. Use the
// always-available run-time interface in //turbo/debugging/leak_check.h for
// interacting with LeakSanitizer.
#ifdef KUMO_HAVE_LEAK_SANITIZER
#error "KUMO_HAVE_LEAK_SANITIZER cannot be directly set."
#elif defined(LEAK_SANITIZER)
// GCC provides no method for detecting the presence of the standalone
// LeakSanitizer (-fsanitize=leak), so GCC users of -fsanitize=leak should also
// use -DLEAK_SANITIZER.
#define KUMO_HAVE_LEAK_SANITIZER 1
// Clang standalone LeakSanitizer (-fsanitize=leak)
#elif KUMO_HAVE_FEATURE(leak_sanitizer)
#define KUMO_HAVE_LEAK_SANITIZER 1
#elif KUMO_HAVE_ADDRESS_SANITIZER && !defined(_WIN32)
// GCC or Clang using the LeakSanitizer integrated into AddressSanitizer.
#define KUMO_HAVE_LEAK_SANITIZER 1
#else
#define KUMO_HAVE_LEAK_SANITIZER 0
#endif
