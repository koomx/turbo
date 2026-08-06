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

#pragma once

#if defined(__linux__) || defined(__GLIBC__)
#include <features.h>
#endif

#if defined(__GLIBC__) && !defined(__UCLIBC__)

#define KUMO_LIBC_GLIBC     1
#define KUMO_LIBC_MUSL      0
#define KUMO_LIBC_BIONIC    0

#define KUMO_LIBC_VERSION   (__GLIBC__ * 100 + __GLIBC_MINOR__)
#define KUMO_LIBC_NAME      "glibc"

#endif
