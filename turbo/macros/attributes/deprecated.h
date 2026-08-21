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

#include <turbo/macros/compiler/compiler.h>


// KUMO_DEPRECATED()
//
// Marks a deprecated class, struct, enum, function, method and variable
// declarations. The macro argument is used as a custom diagnostic message (e.g.
// suggestion of a better alternative).
//
// For code or headers that are assured to only build with C++14 and up, prefer
// just using the standard `[[deprecated("message")]]` directly over this macro.
//
// Examples:
//
//   class KUMO_DEPRECATED("Use Bar instead") Foo {...};
//
//   KUMO_DEPRECATED("Use Baz() instead") void Bar() {...}
//
//   template <typename T>
//   KUMO_DEPRECATED("Use DoThat() instead")
//   void DoThis();
//
//   enum FooEnum {
//     kBar KUMO_DEPRECATED("Use kBaz instead"),
//   };
//
// Every usage of a deprecated entity will trigger a warning when compiled with
// GCC/Clang's `-Wdeprecated-declarations` option. Google's production toolchain
// turns this warning off by default, instead relying on clang-tidy to report
// new uses of deprecated code.
#if KUMO_HAVE_ATTRIBUTE(deprecated)
#define KUMO_DEPRECATED(message) __attribute__((deprecated(message)))
#else
#define KUMO_DEPRECATED(message)
#endif

