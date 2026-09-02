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

#include <turbo/uri/wpt/state.h>

namespace turbo {
    std::string_view to_string(WptState s) {
        switch (s) {
        case WptState::AUTHORITY:
            return "Authority";
        case WptState::SCHEME_START:
            return "Scheme Start";
        case WptState::SCHEME:
            return "Scheme";
        case WptState::HOST:
            return "Host";
        case WptState::NO_SCHEME:
            return "No Scheme";
        case WptState::FRAGMENT:
            return "Fragment";
        case WptState::RELATIVE_SCHEME:
            return "Relative Scheme";
        case WptState::RELATIVE_SLASH:
            return "Relative Slash";
        case WptState::FILE:
            return "File";
        case WptState::FILE_HOST:
            return "File Host";
        case WptState::FILE_SLASH:
            return "File Slash";
        case WptState::PATH_OR_AUTHORITY:
            return "Path or Authority";
        case WptState::SPECIAL_AUTHORITY_IGNORE_SLASHES:
            return "Special Authority Ignore Slashes";
        case WptState::SPECIAL_AUTHORITY_SLASHES:
            return "Special Authority Slashes";
        case WptState::SPECIAL_RELATIVE_OR_AUTHORITY:
            return "Special Relative or Authority";
        case WptState::QUERY:
            return "Query";
        case WptState::PATH:
            return "Path";
        case WptState::PATH_START:
            return "Path Start";
        case WptState::OPAQUE_PATH:
            return "Opaque Path";
        case WptState::PORT:
            return "Port";
        default:
            return "unknown state";
        }
    }

} // namespace turbo
