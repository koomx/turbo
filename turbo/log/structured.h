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
// File: log/structured.h
// -----------------------------------------------------------------------------
//
// This header declares APIs supporting structured logging, allowing log
// statements to be more easily parsed, especially by automated processes.
//
// When structured logging is in use, data streamed into a `LOG` statement are
// encoded as `Value` fields in a `logging.proto.Event` protocol buffer message.
// The individual data are exposed programmatically to `LogSink`s and to the
// user via some log reading tools which are able to query the structured data
// more usefully than would be possible if each message was a single opaque
// string.  These helpers allow user code to add additional structure to the
// data they stream.

#ifndef TURBO_LOG_STRUCTURED_H_
#define TURBO_LOG_STRUCTURED_H_

#include <ostream>
#include <turbo/macros/config.h>
#include <turbo/log/internal/structured.h>
#include <string_view>

namespace turbo {
    // log_as_literal()
    //
    // Annotates its argument as a string literal so that structured logging
    // captures it as a `literal` field instead of a `str` field (the default).
    // This does not affect the text representation, only the structure.
    //
    // Streaming `log_as_literal(s)` into a `std::ostream` behaves just like streaming
    // `s` directly.
    //
    // Using `log_as_literal()` is occasionally appropriate and useful when proxying
    // data logged from another system or another language.  For example:
    //
    //   void Logger::LogString(std::string_view str, turbo::LogSeverity severity,
    //                          const char *file, int line) {
    //     KLOG(LEVEL(severity)).at_location(file, line) << str;
    //   }
    //   void Logger::LogStringLiteral(std::string_view str,
    //                                 turbo::LogSeverity severity, const char *file,
    //                                 int line) {
    //     KLOG(LEVEL(severity)).at_location(file, line) << turbo::log_as_literal(str);
    //   }
    //
    // `log_as_literal` should only be used as a streaming operand and not, for
    // example, as a local variable initializer.
    inline log_internal::AsLiteralImpl log_as_literal(
        std::string_view s KUMO_ATTRIBUTE_LIFETIME_BOUND) {
        return log_internal::AsLiteralImpl(s);
    }
} // namespace turbo

#endif  // TURBO_LOG_STRUCTURED_H_
