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
// File: signal.h
// -----------------------------------------------------------------------------
//
// Signal-related feature detection.

#pragma once

// ---------------------------------------------------------------------------
// KUMO_HAVE_SIGACTION
//
// 1 when the POSIX sigaction() function is available.
// ---------------------------------------------------------------------------

#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || \
    defined(__OpenBSD__) || defined(__NetBSD__) || defined(__sun) || \
    defined(__QNXNTO__)
#include <signal.h>
#define KUMO_HAVE_SIGACTION  1
#else
#define KUMO_HAVE_SIGACTION  0
#endif
