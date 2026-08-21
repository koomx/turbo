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

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <functional>
#include <turbo/macros/macros.h>
#include <turbo/base/nullability.h>

namespace turbo {

    // base64_encode()
    //
    // Encodes a `src` string into a base64-encoded `dest` string with padding
    // characters. This function conforms with RFC 4648 section 4 (base64) and RFC
    // 2045.
    std::string base64_encode(std::string_view src);

    // base64_decode()
    //
    // Converts a `src` string encoded in Base64 (RFC 4648 section 4) to its binary
    // equivalent, writing it to a `dest` buffer, returning `true` on success. If
    // `src` contains invalid characters, `dest` is cleared and returns `false`.
    // If padding is included (note that `base64_encode()` does produce it), it must
    // be correct. In the padding, '=' and '.' are treated identically.
    bool base64_decode(std::string_view src, std::string* turbo_nonnull dest);


    // web_safe_base64_encode()
    //
    // Encodes a `src` string into a base64 string, like base64_encode() does, but
    // outputs '-' instead of '+' and '_' instead of '/', and does not pad `dest`.
    // This function conforms with RFC 4648 section 5 (base64url).
    std::string web_safe_base64_encode(std::string_view src);



    // web_safe_base64_decode()
    //
    // Converts a `src` string encoded in "web safe" Base64 (RFC 4648 section 5) to
    // its binary equivalent, writing it to a `dest` buffer, returning `true` on
    // success. If `src` contains invalid characters, `dest` is cleared and returns
    // `false`. If padding is included (note that `web_safe_base64_encode()` does not
    // produce it), it must be correct. In the padding, '=' and '.' are treated
    // identically.
    bool web_safe_base64_decode(std::string_view src,
        std::string* turbo_nonnull dest);


} // namespace turbo
