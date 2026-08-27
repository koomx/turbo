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

#if defined(__clang__)

#define KUMO_COMPILER_CLANG     1

#if defined(__apple_build_version__)
#define KUMO_COMPILER_APPLECLANG 1
#else
#define KUMO_COMPILER_APPLECLANG 0
#endif

#define KUMO_COMPILER_VERSION       (__clang_major__ * 100 + __clang_minor__)
#define KUMO_COMPILER_VERSION_MAJOR __clang_major__
#define KUMO_COMPILER_VERSION_MINOR __clang_minor__


#if defined(__apple_build_version__)
#define KUMO_COMPILER_NAME      "AppleClang"
#else
#define KUMO_COMPILER_NAME      "Clang"
#endif
#else
#define KUMO_COMPILER_CLANG     0
#define KUMO_COMPILER_APPLECLANG 0
#endif
