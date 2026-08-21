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
// AI: This file is a human/AI-readable summary of the project's public API and
// AI: design principles. AI agents should read this instead of scanning all
// AI: source files. For dependency libraries, read their skills.h similarly.
//
// AI: Style: triple-slash Doxygen comments for all entries.

#pragma once

/// @defgroup project_summary Project Summary
/// @brief turbo.macros - portable C/C++ macro utilities
///
/// @brief Purpose
/// AI: turbo.macros provides portable, well-tested macros for platform/compiler/arch
/// AI: detection, common utilities, attributes, and compile-time feature checks.
/// AI: All macros use the KUMO_ prefix and are defined as value macros (no bare
/// AI: #define NAME), so they work directly in #if conditions.
///
/// @brief Design principles
/// AI: - Header-only library, no build required
/// AI: - Zero dependencies beyond standard C/C++ headers
/// AI: - Works with GCC, Clang, MSVC, Intel ICC/ICX
/// AI: - Supports C99/C11/C17 and C++11/14/17/20
/// AI: - Umbrella headers must compile as C; C++-only APIs use #ifdef __cplusplus
/// AI: - Prefer #if KUMO_FOO (0|1 values); do not rely on #ifdef for feature macros
/// AI: - #pragma once only (no #ifndef guards)
/// AI: - Each detection layer has individual files + umbrella header
/// AI: - Platform = OS, Arch = CPU, Compiler = toolchain, libc = runtime
/// AI: - Subdir README.md = API level; lower levels must not include higher or peer dirs
/// AI: - Current focus: C compatibility (see plan.md); fill missing C APIs later
///
/// @brief Directory layout
/// AI: .
/// AI: ├── turbo/macros/
/// AI: │   ├── macros.h / plan.md / skills.h
/// AI: │   ├── platform/          # OS + endian (KUMO_OS_*, KUMO_ENDIAN_*)
/// AI: │   │   ├── platform.h / endian.h
/// AI: │   │   ├── linux.h android.h fuchsia.h ros.h
/// AI: │   │   ├── osx.h ios.h tvos.h watchos.h visionos.h
/// AI: │   │   ├── bsd.h windows.h cygwin.h web.h
/// AI: │   │   ├── nacl.h solaris.h qnx.h haiku.h aix.h
/// AI: │   ├── arch/              # CPU + SIMD (compile-target; not runtime CPUID)
/// AI: │   │   ├── arch.h
/// AI: │   │   ├── x86.h x64.h arm32.h arm64.h
/// AI: │   │   ├── riscv.h loongarch.h ppc.h s390.h mips.h e2k.h wasm.h
/// AI: │   ├── compiler/ libc/ gpu/ macros/
/// AI: └── CMakeLists.txt
///
/// @brief Naming rules
/// AI: OS identity = KUMO_OS_* ; form factor = KUMO_PLATFORM_{DESKTOP,MOBILE}
/// AI: Endian = KUMO_ENDIAN_{LITTLE,BIG} (in platform/endian.h, not guessed from OS)
/// AI: CPU family = KUMO_ARCH_X86/ARM/... ; variant = KUMO_ARCH_X86_64/ARM64/...
/// AI: SIMD = KUMO_SIMD_* (always 0|1). Prefer #if KUMO_FOO over #ifdef.
/// AI: KUMO_SIMD_* = compile flags / predefined macros. Runtime detect = cpu_detect.
/// AI: kmcmake KMCMAKE_ARCH_ENABLE_* sets build flags; KUMO_SIMD_* observes the result.
///
/// @brief Recommended replacements for raw predefined macros
/// AI: __linux__           -> KUMO_OS_LINUX   (also 1 on Android)
/// AI: __ANDROID__         -> KUMO_OS_ANDROID
/// AI: _WIN32              -> KUMO_OS_WINDOWS
/// AI: __APPLE__           -> KUMO_OS_MACOSX / IOS / TVOS / WATCHOS / VISIONOS
/// AI: __Fuchsia__         -> KUMO_OS_FUCHSIA
/// AI: __x86_64__/_M_X64   -> KUMO_ARCH_X86_64
/// AI: __aarch64__/_M_ARM64-> KUMO_ARCH_ARM64 (alias KUMO_ARCH_AARCH64)
/// AI: __powerpc64__       -> KUMO_ARCH_PPC / PPC64 / PPC64LE
/// @}

