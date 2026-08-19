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

#include <turbo/unicode/api/base64.h>
#include <turbo/unicode/engine/backend_select.h>
#include <turbo/unicode/api/base64_implementation.h>

namespace turbo {


    // this has been moved to implementation.h
    // simdutf_warn_unused size_t
    // base64_length_from_binary(size_t length, base64_options option) noexcept;

    // this has been moved to implementation.h
    // simdutf_warn_unused size_t base64_length_from_binary_with_lines(
    //     size_t length, base64_options options, size_t line_length) noexcept;
    // }

    simdutf_warn_unused const char* detail::find(const char* start, const char* end,
        char character) noexcept {
        return get_default_implementation()->find(start, end, character);
    }
    simdutf_warn_unused const char16_t* detail::find(const char16_t* start,
        const char16_t* end,
        char16_t character) noexcept {
        return get_default_implementation()->find(start, end, character);
    }

}  // namespace turbo

