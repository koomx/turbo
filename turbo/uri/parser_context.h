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

#include <cstdint>
#include <cstddef>
#include <string_view>

namespace turbo {

    struct ParserContext {
        ParserContext(std::string_view str)
        : start(str.data()),pos(str.data()),end(str.data() + str.size()) {
            }
        const char* start{nullptr};
        const char* pos{nullptr};
        const char* end{nullptr};

        std::string_view context() const {
            return std::string_view{pos, end - pos};
        }

        std::string_view origin() const {
            return std::string_view{start, end - start};
        }

        constexpr size_t offset() const {
            return pos - start;
        }

        constexpr size_t remain() const {
            return end - pos;
        }
    };
}  // namespace turbo
