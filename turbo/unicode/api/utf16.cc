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

#include <turbo/unicode/api/utf16.h>
#include <turbo/unicode/engine/scalar/utf16.h>

namespace turbo {

    simdutf_warn_unused size_t trim_partial_utf16be(const char16_t* input,
        size_t length) {
        return scalar::utf16::trim_partial_utf16<BIG>(input, length);
    }

    simdutf_warn_unused size_t trim_partial_utf16le(const char16_t* input,
        size_t length) {
        return scalar::utf16::trim_partial_utf16<LITTLE>(input, length);
    }

    simdutf_warn_unused size_t trim_partial_utf16(const char16_t* input,
        size_t length) {
#if SIMDUTF_IS_BIG_ENDIAN
        return trim_partial_utf16be(input, length);
#else
        return trim_partial_utf16le(input, length);
#endif
    }
}  // namespace turbo