/// @defgroup platform Platform Detection Macros
/// @{
/// @brief KUMO_OS_LINUX / MACOSX / WINDOWS / ANDROID — primary OS (0|1)
/// @brief KUMO_OS_IOS / TVOS / WATCHOS / VISIONOS — Apple targets
/// @brief KUMO_OS_BSD / FREEBSD / OPENBSD / NETBSD / DRAGONFLY
/// @brief KUMO_OS_NACL / NACL_SFI / NACL_NONSFI
/// @brief KUMO_OS_SOLARIS / QNX / WEB / FUCHSIA / ROS / CYGWIN / HAIKU / AIX
/// @brief KUMO_OS_POSIX / UNIX / MICROSOFT — OS groups
/// @brief KUMO_PLATFORM_DESKTOP / MOBILE — form factor
/// @brief KUMO_PLATFORM_POSIX_API / POSIX_SOCKETS — API availability (0|1)
/// @brief KUMO_PLATFORM_JS — alias of KUMO_OS_WEB (Emscripten/Wasm)
/// @brief KUMO_ENDIAN_LITTLE / KUMO_ENDIAN_BIG — from __BYTE_ORDER__ or _WIN32
/// @brief KUMO_PTR_SIZE — 4 or 8
/// @brief KUMO_PLATFORM_NAME — string literal ("Linux", "Windows", ...)
/// @}

/// @defgroup arch Architecture Detection Macros
/// @{
/// @brief KUMO_ARCH_X86 / ARM / RISCV / LOONGARCH / PPC / S390 / MIPS / E2K / WASM — family
/// @brief KUMO_ARCH_X86_64 / X86_32 / ARM64 / ARM32 / ARM64EC / AARCH64(=ARM64)
/// @brief KUMO_ARCH_RISCV64 / RISCV32 / LOONGARCH64 / LOONGARCH32
/// @brief KUMO_ARCH_PPC64 / PPC32 / PPC64LE / S390X / S390_31 / MIPS64 / MIPS32
/// @brief KUMO_ARCH_WASM64 / WASM32
/// @brief KUMO_ARCH_32_BIT / KUMO_ARCH_64_BIT
/// @brief KUMO_SIMD_SSE / SSE2 / SSE3 / SSSE3 / SSE4_1 / SSE4_2
/// @brief KUMO_SIMD_AVX / AVX2 / AVX512F / AVX512BW / AVX512VL / AVX512DQ
/// @brief KUMO_SIMD_AVX512IFMA / AVX512CD / AVX512VBMI / AVX512VBMI2
/// @brief KUMO_SIMD_AVX512VNNI / AVX512BITALG / AVX512VPOPCNTDQ
/// @brief KUMO_SIMD_FMA / BMI1 / BMI2 / POPCNT / LZCNT
/// @brief KUMO_SIMD_NEON / SVE / SVE2 / AES / PCLMUL / RVV / LSX / LASX
/// @brief KUMO_SIMD_ALTIVEC / VSX / CRYPTO — PowerPC
/// @brief KUMO_HAVE_ACCELERATED_AES — AES || (x86_64 && AVX); randen compat
/// @brief KUMO_CACHELINE_SIZE / KUMO_SIMD_LEVEL / KUMO_ARCH_NAME
/// @}

/// @defgroup compiler Compiler Detection Macros
/// @{
/// @brief KUMO_COMPILER_GNUC     — 1 if GCC
/// @brief KUMO_COMPILER_CLANG    — 1 if Clang (or AppleClang)
/// @brief KUMO_COMPILER_MSVC     — 1 if MSVC (cl, not clang-cl)
/// @brief KUMO_COMPILER_MSVC_CLANG — 1 if clang-cl
/// @brief KUMO_COMPILER_MSVC_ENV — 1 if MSVC || MSVC_CLANG
/// @brief KUMO_COMPILER_INTEL    — 1 if Intel Classic ICC
/// @brief KUMO_COMPILER_ICC      — alias for KUMO_COMPILER_INTEL
/// @brief KUMO_COMPILER_GCC      — alias for KUMO_COMPILER_GNUC
/// @brief KUMO_COMPILER_MINGW    — 1 if MinGW
/// @brief KUMO_COMPILER_EMSCRIPTEN — 1 if Emscripten
/// @brief KUMO_COMPILER_COSMOPOLLITAN — 1 if Cosmopolitan Libc
/// @brief KUMO_COMPILER_VERSION  — compiler version as integer (e.g. 1200 for 12.0)
/// @brief KUMO_COMPILER_VERSION_MAJOR — major version
/// @brief KUMO_COMPILER_VERSION_MINOR — minor version
/// @brief KUMO_COMPILER_VERSION_PATCH — patch version
/// @brief KUMO_HAVE_ATTRIBUTE(x) — 1 if compiler supports __attribute__((x))
/// @brief KUMO_HAVE_CPP_ATTRIBUTE(x) — 1 if C++ [[x]] attribute is supported
/// @brief KUMO_HAVE_BUILTIN(x) — 1 if compiler has __builtin_x
/// @}

