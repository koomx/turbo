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

#include <array>
#include <cstdint>
#include <string>
#include <string_view>

#include <gtest/gtest.h>
#include <turbo/unicode/api/wchar.h>

namespace {

static const std::string abcd_str = "abcd";
static const std::wstring abcd_wstr = L"abcd";

static const std::array<uint8_t, 12 + 1> egypt_utf8_codeunits {
    { 0xF0, 0x93, 0x82, 0x80, 0xF0, 0x93, 0x82, 0x80, 0xF0, 0x93, 0x82, 0x80 }
};
static const std::string egypt_str(reinterpret_cast<const char*>(egypt_utf8_codeunits.data()));

#ifdef _WIN32
static const std::array<uint16_t, 6 + 1> egypt_utf16_codeunits {
    { 0xD80C, 0xDC80, 0xD80C, 0xDC80, 0xD80C, 0xDC80 }
};
static const std::wstring egypt_wstr(reinterpret_cast<const wchar_t*>(egypt_utf16_codeunits.data()));
#else
static const std::array<uint32_t, 3 + 1> egypt_utf32_codeunits {
    { 0x00013080, 0x00013080, 0x00013080 }
};
static const std::wstring egypt_wstr(reinterpret_cast<const wchar_t*>(egypt_utf32_codeunits.data()));
#endif

static const std::array<uint8_t, 50 + 1> hello_utf8_codeunits {
    { 0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x48, 0x61, 0x6c, 0x6c, 0xc3, 0xb3, 0x20, 0xd0, 0x9f, 0xd1, 0x80,
        0xd0, 0xb8, 0xd0, 0xb2, 0xd0, 0xb5, 0xd1, 0x82, 0x20, 0xe4, 0xbd, 0xa0, 0xe5, 0xa5, 0xbd, 0x20, 0xf0,
        0x9f, 0x91, 0xa9, 0xe2, 0x80, 0x8d, 0xf0, 0x9f, 0x9a, 0x80, 0xe2, 0x9d, 0xa4, 0xef, 0xb8, 0x8f }
};
static const std::string hello_str(reinterpret_cast<const char*>(hello_utf8_codeunits.data()));

#ifdef _WIN32
static const std::array<uint16_t, 29 + 1> hello_utf16_codeunits {
    { 0x0048, 0x0065, 0x006c, 0x006c, 0x006f, 0x0020, 0x0048, 0x0061, 0x006c, 0x006c,
        0x00f3, 0x0020, 0x041f, 0x0440, 0x0438, 0x0432, 0x0435, 0x0442, 0x0020, 0x4f60,
        0x597d, 0x0020, 0xd83d, 0xdc69, 0x200d, 0xd83d, 0xde80, 0x2764, 0xfe0f }
};
static const std::wstring hello_wstr(reinterpret_cast<const wchar_t*>(hello_utf16_codeunits.data()));
#else
static const std::array<uint32_t, 27 + 1> hello_utf32_codeunits {
    { 0x00000048, 0x00000065, 0x0000006c, 0x0000006c, 0x0000006f, 0x00000020, 0x00000048, 0x00000061, 0x0000006c,
        0x0000006c, 0x000000f3, 0x00000020, 0x0000041f, 0x00000440, 0x00000438, 0x00000432, 0x00000435, 0x00000442,
        0x00000020, 0x00004f60, 0x0000597d, 0x00000020, 0x0001f469, 0x0000200d, 0x0001f680, 0x00002764, 0x0000fe0f }
};
static const std::wstring hello_wstr(reinterpret_cast<const wchar_t*>(hello_utf32_codeunits.data()));
#endif

TEST(WcharConvert, Utf8ToWchar) {
    EXPECT_EQ(abcd_wstr, turbo::convert_utf8_to_wchar(abcd_str));
    EXPECT_EQ(egypt_wstr, turbo::convert_utf8_to_wchar(egypt_str));
    EXPECT_EQ(hello_wstr, turbo::convert_utf8_to_wchar(hello_str));
    EXPECT_EQ(hello_wstr, turbo::convert_utf8_to_wchar(std::string_view { hello_str }));
}

TEST(WcharConvert, Utf8ToWcharSize) {
    const std::string longer = "Hello world";
    const std::wstring expected = L"Hello ";
    EXPECT_EQ(expected, turbo::convert_utf8_to_wchar(std::string_view { longer }.substr(0, 6)));

    const std::array<char, 5> not_terminated { { 'a', 'b', 'c', 'd', 'e' } };
    EXPECT_EQ(std::wstring(L"abc"),
        turbo::convert_utf8_to_wchar(std::string_view(not_terminated.data(), 3)));
}

TEST(WcharConvert, WcharToUtf8) {
    EXPECT_EQ(abcd_str, turbo::convert_wchar_to_utf8(abcd_wstr));
    EXPECT_EQ(egypt_str, turbo::convert_wchar_to_utf8(egypt_wstr));
    EXPECT_EQ(hello_str, turbo::convert_wchar_to_utf8(hello_wstr));
    EXPECT_EQ(hello_str, turbo::convert_wchar_to_utf8(std::wstring_view { hello_wstr }));
}

TEST(WcharConvert, WcharToUtf8Size) {
    const std::wstring longer = L"Hello world";
    const std::string expected = "Hello ";
    EXPECT_EQ(expected, turbo::convert_wchar_to_utf8(std::wstring_view { longer }.substr(0, 6)));

    const std::array<wchar_t, 5> not_terminated { { L'a', L'b', L'c', L'd', L'e' } };
    EXPECT_EQ(std::string("abc"),
        turbo::convert_wchar_to_utf8(std::wstring_view(not_terminated.data(), 3)));
}

TEST(WcharConvert, Empty) {
    EXPECT_TRUE(turbo::convert_wchar_to_utf8(std::wstring_view {}).empty());
    EXPECT_TRUE(turbo::convert_utf8_to_wchar(std::string_view {}).empty());
}

} // namespace
