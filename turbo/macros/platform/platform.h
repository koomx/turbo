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
// File: platform.h
// -----------------------------------------------------------------------------
//
// Platform (OS) detection umbrella.
//
// Every matching platform header defines the SAME KUMO_OS_* /
// KUMO_PLATFORM_* set as 0|1 (or string/int properties).  Endianness is
// detected once in endian.h (not guessed from OS).
//
// Unified macro set:
//
//   KUMO_OS_LINUX / MACOSX / WINDOWS / ANDROID
//   KUMO_OS_IOS / TVOS / WATCHOS / VISIONOS
//   KUMO_OS_BSD / FREEBSD / OPENBSD / NETBSD / DRAGONFLY
//   KUMO_OS_NACL / NACL_SFI / NACL_NONSFI
//   KUMO_OS_SOLARIS / QNX / WEB / FUCHSIA / ROS / CYGWIN / HAIKU / AIX
//   KUMO_OS_POSIX / UNIX / MICROSOFT
//   KUMO_PLATFORM_DESKTOP / MOBILE / POSIX_API / POSIX_SOCKETS
//   KUMO_ENDIAN_LITTLE / KUMO_ENDIAN_BIG
//   KUMO_PTR_SIZE / KUMO_PLATFORM_NAME
//   KUMO_PLATFORM_JS  — alias of KUMO_OS_WEB
//
// Prefer these over raw predefined macros, e.g.:
//   __linux__          -> KUMO_OS_LINUX   (also 1 on Android)
//   __ANDROID__        -> KUMO_OS_ANDROID
//   _WIN32             -> KUMO_OS_WINDOWS
//   __APPLE__          -> KUMO_OS_MACOSX / IOS / TVOS / ...

#pragma once

// More-specific variants first.
#include <turbo/macros/platform/fuchsia.h>
#include <turbo/macros/platform/ros.h>
#include <turbo/macros/platform/apple.h>
#include <turbo/macros/platform/bsd.h>
#include <turbo/macros/platform/nacl.h>
#include <turbo/macros/platform/solaris.h>
#include <turbo/macros/platform/qnx.h>
#include <turbo/macros/platform/haiku.h>
#include <turbo/macros/platform/aix.h>
#include <turbo/macros/platform/web.h>
#include <turbo/macros/platform/linux.h>
#include <turbo/macros/platform/windows.h>

#include <turbo/macros/platform/endian.h>

#if KUMO_OS_MACOSX || KUMO_OS_IOS || KUMO_OS_AIX \
    || KUMO_OS_FUCHSIA || KUMO_OS_HAIKU || KUMO_OS_NACL \
    || KUMO_OS_QNX || KUMO_OS_SOLARIS || KUMO_OS_WEB || KUMO_OS_TVOS \
    || KUMO_OS_WATCHOS || KUMO_OS_VISIONOS || KUMO_OS_CYGWIN || KUMO_OS_ROS \
    || KUMO_OS_ANDROID || KUMO_OS_BSD
#define KUMO_OS_POSIX            1
#else
#define KUMO_OS_POSIX            0
#endif

#if KUMO_OS_MACOSX || KUMO_OS_IOS || KUMO_OS_AIX \
    || KUMO_OS_FUCHSIA || KUMO_OS_HAIKU || KUMO_OS_NACL \
    || KUMO_OS_QNX || KUMO_OS_SOLARIS || KUMO_OS_TVOS \
    || KUMO_OS_WATCHOS || KUMO_OS_VISIONOS || KUMO_OS_CYGWIN \
    || KUMO_OS_ROS || KUMO_OS_ANDROID || KUMO_OS_BSD
#define KUMO_OS_UNIX             1
#else
#define KUMO_OS_UNIX             0
#endif

#if KUMO_OS_WINDOWS
#define KUMO_OS_MICROSOFT        1
#else
#define KUMO_OS_MICROSOFT        0
#endif


#if KUMO_OS_IOS || KUMO_OS_TVOS || KUMO_OS_WATCHOS || KUMO_OS_VISIONOS || KUMO_OS_ANDROID
#define KUMO_PLATFORM_MOBILE       1
#else
#define KUMO_PLATFORM_MOBILE       0
#endif

#if KUMO_OS_MACOSX || KUMO_OS_AIX | KUMO_OS_FUCHSIA || KUMO_OS_HAIKU \
    || KUMO_OS_QNX || KUMO_OS_SOLARIS || KUMO_OS_MICROSOFT \
    || KUMO_OS_CYGWIN || KUMO_OS_ROS || KUMO_OS_BSD
#define KUMO_PLATFORM_DESKTOP      1
#else
#define KUMO_PLATFORM_DESKTOP      0
#endif


#if KUMO_OS_MACOSX || KUMO_OS_IOS || KUMO_OS_AIX || KUMO_OS_FUCHSIA \
    || KUMO_OS_HAIKU || KUMO_OS_NACL || KUMO_OS_QNX || KUMO_OS_SOLARIS \
    || KUMO_OS_WEB || KUMO_OS_TVOS || KUMO_OS_WATCHOS || KUMO_OS_VISIONOS \
    || KUMO_OS_CYGWIN || KUMO_OS_ROS || KUMO_OS_ANDROID || KUMO_OS_BSD
#define KUMO_PLATFORM_POSIX_API    1
#else
#define KUMO_PLATFORM_POSIX_API    1
#endif