/// @defgroup libc C Runtime Library Detection
/// @{
/// @brief KUMO_LIBC_GLIBC   — 1 if glibc
/// @brief KUMO_LIBC_MUSL    — 1 if musl libc
/// @brief KUMO_LIBC_BIONIC  — 1 if Bionic (Android)
/// @brief KUMO_LIBC_IS_GLIBC — alias for KUMO_LIBC_GLIBC
/// @brief KUMO_LIBC_VERSION — glibc version as integer (e.g. 23100 for 2.31)
/// @brief KUMO_LIBC_VERSION_MAJOR — glibc major
/// @brief KUMO_LIBC_VERSION_MINOR — glibc minor
/// @brief KUMO_LIBC_VERSION_PATCH — glibc patch (usually 0)
/// @brief KUMO_GLIBC_PREREQ(maj, min) — 1 if glibc >= specified version
/// @}

/// @defgroup visibility Visibility Macros
/// @{
/// @brief KUMO_EXPORT — __declspec(dllexport) on Windows, __attribute__((visibility("default"))) elsewhere
/// @brief KUMO_IMPORT — __declspec(dllimport) on Windows, empty elsewhere
/// @brief KUMO_LOCAL  — __attribute__((visibility("hidden"))) on GCC/Clang, empty on MSVC
/// @}

/// @defgroup optimization Optimization Hints
/// @{
/// @brief KUMO_LIKELY(x)   — __builtin_expect(x, 1) on GCC/Clang, (x) elsewhere
/// @brief KUMO_UNLIKELY(x) — __builtin_expect(x, 0) on GCC/Clang, (x) elsewhere
/// @brief KUMO_UNREACHABLE — __builtin_unreachable() on GCC/Clang, __assume(false) on MSVC
/// @brief KUMO_ASSUME(cond) — same as KUMO_ASSUME: debug assert; else __builtin_assume / __assume / unreachable
/// @brief KUMO_BLOCK_TAIL_CALL_OPTIMIZATION() — asm volatile("" ::: "memory") on GCC/Clang, empty elsewhere
/// @}

/// @defgroup pretty_function Pretty Function Macros
/// @{
/// @brief KUMO_PRETTY_FUNCTION — full function signature (const char*)
/// @brief KUMO_FUNC            — short function name (const char*)
/// @brief KUMO_FILE            — source file path (const char*)
/// @brief KUMO_LINE            — line number (int)
/// @}

/// @defgroup basic Basic Utility Macros
/// @{
/// @brief KUMO_CONCAT(a, b)    — token pasting: a ## b
/// @brief KUMO_STRINGIFY(x)    — stringification: #x
/// @brief KUMO_COUNTER         — unique counter (__COUNTER__ or __LINE__ fallback)
/// @brief KUMO_UNUSED(x)       — suppress unused variable warnings
/// @brief KUMO_RESTRICT        — restrict keyword (C) or __restrict (C++)
/// @brief KUMO_DISABLE_UBSAN   — __attribute__((no_sanitize("undefined")))
/// @brief KUMO_ARRAYSIZE(arr)  — compile-time array element count
/// @brief KUMO_CONTAINER_OF(ptr, type, member) — get container struct from member pointer
/// @brief KUMO_DELETEFUNCTION(class) — delete copy constructor/assignment
/// @brief KUMO_DISALLOW_COPY_AND_ASSIGN(class) — private copy, no definition
/// @brief KUMO_DEFAULT_COPY_AND_ASSIGN(class) — = default for copy ops
/// @brief KUMO_DISALLOW_MOVE_AND_ASSIGN(class) — private move, no definition
/// @brief KUMO_DEFAULT_MOVE_AND_ASSIGN(class) — = default for move ops
/// @brief KUMO_GLOBAL_INIT(func) — static initialization (GCC/Clang: __attribute__((constructor)), MSVC: #pragma section)
/// @brief KUMO_MANUALLY_ALIGNED_STRUCT(alignment) — begin packed + aligned struct
/// @brief KUMO_STRUCT_END(name, size) — end struct + static_assert size
/// @}

