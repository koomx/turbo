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
//
// -----------------------------------------------------------------------------
// File: log/internal/nullstream.h
// -----------------------------------------------------------------------------
//
// Classes `NullStream`, `NullStreamMaybeFatal ` and `NullStreamFatal`
// implement a subset of the `LogMessage` API and are used instead when logging
// of messages has been disabled.

#ifndef TURBO_LOG_INTERNAL_NULLSTREAM_H_
#define TURBO_LOG_INTERNAL_NULLSTREAM_H_

#ifdef _WIN32
#include <cstdlib>
#else
#include <unistd.h>
#endif
#include <ios>
#include <ostream>
#include <turbo/macros/config.h>
#include <turbo/base/log_severity.h>
#include <string_view>
#include <turbo/format/str_format.h>

namespace turbo {
    namespace log_internal {
        // A `NullStream` implements the API of `LogMessage` (a few methods and
        // `operator<<`) but does nothing.  All methods are defined inline so the
        // compiler can eliminate the whole instance and discard anything that's
        // streamed in.
        class NullStream {
        public:
            NullStream &at_location(std::string_view, int) { return *this; }

            template<typename SourceLocationType>
            NullStream &at_location(SourceLocationType) {
                return *this;
            }

            NullStream &no_prefix() { return *this; }
            NullStream &with_verbosity(int) { return *this; }

            template<typename TimeType>
            NullStream &with_timestamp(TimeType) {
                return *this;
            }

            template<typename Tid>
            NullStream &with_thread_id(Tid) {
                return *this;
            }

            template<typename LogEntryType>
            NullStream &with_metadata_from(const LogEntryType &) {
                return *this;
            }

            NullStream &with_perror() { return *this; }

            template<typename LogSinkType>
            NullStream &to_sink_also(LogSinkType *) {
                return *this;
            }

            template<typename LogSinkType>
            NullStream &to_sink_only(LogSinkType *) {
                return *this;
            }

            template<typename LogSinkType>
            NullStream &output_to_sink(LogSinkType *, bool) {
                return *this;
            }

            NullStream &internal_stream() { return *this; }

            // printf-style formatted logging. Uses turbo::str_printf internally, writing
            // the result into the encoded buffer via the StringifySink.
            template <typename... Args>
            NullStream& printf(const turbo::FormatSpec<Args...>& format,
                               const Args&... args) {
                return *this;
            }

            void flush() {
            }
        };


        template<typename T>
        inline NullStream &operator<<(NullStream &str, const T &) {
            return str;
        }

        inline NullStream &operator<<(NullStream & str,
                                      std::ostream & (*)(std::ostream & os)) {
            return str;
        }

        inline NullStream &operator<<(NullStream & str,
                                      std::ios_base & (*)(std::ios_base & os)) {
            return str;
        }

        // `NullStreamMaybeFatal` implements the process termination semantics of
        // `LogMessage`, which is used for `DFATAL` severity and expression-defined
        // severity e.g. `KLOG(LEVEL(HowBadIsIt()))`.  Like `LogMessage`, it terminates
        // the process when destroyed if the passed-in severity equals `FATAL`.
        class NullStreamMaybeFatal final : public NullStream {
        public:
            explicit NullStreamMaybeFatal(turbo::LogSeverity severity)
                : fatal_(severity == turbo::LogSeverity::kFatal) {
            }

            ~NullStreamMaybeFatal() {
                if (fatal_) {
                    _exit(1);
                }
            }

        private:
            bool fatal_;
        };

        // `NullStreamFatal` implements the process termination semantics of
        // `LogMessageFatal`, which means it always terminates the process.  `DFATAL`
        // and expression-defined severity use `NullStreamMaybeFatal` above.
        class NullStreamFatal final : public NullStream {
        public:
            NullStreamFatal() = default;

            [[noreturn]] ~NullStreamFatal() { _exit(1); }
        };
    } // namespace log_internal
} // namespace turbo

#endif  // TURBO_LOG_INTERNAL_NULLSTREAM_H_
