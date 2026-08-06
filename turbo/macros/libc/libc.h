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
// File: libc.h
// -----------------------------------------------------------------------------
//
// C runtime library detection umbrella header.
//
// Unified macro set:
//
//   // libc identity (exactly one is 1)
//   KUMO_LIBC_GLIBC         0|1
//   KUMO_LIBC_MUSL          0|1
//   KUMO_LIBC_BIONIC        0|1
//
//   // Properties
//   KUMO_LIBC_VERSION       integer   (glibc: 234 = 2.34, musl/bionic: 0)
//   KUMO_LIBC_NAME          string    "glibc" | "musl" | "bionic"

#pragma once

#include <turbo/macros/libc/glibc.h>
#include <turbo/macros/libc/musl.h>
#include <turbo/macros/libc/bionic.h>

// ---------------------------------------------------------------------------
// Completeness check
// ---------------------------------------------------------------------------

#ifndef KUMO_LIBC_GLIBC
#define KUMO_LIBC_GLIBC 0
#endif

#ifndef KUMO_LIBC_MUSL
#define KUMO_LIBC_MUSL 0
#endif

#ifndef KUMO_LIBC_BIONIC
#define KUMO_LIBC_BIONIC 0
#endif

#ifndef KUMO_LIBC_VERSION
#define KUMO_LIBC_VERSION 0
#endif

#ifndef KUMO_LIBC_NAME
#define KUMO_LIBC_NAME    ""
#endif
