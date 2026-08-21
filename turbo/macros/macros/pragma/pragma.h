// Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// ...
// -----------------------------------------------------------------------------
// File: pragma.h
// -----------------------------------------------------------------------------
//
// Facade header — routes to the correct compiler-specific pragma file.

#pragma once

#if defined(__clang__)

#include <turbo/macros/macros/pragma/pragma_clang.h>

#elif defined(__GNUC__)

#include <turbo/macros/macros/pragma/pragma_gcc.h>

#elif defined(_MSC_VER)

#include <turbo/macros/macros/pragma/pragma_msvc.h>

#else

// Fallback stubs for unknown compilers
#include <turbo/macros/macros/basic.h>

#define KUMO_PRAGMA_DIAG_PUSH
#define KUMO_PRAGMA_DIAG_POP
#define KUMO_PRAGMA_DIAG_IGNORED(flag)
#define KUMO_DISABLE_DEPRECATED_WARNINGS
#define KUMO_RESTORE_DEPRECATED_WARNINGS
#define KUMO_DISABLE_UNUSED_WARNING
#define KUMO_RESTORE_UNUSED_WARNING
#define KUMO_DISABLE_UNDESIRED_WARNINGS
#define KUMO_RESTORE_UNDESIRED_WARNINGS

#define KUMO_TARGET_REGION(T)
#define KUMO_UNTARGET_REGION

#endif

// ---------------------------------------------------------------------------
// Completeness check
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

#ifndef KUMO_TARGET_REGION
#error "KUMO_TARGET_REGION is not defined"
#endif

#ifndef KUMO_UNTARGET_REGION
#error "KUMO_UNTARGET_REGION is not defined"
#endif
