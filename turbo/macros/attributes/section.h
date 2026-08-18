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


// KUMO_HAVE_ATTRIBUTE_SECTION
//
// Indicates whether labeled sections are supported. Weak symbol support is
// a prerequisite. Labeled sections are not supported on Darwin/iOS.
#ifdef KUMO_HAVE_ATTRIBUTE_SECTION
#error KUMO_HAVE_ATTRIBUTE_SECTION cannot be directly set
#elif (KUMO_HAVE_ATTRIBUTE(section) ||                \
       (defined(__GNUC__))) && \
    !defined(__APPLE__) && KUMO_HAVE_ATTRIBUTE_WEAK
#define KUMO_HAVE_ATTRIBUTE_SECTION 1

// KUMO_ATTRIBUTE_SECTION
//
// Tells the compiler/linker to put a given function into a section and define
// `__start_ ## name` and `__stop_ ## name` symbols to bracket the section.
// This functionality is supported by GNU linker.  Any function annotated with
// `KUMO_ATTRIBUTE_SECTION` must not be inlined, or it will be placed into
// whatever section its caller is placed into.
//
#ifndef KUMO_ATTRIBUTE_SECTION
#define KUMO_ATTRIBUTE_SECTION(name) \
  __attribute__((section(#name))) __attribute__((noinline))
#endif

// KUMO_ATTRIBUTE_SECTION_VARIABLE
//
// Tells the compiler/linker to put a given variable into a section and define
// `__start_ ## name` and `__stop_ ## name` symbols to bracket the section.
// This functionality is supported by GNU linker.
#ifndef KUMO_ATTRIBUTE_SECTION_VARIABLE
#ifdef _AIX
// __attribute__((section(#name))) on AIX is achieved by using the `.csect`
// pseudo op which includes an additional integer as part of its syntax
// indicating alignment. If data fall under different alignments then you might
// get a compilation error indicating a `Section type conflict`.
#define KUMO_ATTRIBUTE_SECTION_VARIABLE(name)
#else
#define KUMO_ATTRIBUTE_SECTION_VARIABLE(name) __attribute__((section(#name)))
#endif
#endif

// KUMO_DECLARE_ATTRIBUTE_SECTION_VARS
//
// A weak section declaration to be used as a global declaration
// for KUMO_ATTRIBUTE_SECTION_START|STOP(name) to compile and link
// even without functions with KUMO_ATTRIBUTE_SECTION(name).
// KUMO_DEFINE_ATTRIBUTE_SECTION should be in the exactly one file; it's
// a no-op on ELF but not on Mach-O.
//
#ifndef KUMO_DECLARE_ATTRIBUTE_SECTION_VARS
#define KUMO_DECLARE_ATTRIBUTE_SECTION_VARS(name)   \
  extern char __start_##name[] KUMO_ATTRIBUTE_WEAK; \
  extern char __stop_##name[] KUMO_ATTRIBUTE_WEAK
#endif
#ifndef KUMO_DEFINE_ATTRIBUTE_SECTION_VARS
#define KUMO_INIT_ATTRIBUTE_SECTION_VARS(name)
#define KUMO_DEFINE_ATTRIBUTE_SECTION_VARS(name)
#endif

// KUMO_ATTRIBUTE_SECTION_START
//
// Returns `void*` pointers to start/end of a section of code with
// functions having KUMO_ATTRIBUTE_SECTION(name).
// Returns 0 if no such functions exist.
// One must KUMO_DECLARE_ATTRIBUTE_SECTION_VARS(name) for this to compile and
// link.
//
#define KUMO_ATTRIBUTE_SECTION_START(name) \
  (reinterpret_cast<void *>(__start_##name))
#define KUMO_ATTRIBUTE_SECTION_STOP(name) \
  (reinterpret_cast<void *>(__stop_##name))

#else  // !KUMO_HAVE_ATTRIBUTE_SECTION

#define KUMO_HAVE_ATTRIBUTE_SECTION 0

// provide dummy definitions
#define KUMO_ATTRIBUTE_SECTION(name)
#define KUMO_ATTRIBUTE_SECTION_VARIABLE(name)
#define KUMO_INIT_ATTRIBUTE_SECTION_VARS(name)
#define KUMO_DEFINE_ATTRIBUTE_SECTION_VARS(name)
#define KUMO_DECLARE_ATTRIBUTE_SECTION_VARS(name)
#define KUMO_ATTRIBUTE_SECTION_START(name) (reinterpret_cast<void *>(0))
#define KUMO_ATTRIBUTE_SECTION_STOP(name) (reinterpret_cast<void *>(0))

#endif  // KUMO_ATTRIBUTE_SECTION
