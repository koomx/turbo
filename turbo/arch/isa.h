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
#include <cstdlib>
#include <sstream>
#include <string>
#include <string_view>
#include <turbo/arch/instruction.h>
#include <turbo/arch/cpu_detect.h>
#include <turbo/macros/macros.h>
#include <type_traits>
#include <vector>

namespace turbo {

    ////////////////////////////////
    /// Bitmask of SIMD ISAs this translation unit was compiled against, from
    /// `KUMO_SIMD_*`. Independent of what the current CPU actually supports.
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

    /// One SIMD (or scalar) kernel that a domain can register.
    ///
    /// Fill `compiled`, `failback`, `required_isa`, `isa_name`, and `engine`.
    /// The register fills `current_compiled`, `current_isa`, and `rank`.
    struct IsaInfo {
        /// True if this kernel's sources were built into the library.
        bool compiled { false };

        /// True for the portable scalar path. Rank is always 1 when compiled.
        bool failback { false };

        /// InstructionSet bits this kernel needs at runtime.
        std::vector<uint32_t> required_isa {};

        /// Stable id used by force-select and dump, e.g. "arm64", "haswell".
        const char* isa_name { "" };

        /// Domain engine instance, or nullptr if this kernel was not compiled.
        void* engine { nullptr };

        /// Set by IsaRegister: `make_compiled_architectures()` for this TU.
        uint32_t current_compiled { 0 };

        /// Set by IsaRegister: `detect_supported_architectures()` for this CPU.
        uint32_t current_isa { 0 };

        /// Set by IsaRegister via `make_isa_rank`. 0 = unusable, 1 = fallback,
        /// higher = preferred SIMD. Best-of-avail is the max rank (unless forced).
        uint32_t rank { 0 };
    };

    /// Rank for `info` on this CPU. 0 if missing engine, not compiled, or the
    /// CPU/compile mask cannot satisfy `required_isa`. Fallback is 1.
    uint32_t make_isa_rank(const IsaInfo& info);

    /// CRTP runtime dispatcher. `Sub` is the domain registry (e.g. UnicodeRegistry).
    /// `T` is the engine base type (e.g. UnicodeImplement).
    ///
    /// Construct with every known kernel (compiled or not). Optional `force_isa`
    /// must match an available `isa_name` with a non-null engine, else abort.
    /// Meyers singleton: `get_isa()` builds one `Sub` on first use.
    template <typename Sub, typename T>
    class IsaRegister {
    public:
        virtual ~IsaRegister() = default;

        /// Highest-rank compiled kernel that this CPU can run, or the forced one.
        static T* get_best_isa() {
            return static_cast<T*>(get_isa()->_isa_best.engine);
        }

        /// Scalar fallback kernel, or nullptr if none was registered as failback.
        static T* get_failback_isa() {
            return static_cast<T*>(get_isa()->_isa_failback.engine);
        }

        /// Kernels with rank > 0 (compiled, engine present, ISA satisfied).
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

        /// Every registered kernel, including not compiled / rank 0.
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

        /// Human-readable table: force/best/fallback plus every IsaInfo.
        static std::string dump() {
            const Sub* self = get_isa();
            std::ostringstream os;
            os << "IsaRegister dump\n";
            os << "  force=" << (self->_force_isa.empty() ? "(none)" : self->_force_isa)
               << '\n';
            os << "  best=" << self->_isa_best.isa_name
               << " engine=" << self->_isa_best.engine
               << " rank=" << self->_isa_best.rank << '\n';
            os << "  fallback=" << self->_isa_failback.isa_name
               << " engine=" << self->_isa_failback.engine
               << " rank=" << self->_isa_failback.rank << '\n';
            os << "  all (" << self->_all_isa_info.size() << "):\n";
            for (size_t i = 0; i < self->_all_isa_info.size(); ++i) {
                const IsaInfo& info = self->_all_isa_info[i];
                const bool is_best = info.engine != nullptr && info.engine == self->_isa_best.engine;
                const bool is_fallback = info.failback || (info.engine != nullptr && info.engine == self->_isa_failback.engine);
                const bool is_force = !self->_force_isa.empty() && self->_force_isa == info.isa_name;
                os << "  [" << i << "] name=" << info.isa_name
                   << " compiled=" << static_cast<int>(info.compiled)
                   << " failback=" << static_cast<int>(info.failback)
                   << " required_isa=[";
                for (size_t j = 0; j < info.required_isa.size(); ++j) {
                    if (j != 0) {
                        os << ',';
                    }
                    const uint32_t fno = info.required_isa[j];
                    const CpuIsaMeta* meta = cpu_isa_meta(fno);
                    if (meta != nullptr && meta->name != nullptr && meta->name[0] != '\0') {
                        os << meta->name << '(' << fno << ')';
                    } else {
                        os << fno;
                    }
                }
                os << "] engine=" << info.engine
                   << " current_compiled=0x" << std::hex << info.current_compiled
                   << " current_isa=0x" << info.current_isa << std::dec
                   << " rank=" << info.rank;
                if (is_best) {
                    os << " [best]";
                }
                if (is_fallback) {
                    os << " [fallback]";
                }
                if (is_force) {
                    os << " [force]";
                }
                os << '\n';
            }
            os << "  avail=" << self->_avail_isa_info.size()
               << " unavail=" << self->_unavail_isa_info.size() << '\n';
            return os.str();
        }

    protected:
        /// @param engines all domain kernels; order does not matter except ties
        ///        keep the first highest rank.
        /// @param force_isa if non-empty, must be an available `isa_name`.
        explicit IsaRegister(std::vector<IsaInfo> engines, std::string_view force_isa = "")
            : _force_isa(force_isa)
            , _all_isa_info(std::move(engines)) {
            initialize();
        }

        void initialize() {
            initialize_default();
        }

        /// Rank every kernel, split avail/unavail, pick best and fallback.
        /// If `_force_isa` is set, replace best; missing name or null engine aborts.
        virtual void initialize_default() {
            uint32_t current_compiled = make_compiled_architectures();
            uint32_t current_isa = detect_supported_architectures();
            for (auto& info : _all_isa_info) {
                info.current_compiled = current_compiled;
                info.current_isa = current_isa;
                info.rank = make_isa_rank(info);
                if (info.rank == 0) {
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
            if (!_force_isa.empty()) {
                const IsaInfo* forced = nullptr;
                for (const auto& info : _avail_isa_info) {
                    if (_force_isa == info.isa_name) {
                        forced = &info;
                        break;
                    }
                }
                if (forced == nullptr || forced->engine == nullptr) {
                    abort();
                }
                _isa_best = *forced;
            }
        }

    protected:
        IsaInfo _isa_failback;
        IsaInfo _isa_best;

        /// Empty means pick by rank. Non-empty must match an available isa_name.
        std::string _force_isa;
        std::vector<IsaInfo> _all_isa_info;
        std::vector<IsaInfo> _avail_isa_info;
        std::vector<IsaInfo> _unavail_isa_info;
    };
} // namespace turbo
