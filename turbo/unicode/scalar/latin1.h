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

namespace turbo {
    namespace scalar {
        namespace {
            namespace latin1 {

                KUMO_FORCE_INLINE size_t utf8_length_from_latin1(const char* buf,
                    size_t len) {
                    const uint8_t* c = reinterpret_cast<const uint8_t*>(buf);
                    size_t answer = 0;
                    for (size_t i = 0; i < len; i++) {
                        if ((c[i] >> 7)) {
                            answer++;
                        }
                    }
                    return answer + len;
                }

            } // namespace latin1
        } // unnamed namespace
    } // namespace scalar
} // namespace turbo
