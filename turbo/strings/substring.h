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

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

#include <turbo/macros/macros.h>

namespace turbo {
namespace substring_internal {

    template <typename Char>
    KUMO_FORCE_INLINE std::basic_string_view<Char> subview_impl(
        std::basic_string_view<Char> buffer, size_t pos_start, size_t pos_end) {
        if (pos_start >= buffer.size() || pos_start >= pos_end) {
            return {};
        }
        if (pos_end >= buffer.size()) {
            return {buffer.data() + pos_start, buffer.size() - pos_start};
        }
        return {buffer.data() + pos_start, pos_end - pos_start};
    }

    template <typename Char>
    KUMO_FORCE_INLINE std::basic_string_view<Char> subview_impl(
        std::basic_string_view<Char> buffer, size_t pos) {
        if (pos >= buffer.size()) {
            return {};
        }
        return {buffer.data() + pos, buffer.size() - pos};
    }

    template <typename Char>
    KUMO_FORCE_INLINE std::basic_string<Char> substring_impl(
        std::basic_string_view<Char> buffer, size_t pos) {
        return std::basic_string<Char>(subview_impl(buffer, pos));
    }

    template <typename Char>
    KUMO_FORCE_INLINE std::basic_string<Char> substring_impl(
        std::basic_string_view<Char> buffer, size_t pos_start, size_t pos_end) {
        return std::basic_string<Char>(subview_impl(buffer, pos_start, pos_end));
    }

    template <typename Char>
    KUMO_FORCE_INLINE bool overlaps_impl(std::basic_string_view<Char> input1,
        std::basic_string_view<Char> input2) noexcept {
        return !input1.empty() && !input2.empty() && input1.data() >= input2.data() &&
            input1.data() < input2.data() + input2.size();
    }

    template <typename Char>
    KUMO_FORCE_INLINE void resize_impl(std::basic_string_view<Char>& input, size_t pos) noexcept {
        input.remove_suffix(input.size() - pos);
    }

    template <typename Char>
    KUMO_FORCE_INLINE bool absolute_offset_impl(std::basic_string_view<Char> str,
        std::basic_string_view<Char> substr, size_t suboff, size_t& aboff) {
        const Char* s0 = str.data();
        const Char* s1 = s0 + str.size();
        const Char* t0 = substr.data();
        const Char* t1 = t0 + substr.size();
        if (t0 < s0 || t1 > s1 || suboff > substr.size()) {
            return false;
        }
        aboff = static_cast<size_t>(t0 - s0) + suboff;
        return true;
    }

    template <typename Char>
    KUMO_FORCE_INLINE std::optional<size_t> absolute_offset_impl(std::basic_string_view<Char> str,
        std::basic_string_view<Char> substr, size_t suboff) {
        size_t aboff = 0;
        if (!absolute_offset_impl(str, substr, suboff, aboff)) {
            return std::nullopt;
        }
        return aboff;
    }

    template <typename Char>
    KUMO_FORCE_INLINE bool sub_offset_impl(std::basic_string_view<Char> str,
        std::basic_string_view<Char> substr, size_t aboff, size_t& suboff) {
        const Char* s0 = str.data();
        const Char* s1 = s0 + str.size();
        const Char* t0 = substr.data();
        const Char* t1 = t0 + substr.size();
        if (t0 < s0 || t1 > s1 || aboff > str.size()) {
            return false;
        }
        const size_t base = static_cast<size_t>(t0 - s0);
        if (aboff < base || aboff - base > substr.size()) {
            return false;
        }
        suboff = aboff - base;
        return true;
    }

    template <typename Char>
    KUMO_FORCE_INLINE std::optional<size_t> sub_offset_impl(std::basic_string_view<Char> str,
        std::basic_string_view<Char> substr, size_t aboff) {
        size_t off = 0;
        if (!sub_offset_impl(str, substr, aboff, off)) {
            return std::nullopt;
        }
        return off;
    }

}  // namespace substring_internal

