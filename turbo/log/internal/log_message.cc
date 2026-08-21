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

#include <turbo/log/internal/log_message.h>

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
#include <unistd.h>
#endif

#include <algorithm>
#include <array>
#include <atomic>
#include <ios>
#include <memory>
#include <ostream>
#include <string>
#include <string_view>

#include <turbo/macros/config.h>
#include <turbo/base/internal/raw_logging.h>
#include <turbo/platform/strerror.h>
#include <turbo/base/internal/sysinfo.h>
#include <turbo/base/log_severity.h>
#include <turbo/base/nullability.h>
#include <turbo/types/inlined_vector.h>
#include <turbo/debugging/internal/examine_stack.h>
#include <turbo/log/globals.h>
#include <turbo/log/internal/append_truncated.h>
#include <turbo/log/internal/globals.h>
#include <turbo/log/internal/log_format.h>
#include <turbo/log/internal/log_sink_set.h>
#include <turbo/log/internal/nullguard.h>
#include <turbo/log/internal/proto.h>
#include <turbo/log/internal/structured_proto.h>
#include <turbo/log/log_entry.h>
#include <turbo/log/log_sink.h>
#include <turbo/log/log_sink_registry.h>
#include <turbo/format/internal/utf8.h>
#include <string_view>
#include <turbo/time/clock.h>
#include <turbo/time/time.h>
#include <turbo/types/span.h>

extern "C" KUMO_ATTRIBUTE_WEAK void TURBO_INTERNAL_C_SYMBOL(
            TurboInternalOnFatalLogMessage)(
const turbo::LogEntry &
)
 {
  // Default - Do nothing
}

namespace turbo {
    namespace log_internal {
        namespace {
            // message `logging.proto.Event`
            enum EventTag : uint8_t {
                kFileName = 2,
                kFileLine = 3,
                kTimeNsecs = 4,
                kSeverity = 5,
                kThreadId = 6,
                kValue = 7,
                kSequenceNumber = 9,
                kThreadName = 10,
            };

            // message `logging.proto.Value`
            enum ValueTag : uint8_t {
                kString = 1,
                kStringLiteral = 6,
            };

            // Decodes a `logging.proto.Value` from `buf` and writes a string representation
            // into `dst`.  The string representation will be truncated if `dst` is not
            // large enough to hold it.  Returns false if `dst` has size zero or one (i.e.
            // sufficient only for a nul-terminator) and no decoded data could be written.
            // This function may or may not write a nul-terminator into `dst`, and it may or
            // may not truncate the data it writes in order to do make space for that nul
            // terminator.  In any case, `dst` will be advanced to point at the byte where
            // subsequent writes should begin.
            bool PrintValue(turbo::Span<char> &dst, turbo::Span<const char> buf) {
                if (dst.size() <= 1) return false;
                ProtoField field;
                while (field.DecodeFrom(&buf)) {
                    switch (field.tag()) {
                        case ValueTag::kString:
                        case ValueTag::kStringLiteral:
                            if (field.type() == WireType::kLengthDelimited)
                                if (log_internal::append_truncated(field.string_value(), dst) <
                                    field.string_value().size())
                                    return false;
                    }
                }
                return true;
            }

            // See `logging.proto.Severity`
            int32_t proto_severity(turbo::LogSeverity severity, int verbose_level) {
                switch (severity) {
                    case turbo::LogSeverity::kTrace:
                        return 500;
                    case turbo::LogSeverity::kDebug:
                        return 700;
                    case turbo::LogSeverity::kInfo:
                        if (verbose_level == turbo::LogEntry::kNoVerbosityLevel) return 800;
                        return 600 - verbose_level;
                    case turbo::LogSeverity::kWarning:
                        return 900;
                    case turbo::LogSeverity::kError:
                        return 950;
                    case turbo::LogSeverity::kFatal:
                        return 1100;
                    default:
                        return 800;
                }
            }

            std::string_view Basename(std::string_view filepath) {
#ifdef _WIN32
                size_t path = filepath.find_last_of("/\\");
#else
                size_t path = filepath.find_last_of('/');
#endif
                if (path != filepath.npos) filepath.remove_prefix(path + 1);
                return filepath;
            }

