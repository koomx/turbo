// Copyright 2022 The Abseil Authors.
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

#include <turbo/log/globals.h>

#include <atomic>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <string>

#include <turbo/macros/config.h>
#include <turbo/base/internal/raw_logging.h>
#include <turbo/base/log_severity.h>
#include <turbo/hash/hash.h>
#include <string_view>

namespace turbo {
    namespace {
        // These atomics represent logging library configuration.
        // Integer types are used instead of turbo::LogSeverity to ensure that a
        // lock-free std::atomic is used when possible.
        KUMO_CONST_INIT std::atomic<int> min_log_level_{
            static_cast<int>(turbo::LogSeverityAtLeast::kInfo)
        };
        KUMO_CONST_INIT std::atomic<int> stderrthreshold{
            static_cast<int>(turbo::LogSeverityAtLeast::kError)
        };
        // Hash comparison avoids holding a mutex or copying a string on the hot path.
        KUMO_CONST_INIT std::atomic<size_t> log_backtrace_at_hash{0};
        KUMO_CONST_INIT std::atomic<bool> prepend_log_prefix{true};

        constexpr char kDefaultAndroidTag[] = "native";
        KUMO_CONST_INIT std::atomic<const char *> android_log_tag{kDefaultAndroidTag};

        size_t hash_site_for_log_backtrace_at(std::string_view file, int line) {
            return turbo::HashOf(file, line);
        }
    } // namespace

    turbo::LogSeverityAtLeast min_log_level() {
        return static_cast<turbo::LogSeverityAtLeast>(
            min_log_level_.load(std::memory_order_acquire));
    }

    void set_min_log_level(turbo::LogSeverityAtLeast severity) {
        min_log_level_.store(static_cast<int>(severity), std::memory_order_release);
    }

    namespace log_internal {
        ScopedMinLogLevel::ScopedMinLogLevel(turbo::LogSeverityAtLeast severity)
            : saved_severity_(turbo::min_log_level()) {
            turbo::set_min_log_level(severity);
        }

        ScopedMinLogLevel::~ScopedMinLogLevel() {
            turbo::set_min_log_level(saved_severity_);
        }
    } // namespace log_internal

    turbo::LogSeverityAtLeast stderr_threshold() {
        return static_cast<turbo::LogSeverityAtLeast>(
            stderrthreshold.load(std::memory_order_acquire));
    }

    void set_stderr_threshold(turbo::LogSeverityAtLeast severity) {
        stderrthreshold.store(static_cast<int>(severity), std::memory_order_release);
    }

    ScopedStderrThreshold::ScopedStderrThreshold(turbo::LogSeverityAtLeast severity)
        : saved_severity_(turbo::stderr_threshold()) {
        turbo::set_stderr_threshold(severity);
    }

    ScopedStderrThreshold::~ScopedStderrThreshold() {
        turbo::set_stderr_threshold(saved_severity_);
    }

    namespace log_internal {
        const char *get_android_native_tag() {
            return android_log_tag.load(std::memory_order_acquire);
        }
    } // namespace log_internal

    void set_android_native_tag(const char *tag) {
        KUMO_CONST_INIT static std::atomic<const std::string *> user_log_tag(nullptr);
        TURBO_INTERNAL_CHECK(tag, "tag must be non-null.");

        const std::string *tag_str = new std::string(tag);
        TURBO_INTERNAL_CHECK(
            android_log_tag.exchange(tag_str->c_str(), std::memory_order_acq_rel) ==
            kDefaultAndroidTag,
            "set_android_native_tag() must only be called once per process!");
        user_log_tag.store(tag_str, std::memory_order_relaxed);
    }

    namespace log_internal {
        bool should_log_backtrace_at(std::string_view file, int line) {
            const size_t flag_hash =
                    log_backtrace_at_hash.load(std::memory_order_relaxed);

            return flag_hash != 0 && flag_hash == hash_site_for_log_backtrace_at(file, line);
        }
    } // namespace log_internal

    void set_log_backtrace_location(std::string_view file, int line) {
        log_backtrace_at_hash.store(hash_site_for_log_backtrace_at(file, line),
                                    std::memory_order_relaxed);
    }

    void clear_log_backtrace_location() {
        log_backtrace_at_hash.store(0, std::memory_order_relaxed);
    }

    bool should_prepend_log_prefix() {
        return prepend_log_prefix.load(std::memory_order_acquire);
    }

    void enable_log_prefix(bool on_off) {
        prepend_log_prefix.store(on_off, std::memory_order_release);
    }
} // namespace turbo
