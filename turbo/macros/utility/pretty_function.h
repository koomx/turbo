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

// KUMO_PRETTY_FUNCTION — full function signature including return type and params
#if defined(_MSC_VER)
#define KUMO_PRETTY_FUNCTION __FUNCSIG__
#else
#define KUMO_PRETTY_FUNCTION __PRETTY_FUNCTION__
#endif

// KUMO_FUNC — plain function name (C99/C++11 __func__)
#define KUMO_FUNC __func__

// KUMO_FILE — source file path
#define KUMO_FILE __FILE__

// KUMO_LINE — source line number (integer)
#define KUMO_LINE __LINE__
