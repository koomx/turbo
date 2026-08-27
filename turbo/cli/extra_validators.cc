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


#include <turbo/cli/extra_validators.h>

#if (defined(XCLI_ENABLE_EXTRA_VALIDATORS) && XCLI_ENABLE_EXTRA_VALIDATORS == 1) || (!defined(XCLI_DISABLE_EXTRA_VALIDATORS) || XCLI_DISABLE_EXTRA_VALIDATORS == 0)

#include <turbo/macros/macros.h>
#include <turbo/cli/encoding.h>
#include <turbo/cli/string_tools.h>
#include <turbo/cli/type_tools.h>

#include <algorithm>
#include <fstream>
#include <map>
#include <string>
#include <utility>


namespace xcli {
    namespace detail {

         IPV4Validator::IPV4Validator()
            : Validator("IPV4") {
            func_ = [](std::string& ip_addr) {
                auto cdot = std::count(ip_addr.begin(), ip_addr.end(), '.');
                if (cdot != 3u) {
                    return std::string("Invalid IPV4 address: must have 3 separators");
                }
                auto result = xcli::detail::split(ip_addr, '.');
                if (result.size() != 4) {
                    return std::string("Invalid IPV4 address: must have four parts (") + ip_addr + ')';
                }
                int num = 0;
                for (const auto& var : result) {
                    using xcli::detail::lexical_cast;
                    bool retval = lexical_cast(var, num);
                    if (!retval) {
                        return std::string("Failed parsing number (") + var + ')';
                    }
                    if (num < 0 || num > 255) {
                        return std::string("Each IP number must be between 0 and 255 ") + var;
                    }
                }
                return std::string { };
            };
        }

    } // namespace detail

     AsSizeValue::AsSizeValue(bool kb_is_1000)
        : AsNumberWithUnit(get_mapping(kb_is_1000)) {
        if (kb_is_1000) {
            description("SIZE [b, kb(=1000b), kib(=1024b), ...]");
        } else {
            description("SIZE [b, kb(=1024b), ...]");
        }
    }

     std::map<std::string, AsSizeValue::result_t> AsSizeValue::init_mapping(bool kb_is_1000) {
        std::map<std::string, result_t> m;
        result_t k_factor = kb_is_1000 ? 1000 : 1024;
        result_t ki_factor = 1024;
        result_t k = 1;
        result_t ki = 1;
        m["b"] = 1;
        for (std::string p : { "k", "m", "g", "t", "p", "e" }) {
            k *= k_factor;
            ki *= ki_factor;
            m[p] = k;
            m[p + "b"] = k;
            m[p + "i"] = ki;
            m[p + "ib"] = ki;
        }
        return m;
    }

     const std::map<std::string, AsSizeValue::result_t>& AsSizeValue::get_mapping(bool kb_is_1000) {
        if (kb_is_1000) {
            static auto m = init_mapping(true);
            return m;
        }
        static auto m = init_mapping(false);
        return m;
    }

#if defined(XCLI_ENABLE_EXTRA_VALIDATORS) && XCLI_ENABLE_EXTRA_VALIDATORS != 0
    // new extra validators
    namespace detail {
         PermissionValidator::PermissionValidator(Permission permission) {
            std::filesystem::perms permission_code = std::filesystem::perms::none;
            std::string permission_name;
            switch (permission) {
            case Permission::read:
                permission_code = std::filesystem::perms::owner_read | std::filesystem::perms::group_read | std::filesystem::perms::others_read;
                permission_name = "read";
                break;
            case Permission::write:
                permission_code = std::filesystem::perms::owner_write | std::filesystem::perms::group_write | std::filesystem::perms::others_write;
                permission_name = "write";
                break;
            case Permission::exec:
                permission_code = std::filesystem::perms::owner_exec | std::filesystem::perms::group_exec | std::filesystem::perms::others_exec;
                permission_name = "exec";
                break;
            case Permission::none:
            default:
                permission_code = std::filesystem::perms::none;
                break;
            }
            func_ = [permission_code](std::string& path) {
                std::error_code ec;
                auto p = to_path(path);
                if (!std::filesystem::exists(p, ec)) {
                    return std::string("Path does not exist: ") + path;
                }
                if (ec) {
                    return std::string("Error checking path: ") + ec.message(); // LCOV_EXCL_LINE
                }
                if (permission_code == std::filesystem::perms::none) {
                    return std::string { };
                }
                auto perms = std::filesystem::status(p, ec).permissions();
                if (ec) {
                    return std::string("Error checking path status: ") + ec.message(); // LCOV_EXCL_LINE
                }
                if ((perms & permission_code) == std::filesystem::perms::none) {
                    return std::string("Path does not have required permissions: ") + path;
                }
                return std::string { };
            };
            description("Path with " + permission_name + " permission");
        }
    } // namespace detail

     FileSizeValidator::FileSizeValidator(std::uint64_t min_size, std::uint64_t max_size) {
        std::string desc;
        if (max_size == 0) {
            desc = "File size at least " + std::to_string(min_size) + " bytes";
        } else {
            desc = "File size between " + std::to_string(min_size) + " and " + std::to_string(max_size) + " bytes";
        }
        description(desc);
        func_ = [min_size, max_size](std::string& path) {
            std::error_code ec;
            auto p = to_path(path);
            if (!std::filesystem::exists(p, ec)) {
                return std::string("File does not exist: ") + path;
            }
            if (ec) {
                return std::string("Error checking file: ") + ec.message(); // LCOV_EXCL_LINE
            }
            auto size = std::filesystem::file_size(p, ec);
            if (ec) {
                return std::string("Error getting file size: ") + ec.message(); // LCOV_EXCL_LINE
            }
            if (size < min_size) {
                return std::string("File size ") + std::to_string(size) + " bytes is less than minimum " + std::to_string(min_size) + " bytes";
            }
            if (max_size > 0 && size > max_size) {
                return std::string("File size ") + std::to_string(size) + " bytes exceeds maximum " + std::to_string(max_size) + " bytes";
            }
            return std::string { };
        };
    }


#endif

} // namespace xcli

#endif
