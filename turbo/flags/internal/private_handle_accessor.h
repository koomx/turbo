//
// Copyright 2020 The Abseil Authors.
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

#ifndef TURBO_FLAGS_INTERNAL_PRIVATE_HANDLE_ACCESSOR_H_
#define TURBO_FLAGS_INTERNAL_PRIVATE_HANDLE_ACCESSOR_H_

#include <memory>
#include <string>

#include <string_view>
#include <turbo/flags/commandlineflag.h>
#include <turbo/flags/internal/commandlineflag.h>
#include <turbo/macros/config.h>

namespace turbo {

    namespace flags_internal {

        // This class serves as a trampoline to access private methods of
        // CommandLineFlag. This class is intended for use exclusively internally inside
        // of the Abseil Flags implementation.
        class PrivateHandleAccessor {
        public:
            // Access to CommandLineFlag::TypeId.
            static FlagFastTypeId TypeId(const CommandLineFlag& flag);

            // Access to CommandLineFlag::SaveState.
            static std::unique_ptr<FlagStateInterface> SaveState(CommandLineFlag& flag);

            // Access to CommandLineFlag::IsSpecifiedOnCommandLine.
            static bool IsSpecifiedOnCommandLine(const CommandLineFlag& flag);

            // Access to CommandLineFlag::ValidateInputValue.
            static bool ValidateInputValue(const CommandLineFlag& flag,
                std::string_view value);

            // Access to CommandLineFlag::CheckDefaultValueParsingRoundtrip.
            static void CheckDefaultValueParsingRoundtrip(const CommandLineFlag& flag);

            static bool ParseFrom(CommandLineFlag& flag, std::string_view value,
                flags_internal::FlagSettingMode set_mode,
                flags_internal::ValueSource source, std::string& error);

            // Access to CommandLineFlag::TypeName.
            static std::string_view TypeName(const CommandLineFlag& flag);
        };

    } // namespace flags_internal

} // namespace turbo

#endif // TURBO_FLAGS_INTERNAL_PRIVATE_HANDLE_ACCESSOR_H_
