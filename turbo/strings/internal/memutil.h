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
//

#ifndef TURBO_STRINGS_INTERNAL_MEMUTIL_H_
#define TURBO_STRINGS_INTERNAL_MEMUTIL_H_

#include <cstddef>
#include <cstring>

#include <turbo/macros/config.h>
#include <turbo/strings/ascii.h> // for turbo::ascii_tolower

namespace turbo {

    namespace strings_internal {

        // Performs a byte-by-byte comparison of `len` bytes of the strings `s1` and
        // `s2`, ignoring the case of the characters. It returns an integer less than,
        // equal to, or greater than zero if `s1` is found, respectively, to be less
        // than, to match, or be greater than `s2`.
        int memcasecmp(const char* s1, const char* s2, size_t len);

    } // namespace strings_internal

} // namespace turbo

#endif // TURBO_STRINGS_INTERNAL_MEMUTIL_H_