            void WriteToString(const char *data, void *str) {
                reinterpret_cast<std::string *>(str)->append(data);
            }

            void WriteToStream(const char *data, void *os) {
                auto *cast_os = static_cast<std::ostream *>(os);
                *cast_os << data;
            }
        } // namespace

        struct LogMessage::LogMessageData final {
            LogMessageData(std::string_view file, int line,
                           turbo::LogSeverity severity, turbo::Time timestamp);

            LogMessageData(const LogMessageData &) = delete;

            LogMessageData &operator=(const LogMessageData &) = delete;

            // `LogEntry` sent to `LogSink`s; contains metadata.
            turbo::LogEntry entry;

            // true => this was first fatal msg
            bool first_fatal;
            // true => all failures should be quiet
            bool fail_quietly;
            // true => PLOG was requested
            bool is_perror;

            // Extra `LogSink`s to log to, in addition to `global_sinks`.
            turbo::InlinedVector<turbo::LogSink * turbo_nonnull, 16> extra_sinks;
            // If true, log to `extra_sinks` but not to `global_sinks` or hardcoded
            // non-sink targets (e.g. stderr, log files).
            bool extra_sinks_only;

            std::ostream manipulated; // ostream with IO manipulators applied

            // A `logging.proto.Event` proto message is built into `encoded_buf`.
            std::array<char, kLogMessageBufferSize> encoded_buf;
            // `encoded_remaining()` is the suffix of `encoded_buf` that has not been
            // filled yet.  If a datum to be encoded does not fit into
            // `encoded_remaining()` and cannot be truncated to fit, the size of
            // `encoded_remaining()` will be zeroed to prevent encoding of any further
            // data.  Note that in this case its `data()` pointer will not point past the
            // end of `encoded_buf`.
            // The first use of `encoded_remaining()` is our chance to record metadata
            // after any modifications (e.g. by `at_location()`) but before any data have
            // been recorded.  We want to record metadata before data so that data are
            // preferentially truncated if we run out of buffer.
            turbo::Span<char> &encoded_remaining() {
                if (encoded_remaining_actual_do_not_use_directly.data() == nullptr) {
                    encoded_remaining_actual_do_not_use_directly =
                            turbo::make_span(encoded_buf);
                    initialize_encoding_and_format();
                }
                return encoded_remaining_actual_do_not_use_directly;
            }

            turbo::Span<char> encoded_remaining_actual_do_not_use_directly;

            // A formatted string message is built in `string_buf`.
            std::array<char, kLogMessageBufferSize> string_buf;

            void initialize_encoding_and_format();

            void finalize_encoding_and_format();
        };

        LogMessage::LogMessageData::LogMessageData(std::string_view file,
                                                   int line, turbo::LogSeverity severity,
                                                   turbo::Time timestamp)
            : extra_sinks_only(false), manipulated(nullptr) {
            // Legacy defaults for LOG's ostream:
            manipulated.setf(std::ios_base::showbase | std::ios_base::boolalpha);
            entry.full_filename_ = file;
            entry.base_filename_ = Basename(file);
            entry.line_ = line;
            entry.prefix_ = turbo::should_prepend_log_prefix();
            entry.severity_ = turbo::NormalizeLogSeverity(severity);
            entry.verbose_level_ = turbo::LogEntry::kNoVerbosityLevel;
            entry.timestamp_ = timestamp;
            entry.tid_ = turbo::base_internal::get_cached_tid();
        }

        void LogMessage::LogMessageData::initialize_encoding_and_format() {
            EncodeStringTruncate(EventTag::kFileName, entry.source_filename(),
                                 &encoded_remaining());
            EncodeVarint(EventTag::kFileLine, entry.source_line(), &encoded_remaining());
            EncodeVarint(EventTag::kTimeNsecs, turbo::ToUnixNanos(entry.timestamp()),
                         &encoded_remaining());
            EncodeVarint(EventTag::kSeverity,
                         proto_severity(entry.log_severity(), entry.verbosity()),
                         &encoded_remaining());
            EncodeVarint(EventTag::kThreadId, static_cast<uint64_t>(entry.tid()),
                         &encoded_remaining());
        }

