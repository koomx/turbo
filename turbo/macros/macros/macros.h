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
// File: macros.h
// -----------------------------------------------------------------------------
//
// Umbrella header for the xmacros macros layer (attributes, optimization,
// pragma, visibility, etc.).

#pragma once

#include <turbo/macros/utility/basic.h>
#include <turbo/macros/utility/pretty_function.h>
#include <turbo/macros/utility/testing.h>
#include <turbo/macros/macros/deprecated.h>

#include <turbo/macros/have/have.h>

#include <turbo/macros/macros/visibility.h>
#include <turbo/macros/optimization/optimization.h>
#include <turbo/macros/macros/cache_line.h>
#include <turbo/macros/macros/raw_log.h>
#include <turbo/macros/macros/pragma/pragma.h>

#include <turbo/macros/macros/assert.h>
#include <turbo/macros/macros/using_std.h>
#include <turbo/macros/macros/memory.h>



// ---------------------------------------------------------------------------
// Completeness check
// ---------------------------------------------------------------------------

#ifndef KUMO_EXPORT
#error "KUMO_EXPORT is not defined"
#endif

#ifndef KUMO_IMPORT
#error "KUMO_IMPORT is not defined"
#endif

#ifndef KUMO_LOCAL
#error "KUMO_LOCAL is not defined"
#endif

#ifndef KUMO_LIKELY
#error "KUMO_LIKELY is not defined"
#endif

#ifndef KUMO_UNLIKELY
#error "KUMO_UNLIKELY is not defined"
#endif

#ifndef KUMO_UNREACHABLE
#error "KUMO_UNREACHABLE is not defined"
#endif

#ifndef KUMO_ASSUME
#error "KUMO_ASSUME is not defined"
#endif

#ifndef KUMO_BLOCK_TAIL_CALL_OPTIMIZATION
#error "KUMO_BLOCK_TAIL_CALL_OPTIMIZATION is not defined"
#endif

#ifndef KUMO_PRETTY_FUNCTION
#error "KUMO_PRETTY_FUNCTION is not defined"
#endif

#ifndef KUMO_FUNC
#error "KUMO_FUNC is not defined"
#endif

#ifndef KUMO_FILE
#error "KUMO_FILE is not defined"
#endif

#ifndef KUMO_LINE
#error "KUMO_LINE is not defined"
#endif

#ifndef KUMO_CONCAT
#error "KUMO_CONCAT is not defined"
#endif

#ifndef KUMO_STRINGIFY
#error "KUMO_STRINGIFY is not defined"
#endif

#ifndef KUMO_COUNTER
#error "KUMO_COUNTER is not defined"
#endif

#ifndef KUMO_UNUSED
#error "KUMO_UNUSED is not defined"
#endif

#ifndef KUMO_RESTRICT
#error "KUMO_RESTRICT is not defined"
#endif

#ifndef KUMO_DISABLE_UBSAN
#error "KUMO_DISABLE_UBSAN is not defined"
#endif

#ifndef KUMO_ARRAYSIZE
#error "KUMO_ARRAYSIZE is not defined"
#endif

#ifndef KUMO_CONTAINER_OF
#error "KUMO_CONTAINER_OF is not defined"
#endif

#ifndef KUMO_MANUALLY_ALIGNED_STRUCT
#error "KUMO_MANUALLY_ALIGNED_STRUCT is not defined"
#endif

#ifndef KUMO_STRUCT_END
#error "KUMO_STRUCT_END is not defined"
#endif

#ifndef KUMO_HAVE_EXCEPTIONS
#error "KUMO_HAVE_EXCEPTIONS is not defined"
#endif

#ifndef KUMO_HAVE_RTTI
#error "KUMO_HAVE_RTTI is not defined"
#endif

#ifndef KUMO_HAVE_THREAD_LOCAL
#error "KUMO_HAVE_THREAD_LOCAL is not defined"
#endif

#ifndef KUMO_HAVE_TLS
#error "KUMO_HAVE_TLS is not defined"
#endif

#ifndef KUMO_HAVE_INTRINSIC_INT128
#error "KUMO_HAVE_INTRINSIC_INT128 is not defined"
#endif

#ifndef KUMO_HAVE_CONSTANT_EVALUATED
#error "KUMO_HAVE_CONSTANT_EVALUATED is not defined"
#endif

#ifndef KUMO_HAVE_MMAP
#error "KUMO_HAVE_MMAP is not defined"
#endif

#ifndef KUMO_HAVE_UNISTD_H
#error "KUMO_HAVE_UNISTD_H is not defined"
#endif

#ifndef KUMO_HAVE_FEATURE
#error "KUMO_HAVE_FEATURE is not defined"
#endif

#ifndef KUMO_HAVE_DLADDR
#error "KUMO_HAVE_DLADDR is not defined"
#endif

// ---------------------------------------------------------------------------
// have_ext.h
// ---------------------------------------------------------------------------

#ifndef KUMO_HAVE_STD_IS_TRIVIALLY_DESTRUCTIBLE
#error "KUMO_HAVE_STD_IS_TRIVIALLY_DESTRUCTIBLE is not defined"
#endif

#ifndef KUMO_HAVE_STD_IS_TRIVIALLY_CONSTRUCTIBLE
#error "KUMO_HAVE_STD_IS_TRIVIALLY_CONSTRUCTIBLE is not defined"
#endif

#ifndef KUMO_HAVE_STD_IS_TRIVIALLY_ASSIGNABLE
#error "KUMO_HAVE_STD_IS_TRIVIALLY_ASSIGNABLE is not defined"
#endif

#ifndef KUMO_HAVE_STD_IS_TRIVIALLY_COPYABLE
#error "KUMO_HAVE_STD_IS_TRIVIALLY_COPYABLE is not defined"
#endif

