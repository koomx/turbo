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

#include <turbo/log/sinks/log_filename.h>

#include <filesystem>

#include <turbo/format/str_format.h>
#include <turbo/time/civil_time.h>

namespace turbo {

    BaseFilename::BaseFilename(std::string_view filename) {
        const std::filesystem::path path{std::string(filename)};
        // generic_string() keeps '/' so tests and logs are portable across
        // Windows (preferred '\\') and POSIX.
        directory =
            path.has_parent_path() ? path.parent_path().generic_string() : "";
        basename = path.stem().generic_string();
        extension = path.extension().generic_string();
    }

    namespace {
        std::string Join(const BaseFilename &base, std::string_view stem_suffix) {
            const std::string filename =
                turbo::str_sprintf("%s%s%s", base.basename, stem_suffix,
                                   base.extension);
            if (base.directory.empty()) {
                return filename;
            }
            return (std::filesystem::path(base.directory) / filename)
                .generic_string();
        }

        turbo::CivilSecond ToCivilSecond(turbo::Time tp, bool utc) {
            const turbo::TimeZone tz =
                utc ? turbo::UTCTimeZone() : turbo::LocalTimeZone();
            return tz.At(tp).cs;
        }
    }  // namespace

    std::string sequential_log_path(const BaseFilename &base, int64_t seq) {
        return Join(base, turbo::str_sprintf("_%04d", static_cast<int>(seq)));
    }

    std::string daily_log_path(const BaseFilename &base, turbo::Time tp,
                               bool utc) {
        const turbo::CivilSecond cs = ToCivilSecond(tp, utc);
        return Join(base, turbo::str_sprintf("_%04d-%02d-%02d",
                                             static_cast<int>(cs.year()),
                                             static_cast<int>(cs.month()),
                                             static_cast<int>(cs.day())));
    }

    std::string hourly_log_path(const BaseFilename &base, turbo::Time tp,
                                bool utc) {
        const turbo::CivilSecond cs = ToCivilSecond(tp, utc);
        return Join(base, turbo::str_sprintf("_%04d-%02d-%02d-%02d",
                                             static_cast<int>(cs.year()),
                                             static_cast<int>(cs.month()),
                                             static_cast<int>(cs.day()),
                                             static_cast<int>(cs.hour())));
    }

}  // namespace turbo
