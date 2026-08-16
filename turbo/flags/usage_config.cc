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

#include <turbo/flags/usage_config.h>

#include <iostream>
#include <mutex>
#include <string>
#include <string_view>

#include <turbo/base/const_init.h>
#include <turbo/base/thread_annotations.h>
#include <turbo/macros/config.h>

extern "C" {

KUMO_ATTRIBUTE_WEAK void TURBO_INTERNAL_C_SYMBOL(
    TurboInternalReportFatalUsageError)(std::string_view) {}

}  // extern "C"

namespace turbo {

namespace flags_internal {

namespace {

std::string NormalizeFilename(std::string_view filename) {
    auto pos = filename.find_first_not_of("\\/");
    if (pos == std::string_view::npos) {
        return "";
    }
    filename.remove_prefix(pos);
    return std::string(filename);
}

std::mutex& CustomUsageConfigMutex() {
    static std::mutex mutex;
    return mutex;
}

KUMO_CONST_INIT FlagsUsageConfig* custom_usage_config TURBO_GUARDED_BY(
    CustomUsageConfigMutex()) TURBO_PT_GUARDED_BY(CustomUsageConfigMutex()) =
    nullptr;

}  // namespace

FlagsUsageConfig GetUsageConfig() {
    std::lock_guard<std::mutex> l(CustomUsageConfigMutex());

    if (custom_usage_config) {
        return *custom_usage_config;
    }

    FlagsUsageConfig default_config;
    default_config.normalize_filename = &NormalizeFilename;
    return default_config;
}

void ReportUsageError(std::string_view msg, bool is_fatal) {
    std::cerr << "ERROR: " << msg << std::endl;

    if (is_fatal) {
        TURBO_INTERNAL_C_SYMBOL(TurboInternalReportFatalUsageError)(msg);
    }
}

void SetUsageConfig(FlagsUsageConfig usage_config) {
    std::lock_guard<std::mutex> l(CustomUsageConfigMutex());

    if (!usage_config.normalize_filename) {
        usage_config.normalize_filename = &NormalizeFilename;
    }

    if (custom_usage_config) {
        *custom_usage_config = usage_config;
    } else {
        custom_usage_config = new FlagsUsageConfig(usage_config);
    }
}

}  // namespace flags_internal

void SetFlagsUsageConfig(FlagsUsageConfig usage_config) {
    flags_internal::SetUsageConfig(std::move(usage_config));
}

}  // namespace turbo
