// Copyright (c) 2017-2026, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

// IWYU pragma: private, include "CLI/CLI.hpp"

// [CLI11:public_includes:set]
#include <algorithm>
#include <string>
#include <vector>
// [CLI11:public_includes:end]

#include <turbo/cli/app.h>
#include <turbo/cli/formatter_fwd.h>

namespace xcli {
// [CLI11:formatter_hpp:verbatim]
// [CLI11:formatter_hpp:end]
}  // namespace xcli

#ifndef XCLI_COMPILE
#include <turbo/cli/impl/formatter_inl.h>  // IWYU pragma: export
#endif
