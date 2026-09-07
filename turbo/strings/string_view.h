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

#include <turbo/base/nullability.h>
#include <turbo/macros/config.h>

namespace turbo {
    // ClippedSubstr()
    //
    // Like `s.substr(pos, n)`, but clips `pos` to an upper bound of `s.size()`.
    // Provided because std::string_view::substr throws if `pos > size()`
    inline std::string_view ClippedSubstr(std::string_view s KUMO_ATTRIBUTE_LIFETIME_BOUND,
        size_t pos, size_t n = std::string_view::npos) {
        pos = (std::min)(pos, static_cast<size_t>(s.size()));
        return s.substr(pos, n);
    }

    // NullSafeStringView()
    //
    // Creates an `std::string_view` from a pointer `p` even if it's null-valued.
    // This function should be used where an `std::string_view` can be created from
    // a possibly-null pointer.
    constexpr std::string_view NullSafeStringView(const char* turbo_nullable p) {
        return p ? std::string_view(p) : std::string_view();
    }

} // namespace turbo
