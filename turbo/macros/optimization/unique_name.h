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



// KUMO_UNIQUE_SMALL_NAME(cond)
// This macro forces small unique name on a static file level symbols like
// static local variables or static functions. This is intended to be used in
// macro definitions to optimize the cost of generated code. Do NOT use it on
// symbols exported from translation unit since it may cause a link time
// conflict.
//
// Example:
//
// #define MY_MACRO(txt)
// namespace {
//  char VeryVeryLongVarName[] KUMO_UNIQUE_SMALL_NAME() = txt;
//  const char* VeryVeryLongFuncName() KUMO_UNIQUE_SMALL_NAME();
//  const char* VeryVeryLongFuncName() { return txt; }
// }
//

#if defined(__GNUC__)
#define KUMO_INTERNAL_UNIQUE_SMALL_NAME2(x) #x
#define KUMO_INTERNAL_UNIQUE_SMALL_NAME1(x) KUMO_INTERNAL_UNIQUE_SMALL_NAME2(x)
#define KUMO_UNIQUE_SMALL_NAME() \
  asm(KUMO_INTERNAL_UNIQUE_SMALL_NAME1(.kumo.__COUNTER__))
#else
#define KUMO_UNIQUE_SMALL_NAME()
#endif