        void LogMessage::LogMessageData::finalize_encoding_and_format() {
            // Note that `encoded_remaining()` may have zero size without pointing past
            // the end of `encoded_buf`, so the difference between `data()` pointers is
            // used to compute the size of `encoded_data`.
            turbo::Span<const char> encoded_data(
                encoded_buf.data(),
                static_cast<size_t>(encoded_remaining().data() - encoded_buf.data()));
            // `string_remaining` is the suffix of `string_buf` that has not been filled
            // yet.
            turbo::Span<char> string_remaining(string_buf);
            // We may need to write a newline and nul-terminator at the end of the decoded
            // string data.  Rather than worry about whether those should overwrite the
            // end of the string (if the buffer is full) or be appended, we avoid writing
            // into the last two bytes so we always have space to append.
            string_remaining.remove_suffix(2);
            entry.prefix_len_ =
                    entry.prefix()
                        ? log_internal::format_log_prefix(
                            entry.log_severity(), entry.timestamp(), entry.tid(),
                            entry.source_basename(), entry.source_line(),
                            log_internal::thread_is_logging_to_log_sink()
                                ? PrefixFormat::kRaw
                                : PrefixFormat::kNotRaw,
                            string_remaining)
                        : 0;
            // Decode data from `encoded_buf` until we run out of data or we run out of
            // `string_remaining`.
            ProtoField field;
            while (field.DecodeFrom(&encoded_data)) {
                switch (field.tag()) {
                    case EventTag::kValue:
                        if (field.type() != WireType::kLengthDelimited) continue;
                        if (PrintValue(string_remaining, field.bytes_value())) continue;
                        break;
                }
            }
            auto chars_written =
                    static_cast<size_t>(string_remaining.data() - string_buf.data());
            string_buf[chars_written++] = '\n';
            string_buf[chars_written++] = '\0';
            entry.text_message_with_prefix_and_newline_and_nul_ =
                    turbo::make_span(string_buf).subspan(0, chars_written);
        }

        LogMessage::LogMessage(const char * turbo_nonnull file, int line,
                               turbo::LogSeverity severity)
            : LogMessage(std::string_view(file), line, severity) {
        }

        LogMessage::LogMessage(std::string_view file, int line,
                               turbo::LogSeverity severity)
            : data_(
                std::make_unique < LogMessageData > (file, line, severity, turbo::Now())) {
            data_->first_fatal = false;
            data_->is_perror = false;
            data_->fail_quietly = false;

            // This logs a backtrace even if the location is subsequently changed using
            // at_location.  This quirk, and the behavior when at_location is called twice,
            // are fixable but probably not worth fixing.
            log_backtrace_if_needed();
        }

        LogMessage::LogMessage(const char * turbo_nonnull file, int line, InfoTag)
            : LogMessage(file, line, turbo::LogSeverity::kInfo) {
        }

        LogMessage::LogMessage(const char * turbo_nonnull file, int line, WarningTag)
            : LogMessage(file, line, turbo::LogSeverity::kWarning) {
        }

        LogMessage::LogMessage(const char * turbo_nonnull file, int line, ErrorTag)
            : LogMessage(file, line, turbo::LogSeverity::kError) {
        }

        // This cannot go in the header since LogMessageData is defined in this file.
        LogMessage::~LogMessage() = default;

        LogMessage &LogMessage::at_location(std::string_view file, int line) {
            data_->entry.full_filename_ = file;
            data_->entry.base_filename_ = Basename(file);
            data_->entry.line_ = line;
            log_backtrace_if_needed();
            return *this;
        }

        LogMessage &LogMessage::no_prefix() {
            data_->entry.prefix_ = false;
            return *this;
        }

        LogMessage &LogMessage::with_verbosity(int verbose_level) {
            if (verbose_level == turbo::LogEntry::kNoVerbosityLevel) {
                data_->entry.verbose_level_ = turbo::LogEntry::kNoVerbosityLevel;
            } else {
                data_->entry.verbose_level_ = std::max(0, verbose_level);
            }
            return *this;
        }

