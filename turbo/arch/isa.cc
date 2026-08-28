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

#include <turbo/arch/isa.h>

#include <string>

namespace turbo {

    uint32_t make_isa_rank(const IsaInfo& info) {
        if (info.engine == nullptr || !info.compiled) {
            return 0;
        }
        if (info.failback) {
            return 1;
        }

        std::string err;
        const std::map<uint32_t, CpuIsaMeta*> cpu =
            convert_isa_info_to_feature(detect_cpu_isa_info(), err);
        if (!err.empty()) {
            return 0;
        }
        err.clear();
        const std::map<uint32_t, CpuIsaMeta*> enabled =
            convert_isa_info_to_feature(detect_current_enabled_isa_info(), err);
        if (!err.empty()) {
            return 0;
        }

        uint32_t rank = 0;
        for (uint32_t fno : info.required_isa) {
            if (cpu_isa_meta(fno) == nullptr) {
                return 0;
            }
            if (cpu.find(fno) == cpu.end() || enabled.find(fno) == enabled.end()) {
                return 0;
            }
            const uint32_t gen = cpu_isa_level_rank(fno);
            if (gen > rank) {
                rank = gen;
            }
        }
        return rank;
    }

} // namespace turbo
