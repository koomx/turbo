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
// File: log/internal/log_message.h
// -----------------------------------------------------------------------------
//
// This file declares `class turbo::log_internal::LogMessage`. This class more or
// less represents a particular log message. LOG/KCHECK macros create a temporary
// instance of `LogMessage` and then stream values to it.  At the end of the
// LOG/KCHECK statement, the LogMessage is voidified by operator&&, and `flush()`
// directs the message to the registered log sinks.  Heap-allocation of
// `LogMessage` is unsupported.  Construction outside of a `LOG` macro is
// unsupported.

#pragma once

#include <wchar.h>

#include <cstddef>
#include <ios>
#include <memory>
#include <ostream>
#include <streambuf>
#include <string>
#include <string_view>
#include <type_traits>
#include <turbo/format/str_format.h>
#include <turbo/macros/config.h>
#include <turbo/base/internal/errno_saver.h>
#include <turbo/base/log_severity.h>
#include <turbo/base/nullability.h>
#include <turbo/log/internal/nullguard.h>
#include <turbo/log/internal/structured_proto.h>
#include <turbo/log/log_entry.h>
#include <turbo/log/log_sink.h>
#include <turbo/format/has_turbo_stringify.h>
#include <string_view>
#include <turbo/time/time.h>
#include <turbo/types/source_location.h>
#include <turbo/types/span.h>

namespace turbo {
    namespace log_internal {
        constexpr int kLogMessageBufferSize = 15000;

        enum class StructuredStringType;

        class LogMessage {
        public:
            struct InfoTag {
            };

            struct WarningTag {
            };

            struct ErrorTag {
            };

            // Used for `LOG`.  Taking `const char *` instead of `string_view` keeps
            // callsites a little bit smaller at the cost of doing `strlen` at runtime.
            LogMessage(const char * turbo_nonnull file, int line,
                       turbo::LogSeverity severity) KUMO_ATTRIBUTE_COLD;

            // Used for FFI integrations that don't have a NUL-terminated string.
            LogMessage(std::string_view file, int line,
                       turbo::LogSeverity severity) KUMO_ATTRIBUTE_COLD;

            // These constructors are slightly smaller/faster to call; the severity is
            // curried into the function pointer.
           KUMO_ATTRIBUTE_NOINLINE LogMessage(const char * turbo_nonnull file, int line,
                       InfoTag) KUMO_ATTRIBUTE_COLD;

           KUMO_ATTRIBUTE_NOINLINE LogMessage(const char * turbo_nonnull file, int line,
                       WarningTag) KUMO_ATTRIBUTE_COLD;

           KUMO_ATTRIBUTE_NOINLINE LogMessage(const char * turbo_nonnull file, int line,
                       ErrorTag) KUMO_ATTRIBUTE_COLD;

            LogMessage(const LogMessage &) = delete;

            LogMessage &operator=(const LogMessage &) = delete;

            ~LogMessage() KUMO_ATTRIBUTE_COLD;

            // Overrides the location inferred from the callsite.  The string pointed to
            // by `file` must be valid until the end of the statement.
            LogMessage &at_location(std::string_view file, int line);

            // `loc` doesn't default to `turbo::SourceLocation::current()` here since the
            // callsite is already the default location for `LOG` statements.
            LogMessage &at_location(turbo::SourceLocation loc) {
                return at_location(loc.file_name(), static_cast<int>(loc.line()));
            }

            // Omits the prefix from this line.  The prefix includes metadata about the
            // logged data such as source code location and timestamp.
            LogMessage &no_prefix();

            // Sets the verbosity field of the logged message as if it was logged by
            // `VKLOG(verbose_level)`.  Unlike `VKLOG`, this method does not affect
            // evaluation of the statement when the specified `verbose_level` has been
            // disabled.  The only effect is on `turbo::LogSink` implementations which
            // make use of the `turbo::LogSink::verbosity()` value.  The value
            // `turbo::LogEntry::kNoVerbosityLevel` can be specified to mark the message
            // not verbose.
            LogMessage &with_verbosity(int verbose_level);

