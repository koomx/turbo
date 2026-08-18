//
// Copyright 2017 The Abseil Authors.
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

#include <turbo/format/str_format/extension.h>

#include <algorithm>
#include <errno.h>
#include <string>

namespace turbo {
    namespace str_format_internal {
        std::string flags_to_string(Flags v) {
            std::string s;
            s.append(flags_contains(v, Flags::kLeft) ? "-" : "");
            s.append(flags_contains(v, Flags::kShowPos) ? "+" : "");
            s.append(flags_contains(v, Flags::kSignCol) ? " " : "");
            s.append(flags_contains(v, Flags::kAlt) ? "#" : "");
            s.append(flags_contains(v, Flags::kZero) ? "0" : "");
            return s;
        }

        bool FormatSinkImpl::put_padded_string(std::string_view value, int width,
            int precision, bool left) {
            size_t space_remaining = 0;
            if (width >= 0)
                space_remaining = static_cast<size_t>(width);
            size_t n = value.size();
            if (precision >= 0)
                n = std::min(n, static_cast<size_t>(precision));
            std::string_view shown(value.data(), n);
            space_remaining = Excess(shown.size(), space_remaining);
            if (!left)
                append(space_remaining, ' ');
            append(shown);
            if (left)
                append(space_remaining, ' ');
            return true;
        }
    } // namespace str_format_internal
} // namespace turbo