        LogMessage &LogMessage::with_timestamp(turbo::Time timestamp) {
            data_->entry.timestamp_ = timestamp;
            return *this;
        }

        LogMessage &LogMessage::with_thread_id(turbo::LogEntry::tid_t tid) {
            data_->entry.tid_ = tid;
            return *this;
        }

        LogMessage &LogMessage::with_metadata_from(const turbo::LogEntry &entry) {
            data_->entry.full_filename_ = entry.full_filename_;
            data_->entry.base_filename_ = entry.base_filename_;
            data_->entry.line_ = entry.line_;
            data_->entry.prefix_ = entry.prefix_;
            data_->entry.severity_ = entry.severity_;
            data_->entry.verbose_level_ = entry.verbose_level_;
            data_->entry.timestamp_ = entry.timestamp_;
            data_->entry.tid_ = entry.tid_;
            return *this;
        }

        LogMessage &LogMessage::with_perror() {
            data_->is_perror = true;
            return *this;
        }

        LogMessage &LogMessage::to_sink_also(turbo::LogSink * turbo_nonnull sink) {
            TURBO_INTERNAL_CHECK(sink, "null LogSink*");
            data_->extra_sinks.push_back(sink);
            return *this;
        }

        LogMessage &LogMessage::to_sink_only(turbo::LogSink * turbo_nonnull sink) {
            TURBO_INTERNAL_CHECK(sink, "null LogSink*");
            data_->extra_sinks.clear();
            data_->extra_sinks.push_back(sink);
            data_->extra_sinks_only = true;
            return *this;
        }

#ifdef __ELF__
        extern "C" void __gcov_dump() KUMO_ATTRIBUTE_WEAK;
        extern "C" void __gcov_flush() KUMO_ATTRIBUTE_WEAK;
#endif

        void LogMessage::fail_without_stack_trace() {
            // Now suppress repeated trace logging:
            log_internal::set_suppress_sigabort_trace(true);
#if defined _DEBUG && defined COMPILER_MSVC
            // When debugging on windows, avoid the obnoxious dialog.
            __debugbreak();
#endif

#ifdef __ELF__
            // For b/8737634, flush coverage if we are in coverage mode.
            if (&__gcov_dump != nullptr) {
                __gcov_dump();
            } else if (&__gcov_flush != nullptr) {
                __gcov_flush();
            }
#endif

            abort();
        }

        void LogMessage::fail_quietly() {
            // _exit. Calling abort() would trigger all sorts of death signal handlers
            // and a detailed stack trace. Calling exit() would trigger the onexit
            // handlers, including the heap-leak checker, which is guaranteed to fail in
            // this case: we probably just new'ed the std::string that we logged.
            // Anyway, if you're calling Fail or fail_quietly, you're trying to bail out
            // of the program quickly, and it doesn't make much sense for fail_quietly to
            // offer different guarantees about exit behavior than Fail does. (And as a
            // consequence for QKCHECK and KCHECK to offer different exit behaviors)
            _exit(1);
        }

        LogMessage &LogMessage::operator<<(const std::string &v) {
            copy_to_encoded_buffer<StringType::kNotLiteral>(v);
            return *this;
        }

        LogMessage &LogMessage::operator<<(std::string_view v) {
            copy_to_encoded_buffer<StringType::kNotLiteral>(v);
            return *this;
        }

        LogMessage &LogMessage::operator<<(const std::wstring &v) {
            copy_to_encoded_buffer<StringType::kNotLiteral>(v);
            return *this;
        }

        LogMessage &LogMessage::operator<<(std::wstring_view v) {
            copy_to_encoded_buffer<StringType::kNotLiteral>(v);
            return *this;
        }

        template<>
        LogMessage &LogMessage::operator<<<const wchar_t *>(
            const wchar_t *turbo_nullable const& v) {
            if (v == nullptr) {
                copy_to_encoded_buffer<StringType::kNotLiteral>(
                    std::string_view(kCharNull.data(), kCharNull.size() - 1));
            } else {
                copy_to_encoded_buffer<StringType::kNotLiteral>(v);
            }
            return *this;
        }