            // Uses the specified timestamp instead of one collected in the constructor.
            LogMessage &with_timestamp(turbo::Time timestamp);

            // Uses the specified thread ID instead of one collected in the constructor.
            LogMessage &with_thread_id(turbo::LogEntry::tid_t tid);

            // Copies all metadata (but no data) from the specified `turbo::LogEntry`.
            LogMessage &with_metadata_from(const turbo::LogEntry &entry);

            // Appends to the logged message a colon, a space, a textual description of
            // the current value of `errno` (as by strerror(3)), and the numerical value
            // of `errno`.
            LogMessage &with_perror();

            // Sends this message to `*sink` in addition to whatever other sinks it would
            // otherwise have been sent to.
            LogMessage &to_sink_also(turbo::LogSink * turbo_nonnull sink);

            // Sends this message to `*sink` and no others.
            LogMessage &to_sink_only(turbo::LogSink * turbo_nonnull sink);

            // Don't call this method from outside this library.
            LogMessage &internal_stream() { return *this; }

            // printf-style formatted logging. Uses turbo::str_printf internally, writing
            // the result into the encoded buffer via the StringifySink.
            template <typename... Args>
            LogMessage& printf(const turbo::FormatSpec<Args...>& format,
                               const Args&... args);

            // By-value overloads for small, common types let us overlook common failures
            // to define globals and static data members (i.e. in a .cc file).
            // NOLINTBEGIN(runtime/int)
            // NOLINTBEGIN(google-runtime-int)
            // clang-format off:  The CUDA toolchain cannot handle these <<<'s
            LogMessage& operator<<(char v) { return operator<< <char>(v); }
            LogMessage& operator<<(signed char v) { return operator<< <signed char>(v); }
            LogMessage& operator<<(unsigned char v) {
                return operator<< <unsigned char>(v);
            }
            LogMessage& operator<<(signed short v) {
                return operator<< <signed short>(v);
            }
            LogMessage& operator<<(signed int v) { return operator<< <signed int>(v); }
            LogMessage& operator<<(signed long v) {
                return operator<< <signed long>(v);
            }
            LogMessage& operator<<(signed long long v) {
                return operator<< <signed long long>(v);
            }
            LogMessage& operator<<(unsigned short v) {
                return operator<< <unsigned short>(v);
            }
            LogMessage& operator<<(unsigned int v) {
                return operator<< <unsigned int>(v);
            }
            LogMessage& operator<<(unsigned long v) {
                return operator<< <unsigned long>(v);
            }
            LogMessage& operator<<(unsigned long long v) {
                return operator<< <unsigned long long>(v);
            }
            LogMessage& operator<<(void* turbo_nullable  v) {
                return operator<< <void*>(v);
            }
            LogMessage& operator<<(const void* turbo_nullable  v) {
                return operator<< <const void*>(v);
            }
            LogMessage& operator<<(float v) { return operator<< <float>(v); }
            LogMessage& operator<<(double v) { return operator<< <double>(v); }
            LogMessage& operator<<(bool v) { return operator<< <bool>(v); }
            // clang-format on
            // NOLINTEND(google-runtime-int)
            // NOLINTEND(runtime/int)

            // These overloads are more efficient since no `ostream` is involved.
            LogMessage &operator<<(const std::string &v);

            LogMessage &operator<<(std::string_view v);

            // Wide string overloads (since std::ostream does not provide them).
            LogMessage &operator<<(const std::wstring &v);

            LogMessage &operator<<(std::wstring_view v);

            // `const wchar_t*` is handled by `operator<< <const wchar_t*>`.
            LogMessage &operator<<(wchar_t * turbo_nullable v);

            LogMessage &operator<<(wchar_t v);

            // Overload for turbo::SourceLocation.
            LogMessage &operator<<(const turbo::SourceLocation &loc) {
                OstreamView view(*data_);
                view.stream() << loc.file_name() << ':' << loc.line();
                return *this;
            }

            // Handle stream manipulators e.g. std::endl.
            LogMessage &operator<<(std::ostream & (*turbo_nonnull m)(std::ostream &os));

            LogMessage &operator<<(std::ios_base & (*turbo_nonnull m)(std::ios_base &os));

