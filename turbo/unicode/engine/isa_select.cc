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

#include <turbo/unicode/engine/isa_select.h>
KUMO_DISABLE_UNUSED_WARNING
#include <turbo/unicode/engine/arm64.h>
#include <turbo/unicode/engine/icelake.h>
#include <turbo/unicode/engine/haswell.h>
#include <turbo/unicode/engine/westmere.h>
#include <turbo/unicode/engine/ppc64.h>
#include <turbo/unicode/engine/rvv.h>
#include <turbo/unicode/engine/lasx.h>
#include <turbo/unicode/engine/lsx.h>
#include <turbo/unicode/engine/fallback.h> // have it always last.
KUMO_RESTORE_UNUSED_WARNING

namespace turbo {

    static std::vector<IsaInfo> get_built_infos() {
        std::vector<IsaInfo> infos = {
            get_arm64_info(),
            get_fallback_info(),
            get_icelake_info(),
            get_haswell_info(),
            get_westmere_info(),
            get_ppc64_info(),
            get_rvv_info(),
            get_lasx_info(),
            get_lsx_info(),
        };
        return infos;
    };
    UnicodeRegistry::UnicodeRegistry() :IsaRegister<turbo::UnicodeRegistry, turbo::UnicodeImplement>(get_built_infos()) {

    }
}  // namespace turbo
