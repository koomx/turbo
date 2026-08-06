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

#include <turbo/macros/option.h>
#include <turbo/macros/have/have.h>


// KUMO_USES_STD_ORDERING
//
// Indicates whether turbo::{partial,weak,strong}_ordering are aliases for the
// std:: ordering types.
#if !defined(KUMO_OPTION_USE_STD_ORDERING)
#error options.h is misconfigured.
#elif KUMO_OPTION_USE_STD_ORDERING == 0 || \
    (KUMO_OPTION_USE_STD_ORDERING == 2 && !KUMO_HAVE_STD_ORDERING)
#define KUMO_USES_STD_ORDERING 0
#elif KUMO_OPTION_USE_STD_ORDERING == 1 || \
    (KUMO_OPTION_USE_STD_ORDERING == 2 && KUMO_HAVE_STD_ORDERING)
#define KUMO_USES_STD_ORDERING 1
#else
#error options.h is misconfigured.
#endif


// KUMO_USES_STD_SOURCE_LOCATION
//
// Indicates whether turbo::SourceLocation is an alias for std::source_location.
#if !defined(KUMO_OPTION_USE_STD_SOURCE_LOCATION)
#error options.h is misconfigured.
#elif KUMO_OPTION_USE_STD_SOURCE_LOCATION == 0 || \
    (KUMO_OPTION_USE_STD_SOURCE_LOCATION == 2 &&  \
     !KUMO_HAVE_STD_SOURCE_LOCATION)
#define KUMO_USES_STD_SOURCE_LOCATION 0
#elif KUMO_OPTION_USE_STD_SOURCE_LOCATION == 1 || \
    (KUMO_OPTION_USE_STD_SOURCE_LOCATION == 2 &&  \
     KUMO_HAVE_STD_SOURCE_LOCATION)
#define KUMO_USES_STD_SOURCE_LOCATION 1
#else
#error options.h is misconfigured.
#endif