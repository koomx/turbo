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
// File: macros.h
// -----------------------------------------------------------------------------
//
// Umbrella header for the xmacros macros layer (attributes, optimization,
// pragma, visibility, etc.).

#pragma once


// KUMO_CPLUSPLUS_LANG
//
// MSVC does not set the value of __cplusplus correctly, but instead uses
// _MSVC_LANG as a stand-in.
// https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros
//
// However, there are reports that MSVC even sets _MSVC_LANG incorrectly at
// times, for example:
// https://github.com/microsoft/vscode-cpptools/issues/1770
// https://reviews.llvm.org/D70996
//
// For this reason, this symbol is considered INTERNAL and code outside of
// library must not use it.
#if defined(_MSVC_LANG)
#define KUMO_CPLUSPLUS_LANG _MSVC_LANG
#elif defined(__cplusplus)
#define KUMO_CPLUSPLUS_LANG __cplusplus
#else
#define KUMO_CPLUSPLUS_LANG 0
#endif



#if KUMO_CPLUSPLUS_LANG >= 202602L
#define KUMO_CPLUSPLUS_STD26 1
#else
#define KUMO_CPLUSPLUS_STD26 0
#endif



#if KUMO_CPLUSPLUS_LANG >= 202302L
#define KUMO_CPLUSPLUS_STD23 1
#else
#define KUMO_CPLUSPLUS_STD23 0
#endif


#if KUMO_CPLUSPLUS_LANG >= 202002L
#define KUMO_CPLUSPLUS_STD20 1
#else
#define KUMO_CPLUSPLUS_STD20 0
#endif

#if KUMO_CPLUSPLUS_LANG >= 201703L
#define KUMO_CPLUSPLUS_STD17 1
#else
#define KUMO_CPLUSPLUS_STD17 0
#endif

#if KUMO_CPLUSPLUS_LANG >= 201402L
#define KUMO_CPLUSPLUS_STD14 1
#else
#define KUMO_CPLUSPLUS_STD14 0
#endif

#if KUMO_CPLUSPLUS_LANG >= 201103L
#define KUMO_CPLUSPLUS_STD11 1
#else
#define KUMO_CPLUSPLUS_STD11 0
#endif