    KUMO_FORCE_INLINE std::string_view subview(std::string_view buffer, size_t pos_start, size_t pos_end) {
        return substring_internal::subview_impl(buffer, pos_start, pos_end);
    }
    KUMO_FORCE_INLINE std::u16string_view subview(std::u16string_view buffer, size_t pos_start, size_t pos_end) {
        return substring_internal::subview_impl(buffer, pos_start, pos_end);
    }
    KUMO_FORCE_INLINE std::u32string_view subview(std::u32string_view buffer, size_t pos_start, size_t pos_end) {
        return substring_internal::subview_impl(buffer, pos_start, pos_end);
    }
    KUMO_FORCE_INLINE std::wstring_view subview(std::wstring_view buffer, size_t pos_start, size_t pos_end) {
        return substring_internal::subview_impl(buffer, pos_start, pos_end);
    }

    KUMO_FORCE_INLINE std::string_view subview(std::string_view buffer, size_t pos) {
        return substring_internal::subview_impl(buffer, pos);
    }
    KUMO_FORCE_INLINE std::u16string_view subview(std::u16string_view buffer, size_t pos) {
        return substring_internal::subview_impl(buffer, pos);
    }
    KUMO_FORCE_INLINE std::u32string_view subview(std::u32string_view buffer, size_t pos) {
        return substring_internal::subview_impl(buffer, pos);
    }
    KUMO_FORCE_INLINE std::wstring_view subview(std::wstring_view buffer, size_t pos) {
        return substring_internal::subview_impl(buffer, pos);
    }

    KUMO_FORCE_INLINE std::string substring(std::string_view buffer, size_t pos) {
        return substring_internal::substring_impl(buffer, pos);
    }
    KUMO_FORCE_INLINE std::u16string substring(std::u16string_view buffer, size_t pos) {
        return substring_internal::substring_impl(buffer, pos);
    }
    KUMO_FORCE_INLINE std::u32string substring(std::u32string_view buffer, size_t pos) {
        return substring_internal::substring_impl(buffer, pos);
    }
    KUMO_FORCE_INLINE std::wstring substring(std::wstring_view buffer, size_t pos) {
        return substring_internal::substring_impl(buffer, pos);
    }

    KUMO_FORCE_INLINE std::string substring(std::string_view buffer, size_t pos_start, size_t pos_end) {
        return substring_internal::substring_impl(buffer, pos_start, pos_end);
    }
    KUMO_FORCE_INLINE std::u16string substring(std::u16string_view buffer, size_t pos_start, size_t pos_end) {
        return substring_internal::substring_impl(buffer, pos_start, pos_end);
    }
    KUMO_FORCE_INLINE std::u32string substring(std::u32string_view buffer, size_t pos_start, size_t pos_end) {
        return substring_internal::substring_impl(buffer, pos_start, pos_end);
    }
    KUMO_FORCE_INLINE std::wstring substring(std::wstring_view buffer, size_t pos_start, size_t pos_end) {
        return substring_internal::substring_impl(buffer, pos_start, pos_end);
    }

    /// input2 contains input1
    KUMO_FORCE_INLINE bool overlaps(std::string_view input1, std::string_view input2) noexcept {
        return substring_internal::overlaps_impl(input1, input2);
    }
    KUMO_FORCE_INLINE bool overlaps(std::u16string_view input1, std::u16string_view input2) noexcept {
        return substring_internal::overlaps_impl(input1, input2);
    }
    KUMO_FORCE_INLINE bool overlaps(std::u32string_view input1, std::u32string_view input2) noexcept {
        return substring_internal::overlaps_impl(input1, input2);
    }
    KUMO_FORCE_INLINE bool overlaps(std::wstring_view input1, std::wstring_view input2) noexcept {
        return substring_internal::overlaps_impl(input1, input2);
    }

