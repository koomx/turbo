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

#ifndef TURBO_STRINGS_INTERNAL_STRINGIFY_SINK_H_
#define TURBO_STRINGS_INTERNAL_STRINGIFY_SINK_H_

#include <array>
#include <string>
#include <type_traits>
#include <utility>

#include <turbo/types/source_location.h>
#include <string_view>
#include <turbo/strings/numbers.h>

namespace turbo {
    namespace strings_internal {
        class StringifySink {
        public:
            void append(size_t count, char ch);

            void append(std::string_view v);

            // Support `turbo::str_printf_to(&sink, format, args...)`.
            friend void turbo_format_flush(StringifySink* sink, std::string_view v) {
                sink->append(v);
            }

        private:
            template <typename T>
            friend std::string_view extract_stringification(StringifySink& sink, const T& v);

            std::string buffer_;
        };

        template <typename T>
        std::string_view extract_stringification(StringifySink& sink, const T& v) {
            turbo_stringify(sink, v);
            return sink.buffer_;
        }
    } // namespace strings_internal

    template <typename Sink>
    void turbo_stringify(Sink& sink, turbo::SourceLocation l) {
        sink.append(l.file_name());
        sink.append(":");
        std::array<char, format_internal::kFastToBufferSize> buffer;
        format_internal::fast_int_to_buffer(l.line(), buffer.data());
        sink.append(buffer.data());
    }
} // namespace turbo

#endif // TURBO_STRINGS_INTERNAL_STRINGIFY_SINK_H_
