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

#include <turbo/log/internal/vlog_config.h>

#include <stddef.h>

#include <algorithm>
#include <atomic>
#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <turbo/macros/config.h>
#include <turbo/base/internal/scheduling_mode.h>
#include <turbo/base/internal/spinlock.h>
#include <turbo/base/thread_annotations.h>
#include <turbo/log/internal/fnmatch.h>
#include <turbo/strings/numbers.h>
#include <turbo/strings/str_split.h>
#include <string_view>
#include <turbo/strings/strip.h>
#include <mutex>

namespace turbo {
    namespace log_internal {
        namespace {
#ifdef _WIN32
            constexpr char kPathSeparators[] = "/\\";
#else
            constexpr char kPathSeparators[] = "/";
#endif

            bool ModuleIsPath(std::string_view module_pattern) {
                return module_pattern.find_first_of(kPathSeparators) != module_pattern.npos;
            }

            std::string_view Basename(std::string_view file) {
                auto sep = file.find_last_of(kPathSeparators);
                if (sep != file.npos) {
                    file.remove_prefix(sep + 1);
                }
                return file;
            }
        } // namespace

        bool VLogSite::slow_is_enabled(int stale_v, int level) {
            if (KUMO_LIKELY(stale_v != kUninitialized)) {
                // Because of the prerequisites to this function, we know that stale_v is
                // either uninitialized or >= level. If it's not uninitialized, that means
                // it must be >= level, thus we should log.
                return true;
            }
            stale_v = log_internal::register_and_initialize(this);
            return KUMO_UNLIKELY(stale_v >= level);
        }

        bool VLogSite::slow_is_enabled0(int stale_v) { return slow_is_enabled(stale_v, 0); }
        bool VLogSite::slow_is_enabled1(int stale_v) { return slow_is_enabled(stale_v, 1); }
        bool VLogSite::slow_is_enabled2(int stale_v) { return slow_is_enabled(stale_v, 2); }
        bool VLogSite::slow_is_enabled3(int stale_v) { return slow_is_enabled(stale_v, 3); }
        bool VLogSite::slow_is_enabled4(int stale_v) { return slow_is_enabled(stale_v, 4); }
        bool VLogSite::slow_is_enabled5(int stale_v) { return slow_is_enabled(stale_v, 5); }

        namespace {
            struct VModuleInfo final {
                std::string module_pattern;
                bool module_is_path; // i.e. it contains a path separator.
                int vlog_level;

                // Allocates memory.
                VModuleInfo(std::string_view module_pattern_arg, bool module_is_path_arg,
                            int vlog_level_arg)
                    : module_pattern(std::string(module_pattern_arg)),
                      module_is_path(module_is_path_arg),
                      vlog_level(vlog_level_arg) {
                }
            };

            // `mutex` guards all of the data structures that aren't lock-free.
            // To avoid problems with the heap checker which calls into `VKLOG`, `mutex` must
            // be a `SpinLock` that prevents fiber scheduling instead of a `Mutex`.
            KUMO_CONST_INIT turbo::base_internal::SpinLock mutex(
                turbo::base_internal::SCHEDULE_KERNEL_ONLY);

            // `get_update_sites_mutex()` serializes updates to all of the sites (i.e. those in
            // `site_list_head`) themselves.
            std::mutex &get_update_sites_mutex() {
                static std::mutex update_sites_mutex TURBO_ACQUIRED_AFTER(mutex);
                return update_sites_mutex;
            }

KUMO_CONST_INIT int global_v TURBO_GUARDED_BY(mutex) =
            0;
            // `site_list_head` is the head of a singly-linked list.  Traversal, insertion,
            // and reads are atomic, so no locks are required, but updates to existing
            // elements are guarded by `get_update_sites_mutex()`.
            KUMO_CONST_INIT std::atomic<VLogSite *> site_list_head{nullptr};
            KUMO_CONST_INIT std::vector<VModuleInfo> * vmodule_info TURBO_GUARDED_BY(mutex)
            TURBO_PT_GUARDED_BY (mutex){nullptr};

            // Only used for lisp.
            KUMO_CONST_INIT std::vector<std::function<void()> > *update_callbacks
            TURBO_GUARDED_BY (get_update_sites_mutex())
            TURBO_PT_GUARDED_BY (get_update_sites_mutex()) { nullptr };

            // Allocates memory.
            std::vector<VModuleInfo> &get_vmodule_info()

