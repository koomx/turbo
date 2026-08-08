//
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

#include <turbo/log/internal/log_sink_set.h>

#if !KUMO_HAVE_THREAD_LOCAL
#include <pthread.h>
#endif

#ifdef __ANDROID__
#include <android/log.h>
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#include <algorithm>
#include <vector>

#include <turbo/base/call_once.h>
#include <turbo/macros/config.h>
#include <turbo/base/internal/raw_logging.h>
#include <turbo/base/log_severity.h>
#include <turbo/base/no_destructor.h>
#include <turbo/base/thread_annotations.h>
#include <turbo/cleanup/cleanup.h>
#include <turbo/log/globals.h>
#include <turbo/log/internal/config.h>
#include <turbo/log/internal/globals.h>
#include <turbo/log/log_entry.h>
#include <turbo/log/log_sink.h>
#include <string_view>
#include <mutex>
#include <shared_mutex>
#include <turbo/types/span.h>

namespace turbo {
    namespace log_internal {
        namespace {
            // Returns a mutable reference to a thread-local variable that should be true if
            // a globally-registered `LogSink`'s `send()` is currently being invoked on this
            // thread.
            bool &thread_is_logging_status() {
#if KUMO_HAVE_THREAD_LOCAL
  KUMO_CONST_INIT thread_local bool thread_is_logging = false;
                return thread_is_logging;
#else
                KUMO_CONST_INIT static pthread_key_t thread_is_logging_key;
                static const bool unused = [] {
                    if (pthread_key_create(&thread_is_logging_key, [](void *data) {
                        delete reinterpret_cast<bool *>(data);
                    })) {
                        perror("pthread_key_create failed!");
                        abort();
                    }
                    return true;
                }();
                (void) unused; // Fixes -wunused-variable warning
                bool *thread_is_logging_ptr =
                        reinterpret_cast<bool *>(pthread_getspecific(thread_is_logging_key));

                if (KUMO_UNLIKELY(!thread_is_logging_ptr)) {
                    thread_is_logging_ptr = new bool{false};
                    if (pthread_setspecific(thread_is_logging_key, thread_is_logging_ptr)) {
                        perror("pthread_setspecific failed");
                        abort();
                    }
                }
                return *thread_is_logging_ptr;
#endif
            }

            class StderrLogSink final : public LogSink {
            public:
                ~StderrLogSink() override = default;

                void send(const turbo::LogEntry &entry) override {
                    if (entry.log_severity() < turbo::stderr_threshold() &&
                        turbo::log_internal::is_initialized()) {
                        return;
                    }

                    KUMO_CONST_INIT static turbo::once_flag warn_if_not_initialized;
                    turbo::call_once(warn_if_not_initialized, []() {
                        if (turbo::log_internal::is_initialized()) return;
                        const char w[] =
                                "WARNING: All log messages before turbo::initialize_log() is called"
                                " are written to STDERR\n";
                        turbo::log_internal::write_to_stderr(w, turbo::LogSeverity::kWarning);
                    });

                    if (!entry.stacktrace().empty()) {
                        turbo::log_internal::write_to_stderr(entry.stacktrace(),
                                                           entry.log_severity());
                    } else {
                        // TODO(b/226937039): do this outside else condition once we avoid
                        // ReprintFatalMessage
                        turbo::log_internal::write_to_stderr(
                            entry.text_message_with_prefix_and_newline(), entry.log_severity());
                    }
                }
            };

#if defined(__ANDROID__)
            class AndroidLogSink final : public LogSink {
            public:
                ~AndroidLogSink() override = default;

                void send(const turbo::LogEntry &entry) override {
                    const int level = android_log_level(entry);
                    const char *const tag = get_android_native_tag();
                    __android_log_write(level, tag,
                                        entry.text_message_with_prefix_and_newline_c_str());
                    if (entry.log_severity() == turbo::LogSeverity::kFatal)
                        __android_log_write(ANDROID_LOG_FATAL, tag, "terminating.\n");
                }

            private:
                static int android_log_level(const turbo::LogEntry &entry) {
                    switch (entry.log_severity()) {
                        case turbo::LogSeverity::kFatal:
                            return ANDROID_LOG_FATAL;
                        case turbo::LogSeverity::kError:
                            return ANDROID_LOG_ERROR;
                        case turbo::LogSeverity::kWarning:
                            return ANDROID_LOG_WARN;
                        case turbo::LogSeverity::kTrace:
                            return ANDROID_LOG_VERBOSE;
                        case turbo::LogSeverity::kDebug:
                            return ANDROID_LOG_DEBUG;
                        default:
                            if (entry.verbosity() >= 2) return ANDROID_LOG_VERBOSE;
                            if (entry.verbosity() == 1) return ANDROID_LOG_DEBUG;
                            return ANDROID_LOG_INFO;
                    }
                }
            };
#endif  // !defined(__ANDROID__)

#if defined(_WIN32)
            class WindowsDebuggerLogSink final : public LogSink {
            public:
                ~WindowsDebuggerLogSink() override = default;

                void send(const turbo::LogEntry &entry) override {
                    if (entry.log_severity() < turbo::stderr_threshold() &&
                        turbo::log_internal::is_initialized()) {
                        return;
                    }
                    ::OutputDebugStringA(entry.text_message_with_prefix_and_newline_c_str());
                }
            };
#endif  // !defined(_WIN32)

