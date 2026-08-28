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

#include <filesystem>
#include <string_view>
#include <turbo/unicode/api/wchar.h>

namespace xcli {

    /// UTF-8 command-line text to a native filesystem path.
    inline std::filesystem::path to_path(std::string_view utf8) {
#ifdef _WIN32
        return std::filesystem::path(turbo::convert_utf8_to_wchar(utf8));
#else
        return std::filesystem::path(utf8.begin(), utf8.end());
#endif
    }

} // namespace xcli
