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

#include <turbo/unicode/engine/common_defs.h>
#include <turbo/unicode/engine/compiler_check.h>
#include <turbo/unicode/text_encoding.h>
#include <turbo/unicode/error.h>

namespace turbo {

    /// Validate the ASCII string.
    ///
    /// Overridden by each implementation.
    ///
    /// @param buf the ASCII string to validate.
    /// @param len the length of the string in bytes.
    /// @return true if and only if the string is valid ASCII.
    simdutf_warn_unused bool validate_ascii(const char* buf, size_t len) noexcept;

    /// Validate the ASCII string and stop on error. It might be faster than
    /// validate_utf8 when an error is expected to occur early.
    ///
    /// Overridden by each implementation.
    ///
    /// @param buf the ASCII string to validate.
    /// @param len the length of the string in bytes.
    /// @return a result pair struct (of type turbo::result containing the two
    /// fields error and count) with an error code and either position of the error
    /// (in the input in code units) if any, or the number of code units validated if
    /// successful.
    simdutf_warn_unused result validate_ascii_with_errors(const char* buf,
        size_t len) noexcept;

}  // namespace turbo
