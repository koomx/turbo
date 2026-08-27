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

#include <turbo/hash/xx/isa_select.h>

#include <turbo/hash/xx/engine/avx2/interface.h>
#include <turbo/hash/xx/engine/avx512/interface.h>
#include <turbo/hash/xx/engine/fallback/interface.h>
#include <turbo/hash/xx/engine/lasx/interface.h>
#include <turbo/hash/xx/engine/lsx/interface.h>
#include <turbo/hash/xx/engine/neon/interface.h>
#include <turbo/hash/xx/engine/ppc64/interface.h>
#include <turbo/hash/xx/engine/rvv/interface.h>
#include <turbo/hash/xx/engine/sse2/interface.h>
#include <turbo/hash/xx/engine/sve/interface.h>

namespace turbo {

    static std::vector<IsaInfo> get_built_infos() {
        std::vector<IsaInfo> infos = {
            xxhash::get_xxhash_neon_info(),
            xxhash::get_xxhash_sve_info(),
            xxhash::get_xxhash_avx512_info(),
            xxhash::get_xxhash_avx2_info(),
            xxhash::get_xxhash_sse2_info(),
            xxhash::get_xxhash_lasx_info(),
            xxhash::get_xxhash_lsx_info(),
            xxhash::get_xxhash_rvv_info(),
            xxhash::get_xxhash_ppc64_info(),
            xxhash::get_xxhash_fallback_info(),
        };
        return infos;
    };

    XXHashRegistry::XXHashRegistry()
        : IsaRegister<turbo::XXHashRegistry, turbo::xxhash::XXHashEngine>(get_built_infos()) {
    }
} // namespace turbo