/// @defgroup have Feature Detection
/// @{
/// @brief KUMO_HAVE_EXCEPTIONS            — 1 if C++ exceptions enabled
/// @brief KUMO_HAVE_RTTI                  — 1 if RTTI enabled
/// @brief KUMO_HAVE_THREAD_LOCAL          — 1 if thread_local supported
/// @brief KUMO_HAVE_TLS                   — 1 if __thread supported
/// @brief KUMO_HAVE_INTRINSIC_INT128      — 1 if __int128 available
/// @brief KUMO_HAVE_CONSTANT_EVALUATED    — 1 if consteval/constexpr if available
/// @brief KUMO_HAVE_MMAP                  — 1 if mmap() available
/// @brief KUMO_HAVE_UNISTD_H              — 1 if <unistd.h> available
/// @brief KUMO_HAVE_FEATURE(x)            — __has_feature(x) on Clang, 0 elsewhere
/// @}

/// @defgroup have_ext Extended Feature Detection (C++ only in C mode)
/// @{
/// @brief KUMO_HAVE_STD_IS_TRIVIALLY_DESTRUCTIBLE — 1 if std::is_trivially_destructible
/// @brief KUMO_HAVE_STD_IS_TRIVIALLY_CONSTRUCTIBLE — 1 if std::is_trivially_constructible
/// @brief KUMO_HAVE_STD_IS_TRIVIALLY_ASSIGNABLE    — 1 if std::is_trivially_assignable
/// @brief KUMO_HAVE_STD_IS_TRIVIALLY_COPYABLE      — 1 if std::is_trivially_copyable
/// @brief KUMO_HAVE_STD_ORDERING                   — 1 if std::strong_ordering available
/// @brief KUMO_HAVE_CLASS_TEMPLATE_ARGUMENT_DEDUCTION — 1 if CTAD available
/// @brief KUMO_HAVE_MIN_GNUC_VERSION(maj, min)     — 1 if GCC >= specified version
/// @brief KUMO_HAVE_MIN_CLANG_VERSION(maj, min)    — 1 if Clang >= specified version
/// @brief KUMO_HAVE_PTHREAD_GETSCHEDPARAM          — 1 if pthread_getschedparam available
/// @brief KUMO_HAVE_SCHED_GETCPU                   — 1 if sched_getcpu available
/// @brief KUMO_HAVE_SCHED_YIELD                    — 1 if sched_yield available
/// @brief KUMO_HAVE_SEMAPHORE_H                    — 1 if <semaphore.h> available
/// @brief KUMO_HAVE_ALARM                          — 1 if alarm() available
/// @}

/// @defgroup sanitizer Sanitizer Detection
/// @{
/// @brief KUMO_HAVE_ADDRESS_SANITIZER   — 1 if ASAN active
/// @brief KUMO_HAVE_HWADDRESS_SANITIZER — 1 if HWASAN active
/// @brief KUMO_HAVE_THREAD_SANITIZER    — 1 if TSAN active
/// @brief KUMO_HAVE_MEMORY_SANITIZER    — 1 if MSAN active
/// @brief KUMO_HAVE_LEAK_SANITIZER      — 1 if LSAN active
/// @brief KUMO_HAVE_UNDEFINED_SANITIZER — 1 if UBSAN active
/// @brief KUMO_HAVE_DATAFLOW_SANITIZER  — 1 if DFSAN active
/// @brief KUMO_NO_SANITIZE_ADDRESS      — __attribute__((no_sanitize("address")))
/// @brief KUMO_NO_SANITIZE_THREAD       — __attribute__((no_sanitize("thread")))
/// @brief KUMO_NO_SANITIZE_MEMORY       — __attribute__((no_sanitize("memory")))
/// @brief KUMO_NO_SANITIZE_HWADDRESS    — __attribute__((no_sanitize("hwaddress")))
/// @brief KUMO_NO_SANITIZE_LEAK         — __attribute__((no_sanitize("leak")))
/// @brief KUMO_NO_SANITIZE_UNDEFINED    — __attribute__((no_sanitize("undefined")))
/// @brief KUMO_NO_SANITIZE_DATAFLOW     — __attribute__((no_sanitize("dataflow")))
/// @}