        LogMessage &LogMessage::operator<<(wchar_t v) {
            copy_to_encoded_buffer<StringType::kNotLiteral>(std::wstring_view(&v, 1));
            return *this;
        }

        LogMessage &LogMessage::operator<<(std::ostream & (*m)(std::ostream & os)) {
            OstreamView view(*data_);
            data_->manipulated << m;
            return *this;
        }

        LogMessage &LogMessage::operator<<(std::ios_base & (*m)(std::ios_base & os)) {
            OstreamView view(*data_);
            data_->manipulated << m;
            return *this;
        }

        // NOLINTBEGIN(runtime/int)
        // NOLINTBEGIN(google-runtime-int)
        template LogMessage &LogMessage::operator<<(const char &v);

        template LogMessage &LogMessage::operator<<(const signed char &v);

        template LogMessage &LogMessage::operator<<(const unsigned char &v);

        template LogMessage &LogMessage::operator<<(const short &v);

        template LogMessage &LogMessage::operator<<(const unsigned short &v);

        template LogMessage &LogMessage::operator<<(const int &v);

        template LogMessage &LogMessage::operator<<(const unsigned int &v);

        template LogMessage &LogMessage::operator<<(const long &v);

        template LogMessage &LogMessage::operator<<(const unsigned long &v);

        template LogMessage &LogMessage::operator<<(const long long &v);

        template LogMessage &LogMessage::operator<<(const unsigned long long &v);

        template LogMessage &LogMessage::operator<<(void *const&v);

        template LogMessage &LogMessage::operator<<(const void *const&v);

        template LogMessage &LogMessage::operator<<(const float &v);

        template LogMessage &LogMessage::operator<<(const double &v);

        template LogMessage &LogMessage::operator<<(const bool &v);

        // NOLINTEND(google-runtime-int)
        // NOLINTEND(runtime/int)

        void LogMessage::flush() {
            if (data_->entry.log_severity() < turbo::min_log_level()) return;

            if (data_->is_perror) {
                internal_stream() << ": " << turbo::str_error(errno_saver_())
                        << " [" << errno_saver_() << "]";
            }

            // Have we already seen a fatal message?
            KUMO_CONST_INIT static std::atomic<bool> seen_fatal(false);
            if (data_->entry.log_severity() == turbo::LogSeverity::kFatal &&
                turbo::log_internal::exit_on_dfatal()) {
                // Exactly one KLOG(FATAL) message is responsible for aborting the process,
                // even if multiple threads KLOG(FATAL) concurrently.
                bool expected_seen_fatal = false;
                if (seen_fatal.compare_exchange_strong(expected_seen_fatal, true,
                                                       std::memory_order_relaxed)) {
                    data_->first_fatal = true;
                }
            }

            data_->finalize_encoding_and_format();
            data_->entry.encoding_ =
                    std::string_view(data_->encoded_buf.data(),
                                     static_cast<size_t>(data_->encoded_remaining().data() -
                                                         data_->encoded_buf.data()));
            send_to_log();
        }

        void LogMessage::set_fail_quietly() { data_->fail_quietly = true; }

        LogMessage::OstreamView::OstreamView(LogMessageData &message_data)
            : data_(message_data), encoded_remaining_copy_(data_.encoded_remaining()) {
            // This constructor sets the `streambuf` up so that streaming into an attached
            // ostream encodes string data in-place.  To do that, we write appropriate
            // headers into the buffer using a copy of the buffer view so that we can
            // decide not to keep them later if nothing is ever streamed in.  We don't
            // know how much data we'll get, but we can use the size of the remaining
            // buffer as an upper bound and fill in the right size once we know it.
            message_start_ =
                    EncodeMessageStart(EventTag::kValue, encoded_remaining_copy_.size(),
                                       &encoded_remaining_copy_);
            string_start_ =
                    EncodeMessageStart(ValueTag::kString, encoded_remaining_copy_.size(),
                                       &encoded_remaining_copy_);
            setp(encoded_remaining_copy_.data(),
                 encoded_remaining_copy_.data() + encoded_remaining_copy_.size());
            data_.manipulated.rdbuf(this);
        }

