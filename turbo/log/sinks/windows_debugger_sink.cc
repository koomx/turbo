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

#include <turbo/log/sinks/windows_debugger_sink.h>

#if defined(_WIN32)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <turbo/log/globals.h>
#include <turbo/log/internal/globals.h>

namespace turbo {

    void WindowsDebuggerLogSink::send(const turbo::LogEntry &entry) {
        // Match absl/turbo: debugger channel respects stderr_threshold.
        if (entry.log_severity() < turbo::stderr_threshold() &&
            turbo::log_internal::is_initialized()) {
            return;
        }
        ::OutputDebugStringA(entry.text_message_with_prefix_and_newline_c_str());
    }

}  // namespace turbo

#endif  // _WIN32