            // Literal strings.  This allows us to record C string literals as literals in
            // the logging.proto.Value.
            //
            // Allow this overload to be inlined to prevent generating instantiations of
            // this template for every value of `SIZE` encountered in each source code
            // file. That significantly increases linker input sizes. Inlining is cheap
            // because the argument to this overload is almost always a string literal so
            // the call to `strlen` can be replaced at compile time. The overloads for
            // `char[]`/`wchar_t[]` below should not be inlined. The compiler typically
            // does not have the string at compile time and cannot replace the call to
            // `strlen` so inlining it increases the binary size. See the discussion on
            // cl/107527369.
            template<int SIZE>
            LogMessage &operator<<(const char (&buf)[SIZE]);

            template<int SIZE>
            LogMessage &operator<<(const wchar_t (&buf)[SIZE]);

            // This prevents non-const `char[]` arrays from looking like literals.
            template<int SIZE>
            LogMessage &operator<<(char (&buf)[SIZE]) KUMO_ATTRIBUTE_NOINLINE;

            // `wchar_t[SIZE]` is handled by `operator<< <const wchar_t*>`.

            // Types that support `turbo_stringify()` are serialized that way.
            // Types that don't support `turbo_stringify()` but do support streaming into a
            // `std::ostream&` are serialized that way.
            template<typename T>
            LogMessage &operator<<(const T &v) KUMO_ATTRIBUTE_NOINLINE;

            // Dispatches the completed `turbo::LogEntry` to applicable `turbo::LogSink`s.
            void flush();

            // Note: We explicitly do not support `operator<<` for non-const references
            // because it breaks logging of non-integer bitfield types (i.e., enums).

        protected:
            // Call `abort()` or similar to perform `KLOG(FATAL)` crash.  It is assumed
            // that the caller has already generated and written the trace as appropriate.
            [[noreturn]] static void fail_without_stack_trace();

            // Similar to `fail_without_stack_trace()`, but without `abort()`.  Terminates
            // the process with an error exit code.
            [[noreturn]] static void fail_quietly();

            // After this is called, failures are done as quiet as possible for this log
            // message.
            void set_fail_quietly();

        private:
            struct LogMessageData; // Opaque type containing message state
            friend class AsLiteralImpl;
            friend class StringifySink;
            template<StructuredStringType str_type>
            friend class AsStructuredStringTypeImpl;
            template<typename T>
            friend class AsStructuredValueImpl;

            // This streambuf writes directly into the structured logging buffer so that
            // arbitrary types can be encoded as string data (using
            // `operator<<(std::ostream &, ...)` without any extra allocation or copying.
            // Space is reserved before the data to store the length field, which is
            // filled in by `~OstreamView`.
            class OstreamView final : public std::streambuf {
            public:
                explicit OstreamView(LogMessageData &message_data);

                ~OstreamView() override;

                OstreamView(const OstreamView &) = delete;

                OstreamView &operator=(const OstreamView &) = delete;

                std::ostream &stream();

            private:
                LogMessageData &data_;
                turbo::Span<char> encoded_remaining_copy_;
                turbo::Span<char> message_start_;
                turbo::Span<char> string_start_;
            };

            enum class StringType {
                kLiteral,
                kNotLiteral,
            };

            template<StringType str_type>
            void copy_to_encoded_buffer(std::string_view str) KUMO_ATTRIBUTE_NOINLINE;

            template<StringType str_type>
            void copy_to_encoded_buffer(char ch, size_t num) KUMO_ATTRIBUTE_NOINLINE;

            template<StringType str_type>
            void copy_to_encoded_buffer(std::wstring_view str) KUMO_ATTRIBUTE_NOINLINE;

            // Copies `field` to the encoded buffer, then appends `str` after it
            // (truncating `str` if necessary to fit).
            template<StringType str_type>
            void copy_to_encoded_buffer_with_structured_proto_field(StructuredProtoField field,
                                                                    std::string_view str)

            KUMO_ATTRIBUTE_NOINLINE;

            // Returns `true` if the message is fatal or enabled debug-fatal.
            bool is_fatal() const;