        LogMessage::OstreamView::~OstreamView() {
            data_.manipulated.rdbuf(nullptr);
            if (!string_start_.data()) {
                // The second field header didn't fit.  Whether the first one did or not, we
                // shouldn't commit `encoded_remaining_copy_`, and we also need to zero the
                // size of `data_->encoded_remaining()` so that no more data are encoded.
                data_.encoded_remaining().remove_suffix(data_.encoded_remaining().size());
                return;
            }
            const turbo::Span<const char> contents(pbase(),
                                                   static_cast<size_t>(pptr() - pbase()));
            if (contents.empty()) return;
            encoded_remaining_copy_.remove_prefix(contents.size());
            EncodeMessageLength(string_start_, &encoded_remaining_copy_);
            EncodeMessageLength(message_start_, &encoded_remaining_copy_);
            data_.encoded_remaining() = encoded_remaining_copy_;
        }

        std::ostream &LogMessage::OstreamView::stream() { return data_.manipulated; }

        bool LogMessage::is_fatal() const {
            return data_->entry.log_severity() == turbo::LogSeverity::kFatal &&
                   turbo::log_internal::exit_on_dfatal();
        }

        void LogMessage::prepare_to_die() {
            // If we log a FATAL message, flush all the log destinations, then toss
            // a signal for others to catch. We leave the logs in a state that
            // someone else can use them (as long as they flush afterwards)
            if (data_->first_fatal) {
                // Notify observers about the upcoming fatal error.
                TURBO_INTERNAL_C_SYMBOL(TurboInternalOnFatalLogMessage)(data_->entry);
            }

            if (!data_->fail_quietly) {
                // Log the message first before we start collecting stack trace.
                log_internal::log_to_sinks(data_->entry, turbo::make_span(data_->extra_sinks),
                                         data_->extra_sinks_only);

                // `DumpStackTrace` generates an empty string under MSVC.
                // Adding the constant prefix here simplifies testing.
                data_->entry.stacktrace_ = "*** Check failure stack trace: ***\n";
                debugging_internal::DumpStackTrace(
                    0, log_internal::max_frames_in_log_stack_trace(),
                    log_internal::should_symbolize_log_stack_trace(), WriteToString,
                    &data_->entry.stacktrace_);
            }
        }

        void LogMessage::die() {
            turbo::flush_log_sinks();

            if (data_->fail_quietly) {
                fail_quietly();
            } else {
                fail_without_stack_trace();
            }
        }

        void LogMessage::send_to_log() {
            if (is_fatal()) prepare_to_die();
            // Also log to all registered sinks, even if only_log_to_stderr() is set.
            log_internal::log_to_sinks(data_->entry, turbo::make_span(data_->extra_sinks),
                                     data_->extra_sinks_only);
            if (is_fatal()) die();
        }

        void LogMessage::log_backtrace_if_needed() {
            if (!turbo::log_internal::is_initialized()) return;

            if (!turbo::log_internal::should_log_backtrace_at(data_->entry.source_basename(),
                                                           data_->entry.source_line()))
                return;
            OstreamView view(*data_);
            view.stream() << " (stacktrace:\n";
            debugging_internal::DumpStackTrace(
                1, log_internal::max_frames_in_log_stack_trace(),
                log_internal::should_symbolize_log_stack_trace(), WriteToStream,
                &view.stream());
            view.stream() << ") ";
        }

