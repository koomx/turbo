// Copyright (c) 2017-2026, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

// IWYU pragma: private, include "CLI/CLI.hpp"

// [CLI11:public_includes:set]
#include <string>
#include <vector>
// [CLI11:public_includes:end]

#include <turbo/cli/macros.h>

namespace xcli {
// [CLI11:argv_hpp:verbatim]
namespace detail {
#ifdef _WIN32
/// Decode and return UTF-8 argv from GetCommandLineW.
XCLI_INLINE std::vector<std::string> compute_win32_argv();
#endif

XCLI_INLINE auto& parsed_args_storage() {
    static std::pair<int, std::vector<std::string>> storage;
    return storage;
}
}  // namespace detail

XCLI_INLINE void save_parsed_args(int argc, const char *const *argv) {
    auto &storage = detail::parsed_args_storage();
    storage.first = argc;
    storage.second.assign(argv, argv + argc);
}

XCLI_INLINE int argc() noexcept { return detail::parsed_args_storage().first; }

XCLI_INLINE const std::vector<std::string> &argv() noexcept {
    return detail::parsed_args_storage().second;
}
// [CLI11:argv_hpp:end]
}  // namespace xcli

#ifndef XCLI_COMPILE
#include <turbo/cli/impl/argv_inl.h>  // IWYU pragma: export
#endif
