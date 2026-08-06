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
// File: testing.h
// -----------------------------------------------------------------------------
//
// Test-related convenience macros.

#pragma once

#include <turbo/macros/compiler/compiler.h>

// ---------------------------------------------------------------------------
// FRIEND_TEST
//
// Allows a Google Test / Google Mock test fixture to access private members
// of a class.
//
//   class MyClass {
//    private:
//     FRIEND_TEST(MyClassTest, FooWorks);
//     int secret_;
//   };
// ---------------------------------------------------------------------------

#define FRIEND_TEST(test_case_name, test_name) \
  friend class test_case_name##_##test_name##_Test
