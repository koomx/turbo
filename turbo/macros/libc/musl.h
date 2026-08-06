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

#if defined(__linux__)
#include <features.h>
#endif

#if defined(__musl__)

#define KUMO_LIBC_GLIBC     0
#define KUMO_LIBC_MUSL      1
#define KUMO_LIBC_BIONIC    0

#define KUMO_LIBC_VERSION   0
#define KUMO_LIBC_NAME      "musl"

#endif
