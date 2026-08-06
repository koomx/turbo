// Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <turbo/log/sinks/android_log_sink.h>

#if defined(__ANDROID__)

#include <android/log.h>

#include <turbo/base/log_severity.h>
#include <turbo/log/globals.h>

namespace turbo {

    void AndroidLogSink::send(const turbo::LogEntry &entry) {
        const int level = android_log_level(entry);
        const char *const tag = turbo::get_android_native_tag();
        __android_log_write(level, tag,
                            entry.text_message_with_prefix_and_newline_c_str());
        if (entry.log_severity() == turbo::LogSeverity::kFatal)
            __android_log_write(ANDROID_LOG_FATAL, tag, "terminating.\n");
    }

    int AndroidLogSink::android_log_level(const turbo::LogEntry &entry) {
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

}  // namespace turbo

#endif  // __ANDROID__