            // Records some tombstone-type data in anticipation of `Die`.
            void prepare_to_die();

            void die();

            void send_to_log();

            // Checks set_log_backtrace_location and appends a backtrace if appropriate.
            void log_backtrace_if_needed();

            // This should be the first data member so that its initializer captures errno
            // before any other initializers alter it (e.g. with calls to new) and so that
            // no other destructors run afterward an alter it (e.g. with calls to delete).
            turbo::base_internal::ErrnoSaver errno_saver_;

            // We keep the data in a separate struct so that each instance of `LogMessage`
            // uses less stack space.
            turbo_nonnull std::unique_ptr<LogMessageData> data_;
        };

        // Explicitly specializes the generic operator<< for `const wchar_t*`
        // arguments.
        //
        // This method is used instead of a non-template `const wchar_t*` overload,
        // as the latter was found to take precedence over the array template
        // (`operator<<(const wchar_t(&)[SIZE])`) when handling string literals.
        // This specialization ensures the array template now correctly processes
        // literals.
        template<>
        LogMessage &LogMessage::operator<<<const wchar_t *>(
            const wchar_t *turbo_nullable const&v);

        inline LogMessage &LogMessage::operator<<(wchar_t * turbo_nullable v) {
            return operator<<(const_cast<const wchar_t *>(v));
        }

        // Helper class so that `turbo_stringify()` can modify the LogMessage.
        class StringifySink final {
        public:
            explicit StringifySink(LogMessage &message) : message_(message) {
            }

            void append(size_t count, char ch) {
                message_.copy_to_encoded_buffer<LogMessage::StringType::kNotLiteral>(ch,
                    count);
            }

            void append(std::string_view v) {
                message_.copy_to_encoded_buffer<LogMessage::StringType::kNotLiteral>(v);
            }

            // For types that implement `turbo_stringify` using `turbo::str_printf_to()`.
            friend void turbo_format_flush(StringifySink * turbo_nonnull sink,
                                         std::string_view v) {
                sink->append(v);
            }

        private:
            LogMessage &message_;
        };

        template <typename... Args>
        LogMessage& LogMessage::printf(
                const turbo::FormatSpec<Args...>& format, const Args&... args) {
            StringifySink sink(*this);
            turbo::str_printf_to(&sink, format, args...);
            return *this;
        }

        // Note: the following is declared `KUMO_ATTRIBUTE_NOINLINE`
        template<typename T>
        LogMessage &LogMessage::operator<<(const T &v) {
            if constexpr (turbo::HasTurboStringify<T>::value) {
                StringifySink sink(*this);
                // Replace with public API.
                turbo_stringify(sink, v);
            } else {
                OstreamView view(*data_);
                view.stream() << log_internal::NullGuard<T>().Guard(v);
            }
            return *this;
        }

        template<int SIZE>
        LogMessage &LogMessage::operator<<(const char (&buf)[SIZE]) {
            copy_to_encoded_buffer<StringType::kLiteral>(buf);
            return *this;
        }

        template<int SIZE>
        LogMessage &LogMessage::operator<<(const wchar_t (&buf)[SIZE]) {
            copy_to_encoded_buffer<StringType::kLiteral>(buf);
            return *this;
        }

        // Note: the following is declared `KUMO_ATTRIBUTE_NOINLINE`
        template<int SIZE>
        LogMessage &LogMessage::operator<<(char (&buf)[SIZE]) {
            copy_to_encoded_buffer<StringType::kNotLiteral>(buf);
            return *this;
        }

        // We instantiate these specializations in the library's TU to save space in
        // other TUs.  Since the template is marked `KUMO_ATTRIBUTE_NOINLINE` we will be
        // emitting a function call either way.
        // NOLINTBEGIN(runtime/int)
        // NOLINTBEGIN(google-runtime-int)
        extern template LogMessage &LogMessage::operator<<(const char &v);

        extern template LogMessage &LogMessage::operator<<(const signed char &v);

        extern template LogMessage &LogMessage::operator<<(const unsigned char &v);

        extern template LogMessage &LogMessage::operator<<(const short &v);