            class GlobalLogSinkSet final {
            public:
                GlobalLogSinkSet() {
#if defined(__myriad2__) || defined(__Fuchsia__)
                    // myriad2 and Fuchsia do not log to stderr by default.
#else
                    static turbo::NoDestructor<StderrLogSink> stderr_log_sink;
                    add_log_sink(stderr_log_sink.get());
#endif
#ifdef __ANDROID__
                    static turbo::NoDestructor<AndroidLogSink> android_log_sink;
                    add_log_sink(android_log_sink.get());
#endif
#if defined(_WIN32)
                    static turbo::NoDestructor<WindowsDebuggerLogSink> debugger_log_sink;
                    add_log_sink(debugger_log_sink.get());
#endif  // !defined(_WIN32)
                }

                void log_to_sinks(const turbo::LogEntry &entry,
                                turbo::Span<turbo::LogSink *> extra_sinks, bool extra_sinks_only)

                TURBO_LOCKS_EXCLUDED (guard_) {
                    send_to_sinks(entry, extra_sinks);

                    if (!extra_sinks_only) {
                        if (thread_is_logging_to_log_sink()) {
                            turbo::log_internal::write_to_stderr(
                                entry.text_message_with_prefix_and_newline(), entry.log_severity());
                        } else {
                            // Shared lock: multiple threads may dispatch concurrently (Abseil used
                            // ReaderMutexLock). Exclusive std::mutex deadlocks tests that barrier
                            // inside send across threads.
                            std::shared_lock<std::shared_mutex> global_sinks_lock(guard_);
                            thread_is_logging_status() = true;
                            // Ensure the "thread is logging" status is reverted upon leaving the
                            // scope even in case of exceptions.
                            auto status_cleanup =
                                    turbo::MakeCleanup([] { thread_is_logging_status() = false; });
                            send_to_sinks(entry, turbo::make_span(sinks_));
                        }
                    }
                }

                void add_log_sink(turbo::LogSink *sink) TURBO_LOCKS_EXCLUDED

                (guard_) {
                    {
                        std::unique_lock<std::shared_mutex> global_sinks_lock(guard_);
                        auto pos = std::find(sinks_.begin(), sinks_.end(), sink);
                        if (pos == sinks_.end()) {
                            sinks_.push_back(sink);
                            return;
                        }
                    }
                    TURBO_INTERNAL_LOG(FATAL, "Duplicate log sinks are not supported");
                }

                void remove_log_sink(turbo::LogSink *sink) TURBO_LOCKS_EXCLUDED

                (guard_) {
                    {
                        std::unique_lock<std::shared_mutex> global_sinks_lock(guard_);
                        auto pos = std::find(sinks_.begin(), sinks_.end(), sink);
                        if (pos != sinks_.end()) {
                            sinks_.erase(pos);
                            return;
                        }
                    }
                    TURBO_INTERNAL_LOG(FATAL, "Mismatched log sink being removed");
                }

                void flush_log_sinks() TURBO_LOCKS_EXCLUDED

                (guard_) {
                    if (thread_is_logging_to_log_sink()) {
                        // The thread_local condition demonstrates that we're already holding the
                        // lock in order to iterate over `sinks_` for dispatch.
                        flush_log_sinks_locked();
                    } else {
                        std::shared_lock<std::shared_mutex> global_sinks_lock(guard_);
                        // In case if LogSink::flush overload decides to log
                        thread_is_logging_status() = true;
                        // Ensure the "thread is logging" status is reverted upon leaving the
                        // scope even in case of exceptions.
                        auto status_cleanup =
                                turbo::MakeCleanup([] { thread_is_logging_status() = false; });
                        flush_log_sinks_locked();
                    }
                }

            private:
                void flush_log_sinks_locked() TURBO_SHARED_LOCKS_REQUIRED

                (guard_) {
                    for (turbo::LogSink *sink: sinks_) {
                        sink->flush();
                    }
                }

                // Helper routine for log_to_sinks.
                static void send_to_sinks(const turbo::LogEntry &entry,
                                        turbo::Span<turbo::LogSink *> sinks) {
                    for (turbo::LogSink *sink: sinks) {
                        sink->send(entry);
                    }
                }

                using LogSinksSet = std::vector<turbo::LogSink *>;
                std::shared_mutex guard_;

                LogSinksSet sinks_ TURBO_GUARDED_BY(guard_);
            };

            // Returns reference to the global LogSinks set.
            GlobalLogSinkSet &GlobalSinks() {
                static turbo::NoDestructor<GlobalLogSinkSet> global_sinks;
                return *global_sinks;
            }
        } // namespace

        bool thread_is_logging_to_log_sink() { return thread_is_logging_status(); }

        void log_to_sinks(const turbo::LogEntry &entry,
                        turbo::Span<turbo::LogSink *> extra_sinks, bool extra_sinks_only) {
            log_internal::GlobalSinks().log_to_sinks(entry, extra_sinks, extra_sinks_only);
        }

        void add_log_sink(turbo::LogSink *sink) {
            log_internal::GlobalSinks().add_log_sink(sink);
        }

        void remove_log_sink(turbo::LogSink *sink) {
            log_internal::GlobalSinks().remove_log_sink(sink);
        }

        void flush_log_sinks() { log_internal::GlobalSinks().flush_log_sinks(); }
    } // namespace log_internal
} // namespace turbo
