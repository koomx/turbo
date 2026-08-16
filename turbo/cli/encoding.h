// Copyright (c) 2017-2026, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

// IWYU pragma: private, include "CLI/CLI.hpp"
#include <turbo/cli/macros.h>

// [CLI11:public_includes:set]
#include <string>
// [CLI11:public_includes:end]

// [CLI11:encoding_includes:verbatim]
#if defined(XCLI_CPP17) || (defined(XCLI_HAS_FILESYSTEM) && XCLI_HAS_FILESYSTEM > 0)
#include <string_view>
#if defined XCLI_HAS_FILESYSTEM && XCLI_HAS_FILESYSTEM > 0
#include <filesystem>
#endif
#endif

// [CLI11:encoding_includes:end]

namespace xcli {
// [CLI11:encoding_hpp:verbatim]

/// Convert a wide string to a narrow string.
XCLI_INLINE std::string narrow(const std::wstring &str);
XCLI_INLINE std::string narrow(const wchar_t *str);
XCLI_INLINE std::string narrow(const wchar_t *str, std::size_t size);

/// Convert a narrow string to a wide string.
XCLI_INLINE std::wstring widen(const std::string &str);
XCLI_INLINE std::wstring widen(const char *str);
XCLI_INLINE std::wstring widen(const char *str, std::size_t size);

#ifdef XCLI_CPP17
XCLI_INLINE std::string narrow(std::wstring_view str);
XCLI_INLINE std::wstring widen(std::string_view str);
#endif  // XCLI_CPP17

#if defined XCLI_HAS_FILESYSTEM && XCLI_HAS_FILESYSTEM > 0
/// Convert a char-string to a native path correctly.
XCLI_INLINE std::filesystem::path to_path(std::string_view str);
#endif  // XCLI_HAS_FILESYSTEM

// [CLI11:encoding_hpp:end]
}  // namespace xcli

#ifndef XCLI_COMPILE
#include <turbo/cli/impl/encoding_inl.h>  // IWYU pragma: export
#endif
