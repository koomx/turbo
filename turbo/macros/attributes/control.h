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
// File: control.h
// -----------------------------------------------------------------------------
//
// Control-flow attributes (intentional switch fallthrough).

#pragma once

#include <turbo/macros/compiler/compiler.h>


// KUMO_FALLTHROUGH_INTENDED
//
// Annotates implicit fall-through between switch labels, allowing a case to
// indicate intentional fallthrough and turn off warnings about any lack of a
// `break` statement. The KUMO_FALLTHROUGH_INTENDED macro should be followed by
// a semicolon and can be used in most places where `break` can, provided that
// no statements exist between it and the next switch label.
//
// Example:
//
//  switch (x) {
//    case 40:
//    case 41:
//      if (truth_is_out_there) {
//        ++x;
//        KUMO_FALLTHROUGH_INTENDED;  // Use instead of/along with annotations
//                                    // in comments
//      } else {
//        return x;
//      }
//    case 42:
//      ...
//
// Notes: When supported, GCC and Clang can issue a warning on switch labels
// with unannotated fallthrough using the warning `-Wimplicit-fallthrough`. See
// clang documentation on language extensions for details:
// https://clang.llvm.org/docs/AttributeReference.html#fallthrough-clang-fallthrough
//
// When used with unsupported compilers, the KUMO_FALLTHROUGH_INTENDED macro has
// no effect on diagnostics. In any case this macro has no effect on runtime
// behavior and performance of code.

#ifdef KUMO_FALLTHROUGH_INTENDED
#error "KUMO_FALLTHROUGH_INTENDED should not be defined."
#elif KUMO_HAVE_CPP_ATTRIBUTE(fallthrough)
#define KUMO_FALLTHROUGH_INTENDED [[fallthrough]]
#elif KUMO_HAVE_CPP_ATTRIBUTE(clang::fallthrough)
#define KUMO_FALLTHROUGH_INTENDED [[clang::fallthrough]]
#elif KUMO_HAVE_CPP_ATTRIBUTE(gnu::fallthrough)
#define KUMO_FALLTHROUGH_INTENDED [[gnu::fallthrough]]
#else
#define KUMO_FALLTHROUGH_INTENDED \
  do {                            \
  } while (0)
#endif