#ifndef KUMO_HAVE_STD_ORDERING
#error "KUMO_HAVE_STD_ORDERING is not defined"
#endif

#ifndef KUMO_HAVE_CLASS_TEMPLATE_ARGUMENT_DEDUCTION
#error "KUMO_HAVE_CLASS_TEMPLATE_ARGUMENT_DEDUCTION is not defined"
#endif

#ifndef KUMO_HAVE_MIN_GNUC_VERSION
#error "KUMO_HAVE_MIN_GNUC_VERSION is not defined"
#endif

#ifndef KUMO_HAVE_MIN_CLANG_VERSION
#error "KUMO_HAVE_MIN_CLANG_VERSION is not defined"
#endif

#ifndef KUMO_HAVE_PTHREAD_GETSCHEDPARAM
#error "KUMO_HAVE_PTHREAD_GETSCHEDPARAM is not defined"
#endif

#ifndef KUMO_HAVE_SCHED_GETCPU
#error "KUMO_HAVE_SCHED_GETCPU is not defined"
#endif

#ifndef KUMO_HAVE_SCHED_YIELD
#error "KUMO_HAVE_SCHED_YIELD is not defined"
#endif

#ifndef KUMO_HAVE_SEMAPHORE_H
#error "KUMO_HAVE_SEMAPHORE_H is not defined"
#endif

#ifndef KUMO_HAVE_ALARM
#error "KUMO_HAVE_ALARM is not defined"
#endif

// ---------------------------------------------------------------------------
// sanitizer.h
// ---------------------------------------------------------------------------

#ifndef KUMO_HAVE_ADDRESS_SANITIZER
#error "KUMO_HAVE_ADDRESS_SANITIZER is not defined"
#endif

#ifndef KUMO_HAVE_HWADDRESS_SANITIZER
#error "KUMO_HAVE_HWADDRESS_SANITIZER is not defined"
#endif

#ifndef KUMO_HAVE_THREAD_SANITIZER
#error "KUMO_HAVE_THREAD_SANITIZER is not defined"
#endif

#ifndef KUMO_HAVE_MEMORY_SANITIZER
#error "KUMO_HAVE_MEMORY_SANITIZER is not defined"
#endif

#ifndef KUMO_HAVE_LEAK_SANITIZER
#error "KUMO_HAVE_LEAK_SANITIZER is not defined"
#endif

#ifndef KUMO_HAVE_UNDEFINED_SANITIZER
#error "KUMO_HAVE_UNDEFINED_SANITIZER is not defined"
#endif

#ifndef KUMO_HAVE_DATAFLOW_SANITIZER
#error "KUMO_HAVE_DATAFLOW_SANITIZER is not defined"
#endif

// ---------------------------------------------------------------------------
// pragma.h
// ---------------------------------------------------------------------------

#ifndef KUMO_PRAGMA_DIAG_PUSH
#error "KUMO_PRAGMA_DIAG_PUSH is not defined"
#endif

#ifndef KUMO_PRAGMA_DIAG_POP
#error "KUMO_PRAGMA_DIAG_POP is not defined"
#endif

#ifndef KUMO_PRAGMA_DIAG_IGNORED
#error "KUMO_PRAGMA_DIAG_IGNORED is not defined"
#endif

#ifndef KUMO_DISABLE_DEPRECATED_WARNINGS
#error "KUMO_DISABLE_DEPRECATED_WARNINGS is not defined"
#endif

#ifndef KUMO_RESTORE_DEPRECATED_WARNINGS
#error "KUMO_RESTORE_DEPRECATED_WARNINGS is not defined"
#endif

#ifndef KUMO_DISABLE_UNUSED_WARNING
#error "KUMO_DISABLE_UNUSED_WARNING is not defined"
#endif

#ifndef KUMO_RESTORE_UNUSED_WARNING
#error "KUMO_RESTORE_UNUSED_WARNING is not defined"
#endif

#ifndef KUMO_DISABLE_UNDESIRED_WARNINGS
#error "KUMO_DISABLE_UNDESIRED_WARNINGS is not defined"
#endif

#ifndef KUMO_RESTORE_UNDESIRED_WARNINGS
#error "KUMO_RESTORE_UNDESIRED_WARNINGS is not defined"
#endif

// ---------------------------------------------------------------------------
// assert.h
// ---------------------------------------------------------------------------

#ifndef KUMO_DASSERT
#error "KUMO_DASSERT is not defined"
#endif


#ifndef KUMO_ASSERT
#error "KUMO_ASSERT is not defined"
#endif

// ---------------------------------------------------------------------------
// cache_line.h
// ---------------------------------------------------------------------------

#ifndef KUMO_CACHELINE_ALIGNED
#error "KUMO_CACHELINE_ALIGNED is not defined"
#endif

// ---------------------------------------------------------------------------
// signal.h
// ---------------------------------------------------------------------------

#ifndef KUMO_HAVE_SIGACTION
#error "KUMO_HAVE_SIGACTION is not defined"
#endif

// ---------------------------------------------------------------------------
// testing.h
// ---------------------------------------------------------------------------

#ifndef FRIEND_TEST
#error "FRIEND_TEST is not defined"
#endif

// ---------------------------------------------------------------------------
// raw_log.h
// ---------------------------------------------------------------------------

#ifndef KUMO_RAW_LOG
#error "KUMO_RAW_LOG is not defined"
#endif

#ifndef KUMO_RAW_CHECK
#error "KUMO_RAW_CHECK is not defined"
#endif

#ifndef KUMO_RAW_DLOG
#error "KUMO_RAW_DLOG is not defined"
#endif

#ifndef KUMO_RAW_LOG_SET_OUTPUT
#error "KUMO_RAW_LOG_SET_OUTPUT is not defined"
#endif
