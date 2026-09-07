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

#include <string_view>
#include <string>

namespace turbo {

    template <typename out_iter>
    void encode_json(std::string_view view, out_iter out) {
        // trivial implementation. could be faster.
        const char* hexvalues =
            "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f";
        for (uint8_t c : view) {
            if (c == '\\') {
                *out++ = '\\';
                *out++ = '\\';
            } else if (c == '"') {
                *out++ = '\\';
                *out++ = '"';
            } else if (c <= 0x1f) {
                *out++ = '\\';
                *out++ = 'u';
                *out++ = '0';
                *out++ = '0';
                *out++ = hexvalues[2 * c];
                *out++ = hexvalues[2 * c + 1];
            } else {
                *out++ = c;
            }
        }
    }

}  // namespace turbo