            TURBO_EXCLUSIVE_LOCKS_REQUIRED (mutex) {
                if (!vmodule_info) vmodule_info = new std::vector<VModuleInfo>;
                return *vmodule_info;
            }

            // Does not allocate or take locks.
            int vlog_level(std::string_view file, const std::vector<VModuleInfo> *infos,
                          int current_global_v) {
                // `infos` is null during a call to `VKLOG` prior to setting `vmodule` (e.g. by
                // parsing flags).  We can't allocate in `VKLOG`, so we treat null as empty
                // here and press on.
                if (!infos || infos->empty()) return current_global_v;

                std::string_view stem = file;
                std::string_view stem_basename = Basename(stem);
                {
                    const size_t sep = stem_basename.find('.');
                    if (sep != stem_basename.npos) {
                        stem.remove_suffix(stem_basename.size() - sep);
                        stem_basename.remove_suffix(stem_basename.size() - sep);
                    }
                    if (turbo::ConsumeSuffix(&stem_basename, "-inl")) {
                        stem.remove_suffix(std::string_view("-inl").size());
                    }
                }
                for (const auto &info: *infos) {
                    if (info.module_is_path) {
                        // If there are any slashes in the pattern, try to match the full
                        // name.
                        if (fnmatch(info.module_pattern, stem)) {
                            return info.vlog_level;
                        }
                    } else if (fnmatch(info.module_pattern, stem_basename)) {
                        return info.vlog_level;
                    }
                }

                return current_global_v;
            }

            // Allocates memory.
            int append_vmodule_locked(std::string_view module_pattern, int log_level)

            TURBO_EXCLUSIVE_LOCKS_REQUIRED (mutex) {
                for (const auto &info: get_vmodule_info()) {
                    if (fnmatch(info.module_pattern, module_pattern)) {
                        // This is a memory optimization to avoid storing patterns that will never
                        // match due to exit early semantics. Primarily optimized for our own unit
                        // tests.
                        return info.vlog_level;
                    }
                }
                bool module_is_path = ModuleIsPath(module_pattern);
                get_vmodule_info().emplace_back(std::string(module_pattern), module_is_path,
                                                log_level);
                return global_v;
            }

            // Allocates memory.
            int prepend_vmodule_locked(std::string_view module_pattern, int log_level)

            TURBO_EXCLUSIVE_LOCKS_REQUIRED (mutex) {
                std::optional<int> old_log_level;
                for (const auto &info: get_vmodule_info()) {
                    if (fnmatch(info.module_pattern, module_pattern)) {
                        old_log_level = info.vlog_level;
                        break;
                    }
                }
                bool module_is_path = ModuleIsPath(module_pattern);
                auto iter = get_vmodule_info().emplace(get_vmodule_info().cbegin(),
                                                       std::string(module_pattern),
                                                       module_is_path, log_level);

                // This is a memory optimization to avoid storing patterns that will never
                // match due to exit early semantics. Primarily optimized for our own unit
                // tests.
                get_vmodule_info().erase(
                    std::remove_if(++iter, get_vmodule_info().end(),
                                   [module_pattern](const VModuleInfo &info) {
                                       // Remove the previous pattern if it is less generic than
                                       // the new one. For example, if the new pattern
                                       // `module_pattern` is "foo*" and the previous pattern
                                       // `info.module_pattern` is "foo", we should remove the
                                       // previous pattern. Because the new pattern "foo*" will
                                       // match all the files that the previous pattern "foo"
                                       // matches.
                                       return fnmatch(module_pattern, info.module_pattern);
                                   }),
                    get_vmodule_info().cend());
                return old_log_level.value_or(global_v);
            }
        } // namespace

        int vlog_level(std::string_view file) TURBO_LOCKS_EXCLUDED

        (mutex) {
            turbo::base_internal::SpinLockHolder l(mutex);
            return vlog_level(file, vmodule_info, global_v);
        }