/// @defgroup assert Assert Macros
/// @brief Always-on asserts (compile with -DNDEBUG to disable KUMO_DASSERT only)
/// @{
/// @brief KUMO_ASSERT(expr)             — assert with expression + file/line
/// @brief KUMO_ASSERT_MSG(expr, msg)    — assert with message
/// @brief KUMO_ASSERT_EQ(a, b)          — assert a == b
/// @brief KUMO_ASSERT_NE(a, b)          — assert a != b
/// @brief KUMO_ASSERT_LT(a, b)          — assert a < b
/// @brief KUMO_ASSERT_LE(a, b)          — assert a <= b
/// @brief KUMO_ASSERT_GT(a, b)          — assert a > b
/// @brief KUMO_ASSERT_GE(a, b)          — assert a >= b
/// @brief KUMO_ASSERT_NULL(ptr)         — assert ptr == NULL
/// @brief KUMO_ASSERT_NOT_NULL(ptr)     — assert ptr != NULL
/// @brief KUMO_DASSERT(expr)            — debug-only assert (disabled by NDEBUG)
/// @brief KUMO_DASSERT_MSG(expr, msg)   — debug-only with message
/// @brief KUMO_DASSERT_EQ(a, b)         — debug-only a == b
/// @brief KUMO_DASSERT_NE(a, b)         — debug-only a != b
/// @brief KUMO_DASSERT_LT(a, b)         — debug-only a < b
/// @brief KUMO_DASSERT_LE(a, b)         — debug-only a <= b
/// @brief KUMO_DASSERT_GT(a, b)         — debug-only a > b
/// @brief KUMO_DASSERT_GE(a, b)         — debug-only a >= b
/// @brief KUMO_DASSERT_NULL(ptr)        — debug-only ptr == NULL
/// @brief KUMO_DASSERT_NOT_NULL(ptr)    — debug-only ptr != NULL
/// @}

/// @defgroup attributes Compiler Attributes
/// @{
/// @brief KUMO_ATTRIBUTE_FORCE_INLINE     — always inline
/// @brief KUMO_ATTRIBUTE_NOINLINE         — never inline
/// @brief KUMO_ATTRIBUTE_NORETURN         — function never returns
/// @brief KUMO_ATTRIBUTE_NO_TAIL_CALL     — inhibit tail call optimization
/// @brief KUMO_ATTRIBUTE_WEAK             — weak symbol
/// @brief KUMO_ATTRIBUTE_NONNULL(...)     — arguments must not be NULL
/// @brief KUMO_ATTRIBUTE_RETURNS_NONNULL  — return value never NULL
/// @brief KUMO_ATTRIBUTE_RETURNS_NOALIAS  — return value aliasing
/// @brief KUMO_ATTRIBUTE_HOT              — hot path (optimize aggressively)
/// @brief KUMO_ATTRIBUTE_COLD             — cold path (optimize for size)
/// @brief KUMO_MUST_USE_RESULT            — must use return value
/// @brief KUMO_ATTRIBUTE_PURE_FUNCTION    — no side effects, depends only on args
/// @brief KUMO_ATTRIBUTE_CONST_FUNCTION   — no side effects, depends on nothing
/// @brief KUMO_FALLTHROUGH_INTENDED       — intentional switch fallthrough
/// @brief KUMO_DEPRECATED(msg)            — mark as deprecated
/// @brief KUMO_CONST_INIT                 — constant initialization guarantee
/// @brief KUMO_ATTRIBUTE_PACKED           — struct packed (no padding)
/// @brief KUMO_ATTRIBUTE_FUNC_ALIGN(n)    — function aligned to n bytes
/// @brief KUMO_ATTRIBUTE_LIFETIME_BOUND   — C++ reference lifetime extension
/// @brief KUMO_ATTRIBUTE_NO_UNIQUE_ADDRESS — C++ empty base optimization
/// @brief KUMO_ATTRIBUTE_UNINITIALIZED    — suppress uninitialized warnings
/// @brief KUMO_ATTRIBUTE_WARN_UNUSED      — warn if unused
/// @brief KUMO_ATTRIBUTE_USED             — keep in object file
/// @brief KUMO_ATTRIBUTE_UNUSED            — suppress unused warnings
/// @brief KUMO_ATTRIBUTE_INITIAL_EXEC     — ELF TLS initial-exec model
/// @brief KUMO_GSL_OWNER                  — GSL owner pointer annotation
/// @brief KUMO_GSL_POINTER                — GSL non-owning pointer annotation
/// @brief KUMO_ATTRIBUTE_NO_SANITIZE_*    — disable specific sanitizer (ASAN/TSAN/MSAN/etc)
/// @brief KUMO_XRAY_ALWAYS_INSTRUMENT     — XRay always instrument
/// @brief KUMO_XRAY_NEVER_INSTRUMENT      — XRay never instrument
/// @brief KUMO_XRAY_LOG_ARGS(n)           — XRay log n arguments
/// @brief KUMO_ATTRIBUTE_SECTION(name)    — place in named section
/// @brief KUMO_ATTRIBUTE_SECTION_VARIABLE(name) — variable in named section
/// @brief KUMO_DECLARE_ATTRIBUTE_SECTION_VARS(name) — declare section vars
/// @brief KUMO_DEFINE_ATTRIBUTE_SECTION_VARS(name)  — define section vars
/// @brief KUMO_INIT_ATTRIBUTE_SECTION_VARS(name)    — init section vars
/// @brief KUMO_ATTRIBUTE_SECTION_START(name) — section start pointer
/// @brief KUMO_ATTRIBUTE_SECTION_STOP(name)  — section end pointer
/// @brief KUMO_ATTRIBUTE_STACK_ALIGN_FOR_OLD_LIBC — stack alignment workaround
/// @brief KUMO_REQUIRE_STACK_ALIGN_TRAMPOLINE — require trampoline alignment
/// @}

