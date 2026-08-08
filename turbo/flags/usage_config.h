//
//  Copyright 2019 The Abseil Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// -----------------------------------------------------------------------------
// File: usage_config.h
// -----------------------------------------------------------------------------
//
// Configuration for flag reflection display (filename normalization) and
// registry error reporting. Command-line help/usage belongs to xcli, not flags.

#ifndef TURBO_FLAGS_USAGE_CONFIG_H_
#define TURBO_FLAGS_USAGE_CONFIG_H_

#include <functional>
#include <string>

#include <string_view>
#include <turbo/macros/config.h>

namespace turbo {

// FlagsUsageConfig
//
// Callbacks that customize how flag metadata is presented (e.g. Filename()).
struct FlagsUsageConfig {
    // Normalizes the filename specific to the build system/filesystem used.
    // Used when reporting flag definition location via reflection.
    // For example:
    //   normalize_filename("/my_company/some_long_path/src/project/file.cc")
    // might produce
    //   "project/file.cc".
    std::function<std::string(std::string_view)> normalize_filename;
};

// Sets reflection/display configuration callbacks. Missing callbacks fall back
// to library defaults.
void SetFlagsUsageConfig(FlagsUsageConfig usage_config);

namespace flags_internal {

FlagsUsageConfig GetUsageConfig();

void ReportUsageError(std::string_view msg, bool is_fatal);

}  // namespace flags_internal

}  // namespace turbo

extern "C" {

// Additional report of fatal usage error message before we std::exit. Error is
// fatal if is_fatal argument to ReportUsageError is true.
void TURBO_INTERNAL_C_SYMBOL(TurboInternalReportFatalUsageError)(
    std::string_view);

}  // extern "C"

#endif  // TURBO_FLAGS_USAGE_CONFIG_H_