#if KUMO_OS_MACOSX || KUMO_OS_IOS || KUMO_OS_AIX || KUMO_OS_FUCHSIA \
    || KUMO_OS_HAIKU || KUMO_OS_NACL || KUMO_OS_QNX || KUMO_OS_SOLARIS \
    || KUMO_OS_WEB || KUMO_OS_TVOS || KUMO_OS_WATCHOS || KUMO_OS_VISIONOS \
    || KUMO_OS_CYGWIN || KUMO_OS_ROS || KUMO_OS_ANDROID || KUMO_OS_BSD
#define KUMO_PLATFORM_POSIX_SOCKETS 1
#else
#define KUMO_PLATFORM_POSIX_SOCKETS 0
#endif

// Alias: skills / callers may use JS for the Emscripten/Wasm OS.
#ifndef KUMO_PLATFORM_JS
#define KUMO_PLATFORM_JS KUMO_OS_WEB
#endif

// ---------------------------------------------------------------------------
// Completeness check
// ---------------------------------------------------------------------------

#ifndef KUMO_OS_LINUX
#error "KUMO_OS_LINUX is not defined — no platform header matched this target"
#endif
#ifndef KUMO_OS_APPLE
#error "KUMO_OS_APPLE is not defined — no platform header matched this target"
#endif
#ifndef KUMO_OS_MACOSX
#error "KUMO_OS_MACOSX is not defined — no platform header matched this target"
#endif
#ifndef KUMO_OS_WINDOWS
#error "KUMO_OS_WINDOWS is not defined — no platform header matched this target"
#endif
#ifndef KUMO_OS_ANDROID
#error "KUMO_OS_ANDROID is not defined"
#endif
#ifndef KUMO_OS_IOS
#error "KUMO_OS_IOS is not defined"
#endif
#ifndef KUMO_OS_TVOS
#error "KUMO_OS_TVOS is not defined"
#endif
#ifndef KUMO_OS_WATCHOS
#error "KUMO_OS_WATCHOS is not defined"
#endif
#ifndef KUMO_OS_VISIONOS
#error "KUMO_OS_VISIONOS is not defined"
#endif
#ifndef KUMO_OS_BSD
#error "KUMO_OS_BSD is not defined"
#endif
#ifndef KUMO_OS_FREEBSD
#error "KUMO_OS_FREEBSD is not defined"
#endif
#ifndef KUMO_OS_OPENBSD
#error "KUMO_OS_OPENBSD is not defined"
#endif
#ifndef KUMO_OS_NETBSD
#error "KUMO_OS_NETBSD is not defined"
#endif
#ifndef KUMO_OS_DRAGONFLY
#error "KUMO_OS_DRAGONFLY is not defined"
#endif
#ifndef KUMO_OS_NACL
#error "KUMO_OS_NACL is not defined"
#endif
#ifndef KUMO_OS_NACL_SFI
#error "KUMO_OS_NACL_SFI is not defined"
#endif
#ifndef KUMO_OS_NACL_NONSFI
#error "KUMO_OS_NACL_NONSFI is not defined"
#endif
#ifndef KUMO_OS_SOLARIS
#error "KUMO_OS_SOLARIS is not defined"
#endif
#ifndef KUMO_OS_QNX
#error "KUMO_OS_QNX is not defined"
#endif
#ifndef KUMO_OS_WEB
#error "KUMO_OS_WEB is not defined"
#endif
#ifndef KUMO_OS_FUCHSIA
#error "KUMO_OS_FUCHSIA is not defined"
#endif
#ifndef KUMO_OS_ROS
#error "KUMO_OS_ROS is not defined"
#endif
#ifndef KUMO_OS_CYGWIN
#error "KUMO_OS_CYGWIN is not defined"
#endif
#ifndef KUMO_OS_HAIKU
#error "KUMO_OS_HAIKU is not defined"
#endif
#ifndef KUMO_OS_AIX
#error "KUMO_OS_AIX is not defined"
#endif
#ifndef KUMO_OS_POSIX
#error "KUMO_OS_POSIX is not defined"
#endif
#ifndef KUMO_OS_UNIX
#error "KUMO_OS_UNIX is not defined"
#endif
#ifndef KUMO_OS_MICROSOFT
#error "KUMO_OS_MICROSOFT is not defined"
#endif
#ifndef KUMO_PLATFORM_DESKTOP
#error "KUMO_PLATFORM_DESKTOP is not defined"
#endif
#ifndef KUMO_PLATFORM_MOBILE
#error "KUMO_PLATFORM_MOBILE is not defined"
#endif
#ifndef KUMO_PLATFORM_POSIX_API
#error "KUMO_PLATFORM_POSIX_API is not defined"
#endif
#ifndef KUMO_PLATFORM_POSIX_SOCKETS
#error "KUMO_PLATFORM_POSIX_SOCKETS is not defined"
#endif
#ifndef KUMO_ENDIAN_LITTLE
#error "KUMO_ENDIAN_LITTLE is not defined"
#endif
#ifndef KUMO_ENDIAN_BIG
#error "KUMO_ENDIAN_BIG is not defined"
#endif
#ifndef KUMO_PTR_SIZE
#error "KUMO_PTR_SIZE is not defined"
#endif
#ifndef KUMO_PLATFORM_NAME
#error "KUMO_PLATFORM_NAME is not defined"
#endif
