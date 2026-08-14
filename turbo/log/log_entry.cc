// Copyright 2025 The Abseil Authors
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

#include <turbo/log/log_entry.h>

#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <ios>
#include <ostream>

#include <turbo/macros/config.h>
#include <turbo/log/internal/proto.h>
#include <turbo/strings/escaping.h>
#include <string_view>
#include <turbo/time/time.h>
#include <turbo/types/span.h>

namespace turbo {
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

        // enum `logging.proto.Severity`
        enum Severity : int {
            FINEST = 300,
            FINER = 400,
            FINE = 500,
            VERBOSE_0 = 600,
            CONFIG = 700,
            INFO = 800,
            NOTICE = 850,
            WARNING = 900,
            ERROR = 950,
            SEVERE = 1000,
            FATAL = 1100,
        };

        void PrintEscapedRangeTo(const std::string_view str,
                                 const std::string_view substr, std::ostream *os) {
            const std::string_view head =
                    str.substr(0, static_cast<size_t>(substr.data() - str.data()));
            const char old_fill = os->fill();
            const auto old_flags = os->flags();
            *os << std::right
                    << std::setw(static_cast<int>(turbo::c_hex_escape(head).size())) << "";
            switch (substr.size()) {
                case 0:
                    *os << "\\";
                    break;
                case 1:
                    *os << "^";
                    break;
                default:
                    *os << "[" << std::setw(static_cast<int>(turbo::c_hex_escape(substr).size()))
                            << std::setfill('-') << ")";
                    break;
            }
            os->fill(old_fill);
            os->flags(old_flags);
        }
    } // namespace
    void PrintTo(const LogEntry &entry, std::ostream *os) {
        auto text_message_with_prefix_and_newline_and_nul = std::string_view(
            entry.text_message_with_prefix_and_newline_and_nul_.data(),
            entry.text_message_with_prefix_and_newline_and_nul_.size());
        *os << "LogEntry {\n"
                << "  source_filename: \"" << turbo::c_hex_escape(entry.source_filename())
                << "\"\n"
                << "  source_basename: \"" << turbo::c_hex_escape(entry.source_basename())
                << "\"\n"
                << "  source_line: " << entry.source_line() << "\n"
                << "  prefix: " << std::boolalpha << entry.prefix() << "\n"
                << "  log_severity: " << entry.log_severity() << "\n"
                << "  verbosity: " << entry.verbosity();
        if (entry.verbosity() == turbo::LogEntry::kNoVerbosityLevel) {
            *os << " (kNoVerbosityLevel)";
        }
        *os << "\n"
                << "  timestamp: " << entry.timestamp() << "\n"
                << "  tid: " << entry.tid() << "\n"
                << "  text_message_with_prefix_and_newline_and_nul_: \""
                << turbo::c_hex_escape(text_message_with_prefix_and_newline_and_nul)
                << "\"\n"
                << "  text_message_with_prefix_and_newline:           ";
        PrintEscapedRangeTo(text_message_with_prefix_and_newline_and_nul,
                            entry.text_message_with_prefix_and_newline(), os);
        *os << "\n"
                << "  text_message_with_prefix:                       ";
        PrintEscapedRangeTo(text_message_with_prefix_and_newline_and_nul,
                            entry.text_message_with_prefix(), os);
        *os << "\n"
                << "  text_message_with_newline:                      ";
        PrintEscapedRangeTo(text_message_with_prefix_and_newline_and_nul,
                            entry.text_message_with_newline(), os);
        *os << "\n"
                << "  text_message:                                   ";
        PrintEscapedRangeTo(text_message_with_prefix_and_newline_and_nul,
                            entry.text_message(), os);
        *os << "\n"
                << "  text_message_with_prefix_and_newline_c_str:     ";
        PrintEscapedRangeTo(
            text_message_with_prefix_and_newline_and_nul,
            // NOLINTNEXTLINE(bugprone-string-constructor)
            std::string_view(entry.text_message_with_prefix_and_newline_c_str(), 0),
            os);
        *os << "\n"
                << "  encoded_message (raw): \""
                << turbo::c_hex_escape(entry.encoded_message()) << "\"\n"
                << "  encoded_message {\n";
        turbo::Span<const char> event = entry.encoded_message();
        log_internal::ProtoField field;
        while (field.DecodeFrom(&event)) {
            switch (field.tag()) {
                case EventTag::kFileName:
                    *os << "    file_name: \"" << turbo::c_hex_escape(field.string_value())
                            << "\"\n";
                    break;
                case EventTag::kFileLine:
                    *os << "    file_line: " << field.int32_value() << "\n";
                    break;
                case EventTag::kTimeNsecs:
                    *os << "    time_nsecs: " << field.int64_value() << " ("
                            << turbo::FromUnixNanos(field.int64_value()) << ")\n";
                    break;
                case EventTag::kSeverity:
                    *os << "    severity: " << field.int32_value();
                    switch (field.int32_value()) {
                        case Severity::FINEST:
                            *os << " (FINEST)";
                            break;
                        case Severity::FINER:
                            *os << " (FINER)";
                            break;
                        case Severity::FINE:
                            *os << " (FINE)";
                            break;
                        case Severity::VERBOSE_0:
                            *os << " (VERBOSE_0)";
                            break;
                        case Severity::CONFIG:
                            *os << " (CONFIG)";
                            break;
                        case Severity::INFO:
                            *os << " (INFO)";
                            break;
                        case Severity::NOTICE:
                            *os << " (NOTICE)";
                            break;
                        case Severity::WARNING:
                            *os << " (WARNING)";
                            break;
                        case Severity::ERROR:
                            *os << " (ERROR)";
                            break;
                        case Severity::SEVERE:
                            *os << " (SEVERE)";
                            break;
                        case Severity::FATAL:
                            *os << " (FATAL)";
                            break;
                    }
                    *os << "\n";
                    break;
                case EventTag::kThreadId:
                    *os << "    thread_id: " << field.int64_value() << "\n";
                    break;
                case EventTag::kValue: {
                    *os << "    value {\n";
                    auto value = field.bytes_value();
                    while (field.DecodeFrom(&value)) {
                        switch (field.tag()) {
                            case ValueTag::kString:
                                *os << "      str: \"" << turbo::c_hex_escape(field.string_value())
                                        << "\"\n";
                                break;
                            case ValueTag::kStringLiteral:
                                *os << "      literal: \""
                                        << turbo::c_hex_escape(field.string_value()) << "\"\n";
                                break;
                            default:
                                *os << "      unknown field " << field.tag();
                                switch (field.type()) {
                                    case log_internal::WireType::kVarint:
                                        *os << " (VARINT): " << std::hex << std::showbase
                                                << field.uint64_value() << std::dec << "\n";
                                        break;
                                    case log_internal::WireType::k64Bit:
                                        *os << " (I64): " << std::hex << std::showbase
                                                << field.uint64_value() << std::dec << "\n";
                                        break;
                                    case log_internal::WireType::kLengthDelimited:
                                        *os << " (LEN): \"" << turbo::c_hex_escape(field.string_value())
                                                << "\"\n";
                                        break;
                                    case log_internal::WireType::k32Bit:
                                        *os << " (I32): " << std::hex << std::showbase
                                                << field.uint32_value() << std::dec << "\n";
                                        break;
                                }
                                break;
                        }
                    }
                    *os << "    }\n";
                    break;
                }
                default:
                    *os << "    unknown field " << field.tag();
                    switch (field.type()) {
                        case log_internal::WireType::kVarint:
                            *os << " (VARINT): " << std::hex << std::showbase
                                    << field.uint64_value() << std::dec << "\n";
                            break;
                        case log_internal::WireType::k64Bit:
                            *os << " (I64): " << std::hex << std::showbase
                                    << field.uint64_value() << std::dec << "\n";
                            break;
                        case log_internal::WireType::kLengthDelimited:
                            *os << " (LEN): \"" << turbo::c_hex_escape(field.string_value())
                                    << "\"\n";
                            break;
                        case log_internal::WireType::k32Bit:
                            *os << " (I32): " << std::hex << std::showbase
                                    << field.uint32_value() << std::dec << "\n";
                            break;
                    }
                    break;
            }
        }
        *os << "  }\n"
                << "  stacktrace: \"" << turbo::c_hex_escape(entry.stacktrace()) << "\"\n"
                << "}";
    }
} // namespace turbo
