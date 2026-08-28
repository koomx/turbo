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

#pragma once

#include <optional>
#include <string>
#include <turbo/macros/config.h>

namespace turbo {

    std::optional<std::string> get_environment(const char* var_name);

    void set_environment(const char* var_name, const char* new_value);

    inline bool exists_environment(const char* var_name) {
        auto env = get_environment(var_name);
        return env.has_value();
    }

    class ScopedSetEnv {
    public:
        ScopedSetEnv(const char* var_name, const char* new_value);
        ~ScopedSetEnv();

    private:
        std::string var_name_;
        std::string old_value_;

        // True if the environment variable was initially not set.
        bool was_unset_;
    };

} // namespace turbo
