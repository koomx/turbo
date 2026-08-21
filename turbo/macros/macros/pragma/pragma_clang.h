// Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// ...
// -----------------------------------------------------------------------------
// File: pragma_clang.h
// -----------------------------------------------------------------------------
//
// Clang pragma macros.  Includes both GCC-compatible diagnostic pragmas and
// Clang-specific diagnostic / loop hint pragmas.

#pragma once

#include <turbo/macros/utility/basic.h>

// ---------------------------------------------------------------------------
// GCC-compatible diagnostic control (also supported by Clang)
// ---------------------------------------------------------------------------

#define KUMO_PRAGMA_DIAG_PUSH               _Pragma("GCC diagnostic push")
#define KUMO_PRAGMA_DIAG_POP                _Pragma("GCC diagnostic pop")
#define KUMO_PRAGMA_DIAG_IGNORED(flag)      _Pragma(KUMO_STRINGIFY(GCC diagnostic ignored flag))

#define KUMO_DISABLE_DEPRECATED_WARNINGS    \
    KUMO_PRAGMA_DIAG_PUSH                   \
    KUMO_PRAGMA_DIAG_IGNORED("-Wdeprecated-declarations")

#define KUMO_RESTORE_DEPRECATED_WARNINGS    \
    KUMO_PRAGMA_DIAG_POP

#define KUMO_DISABLE_UNUSED_WARNING         \
    KUMO_PRAGMA_DIAG_PUSH                   \
    KUMO_PRAGMA_DIAG_IGNORED("-Wunused-function") \
    KUMO_PRAGMA_DIAG_IGNORED("-Wunused-const-variable")

#define KUMO_RESTORE_UNUSED_WARNING         \
    KUMO_PRAGMA_DIAG_POP

#if defined(_MSC_VER)
#define KUMO_DISABLE_UNDESIRED_WARNINGS     \
    KUMO_PRAGMA_DIAG_PUSH                   \
    KUMO_PRAGMA_DIAG_IGNORED("-Wmicrosoft-include")
#define KUMO_RESTORE_UNDESIRED_WARNINGS     \
    KUMO_PRAGMA_DIAG_POP
#else
#define KUMO_DISABLE_UNDESIRED_WARNINGS
#define KUMO_RESTORE_UNDESIRED_WARNINGS
#endif


// ---------------------------------------------------------------------------
// Clang-specific diagnostic control
// ---------------------------------------------------------------------------

#define KUMO_PRAGMA_CLANG_DIAG_PUSH          _Pragma("clang diagnostic push")
#define KUMO_PRAGMA_CLANG_DIAG_POP           _Pragma("clang diagnostic pop")
#define KUMO_PRAGMA_CLANG_DIAG_IGNORED(flag) _Pragma(KUMO_STRINGIFY(clang diagnostic ignored flag))

// ---------------------------------------------------------------------------
// Clang loop optimization hints
//
// Place before a for / while / do loop:
//
//   KUMO_PRAGMA_CLANG_LOOP_UNROLL_COUNT(4)
//   for (int i = 0; i < n; ++i) { ... }
// ---------------------------------------------------------------------------

#define KUMO_PRAGMA_CLANG_LOOP_UNROLL             _Pragma("clang loop unroll(enable)")
#define KUMO_PRAGMA_CLANG_LOOP_UNROLL_COUNT(N)     _Pragma(KUMO_STRINGIFY(clang loop unroll_count(N)))
#define KUMO_PRAGMA_CLANG_LOOP_VECTORIZE           _Pragma("clang loop vectorize(enable)")
#define KUMO_PRAGMA_CLANG_LOOP_INTERLEAVE          _Pragma("clang loop interleave(enable)")

// ---------------------------------------------------------------------------
// Function-level ISA target region (must be outside a namespace on Clang < 8)
//
//   KUMO_TARGET_REGION("avx2,bmi,lzcnt,popcnt")
//   ... functions ...
//   KUMO_UNTARGET_REGION
// ---------------------------------------------------------------------------

#define KUMO_TARGET_REGION(T) \
    _Pragma(KUMO_STRINGIFY(clang attribute push(__attribute__((target(T))), apply_to = function)))
#define KUMO_UNTARGET_REGION _Pragma("clang attribute pop")

