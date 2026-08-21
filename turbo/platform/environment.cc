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
//

#include <turbo/platform/environment.h>
#ifdef _WIN32
#include <windows.h>
#endif

#include <cstdlib>
#include <turbo/base/internal/raw_logging.h>

namespace turbo {

#ifdef _WIN32
    const int kMaxEnvVarValueSize = 1024;
#endif

    std::optional<std::string> get_environment(const char* var_name) {
        std::string ename_string;

#ifdef _MSC_VER
        // Windows version
        char* buffer = nullptr;
        std::size_t sz = 0;
        if (_dupenv_s(&buffer, &sz, var_name) == 0 && buffer != nullptr) {
            ename_string = std::string(buffer);
            free(buffer);
            return ename_string;
        }
#else
        // This also works on Windows, but gives a warning

        // MISRA static analysis need. MISRACPP2023-25_5_2-a-1
        const char* buffer = nullptr;
        buffer = std::getenv(var_name);
        if (buffer != nullptr) {
            ename_string = std::string(buffer);
            return ename_string;
        }
#endif
        return std::nullopt;
    }

    void set_environment(const char* var_name, const char* new_value) {
#ifdef _WIN32
        SetEnvironmentVariableA(var_name, new_value);
#else
        if (new_value == nullptr) {
            ::unsetenv(var_name);
        } else {
            ::setenv(var_name, new_value, 1);
        }
#endif
    }

    ScopedSetEnv::ScopedSetEnv(const char* var_name, const char* new_value)
        : var_name_(var_name)
        , was_unset_(false) {
#ifdef _WIN32
        char buf[kMaxEnvVarValueSize];
        auto get_res = GetEnvironmentVariableA(var_name_.c_str(), buf, sizeof(buf));
        TURBO_INTERNAL_CHECK(get_res < sizeof(buf), "value exceeds buffer size");

        if (get_res == 0) {
            was_unset_ = (GetLastError() == ERROR_ENVVAR_NOT_FOUND);
        } else {
            old_value_.assign(buf, get_res);
        }

        SetEnvironmentVariableA(var_name_.c_str(), new_value);
#else
        const char* val = ::getenv(var_name_.c_str());
        if (val == nullptr) {
            was_unset_ = true;
        } else {
            old_value_ = val;
        }
#endif

        set_environment(var_name_.c_str(), new_value);
    }

    ScopedSetEnv::~ScopedSetEnv() {
        set_environment(var_name_.c_str(), was_unset_ ? nullptr : old_value_.c_str());
    }

} // namespace turbo
