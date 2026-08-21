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
// File: struct.h
// -----------------------------------------------------------------------------
//
// Type attributes for view/owner lifetime diagnostics and related annotations.

#pragma once

// KUMO_ATTRIBUTE_VIEW indicates that a type is solely a "view" of data that it
// points to, similarly to a span, std::string_view, or other non-owning reference
// type.
// This enables diagnosing certain lifetime issues similar to those enabled by
// KUMO_ATTRIBUTE_LIFETIME_BOUND, such as:
//
//   struct KUMO_ATTRIBUTE_VIEW StringView {
//     template<class R>
//     StringView(const R&);
//   };
//
//   StringView f(std::string s) {
//     return s;  // warning: address of stack memory returned
//   }
//
// We disable this on Clang versions < 13 because of the following
// false-positive:
//
//   std::string_view f(std::optional<std::string_view> sv) { return *sv; }
//
// See the following links for details:
// https://reviews.llvm.org/D64448
// https://lists.llvm.org/pipermail/cfe-dev/2018-November/060355.html
#if KUMO_HAVE_CPP_ATTRIBUTE(gsl::Pointer) && \
    (!defined(__clang_major__) || __clang_major__ >= 13)
#define KUMO_ATTRIBUTE_VIEW [[gsl::Pointer]]
#else
#define KUMO_ATTRIBUTE_VIEW
#endif


// KUMO_ATTRIBUTE_PACKED
//
// Instructs the compiler not to use natural alignment for a tagged data
// structure, but instead to reduce its alignment to 1.
//
// Use of this attribute is HIGHLY DISCOURAGED. Taking the address of or
// binding a reference to any unaligned member is UB, and it is very easy to
// do so unintentionally when passing such members as function arguments.
//
// DO NOT APPLY THIS ATTRIBUTE TO STRUCTS CONTAINING ATOMICS. Doing
// so can cause atomic variables to be mis-aligned and silently violate
// atomicity on x86.
//
// This attribute can either be applied to members of a structure or to a
// structure in its entirety. Applying this attribute (judiciously) to a
// structure in its entirety to optimize the memory footprint of very
// commonly-used structs is fine. Do not apply this attribute to a structure in
// its entirety if the purpose is to control the offsets of the members in the
// structure. Instead, apply this attribute only to structure members that need
// it.
//
// When applying KUMO_ATTRIBUTE_PACKED only to specific structure members the
// natural alignment of structure members not annotated is preserved. Aligned
// member accesses are faster than non-aligned member accesses even if the
// targeted microprocessor supports non-aligned accesses.
#if KUMO_HAVE_ATTRIBUTE(packed) || (defined(__GNUC__))
#define KUMO_ATTRIBUTE_PACKED __attribute__((__packed__))
#else
#define KUMO_ATTRIBUTE_PACKED
#endif


// KUMO_ATTRIBUTE_OWNER indicates that a type is a container, smart pointer, or
// similar class that owns all the data that it points to.
// This enables diagnosing certain lifetime issues similar to those enabled by
// KUMO_ATTRIBUTE_LIFETIME_BOUND, such as:
//
//   struct KUMO_ATTRIBUTE_VIEW StringView {
//     template<class R>
//     StringView(const R&);
//   };
//
//   struct KUMO_ATTRIBUTE_OWNER String {};
//
//   StringView f(String s) {
//     return s;  // warning: address of stack memory returned
//   }
//
// We disable this on Clang versions < 13 because of the following
// false-positive:
//
//   std::string_view f(std::optional<std::string_view> sv) { return *sv; }
//
// See the following links for details:
// https://reviews.llvm.org/D64448
// https://lists.llvm.org/pipermail/cfe-dev/2018-November/060355.html
#if KUMO_HAVE_CPP_ATTRIBUTE(gsl::Owner) && \
    (!defined(__clang_major__) || __clang_major__ >= 13)
#define KUMO_ATTRIBUTE_OWNER [[gsl::Owner]]
#else
#define KUMO_ATTRIBUTE_OWNER
#endif



// KUMO_ATTRIBUTE_TRIVIAL_ABI
// Indicates that a type is "trivially relocatable" -- meaning it can be
// relocated without invoking the constructor/destructor, using a form of move
// elision.
//
// From a memory safety point of view, putting aside destructor ordering, it's
// safe to apply KUMO_ATTRIBUTE_TRIVIAL_ABI if an object's location
// can change over the course of its lifetime: if a constructor can be run one
// place, and then the object magically teleports to another place where some
// methods are run, and then the object teleports to yet another place where it
// is destroyed. This is notably not true for self-referential types, where the
// move-constructor must keep the self-reference up to date. If the type changed
// location without invoking the move constructor, it would have a dangling
// self-reference.
//
// The use of this teleporting machinery means that the number of paired
// move/destroy operations can change, and so it is a bad idea to apply this to
// a type meant to count the number of moves.
//
// Warning: applying this can, rarely, break callers. Objects passed by value
// will be destroyed at the end of the call, instead of the end of the
// full-expression containing the call. In addition, it changes the ABI
// of functions accepting this type by value (e.g. to pass in registers).
//
// See also the upstream documentation:
// https://clang.llvm.org/docs/AttributeReference.html#trivial-abi
//
// b/321691395 - This is currently disabled in open-source builds since
// compiler support differs. If system libraries compiled with GCC are mixed
// with libraries compiled with Clang, types will have different ideas about
// their ABI, leading to hard to debug crashes.
#define KUMO_ATTRIBUTE_TRIVIAL_ABI


// KUMO_ATTRIBUTE_WARN_UNUSED
//
// Compilers routinely warn about trivial variables that are unused.  For
// non-trivial types, this warning is suppressed since the
// constructor/destructor may be intentional and load-bearing, for example, with
// a RAII scoped lock.
//
// For example:
//
// class KUMO_ATTRIBUTE_WARN_UNUSED MyType {
//  public:
//   MyType();
//   ~MyType();
// };
//
// void foo() {
//   // Warns with KUMO_ATTRIBUTE_WARN_UNUSED attribute present.
//   MyType unused;
// }
//
// See https://clang.llvm.org/docs/AttributeReference.html#warn-unused and
// https://gcc.gnu.org/onlinedocs/gcc/C_002b_002b-Attributes.html#index-warn_005funused-type-attribute
#if KUMO_HAVE_CPP_ATTRIBUTE(gnu::warn_unused)
#define KUMO_ATTRIBUTE_WARN_UNUSED [[gnu::warn_unused]]
#else
#define KUMO_ATTRIBUTE_WARN_UNUSED
#endif

