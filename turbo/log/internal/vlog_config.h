// Copyright 2022 The Abseil Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// -----------------------------------------------------------------------------
// vlog_config.h
// -----------------------------------------------------------------------------
//
// This header file defines `VLogSite`, a public primitive that represents
// a callsite for the `VKLOG` family of macros and related libraries.
// It also declares and defines multiple internal utilities used to implement
// `VKLOG`, such as `VLogSiteManager`.

#ifndef TURBO_LOG_INTERNAL_VLOG_CONFIG_H_
#define TURBO_LOG_INTERNAL_VLOG_CONFIG_H_

// IWYU pragma: private, include "turbo/log/log.h"

#include <atomic>
#include <cstdint>
#include <functional>
#include <limits>
#include <type_traits>

#include <turbo/macros/config.h>
#include <turbo/base/nullability.h>
#include <turbo/base/thread_annotations.h>
#include <string_view>

namespace turbo {
    namespace log_internal {
        class SyntheticBinary;
        class VLogSite;

        int register_and_initialize(VLogSite * turbo_nonnull v);

        void update_vlog_sites();

        // Represents a unique callsite for a `VKLOG()` or `VKLOG_IS_ON()` call.
        //
        // Libraries that provide `VKLOG`-like functionality should use this to
        // efficiently handle per-module VKLOG patterns.
        //
        // VLogSite objects must not be destroyed until the program exits. Doing so will
        // probably yield nasty segfaults in VLogSiteManager::update_log_sites(). The
        // recommendation is to make all such objects function-local statics.
        class VLogSite final {
        public:
            // `f` must not be destroyed until the program exits.
            explicit constexpr VLogSite(const char * turbo_nonnull f)
                : file_(f), v_(kUninitialized), next_(nullptr) {
            }

            VLogSite(const VLogSite &) = delete;

            VLogSite &operator=(const VLogSite &) = delete;

            // Inlining the function yields a ~3x performance improvement at the cost of a
            // 1.5x code size increase at the call site.
            // Takes locks but does not allocate memory.
            KUMO_ATTRIBUTE_ALWAYS_INLINE
            bool is_enabled(int level) {
                int stale_v = v_.load(std::memory_order_relaxed);
                if (KUMO_LIKELY(level > stale_v)) {
                    return false;
                }

                // We put everything other than the fast path, i.e. vlogging is initialized
                // but not on, behind an out-of-line function to reduce code size.
                // "level" is almost always a call-site constant, so we can save a bit
                // of code space by special-casing for a few common levels.
#if KUMO_HAVE_BUILTIN(__builtin_constant_p) || defined(__GNUC__)
                if (__builtin_constant_p(level)) {
                    if (level == 0) return slow_is_enabled0(stale_v);
                    if (level == 1) return slow_is_enabled1(stale_v);
                    if (level == 2) return slow_is_enabled2(stale_v);
                    if (level == 3) return slow_is_enabled3(stale_v);
                    if (level == 4) return slow_is_enabled4(stale_v);
                    if (level == 5) return slow_is_enabled5(stale_v);
                }
#endif
                return slow_is_enabled(stale_v, level);
            }

        private:
            friend int log_internal::register_and_initialize(VLogSite * turbo_nonnull v);

            friend void log_internal::update_vlog_sites();

            friend class log_internal::SyntheticBinary;
            static constexpr int kUninitialized = (std::numeric_limits<int>::max)();

            // slow_is_enabled performs slower checks to determine whether a log site is
            // enabled. Because it is expected to be called somewhat rarely
            // (comparatively), it is not inlined to save on code size.
            //
            // Prerequisites to calling slow_is_enabled:
            //   1) stale_v is uninitialized OR
            //   2) stale_v is initialized and >= level (meaning we must log).
            // Takes locks but does not allocate memory.
            KUMO_ATTRIBUTE_NOINLINE
            bool slow_is_enabled(int stale_v, int level);

  KUMO_ATTRIBUTE_NOINLINE bool slow_is_enabled0(int stale_v);

  KUMO_ATTRIBUTE_NOINLINE bool slow_is_enabled1(int stale_v);

  KUMO_ATTRIBUTE_NOINLINE bool slow_is_enabled2(int stale_v);

  KUMO_ATTRIBUTE_NOINLINE bool slow_is_enabled3(int stale_v);

  KUMO_ATTRIBUTE_NOINLINE bool slow_is_enabled4(int stale_v);

  KUMO_ATTRIBUTE_NOINLINE bool slow_is_enabled5(int stale_v);

            // This object is too size-sensitive to use std::string_view.
            const char *turbo_nonnull
            const file_;
            std::atomic<int> v_;
            std::atomic<VLogSite *> next_;
        };

        static_assert(std::is_trivially_destructible_v<VLogSite>,
                      "VLogSite must be trivially destructible");

        // Returns the current verbose log level of `file`.
        // Does not allocate memory.
        int vlog_level(std::string_view file);

        // Registers a site `v` to get updated as `vmodule` and `v` change.  Also
        // initializes the site based on their current values, and returns that result.
        // Does not allocate memory.
        int register_and_initialize(VLogSite * turbo_nonnull v);

        // Allocates memory.
        void update_vlog_sites();

        // Completely overwrites the saved value of `vmodule`.
        // Allocates memory.
        void update_vmodule(std::string_view vmodule);

        // Updates the global verbosity level to `v` and returns the prior value.
        // Allocates memory.
        int update_global_vlog_level(int v);

        // Atomically prepends `module_pattern=log_level` to the start of vmodule.
        // Returns the prior value for `module_pattern` if there was an exact match and
        // `global_v` otherwise.
        // Allocates memory.
        int prepend_vmodule(std::string_view module_pattern, int log_level);

        // Registers `on_update` to be called whenever `v` or `vmodule` change.
        // Allocates memory.
        void on_vlog_verbosity_update(std::function<void()> cb);

        // Does not allocate memory.
        VLogSite * turbo_nullable set_vmodule_list_head_for_test_only(
            VLogSite * turbo_nullable v);
    } // namespace log_internal
} // namespace turbo

#endif  // TURBO_LOG_INTERNAL_VLOG_CONFIG_H_
