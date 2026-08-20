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
#include <turbo/arch/instruction.h>
#include <turbo/arch/isadetection.h>
#include <turbo/macros/macros.h>
#include <type_traits>
#include <vector>

namespace turbo {

    inline uint32_t make_compiled_architectures() {
        uint32_t result = InstructionSet::DEFAULT;
#if KUMO_SIMD_NEON
        result |= InstructionSet::NEON;
#endif
#if KUMO_SIMD_SSE4_2
        result |= InstructionSet::SSE42;
#endif
#if KUMO_SIMD_PCLMUL
        result |= InstructionSet::PCLMULQDQ;
#endif
#if KUMO_SIMD_AVX2
        result |= InstructionSet::AVX2;
#endif
#if KUMO_SIMD_BMI1
        result |= InstructionSet::BMI1;
#endif
#if KUMO_SIMD_BMI2
        result |= InstructionSet::BMI2;
#endif
#if KUMO_SIMD_AVX512F
        result |= InstructionSet::AVX512F;
#endif
#if KUMO_SIMD_AVX512DQ
        result |= InstructionSet::AVX512DQ;
#endif
#if KUMO_SIMD_AVX512IFMA
        result |= InstructionSet::AVX512IFMA;
#endif
#if KUMO_SIMD_AVX512CD
        result |= InstructionSet::AVX512CD;
#endif
#if KUMO_SIMD_AVX512BW
        result |= InstructionSet::AVX512BW;
#endif
#if KUMO_SIMD_AVX512VL
        result |= InstructionSet::AVX512VL;
#endif
#if KUMO_SIMD_AVX512VBMI2
        result |= InstructionSet::AVX512VBMI2;
#endif
#if KUMO_SIMD_AVX512VPOPCNTDQ
        result |= InstructionSet::AVX512VPOPCNTDQ;
#endif
#if KUMO_SIMD_ALTIVEC
        result |= InstructionSet::ALTIVEC;
#endif
#if KUMO_SIMD_RVV
        result |= InstructionSet::RVV;
#endif
#if KUMO_SIMD_LSX
        result |= InstructionSet::LSX;
#endif
#if KUMO_SIMD_LASX
        result |= InstructionSet::LASX;
#endif
        return result;
    }

    struct IsaInfo {

        bool compiled{false};

        bool failback{false};

        uint32_t required_isa{0};
        const char* isa_name{""};

        void *engine{nullptr};

        /////////////////////////////////////////
        /// user no need fill below, by framework
        /// this should init in a header, to touch
        /// the current defines
        uint32_t current_compiled{0};
        /// by current detect_supported_architectures
        uint32_t current_isa{0};
        /// 0 means no rank no available
        /// 1 mean no simd
        /// users no need care it
        uint32_t rank{0};
    };


    uint32_t make_isa_rank(const IsaInfo& info);

    template <typename Sub, typename T>
    class IsaRegister {
    public:
       // static_assert(std::is_base_of_v<Sub, IsaRegister<Sub,T>>, "must");
    public:
       virtual  ~IsaRegister() = default;

        static T* get_best_isa() {
            return static_cast<T*>(get_isa()->_isa_best.engine);
        }

        static T* get_failback_isa() {
            return static_cast<T*>(get_isa()->_isa_failback.engine);
        }

        static std::vector<IsaInfo> get_avail_isa_info() {
            return get_isa()->_avail_isa_info;
        }

        static std::vector<T*> get_avail_isa() {
            std::vector<T*> ret;
            ret.reserve(get_avail_isa_info().size());
            for (const auto& info : get_avail_isa_info()) {
                ret.push_back(static_cast<T*>(info.engine));
            }
            return ret;
        }

        static std::vector<IsaInfo> get_all_isa_info() {
            return get_isa()->_all_isa_info;
        }

        static std::vector<T*> get_all_isa() {
            std::vector<T*> ret;
            ret.reserve(get_all_isa_info().size());
            for (const auto& info : get_all_isa_info()) {
                ret.push_back(static_cast<T*>(info.engine));
            }
            return ret;
        }

        static Sub* get_isa() {
            static Sub ins;
            return &ins;
        }

    protected:
       explicit IsaRegister(std::vector<IsaInfo> engines)
            :_all_isa_info(std::move(engines)) {
            initialize();
        }

        void initialize() {
            initialize_default();
        }

       virtual void initialize_default() {
            uint32_t current_compiled = make_compiled_architectures();
            uint32_t current_isa = internal::detect_supported_architectures();
            for (auto& info : _all_isa_info) {
                info.current_compiled = current_compiled;
                info.current_isa = current_isa;
                info.rank = make_isa_rank(info);
                if (info.rank  == 0) {
                    _unavail_isa_info.push_back(info);
                    continue;
                }
                _avail_isa_info.push_back(info);
                if (info.rank > _isa_best.rank) {
                    _isa_best = info;
                }
                if (info.failback) {
                    _isa_failback = info;
                }
            }
        }
    protected:
        bool _isa_detected{false};
        IsaInfo _isa_failback;
        IsaInfo _isa_best;

        std::vector<IsaInfo> _all_isa_info;
        std::vector<IsaInfo> _avail_isa_info;
        std::vector<IsaInfo> _unavail_isa_info;
    };
}  // namespace turbo