/// @defgroup pragma Pragma Wrappers
/// @{
/// @brief KUMO_TARGET_REGION(T) — compiler target-ISA region (Clang attribute / GCC target); empty on MSVC
/// @brief KUMO_UNTARGET_REGION  — end target-ISA region
/// @brief KUMO_PRAGMA_DIAG_PUSH           — save diagnostic state
/// @brief KUMO_PRAGMA_DIAG_POP            — restore diagnostic state
/// @brief KUMO_PRAGMA_DIAG_IGNORED(str)   — suppress warning by string
/// @brief KUMO_DISABLE_DEPRECATED_WARNINGS — push + ignore deprecation warnings
/// @brief KUMO_RESTORE_DEPRECATED_WARNINGS — pop diagnostics
/// @brief KUMO_DISABLE_UNUSED_WARNING      — push + ignore unused-function/const
/// @brief KUMO_RESTORE_UNUSED_WARNING      — pop unused-warning diagnostics
/// @brief KUMO_DISABLE_UNDESIRED_WARNINGS  — push + ignore IntelliSense/clang-cl noise
/// @brief KUMO_RESTORE_UNDESIRED_WARNINGS  — pop undesired-warning diagnostics
/// @brief KUMO_PRAGMA_GCC诊断_PUSH  — GCC-specific push
/// @brief KUMO_PRAGMA_GCC诊断_POP   — GCC-specific pop
/// @brief KUMO_PRAGMA_GCC诊断_IGNORED(x) — GCC-specific ignore
/// @brief KUMO_PRAGMA_GCC忽略废弃 — GCC deprecated ignore
/// @brief KUMO_PRAGMA_CLANG诊断_PUSH  — Clang-specific push
/// @brief KUMO_PRAGMA_CLANG诊断_POP   — Clang-specific pop
/// @brief KUMO_PRAGMA_CLANG诊断_IGNORED(x) — Clang-specific ignore
/// @brief KUMO_PRAGMA_CLANG循环提示_UNROLL   — Clang loop unroll hint
/// @brief KUMO_PRAGMA_CLANG循环提示_向量化   — Clang loop vectorize hint
/// @brief KUMO_PRAGMA_CLANG循环提示_INTERLEAVE — Clang loop interleave hint
/// @brief KUMO_PRAGMA_MSVC警告_PUSH    — MSVC warning push
/// @brief KUMO_PRAGMA_MSVC警告_POP     — MSVC warning pop
/// @brief KUMO_PRAGMA_MSVC警告禁用(x) — MSVC warning disable
/// @brief KUMO_PRAGMA_MSVC区域开始     — MSVC region begin
/// @brief KUMO_PRAGMA_MSVC区域结束     — MSVC region end
/// @brief KUMO_PRAGMA_MSVC注释消息     — MSVC #pragma message
/// @}
