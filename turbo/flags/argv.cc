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

#include <turbo/flags/argv.h>

#include <string>
#include <vector>

#include <turbo/flags/flag.h>
#include <turbo/flags/internal/path_util.h>

TURBO_FLAG(std::vector<std::string>, argv, {},
           "Process argv (set by xcli::App::parse or manually)");

namespace turbo {
namespace flags_internal {

std::string ShortProgramInvocationName() {
    const auto& args = turbo::GetFlag(FLAGS_argv);
    if (args.empty()) {
        return "UNKNOWN";
    }
    return std::string(flags_internal::Basename(args[0]));
}

}  // namespace flags_internal
}  // namespace turbo