        // Encodes into `data_->encoded_remaining()` a partial `logging.proto.Event`
        // containing the specified string data using a `Value` field appropriate to
        // `str_type`.  Truncates `str` if necessary, but emits nothing and marks the
        // buffer full if  even the field headers do not fit.
        template<LogMessage::StringType str_type>
        void LogMessage::copy_to_encoded_buffer(std::string_view str) {
            auto encoded_remaining_copy = data_->encoded_remaining();
            constexpr uint8_t tag_value = str_type == StringType::kLiteral
                                              ? ValueTag::kStringLiteral
                                              : ValueTag::kString;
            auto start = EncodeMessageStart(
                EventTag::kValue,
                BufferSizeFor(tag_value, WireType::kLengthDelimited) + str.size(),
                &encoded_remaining_copy);
            // If the `logging.proto.Event.value` field header did not fit,
            // `EncodeMessageStart` will have zeroed `encoded_remaining_copy`'s size and
            // `EncodeStringTruncate` will fail too.
            if (EncodeStringTruncate(tag_value, str, &encoded_remaining_copy)) {
                // The string may have been truncated, but the field header fit.
                EncodeMessageLength(start, &encoded_remaining_copy);
                data_->encoded_remaining() = encoded_remaining_copy;
            } else {
                // The field header(s) did not fit; zero `encoded_remaining()` so we don't
                // write anything else later.
                data_->encoded_remaining().remove_suffix(data_->encoded_remaining().size());
            }
        }

        template void LogMessage::copy_to_encoded_buffer<LogMessage::StringType::kLiteral>(
            std::string_view str);

        template void LogMessage::copy_to_encoded_buffer<
            LogMessage::StringType::kNotLiteral>(std::string_view str);

        template<LogMessage::StringType str_type>
        void LogMessage::copy_to_encoded_buffer(char ch, size_t num) {
            auto encoded_remaining_copy = data_->encoded_remaining();
            constexpr uint8_t tag_value = str_type == StringType::kLiteral
                                              ? ValueTag::kStringLiteral
                                              : ValueTag::kString;
            auto value_start = EncodeMessageStart(
                EventTag::kValue,
                BufferSizeFor(tag_value, WireType::kLengthDelimited) + num,
                &encoded_remaining_copy);
            auto str_start = EncodeMessageStart(tag_value, num, &encoded_remaining_copy);
            if (str_start.data()) {
                // The field headers fit.
                log_internal::append_truncated(ch, num, encoded_remaining_copy);
                EncodeMessageLength(str_start, &encoded_remaining_copy);
                EncodeMessageLength(value_start, &encoded_remaining_copy);
                data_->encoded_remaining() = encoded_remaining_copy;
            } else {
                // The field header(s) did not fit; zero `encoded_remaining()` so we don't
                // write anything else later.
                data_->encoded_remaining().remove_suffix(data_->encoded_remaining().size());
            }
        }

        template void LogMessage::copy_to_encoded_buffer<LogMessage::StringType::kLiteral>(
            char ch, size_t num);

        template void LogMessage::copy_to_encoded_buffer<
            LogMessage::StringType::kNotLiteral>(char ch, size_t num);

        template<LogMessage::StringType str_type>
        void LogMessage::copy_to_encoded_buffer(std::wstring_view str) {
            auto encoded_remaining_copy = data_->encoded_remaining();
            constexpr uint8_t tag_value = str_type == StringType::kLiteral
                                              ? ValueTag::kStringLiteral
                                              : ValueTag::kString;
            size_t max_str_byte_length =
                    turbo::strings_internal::kMaxEncodedUTF8Size * str.length();
            auto value_start =
                    EncodeMessageStart(EventTag::kValue,
                                       BufferSizeFor(tag_value, WireType::kLengthDelimited) +
                                       max_str_byte_length,
                                       &encoded_remaining_copy);
            auto str_start = EncodeMessageStart(tag_value, max_str_byte_length,
                                                &encoded_remaining_copy);
            if (str_start.data()) {
                log_internal::append_truncated(str, encoded_remaining_copy);
                EncodeMessageLength(str_start, &encoded_remaining_copy);
                EncodeMessageLength(value_start, &encoded_remaining_copy);
                data_->encoded_remaining() = encoded_remaining_copy;
            } else {
                // The field header(s) did not fit; zero `encoded_remaining()` so we don't
                // write anything else later.
                data_->encoded_remaining().remove_suffix(data_->encoded_remaining().size());
            }
        }

        template void LogMessage::copy_to_encoded_buffer<LogMessage::StringType::kLiteral>(
            std::wstring_view str);

        template void LogMessage::copy_to_encoded_buffer<
            LogMessage::StringType::kNotLiteral>(std::wstring_view str);