        int register_and_initialize(VLogSite * v) TURBO_LOCKS_EXCLUDED(mutex) {
            // std::memory_order_seq_cst is overkill in this function, but given that this
            // path is intended to be slow, it's not worth the brain power to relax that.
            VLogSite *h = site_list_head.load(std::memory_order_seq_cst);

            VLogSite *old = nullptr;
            if (v->next_.compare_exchange_strong(old, h, std::memory_order_seq_cst,
                                                 std::memory_order_seq_cst)) {
                // Multiple threads may attempt to register this site concurrently.
                // By successfully setting `v->next` this thread commits to being *the*
                // thread that installs `v` in the list.
                while (!site_list_head.compare_exchange_weak(
                    h, v, std::memory_order_seq_cst, std::memory_order_seq_cst)) {
                    v->next_.store(h, std::memory_order_seq_cst);
                }
            }

            int old_v = VLogSite::kUninitialized;
            int new_v = vlog_level(v->file_);
            // No loop, if someone else set this, we should respect their evaluation of
            // `vlog_level`. This may mean we return a stale `v`, but `v` itself will
            // always arrive at the freshest value.  Otherwise, we could be writing a
            // stale value and clobbering the fresher one.
            if (v->v_.compare_exchange_strong(old_v, new_v, std::memory_order_seq_cst,
                                              std::memory_order_seq_cst)) {
                return new_v;
            }
            return old_v;
        }

        void update_vlog_sites() TURBO_UNLOCK_FUNCTION

        (mutex)

        TURBO_LOCKS_EXCLUDED (get_update_sites_mutex()) {
            std::vector<VModuleInfo> infos = get_vmodule_info();
            int current_global_v = global_v;
            // We need to grab `get_update_sites_mutex()` before we release `mutex` to ensure
            // that updates are not interleaved (resulting in an inconsistent final state)
            // and to ensure that the final state in the sites matches the final state of
            // `vmodule_info`. We unlock `mutex` to ensure that uninitialized sites don't
            // have to wait on all updates in order to acquire `mutex` and initialize
            // themselves.
            std::lock_guard<std::mutex> ul(get_update_sites_mutex());
            mutex.unlock();
            VLogSite *n = site_list_head.load(std::memory_order_seq_cst);
            // Because sites are added to the list in the order they are executed, there
            // tend to be clusters of entries with the same file.
            const char *last_file = nullptr;
            int last_file_level = 0;
            while (n != nullptr) {
                if (n->file_ != last_file) {
                    last_file = n->file_;
                    last_file_level = vlog_level(n->file_, &infos, current_global_v);
                }
                n->v_.store(last_file_level, std::memory_order_seq_cst);
                n = n->next_.load(std::memory_order_seq_cst);
            }
            if (update_callbacks) {
                for (auto &cb: *update_callbacks) {
                    cb();
                }
            }
        }

        void update_vmodule(std::string_view vmodule)TURBO_LOCKS_EXCLUDED(mutex, get_update_sites_mutex()) {
            std::vector<std::pair<std::string_view, int> > glob_levels;
            for (std::string_view glob_level: turbo::StrSplit(vmodule, ',')) {
                const size_t eq = glob_level.rfind('=');
                if (eq == glob_level.npos) continue;
                const std::string_view glob = glob_level.substr(0, eq);
                int level;
                if (!turbo::SimpleAtoi(glob_level.substr(eq + 1), &level)) continue;
                glob_levels.emplace_back(glob, level);
            }
            mutex.lock(); // unlocked by update_vlog_sites().
            get_vmodule_info().clear();
            for (const auto &it: glob_levels) {
                const std::string_view glob = it.first;
                const int level = it.second;
                append_vmodule_locked(glob, level);
            }
            update_vlog_sites();
        }

        int update_global_vlog_level(int v)

        TURBO_LOCKS_EXCLUDED(mutex, get_update_sites_mutex()) {
            mutex.lock(); // unlocked by update_vlog_sites().
            const int old_global_v = global_v;
            if (v == global_v) {
                mutex.unlock();
                return old_global_v;
            }
            global_v = v;
            update_vlog_sites();
            return old_global_v;
        }

        int prepend_vmodule(std::string_view module_pattern, int log_level)

        TURBO_LOCKS_EXCLUDED(mutex, get_update_sites_mutex()) {
            mutex.lock(); // unlocked by update_vlog_sites().
            int old_v = prepend_vmodule_locked(module_pattern, log_level);
            update_vlog_sites();
            return old_v;
        }

        void on_vlog_verbosity_update(std::function<void()> cb)

        TURBO_LOCKS_EXCLUDED (get_update_sites_mutex()) {
            std::lock_guard<std::mutex> ul(get_update_sites_mutex());
            if (!update_callbacks)
                update_callbacks = new std::vector<std::function<void()> >;
            update_callbacks->push_back(std::move(cb));
        }

        VLogSite *set_vmodule_list_head_for_test_only(VLogSite *v) {
            return site_list_head.exchange(v, std::memory_order_seq_cst);
        }
    } // namespace log_internal
} // namespace turbo
