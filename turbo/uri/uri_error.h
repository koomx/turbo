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
#include <limits>
#include <string>

namespace turbo {

    enum class  UriErrorCode : uint8_t {
        kUriSuccess = 0,
        kUriNotComplete = 1,
        kUriInvalidOffset = 2,
        kUriOverflow = 3,
        kUriMissSeperator = 4,
        kUriForbiddenHostCodePoint = 5,
        kUriInvalidArgs = 6,
        kNotChecked = 128,
    };

    struct UriError {
        static constexpr uint32_t npos = std::numeric_limits<uint32_t>::max();
        UriErrorCode code{UriErrorCode::kUriSuccess};
        uint32_t     error_pos{npos};
        /// any result, can be result, or error message
        std::string  payload;

        bool ok() const {
            return code == UriErrorCode::kUriSuccess;
        }
        UriError& operator&=(const UriError& rhs) {
            if (&rhs == this ||
                (code != UriErrorCode::kNotChecked && code != UriErrorCode::kUriSuccess)) {
                return *this;
            }
            code = rhs.code;
            error_pos = rhs.error_pos;
            payload = rhs.payload;
            return *this;
        }
    };

}  // namespace turbo

