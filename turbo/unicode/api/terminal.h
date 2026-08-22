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

namespace turbo {

    /// UTF-8 terminal display width (wcswidth-style). Invalid sequences have
    /// width 0. `prefix` is used for `\t`, which extends to the next multiple of 8
    /// from `prefix + current width`.
    size_t utf8_display_width(const uint8_t* data, size_t size, size_t prefix = 0) noexcept;

    /// Max byte length of a prefix whose visible width is <= `limit`.
    size_t utf8_display_bytes_for_width(const uint8_t* data, size_t size, size_t prefix,
        size_t limit) noexcept;

}  // namespace turbo
