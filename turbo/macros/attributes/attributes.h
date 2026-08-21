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
// File: attributes.h
// -----------------------------------------------------------------------------
//
// Umbrella header for portable compiler attributes. Including this file
// pulls in function, variable, type, sanitizer, section, and related macros.

#pragma once

#include <turbo/macros/attributes/args.h>
#include <turbo/macros/attributes/function.h>
#include <turbo/macros/attributes/variable.h>
#include <turbo/macros/attributes/struct.h>
#include <turbo/macros/attributes/sanitizer.h>
#include <turbo/macros/attributes/control.h>
#include <turbo/macros/attributes/deprecated.h>
#include <turbo/macros/attributes/section.h>


// ---------------------------------------------------------------------------
// attributes.h
// ---------------------------------------------------------------------------

#ifndef KUMO_PRINTF_ATTRIBUTE
#error "KUMO_PRINTF_ATTRIBUTE is not defined"
#endif

#ifndef KUMO_SCANF_ATTRIBUTE
#error "KUMO_SCANF_ATTRIBUTE is not defined"
#endif

#ifndef KUMO_ATTRIBUTE_ALWAYS_INLINE
#error "KUMO_ATTRIBUTE_ALWAYS_INLINE is not defined"
#endif

#ifndef KUMO_ATTRIBUTE_NOINLINE
#error "KUMO_ATTRIBUTE_NOINLINE is not defined"
#endif

#ifndef KUMO_ATTRIBUTE_NORETURN
#error "KUMO_ATTRIBUTE_NORETURN is not defined"
#endif

#ifndef KUMO_ATTRIBUTE_NO_TAIL_CALL
#error "KUMO_ATTRIBUTE_NO_TAIL_CALL is not defined"
#endif

#ifndef KUMO_ATTRIBUTE_WEAK
#error "KUMO_ATTRIBUTE_WEAK is not defined"
#endif

#ifndef KUMO_ATTRIBUTE_NONNULL
#error "KUMO_ATTRIBUTE_NONNULL is not defined"
#endif

#ifndef KUMO_ATTRIBUTE_RETURNS_NONNULL
#error "KUMO_ATTRIBUTE_RETURNS_NONNULL is not defined"
#endif

#ifndef KUMO_ATTRIBUTE_HOT
#error "KUMO_ATTRIBUTE_HOT is not defined"
#endif

#ifndef KUMO_ATTRIBUTE_COLD
#error "KUMO_ATTRIBUTE_COLD is not defined"
#endif

#ifndef KUMO_MUST_USE_RESULT
#error "KUMO_MUST_USE_RESULT is not defined"
#endif

#ifndef KUMO_ATTRIBUTE_PURE_FUNCTION
#error "KUMO_ATTRIBUTE_PURE_FUNCTION is not defined"
#endif

#ifndef KUMO_ATTRIBUTE_CONST_FUNCTION
#error "KUMO_ATTRIBUTE_CONST_FUNCTION is not defined"
#endif

#ifndef KUMO_FALLTHROUGH_INTENDED
#error "KUMO_FALLTHROUGH_INTENDED is not defined"
#endif

#ifndef KUMO_DEPRECATED
#error "KUMO_DEPRECATED is not defined"
#endif

#ifndef KUMO_CONST_INIT
#error "KUMO_CONST_INIT is not defined"
#endif

#ifndef KUMO_ATTRIBUTE_LIFETIME_BOUND
#error "KUMO_ATTRIBUTE_LIFETIME_BOUND is not defined"
#endif

#ifndef KUMO_ATTRIBUTE_UNUSED
#error "KUMO_ATTRIBUTE_UNUSED is not defined"
#endif

#ifndef KUMO_ATTRIBUTE_INITIAL_EXEC
#error "KUMO_ATTRIBUTE_INITIAL_EXEC is not defined"
#endif

#ifndef KUMO_ATTRIBUTE_PACKED
#error "KUMO_ATTRIBUTE_PACKED is not defined"
#endif

#ifndef KUMO_ATTRIBUTE_FUNC_ALIGN
#error "KUMO_ATTRIBUTE_FUNC_ALIGN is not defined"
#endif

#ifndef KUMO_ATTRIBUTE_NO_UNIQUE_ADDRESS
#error "KUMO_ATTRIBUTE_NO_UNIQUE_ADDRESS is not defined"
#endif

#ifndef KUMO_ATTRIBUTE_UNINITIALIZED
#error "KUMO_ATTRIBUTE_UNINITIALIZED is not defined"
#endif

#ifndef KUMO_ATTRIBUTE_WARN_UNUSED
#error "KUMO_ATTRIBUTE_WARN_UNUSED is not defined"
#endif

#ifndef KUMO_ATTRIBUTE_VIEW
#error "KUMO_ATTRIBUTE_VIEW is not defined"
#endif

#ifndef KUMO_ATTRIBUTE_OWNER
#error "KUMO_ATTRIBUTE_OWNER is not defined"
#endif

#ifndef KUMO_HAVE_ATTRIBUTE_SECTION
#error "KUMO_HAVE_ATTRIBUTE_SECTION is not defined"
#endif

#ifndef KUMO_ATTRIBUTE_SECTION
#error "KUMO_ATTRIBUTE_SECTION is not defined"
#endif

#ifndef KUMO_ATTRIBUTE_SECTION_VARIABLE
#error "KUMO_ATTRIBUTE_SECTION_VARIABLE is not defined"
#endif

#ifndef KUMO_DECLARE_ATTRIBUTE_SECTION_VARS
#error "KUMO_DECLARE_ATTRIBUTE_SECTION_VARS is not defined"
#endif

#ifndef KUMO_DEFINE_ATTRIBUTE_SECTION_VARS
#error "KUMO_DEFINE_ATTRIBUTE_SECTION_VARS is not defined"
#endif

#ifndef KUMO_INIT_ATTRIBUTE_SECTION_VARS
#error "KUMO_INIT_ATTRIBUTE_SECTION_VARS is not defined"
#endif

#ifndef KUMO_ATTRIBUTE_SECTION_START
#error "KUMO_ATTRIBUTE_SECTION_START is not defined"
#endif

#ifndef KUMO_ATTRIBUTE_SECTION_STOP
#error "KUMO_ATTRIBUTE_SECTION_STOP is not defined"
#endif

#ifndef KUMO_ATTRIBUTE_STACK_ALIGN_FOR_OLD_LIBC
#error "KUMO_ATTRIBUTE_STACK_ALIGN_FOR_OLD_LIBC is not defined"
#endif

#ifndef KUMO_REQUIRE_STACK_ALIGN_TRAMPOLINE
#error "KUMO_REQUIRE_STACK_ALIGN_TRAMPOLINE is not defined"
#endif

#ifndef KUMO_XRAY_ALWAYS_INSTRUMENT
#error "KUMO_XRAY_ALWAYS_INSTRUMENT is not defined"
#endif

#ifndef KUMO_XRAY_NEVER_INSTRUMENT
#error "KUMO_XRAY_NEVER_INSTRUMENT is not defined"
#endif

#ifndef KUMO_XRAY_LOG_ARGS
#error "KUMO_XRAY_LOG_ARGS is not defined"
#endif