        template void LogMessage::copy_to_encoded_buffer_with_structured_proto_field<
            LogMessage::StringType::kLiteral>(StructuredProtoField field,
                                              std::string_view str);

        template void LogMessage::copy_to_encoded_buffer_with_structured_proto_field<
            LogMessage::StringType::kNotLiteral>(StructuredProtoField field,
                                                 std::string_view str);

        template<LogMessage::StringType str_type>
        void LogMessage::copy_to_encoded_buffer_with_structured_proto_field(
            StructuredProtoField field, std::string_view str) {
            auto encoded_remaining_copy = data_->encoded_remaining();
            size_t encoded_field_size = BufferSizeForStructuredProtoField(field);
            constexpr uint8_t tag_value = str_type == StringType::kLiteral
                                              ? ValueTag::kStringLiteral
                                              : ValueTag::kString;
            auto start = EncodeMessageStart(
                EventTag::kValue,
                encoded_field_size +
                BufferSizeFor(tag_value, WireType::kLengthDelimited) + str.size(),
                &encoded_remaining_copy);

            // Write the encoded proto field.
            if (!EncodeStructuredProtoField(field, encoded_remaining_copy)) {
                // The header / field will not fit; zero `encoded_remaining()` so we
                // don't write anything else later.
                data_->encoded_remaining().remove_suffix(data_->encoded_remaining().size());
                return;
            }

            // Write the string, truncating if necessary.
            if (!EncodeStringTruncate(tag_value, str, &encoded_remaining_copy)) {
                // The length of the string itself did not fit; zero `encoded_remaining()`
                // so the value is not encoded at all.
                data_->encoded_remaining().remove_suffix(data_->encoded_remaining().size());
                return;
            }

            EncodeMessageLength(start, &encoded_remaining_copy);
            data_->encoded_remaining() = encoded_remaining_copy;
        }

        // We intentionally don't return from these destructors. Disable MSVC's warning
        // about the destructor never returning as we do so intentionally here.
#if defined(_MSC_VER) && !defined(__clang__)
#pragma warning(push)
#pragma warning(disable : 4722)
#endif

        LogMessageFatal::LogMessageFatal(const char * turbo_nonnull file, int line)
            : LogMessage(file, line, turbo::LogSeverity::kFatal) {
        }

        LogMessageFatal::LogMessageFatal(const char * turbo_nonnull file, int line,
                                         const char * turbo_nonnull failure_msg)
            : LogMessage(file, line, turbo::LogSeverity::kFatal) {
            *this << "Check failed: " << failure_msg << " ";
        }

        LogMessageFatal::~LogMessageFatal() { fail_without_stack_trace(); }

        LogMessageDebugFatal::LogMessageDebugFatal(const char * turbo_nonnull file,
                                                   int line)
            : LogMessage(file, line, turbo::LogSeverity::kFatal) {
        }

        LogMessageDebugFatal::~LogMessageDebugFatal() { fail_without_stack_trace(); }

        LogMessageQuietlyDebugFatal::LogMessageQuietlyDebugFatal(
            const char * turbo_nonnull file, int line)
            : LogMessage(file, line, turbo::LogSeverity::kFatal) {
            set_fail_quietly();
        }

        LogMessageQuietlyDebugFatal::~LogMessageQuietlyDebugFatal() { fail_quietly(); }

        LogMessageQuietlyFatal::LogMessageQuietlyFatal(const char * turbo_nonnull file,
                                                       int line)
            : LogMessage(file, line, turbo::LogSeverity::kFatal) {
            set_fail_quietly();
        }

        LogMessageQuietlyFatal::LogMessageQuietlyFatal(
            const char * turbo_nonnull file, int line,
            const char * turbo_nonnull failure_msg)
            : LogMessageQuietlyFatal(file, line) {
            *this << "Check failed: " << failure_msg << " ";
        }

        LogMessageQuietlyFatal::~LogMessageQuietlyFatal() { fail_quietly(); }
#if defined(_MSC_VER) && !defined(__clang__)
#pragma warning(pop)
#endif
    } // namespace log_internal
} // namespace turbo
