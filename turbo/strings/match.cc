// Copyright 2017 The Abseil Authors.
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

#include <turbo/strings/match.h>

#include <algorithm>
#include <cstdint>

#include <string_view>
#include <turbo/bits/bits.h>
#include <turbo/bits/endian.h>
#include <turbo/macros/config.h>
#include <turbo/strings/ascii.h>
#include <turbo/strings/internal/memutil.h>

namespace turbo {

    bool EqualsIgnoreCase(std::string_view piece1,
        std::string_view piece2) noexcept {
        return (piece1.size() == piece2.size() && 0 == turbo::strings_internal::memcasecmp(piece1.data(), piece2.data(), piece1.size()));
        // memcasecmp uses turbo::ascii_tolower().
    }

    bool StrContainsIgnoreCase(std::string_view haystack,
        std::string_view needle) noexcept {
        while (haystack.size() >= needle.size()) {
            if (StartsWithIgnoreCase(haystack, needle))
                return true;
            haystack.remove_prefix(1);
        }
        return false;
    }

    bool StrContainsIgnoreCase(std::string_view haystack,
        char needle) noexcept {
        char upper_needle = turbo::ascii_toupper(static_cast<unsigned char>(needle));
        char lower_needle = turbo::ascii_tolower(static_cast<unsigned char>(needle));
        if (upper_needle == lower_needle) {
            return StrContains(haystack, needle);
        } else {
            const char both_cstr[3] = { lower_needle, upper_needle, '\0' };
            return haystack.find_first_of(both_cstr) != std::string_view::npos;
        }
    }

    bool StartsWithIgnoreCase(std::string_view text,
        std::string_view prefix) noexcept {
        return (text.size() >= prefix.size()) && EqualsIgnoreCase(text.substr(0, prefix.size()), prefix);
    }

    bool EndsWithIgnoreCase(std::string_view text,
        std::string_view suffix) noexcept {
        return (text.size() >= suffix.size()) && EqualsIgnoreCase(text.substr(text.size() - suffix.size()), suffix);
    }

    std::string_view FindLongestCommonPrefix(std::string_view a,
        std::string_view b) {
        const std::string_view::size_type limit = std::min(a.size(), b.size());
        const char* const pa = a.data();
        const char* const pb = b.data();
        std::string_view::size_type count = (unsigned)0;

        if (KUMO_UNLIKELY(limit < 8)) {
            while (KUMO_LIKELY(count + 2 <= limit)) {
                uint16_t xor_bytes = turbo::little_endian::Load16(pa + count) ^ turbo::little_endian::Load16(pb + count);
                if (KUMO_UNLIKELY(xor_bytes != 0)) {
                    if (KUMO_LIKELY((xor_bytes & 0xff) == 0))
                        ++count;
                    return std::string_view(pa, count);
                }
                count += 2;
            }
            if (KUMO_LIKELY(count != limit)) {
                if (KUMO_LIKELY(pa[count] == pb[count]))
                    ++count;
            }
            return std::string_view(pa, count);
        }

        do {
            uint64_t xor_bytes = turbo::little_endian::Load64(pa + count) ^ turbo::little_endian::Load64(pb + count);
            if (KUMO_UNLIKELY(xor_bytes != 0)) {
                count += static_cast<uint64_t>(turbo::countr_zero(xor_bytes) >> 3);
                return std::string_view(pa, count);
            }
            count += 8;
        } while (KUMO_LIKELY(count + 8 < limit));

        count = limit - 8;
        uint64_t xor_bytes = turbo::little_endian::Load64(pa + count) ^ turbo::little_endian::Load64(pb + count);
        if (KUMO_LIKELY(xor_bytes != 0)) {
            count += static_cast<uint64_t>(turbo::countr_zero(xor_bytes) >> 3);
            return std::string_view(pa, count);
        }
        return std::string_view(pa, limit);
    }

    std::string_view FindLongestCommonSuffix(std::string_view a,
        std::string_view b) {
        const std::string_view::size_type limit = std::min(a.size(), b.size());
        if (limit == 0)
            return std::string_view();

        const char* pa = a.data() + a.size() - 1;
        const char* pb = b.data() + b.size() - 1;
        std::string_view::size_type count = (unsigned)0;
        while (count < limit && *pa == *pb) {
            --pa;
            --pb;
            ++count;
        }

        return std::string_view(++pa, count);
    }

} // namespace turbo
