// Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// ...
// -----------------------------------------------------------------------------
// File: pragma_msvc.h
// -----------------------------------------------------------------------------
//
// MSVC pragma macros.  Uses the `warning(...)` _Pragma API.

#pragma once

#include <turbo/macros/utility/basic.h>

// ---------------------------------------------------------------------------
// Warning control
// ---------------------------------------------------------------------------

#define KUMO_PRAGMA_DIAG_PUSH               _Pragma("warning(push)")
#define KUMO_PRAGMA_DIAG_POP                _Pragma("warning(pop)")
#define KUMO_PRAGMA_DIAG_IGNORED(flag)      _Pragma(KUMO_STRINGIFY(warning(disable: flag)))

#define KUMO_DISABLE_DEPRECATED_WARNINGS    \
    KUMO_PRAGMA_DIAG_PUSH                   \
    KUMO_PRAGMA_DIAG_IGNORED(4996)

#define KUMO_RESTORE_DEPRECATED_WARNINGS    \
    KUMO_PRAGMA_DIAG_POP

#define KUMO_DISABLE_UNUSED_WARNING         \
    KUMO_PRAGMA_DIAG_PUSH                   \
    KUMO_PRAGMA_DIAG_IGNORED(4505)          \
    KUMO_PRAGMA_DIAG_IGNORED(4514)          \
    KUMO_PRAGMA_DIAG_IGNORED(4189)          \
    KUMO_PRAGMA_DIAG_IGNORED(4101)

#define KUMO_RESTORE_UNUSED_WARNING         \
    KUMO_PRAGMA_DIAG_POP

#ifdef __has_include
#if __has_include(<CppCoreCheck\Warnings.h>)
#include <CppCoreCheck\Warnings.h>
#endif
#endif

#ifdef ALL_CPPCORECHECK_WARNINGS
#define KUMO_DISABLE_UNDESIRED_WARNINGS     \
    KUMO_PRAGMA_DIAG_PUSH                   \
    __pragma(warning(disable : ALL_CPPCORECHECK_WARNINGS))
#define KUMO_RESTORE_UNDESIRED_WARNINGS     \
    KUMO_PRAGMA_DIAG_POP
#else
#define KUMO_DISABLE_UNDESIRED_WARNINGS
#define KUMO_RESTORE_UNDESIRED_WARNINGS
#endif



// ---------------------------------------------------------------------------
// Code region collapsing (Visual Studio editor)
// ---------------------------------------------------------------------------

#define KUMO_PRAGMA_REGION_START(name)      _Pragma(KUMO_STRINGIFY(region name))
#define KUMO_PRAGMA_REGION_END              _Pragma("endregion")

// ---------------------------------------------------------------------------
// Linker / build hints
// ---------------------------------------------------------------------------

#define KUMO_PRAGMA_COMMENT(lib)            _Pragma(KUMO_STRINGIFY(comment(lib, lib)))
#define KUMO_PRAGMA_MESSAGE(msg)            _Pragma(KUMO_STRINGIFY(message(msg)))

// ---------------------------------------------------------------------------
// Extended MSVC warning / optimization control
// ---------------------------------------------------------------------------

#define KUMO_MSVC_SUPPRESS_WARNING(n)       __pragma(warning(suppress:n))
#define KUMO_MSVC_PUSH_DISABLE_WARNING(n)   __pragma(warning(push)) __pragma(warning(disable:n))
#define KUMO_MSVC_PUSH_WARNING_LEVEL(n)     __pragma(warning(push, n))
#define KUMO_MSVC_POP_WARNING()             __pragma(warning(pop))
#define KUMO_MSVC_DISABLE_OPTIMIZE()        __pragma(optimize("", off))
#define KUMO_MSVC_ENABLE_OPTIMIZE()         __pragma(optimize("", on))
#define KUMO_NON_EXPORTED_BASE(code)        __pragma(warning(suppress:4275)) code

#define KUMO_STATIC_CONST_MEMBER_DEFINITION __declspec(selectany)

#if defined(__has_feature)
#if __has_feature(memory_sanitizer)
#include <sanitizer/msan_interface.h>
#define KUMO_MSAN_UNPOISON(p, s)            __msan_unpoison(p, s)
#else
#define KUMO_MSAN_UNPOISON(p, s)
#endif
#else
#define KUMO_MSAN_UNPOISON(p, s)
#endif

#define KUMO_CDECL                          __cdecl

#if !defined(KUMO_ALLOW_UNUSED)
#define KUMO_ALLOW_UNUSED                   __pragma(warning(suppress:4100))
#endif

#define KUMO_TARGET_REGION(T)
#define KUMO_UNTARGET_REGION