    KUMO_FORCE_INLINE void resize(std::string_view& input, size_t pos) noexcept {
        substring_internal::resize_impl(input, pos);
    }
    KUMO_FORCE_INLINE void resize(std::u16string_view& input, size_t pos) noexcept {
        substring_internal::resize_impl(input, pos);
    }
    KUMO_FORCE_INLINE void resize(std::u32string_view& input, size_t pos) noexcept {
        substring_internal::resize_impl(input, pos);
    }
    KUMO_FORCE_INLINE void resize(std::wstring_view& input, size_t pos) noexcept {
        substring_internal::resize_impl(input, pos);
    }

    KUMO_FORCE_INLINE bool absolute_offset(std::string_view str, std::string_view substr, size_t suboff, size_t& aboff) {
        return substring_internal::absolute_offset_impl(str, substr, suboff, aboff);
    }
    KUMO_FORCE_INLINE bool absolute_offset(std::u16string_view str, std::u16string_view substr, size_t suboff, size_t& aboff) {
        return substring_internal::absolute_offset_impl(str, substr, suboff, aboff);
    }
    KUMO_FORCE_INLINE bool absolute_offset(std::u32string_view str, std::u32string_view substr, size_t suboff, size_t& aboff) {
        return substring_internal::absolute_offset_impl(str, substr, suboff, aboff);
    }
    KUMO_FORCE_INLINE bool absolute_offset(std::wstring_view str, std::wstring_view substr, size_t suboff, size_t& aboff) {
        return substring_internal::absolute_offset_impl(str, substr, suboff, aboff);
    }

    KUMO_FORCE_INLINE std::optional<size_t> absolute_offset(std::string_view str, std::string_view substr, size_t suboff) {
        return substring_internal::absolute_offset_impl(str, substr, suboff);
    }
    KUMO_FORCE_INLINE std::optional<size_t> absolute_offset(std::u16string_view str, std::u16string_view substr, size_t suboff) {
        return substring_internal::absolute_offset_impl(str, substr, suboff);
    }
    KUMO_FORCE_INLINE std::optional<size_t> absolute_offset(std::u32string_view str, std::u32string_view substr, size_t suboff) {
        return substring_internal::absolute_offset_impl(str, substr, suboff);
    }
    KUMO_FORCE_INLINE std::optional<size_t> absolute_offset(std::wstring_view str, std::wstring_view substr, size_t suboff) {
        return substring_internal::absolute_offset_impl(str, substr, suboff);
    }

    KUMO_FORCE_INLINE bool sub_offset(std::string_view str, std::string_view substr, size_t aboff, size_t& suboff) {
        return substring_internal::sub_offset_impl(str, substr, aboff, suboff);
    }
    KUMO_FORCE_INLINE bool sub_offset(std::u16string_view str, std::u16string_view substr, size_t aboff, size_t& suboff) {
        return substring_internal::sub_offset_impl(str, substr, aboff, suboff);
    }
    KUMO_FORCE_INLINE bool sub_offset(std::u32string_view str, std::u32string_view substr, size_t aboff, size_t& suboff) {
        return substring_internal::sub_offset_impl(str, substr, aboff, suboff);
    }
    KUMO_FORCE_INLINE bool sub_offset(std::wstring_view str, std::wstring_view substr, size_t aboff, size_t& suboff) {
        return substring_internal::sub_offset_impl(str, substr, aboff, suboff);
    }

    KUMO_FORCE_INLINE std::optional<size_t> sub_offset(std::string_view str, std::string_view substr, size_t aboff) {
        return substring_internal::sub_offset_impl(str, substr, aboff);
    }
    KUMO_FORCE_INLINE std::optional<size_t> sub_offset(std::u16string_view str, std::u16string_view substr, size_t aboff) {
        return substring_internal::sub_offset_impl(str, substr, aboff);
    }
    KUMO_FORCE_INLINE std::optional<size_t> sub_offset(std::u32string_view str, std::u32string_view substr, size_t aboff) {
        return substring_internal::sub_offset_impl(str, substr, aboff);
    }
    KUMO_FORCE_INLINE std::optional<size_t> sub_offset(std::wstring_view str, std::wstring_view substr, size_t aboff) {
        return substring_internal::sub_offset_impl(str, substr, aboff);
    }

}  // namespace turbo
