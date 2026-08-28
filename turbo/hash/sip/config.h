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

#pragma once


#ifdef DEBUG_SIPHASH
#include <stdio.h>
#endif


/// default: SipHash-2-4
#ifndef cROUNDS
#define cROUNDS 2
#endif
#ifndef dROUNDS
#define dROUNDS 4
#endif


#ifdef DEBUG_SIPHASH
#define TRACE                                           \
    do {                                                \
        printf("(%3zu) v0 %08" PRIx32 "\n", inlen, v0); \
        printf("(%3zu) v1 %08" PRIx32 "\n", inlen, v1); \
        printf("(%3zu) v2 %08" PRIx32 "\n", inlen, v2); \
        printf("(%3zu) v3 %08" PRIx32 "\n", inlen, v3); \
    } while (0)
#else
#define TRACE
#endif