        extern template LogMessage &LogMessage::operator<<(const unsigned short &v);

        extern template LogMessage &LogMessage::operator<<(const int &v);

        extern template LogMessage &LogMessage::operator<<(const unsigned int &v);

        extern template LogMessage &LogMessage::operator<<(const long &v);

        extern template LogMessage &LogMessage::operator<<(const unsigned long &v);

        extern template LogMessage &LogMessage::operator<<(const long long &v);

        extern template LogMessage &LogMessage::operator<<(const unsigned long long &v);

        extern template LogMessage &LogMessage::operator<<(
            void *turbo_nullable const&v);

        extern template LogMessage &LogMessage::operator<<(
            const void *turbo_nullable const&v);

        extern template LogMessage &LogMessage::operator<<(const float &v);

        extern template LogMessage &LogMessage::operator<<(const double &v);

        extern template LogMessage &LogMessage::operator<<(const bool &v);

        // NOLINTEND(google-runtime-int)
        // NOLINTEND(runtime/int)

        extern template void LogMessage::copy_to_encoded_buffer<LogMessage::StringType::kLiteral>(std::string_view str);

        extern template void LogMessage::copy_to_encoded_buffer<LogMessage::StringType::kNotLiteral>(std::string_view str);

        extern template void
        LogMessage::copy_to_encoded_buffer<LogMessage::StringType::kLiteral>(char ch,
                                                                             size_t num);

        extern template void LogMessage::copy_to_encoded_buffer<
            LogMessage::StringType::kNotLiteral>(char ch, size_t num);

        extern template void LogMessage::copy_to_encoded_buffer<
            LogMessage::StringType::kLiteral>(std::wstring_view str);

        extern template void LogMessage::copy_to_encoded_buffer<
            LogMessage::StringType::kNotLiteral>(std::wstring_view str);

        // `LogMessageFatal` ensures the process will exit in failure after logging this
        // message.
        class LogMessageFatal final : public LogMessage {
        public:
            LogMessageFatal(const char * turbo_nonnull file, int line) KUMO_ATTRIBUTE_COLD;

            LogMessageFatal(const char * turbo_nonnull file, int line,
                            const char * turbo_nonnull failure_msg) KUMO_ATTRIBUTE_COLD;

            [[noreturn]] ~LogMessageFatal();
        };

        // `LogMessageDebugFatal` ensures the process will exit in failure after logging
        // this message. It matches LogMessageFatal but is not [[noreturn]] as it's used
        // for DKLOG(FATAL) variants.
        class LogMessageDebugFatal final : public LogMessage {
        public:
            LogMessageDebugFatal(const char * turbo_nonnull file,
                                 int line) KUMO_ATTRIBUTE_COLD;

            ~LogMessageDebugFatal();
        };

        class LogMessageQuietlyDebugFatal final : public LogMessage {
        public:
            // DKLOG(QFATAL) calls this instead of LogMessageQuietlyFatal to make sure the
            // destructor is not [[noreturn]] even if this is always FATAL as this is only
            // invoked when DKLOG() is enabled.
            LogMessageQuietlyDebugFatal(const char * turbo_nonnull file,
                                        int line) KUMO_ATTRIBUTE_COLD;

            ~LogMessageQuietlyDebugFatal();
        };

        // Used for KLOG(QFATAL) to make sure it's properly understood as [[noreturn]].
        class LogMessageQuietlyFatal final : public LogMessage {
        public:
            LogMessageQuietlyFatal(const char * turbo_nonnull file,
                                   int line) KUMO_ATTRIBUTE_COLD;

            LogMessageQuietlyFatal(const char * turbo_nonnull file, int line,
                                   const char * turbo_nonnull failure_msg)

            KUMO_ATTRIBUTE_COLD;

            [[noreturn]] ~LogMessageQuietlyFatal();
        };
    } // namespace log_internal
} // namespace turbo

extern "C" KUMO_ATTRIBUTE_WEAK void TURBO_INTERNAL_C_SYMBOL(
    TurboInternalOnFatalLogMessage)(
    const turbo::LogEntry &
);
