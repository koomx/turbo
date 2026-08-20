// Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// ...
// -----------------------------------------------------------------------------
// File: pragma_gcc.h
// -----------------------------------------------------------------------------
//
// GCC diagnostic pragma macros.  Uses the `GCC diagnostic` _Pragma API.
// Also compatible with Clang (which implements the same API for GCC compat).

#pragma once

#include <turbo/macros/utility/basic.h>

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

#define KUMO_DISABLE_UNDESIRED_WARNINGS

#define KUMO_RESTORE_UNDESIRED_WARNINGS
