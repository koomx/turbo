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

// -----------------------------------------------------------------------------
// File: endian.h
// -----------------------------------------------------------------------------
//
// Compile-time endianness.  Do not guess from OS; match turbo/macros/config.h:
// prefer __BYTE_ORDER__, else _WIN32 => little, else #error.

#pragma once

#if defined(KUMO_ENDIAN_LITTLE) || defined(KUMO_ENDIAN_BIG)
#error "KUMO_ENDIAN_LITTLE / KUMO_ENDIAN_BIG cannot be directly set"
#endif

#if (defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && \
     __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#define KUMO_ENDIAN_LITTLE 1
#define KUMO_ENDIAN_BIG    0
#elif defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && \
    __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define KUMO_ENDIAN_LITTLE 0
#define KUMO_ENDIAN_BIG    1
#elif defined(_WIN32)
#define KUMO_ENDIAN_LITTLE 1
#define KUMO_ENDIAN_BIG    0
#else
#error "kumo endian detection needs to be set up for your compiler"
#endif
