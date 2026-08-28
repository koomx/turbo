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

#include <turbo/hash/xx/common.h>

namespace turbo {

    void XxHashStateCore::reset(uint64_t sd, const uint8_t* secret, size_t secretSize) {
        buffered_size = 0;
        nb_stripes_so_far = 0;
        totalLen = 0;
        acc[0] = xxhash::kXxhPrime32_3;
        acc[1] = xxhash::kXxhPrime64_1;
        acc[2] = xxhash::kXxhPrime64_2;
        acc[3] = xxhash::kXxhPrime64_3;
        acc[4] = xxhash::kXxhPrime64_4;
        acc[5] = xxhash::kXxhPrime32_2;
        acc[6] = xxhash::kXxhPrime64_5;
        acc[7] = xxhash::kXxhPrime32_1;
        seed = sd;
        use_seed = (seed != 0);
        ext_secret = (const unsigned char*)secret;
        KUMO_DASSERT(secretSize >= xxhash::kXxh3SecretSizeMin);
        secret_limit = secretSize - xxhash::kXxhStripeLen;
        nb_stripes_per_block = secret_limit / xxhash::kXxhSecretConsumeRate;
    }


}  // namespace turbo
