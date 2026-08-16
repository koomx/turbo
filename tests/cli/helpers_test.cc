// Copyright (c) 2017-2026, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "app_helper.hpp"
#include "../config.h"

#include <cmath>

#include <array>
#include <atomic>
#include <complex>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <limits>
#include <map>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

class NotStreamable {};

class Streamable {};

std::ostream &operator<<(std::ostream &out, const Streamable &) { return out << "Streamable"; }

TEST_CASE("TypeTools: Streaming", "[helpers]") {

    CHECK(xcli::detail::to_string(NotStreamable{}).empty());

    CHECK("Streamable" == xcli::detail::to_string(Streamable{}));

    CHECK("5" == xcli::detail::to_string(5));

    CHECK(std::string("string") == xcli::detail::to_string("string"));
    CHECK(std::string("string") == xcli::detail::to_string(std::string("string")));

    enum class t1 : std::uint8_t { enum1, enum2 };
    CHECK(xcli::detail::to_string(t1::enum1) == "0");
}

TEST_CASE("TypeTools: tuple", "[helpers]") {
    CHECK_FALSE(xcli::detail::is_tuple_like<int>::value);
    CHECK_FALSE(xcli::detail::is_tuple_like<std::vector<double>>::value);
    auto v = xcli::detail::is_tuple_like<std::tuple<double, int>>::value;
    CHECK(v);
    v = xcli::detail::is_tuple_like<std::tuple<double, double, double>>::value;
    CHECK(v);
}

TEST_CASE("TypeTools: tuple_to_string", "[helpers]") {
    std::pair<double, std::string> p1{0.999, "kWh"};
    CHECK(xcli::detail::to_string(p1) == "[0.999,kWh]");

    const std::tuple<std::string> t1{"kWh"};
    CHECK(xcli::detail::to_string(t1) == "kWh");

    const std::tuple<double> td{0.999};
    CHECK(xcli::detail::to_string(td) == "0.999");
}

TEST_CASE("TypeTools: type_size", "[helpers]") {
    auto V = xcli::detail::type_count<int>::value;
    CHECK(1 == V);
    V = xcli::detail::type_count<void>::value;
    CHECK(0 == V);
    V = xcli::detail::type_count<std::vector<double>>::value;
    CHECK(1 == V);
    V = xcli::detail::type_count<std::tuple<double, int>>::value;
    CHECK(2 == V);
    V = xcli::detail::type_count<std::tuple<std::string, double, int>>::value;
    CHECK(3 == V);
    V = xcli::detail::type_count<std::array<std::string, 5>>::value;
    CHECK(5 == V);
    V = xcli::detail::type_count<std::vector<std::pair<std::string, double>>>::value;
    CHECK(2 == V);
    V = xcli::detail::type_count<std::tuple<std::pair<std::string, double>>>::value;
    CHECK(2 == V);
    V = xcli::detail::type_count<std::tuple<int, std::pair<std::string, double>>>::value;
    CHECK(3 == V);
    V = xcli::detail::type_count<std::tuple<std::pair<int, double>, std::pair<std::string, double>>>::value;
    CHECK(4 == V);
    // maps
    V = xcli::detail::type_count<std::map<int, std::pair<int, double>>>::value;
    CHECK(3 == V);
    // three level tuples
    V = xcli::detail::type_count<std::tuple<int, std::pair<int, std::tuple<int, double, std::string>>>>::value;
    CHECK(5 == V);
    V = xcli::detail::type_count<std::pair<int, std::vector<int>>>::value;
    CHECK(xcli::detail::expected_max_vector_size <= V);
    V = xcli::detail::type_count<std::vector<std::vector<int>>>::value;
    CHECK(xcli::detail::expected_max_vector_size == V);
}

TEST_CASE("TypeTools: type_size_min", "[helpers]") {
    auto V = xcli::detail::type_count_min<int>::value;
    CHECK(1 == V);
    V = xcli::detail::type_count_min<void>::value;
    CHECK(0 == V);
    V = xcli::detail::type_count_min<std::vector<double>>::value;
    CHECK(1 == V);
    V = xcli::detail::type_count_min<std::tuple<double, int>>::value;
    CHECK(2 == V);
    V = xcli::detail::type_count_min<std::tuple<std::string, double, int>>::value;
    CHECK(3 == V);
    V = xcli::detail::type_count_min<std::array<std::string, 5>>::value;
    CHECK(5 == V);
    V = xcli::detail::type_count_min<std::vector<std::pair<std::string, double>>>::value;
    CHECK(2 == V);
    V = xcli::detail::type_count_min<std::tuple<std::pair<std::string, double>>>::value;
    CHECK(2 == V);
    V = xcli::detail::type_count_min<std::tuple<int, std::pair<std::string, double>>>::value;
    CHECK(3 == V);
    V = xcli::detail::type_count_min<std::tuple<std::pair<int, double>, std::pair<std::string, double>>>::value;
    CHECK(4 == V);
    // maps
    V = xcli::detail::type_count_min<std::map<int, std::pair<int, double>>>::value;
    CHECK(3 == V);
    // three level tuples
    V = xcli::detail::type_count_min<std::tuple<int, std::pair<int, std::tuple<int, double, std::string>>>>::value;
    CHECK(5 == V);
    V = xcli::detail::type_count_min<std::pair<int, std::vector<int>>>::value;
    CHECK(2 == V);
    V = xcli::detail::type_count_min<std::vector<std::vector<int>>>::value;
    CHECK(1 == V);
    V = xcli::detail::type_count_min<std::vector<std::vector<std::pair<int, int>>>>::value;
    CHECK(2 == V);
}

TEST_CASE("TypeTools: expected_count", "[helpers]") {
    auto V = xcli::detail::expected_count<int>::value;
    CHECK(1 == V);
    V = xcli::detail::expected_count<void>::value;
    CHECK(0 == V);
    V = xcli::detail::expected_count<std::vector<double>>::value;
    CHECK(xcli::detail::expected_max_vector_size == V);
    V = xcli::detail::expected_count<std::tuple<double, int>>::value;
    CHECK(1 == V);
    V = xcli::detail::expected_count<std::tuple<std::string, double, int>>::value;
    CHECK(1 == V);
    V = xcli::detail::expected_count<std::array<std::string, 5>>::value;
    CHECK(1 == V);
    V = xcli::detail::expected_count<std::vector<std::pair<std::string, double>>>::value;
    CHECK(xcli::detail::expected_max_vector_size == V);
}

TEST_CASE("Split: SimpleByToken", "[helpers]") {
    auto out = xcli::detail::split("one.two.three", '.');
    REQUIRE(out.size() == 3u);
    CHECK(out.at(0) == "one");
    CHECK(out.at(1) == "two");
    CHECK(out.at(2) == "three");
}

TEST_CASE("Split: Single", "[helpers]") {
    auto out = xcli::detail::split("one", '.');
    REQUIRE(out.size() == 1u);
    CHECK(out.at(0) == "one");
}

TEST_CASE("Split: Empty", "[helpers]") {
    auto out = xcli::detail::split("", '.');
    REQUIRE(out.size() == 1u);
    CHECK(out.at(0).empty());
}

TEST_CASE("String: InvalidName", "[helpers]") {
    CHECK(xcli::detail::valid_name_string("valid"));
    CHECK_FALSE(xcli::detail::valid_name_string("-invalid"));
    CHECK(xcli::detail::valid_name_string("va-li-d"));
    CHECK_FALSE(xcli::detail::valid_name_string("valid{}"));
    CHECK(xcli::detail::valid_name_string("_valid"));
    CHECK(xcli::detail::valid_name_string("/valid"));
    CHECK(xcli::detail::valid_name_string("vali?d"));
    CHECK(xcli::detail::valid_name_string("@@@@"));
    CHECK(xcli::detail::valid_name_string("b@d2?"));
    CHECK(xcli::detail::valid_name_string("2vali?d"));
    CHECK_FALSE(xcli::detail::valid_name_string("!valid"));
    CHECK_FALSE(xcli::detail::valid_name_string("!va\nlid"));
}

TEST_CASE("StringTools: Modify", "[helpers]") {
    int cnt{0};
    std::string newString = xcli::detail::find_and_modify("======", "=", [&cnt](std::string &str, std::size_t index) {
        if((++cnt) % 2 == 0) {
            str[index] = ':';
        }
        return index + 1;
    });
    CHECK("=:=:=:" == newString);
}

TEST_CASE("StringTools: Modify2", "[helpers]") {
    std::string newString =
        xcli::detail::find_and_modify("this is a string test", "is", [](std::string &str, std::size_t index) {
            if((index > 1) && (str[index - 1] != ' ')) {
                str[index] = 'a';
                str[index + 1] = 't';
            }
            return index + 1;
        });
    CHECK("that is a string test" == newString);
}

TEST_CASE("StringTools: Modify3", "[helpers]") {
    // this picks up 3 sets of 3 after the 'b' then collapses the new first set
    std::string newString = xcli::detail::find_and_modify("baaaaaaaaaa", "aaa", [](std::string &str, std::size_t index) {
        str.erase(index, 3);
        str.insert(str.begin(), 'a');
        return 0u;
    });
    CHECK("aba" == newString);
}

TEST_CASE("StringTools: escape_detect", "[helpers]") {
    // a trigger char preceded by a '-' (for '=') is reinterpreted as a space so split_up works
    std::string opt{"--opt=\"value\""};
    xcli::detail::escape_detect(opt, 5);
    CHECK(opt == "--opt \"value\"");

    // a leading trigger character has nothing preceding it; it must not be rewritten
    // (and must not read out of bounds at offset 0)
    std::string leading{"=\"value\""};
    auto ret = xcli::detail::escape_detect(leading, 0);
    CHECK(ret == 1U);
    CHECK(leading == "=\"value\"");

    // make sure a later '-' does not get wrongly matched for a leading trigger
    std::string leading2{"=\"-value\""};
    xcli::detail::escape_detect(leading2, 0);
    CHECK(leading2 == "=\"-value\"");
}

TEST_CASE("StringTools: flagValues", "[helpers]") {
    errno = 0;
    CHECK(-1 == xcli::detail::to_flag_value("0"));
    CHECK(errno == 0);
    CHECK(1 == xcli::detail::to_flag_value("t"));
    CHECK(1 == xcli::detail::to_flag_value("1"));
    CHECK(6 == xcli::detail::to_flag_value("6"));
    CHECK(-6 == xcli::detail::to_flag_value("-6"));
    CHECK(-1 == xcli::detail::to_flag_value("false"));
    CHECK(1 == xcli::detail::to_flag_value("YES"));
    errno = 0;
    xcli::detail::to_flag_value("frog");
    CHECK(errno == EINVAL);
    errno = 0;
    xcli::detail::to_flag_value("q");
    CHECK(errno == EINVAL);
    errno = 0;
    xcli::detail::to_flag_value(
        "77777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777");
    CHECK(errno == ERANGE);
    errno = 0;
    CHECK(-1 == xcli::detail::to_flag_value("NO"));
    CHECK(475555233 == xcli::detail::to_flag_value("475555233"));
}

TEST_CASE("StringTools: Validation", "[helpers]") {
    CHECK(xcli::detail::isalpha(""));
    CHECK(xcli::detail::isalpha("a"));
    CHECK(xcli::detail::isalpha("abcd"));
    CHECK_FALSE(xcli::detail::isalpha("_"));
    CHECK_FALSE(xcli::detail::isalpha("2"));
    CHECK_FALSE(xcli::detail::isalpha("test test"));
    CHECK_FALSE(xcli::detail::isalpha("test "));
    CHECK_FALSE(xcli::detail::isalpha(" test"));
    CHECK_FALSE(xcli::detail::isalpha("test2"));
}

TEST_CASE("StringTools: binaryEscapeConversion", "[helpers]") {
    std::string testString("string1");
    std::string estring = xcli::detail::binary_escape_string(testString);
    CHECK(testString == estring);
    CHECK_FALSE(xcli::detail::is_binary_escaped_string(estring));

    std::string testString2("\nstring1\n");
    estring = xcli::detail::binary_escape_string(testString2);
    CHECK_FALSE(testString == estring);
    CHECK(xcli::detail::is_binary_escaped_string(estring));
    std::string rstring = xcli::detail::extract_binary_string(estring);
    CHECK(rstring == testString2);

    xcli::detail::remove_quotes(estring);
    CHECK(xcli::detail::is_binary_escaped_string(estring));
    std::string rstringrq = xcli::detail::extract_binary_string(estring);
    CHECK(rstringrq == testString2);

    testString2.push_back(0);
    testString2.push_back(static_cast<char>(197));
    testString2.push_back(78);
    testString2.push_back(static_cast<char>(-34));

    rstring = xcli::detail::extract_binary_string(xcli::detail::binary_escape_string(testString2));
    CHECK(rstring == testString2);

    testString2.push_back('b');
    testString2.push_back('G');

    rstring = xcli::detail::extract_binary_string(xcli::detail::binary_escape_string(testString2));
    CHECK(rstring == testString2);
    auto rstring2 = xcli::detail::extract_binary_string(rstring);
    CHECK(rstring == rstring2);
}

TEST_CASE("StringTools: binaryEscapeConversion2", "[helpers]") {
    std::string testString;
    testString.push_back(0);
    testString.push_back(0);
    testString.push_back(0);
    testString.push_back(56);
    testString.push_back(static_cast<char>(-112));
    testString.push_back(static_cast<char>(-112));
    testString.push_back(39);
    testString.push_back(97);
    std::string estring = xcli::detail::binary_escape_string(testString);
    CHECK(xcli::detail::is_binary_escaped_string(estring));
    std::string rstring = xcli::detail::extract_binary_string(estring);
    CHECK(rstring == testString);
}

TEST_CASE("StringTools: binaryEscapeConversion_withX", "[helpers]") {
    std::string testString("hippy\\x35mm\\XF3_helpX26fox19");
    testString.push_back(0);
    testString.push_back(0);
    testString.push_back(0);
    testString.push_back(56);
    testString.push_back(static_cast<char>(-112));
    testString.push_back(static_cast<char>(-112));
    testString.push_back(39);
    testString.push_back(97);
    std::string estring = xcli::detail::binary_escape_string(testString);
    CHECK(xcli::detail::is_binary_escaped_string(estring));
    std::string rstring = xcli::detail::extract_binary_string(estring);
    CHECK(rstring == testString);
}

TEST_CASE("StringTools: binaryEscapeConversion_withBrackets", "[helpers]") {

    std::string vstr = R"raw('B"([\xb0\x0a\xb0/\xb0\xb0\xb0\xb0\xb0\xb0\xb0\xb0\xb0\xb0\xb0\xb0\xb0])"')raw";
    std::string testString("[");
    testString.push_back(static_cast<char>(-80));
    testString.push_back('\n');
    testString.push_back(static_cast<char>(-80));
    testString.push_back('/');
    for(int ii = 0; ii < 13; ++ii) {
        testString.push_back(static_cast<char>(-80));
    }
    testString.push_back(']');

    std::string estring = xcli::detail::binary_escape_string(testString);
    CHECK(xcli::detail::is_binary_escaped_string(estring));
    CHECK(estring == vstr);
    std::string rstring = xcli::detail::extract_binary_string(estring);
    CHECK(rstring == testString);
}

TEST_CASE("StringTools: binaryStrings", "[helpers]") {
    std::string rstring = "B\"()\"";
    CHECK(xcli::detail::extract_binary_string(rstring).empty());

    rstring = "B\"(\\x35\\xa7)\"";
    CHECK(xcli::detail::is_binary_escaped_string(rstring));
    auto result = xcli::detail::extract_binary_string(rstring);
    CHECK(result[0] == static_cast<char>(0x35));
    CHECK(result[1] == static_cast<char>(0xa7));

    rstring = "'B\"(\\x3e\\xf7)\"'";
    CHECK(xcli::detail::is_binary_escaped_string(rstring));
    result = xcli::detail::extract_binary_string(rstring);
    CHECK(result[0] == static_cast<char>(0x3e));
    CHECK(result[1] == static_cast<char>(0xf7));

    rstring = "B\"(\\x3E\\xf7)\"";
    result = xcli::detail::extract_binary_string(rstring);
    CHECK(result[0] == static_cast<char>(0x3e));
    CHECK(result[1] == static_cast<char>(0xf7));

    rstring = "B\"(\\X3E\\XF7)\"";
    result = xcli::detail::extract_binary_string(rstring);
    CHECK(result[0] == static_cast<char>(0x3e));
    CHECK(result[1] == static_cast<char>(0xf7));

    rstring = "B\"(\\XME\\XK7)\"";
    result = xcli::detail::extract_binary_string(rstring);
    CHECK(result == "\\XME\\XK7");

    rstring = "B\"(\\XEM\\X7K)\"";
    result = xcli::detail::extract_binary_string(rstring);
    CHECK(result == "\\XEM\\X7K");
}

TEST_CASE("StringTools: escapeConversion", "[helpers]") {
    CHECK(xcli::detail::remove_escaped_characters("test\\\"") == "test\"");
    CHECK(xcli::detail::remove_escaped_characters("test\\\\") == "test\\");
    CHECK(xcli::detail::remove_escaped_characters("test\\b") == "test\b");
    CHECK(xcli::detail::remove_escaped_characters("test\\t") == "test\t");
    CHECK(xcli::detail::remove_escaped_characters("test\\n\\r\\t\\f") == "test\n\r\t\f");
    CHECK(xcli::detail::remove_escaped_characters("test\\r") == "test\r");
    CHECK(xcli::detail::remove_escaped_characters("test\\f") == "test\f");
    std::string zstring = "test";
    zstring.push_back('\0');
    zstring.append("test\n");
    CHECK(xcli::detail::remove_escaped_characters("test\\0test\\n") == zstring);

    CHECK_THROWS_AS(xcli::detail::remove_escaped_characters("test\\m_bad"), std::invalid_argument);
    CHECK_THROWS_AS(xcli::detail::remove_escaped_characters("test\\"), std::invalid_argument);
}

TEST_CASE("StringTools: quotedString", "[helpers]") {

    std::string rstring = "'B\"(\\x35\\xa7)\"'";
    std::string s2{rstring};
    xcli::detail::process_quoted_string(s2);
    CHECK(s2[0] == static_cast<char>(0x35));
    CHECK(s2[1] == static_cast<char>(0xa7));
    s2 = rstring;
    xcli::detail::remove_quotes(s2);
    xcli::detail::process_quoted_string(s2);
    CHECK(s2[0] == static_cast<char>(0x35));
    CHECK(s2[1] == static_cast<char>(0xa7));

    std::string qbase = R"("this\nis\na\nfour\tline test")";
    std::string qresult = "this\nis\na\nfour\tline test";

    std::string q1 = qbase;

    // test remove quotes and escape processing
    xcli::detail::process_quoted_string(q1);
    CHECK(q1 == qresult);

    std::string q2 = qbase;
    q2.front() = '\'';
    q2.pop_back();
    q2.push_back('\'');
    std::string qliteral = qbase.substr(1);
    qliteral.pop_back();

    // test remove quotes for literal string
    CHECK(xcli::detail::process_quoted_string(q2));
    CHECK(q2 == qliteral);

    std::string q3 = qbase;
    q3.front() = '`';
    q3.pop_back();
    q3.push_back('`');

    // test remove quotes for literal string
    CHECK(xcli::detail::process_quoted_string(q3));
    CHECK(q3 == qliteral);

    std::string q4 = qbase;
    q4.front() = '|';
    q4.pop_back();
    q4.push_back('|');

    // check that it doesn't process
    CHECK_FALSE(xcli::detail::process_quoted_string(q4));
    // test custom string quote character
    CHECK(xcli::detail::process_quoted_string(q4, '|'));
    CHECK(q4 == qresult);

    std::string q5 = qbase;
    q5.front() = '?';
    q5.pop_back();
    q5.push_back('?');

    // test custom literal quote character
    CHECK(xcli::detail::process_quoted_string(q5, '|', '?'));
    CHECK(q5 == qliteral);

    q3 = qbase;
    q3.front() = '`';
    q3.pop_back();
    q3.push_back('`');

    // test that '`' still works regardless of the other specified characters
    CHECK(xcli::detail::process_quoted_string(q3));
    CHECK(q3 == qliteral);
}

TEST_CASE("StringTools: unicode_literals", "[helpers]") {

    CHECK(xcli::detail::remove_escaped_characters("test\\u03C0\\u00e9") == from_u8string(u8"test\u03C0\u00E9"));
    CHECK(xcli::detail::remove_escaped_characters("test\\u73C0\\u0057") == from_u8string(u8"test\u73C0\u0057"));

    CHECK(xcli::detail::remove_escaped_characters("test\\U0001F600\\u00E9") == from_u8string(u8"test\U0001F600\u00E9"));

    CHECK_THROWS_AS(xcli::detail::remove_escaped_characters("test\\U0001M600\\u00E9"), std::invalid_argument);
    CHECK_THROWS_AS(xcli::detail::remove_escaped_characters("test\\U0001E600\\u00M9"), std::invalid_argument);
    CHECK_THROWS_AS(xcli::detail::remove_escaped_characters("test\\U0001E600\\uD8E9"), std::invalid_argument);

    CHECK_THROWS_AS(xcli::detail::remove_escaped_characters("test\\U0001E600\\uD8"), std::invalid_argument);
    CHECK_THROWS_AS(xcli::detail::remove_escaped_characters("test\\U0001E60"), std::invalid_argument);
    // code points above U+10FFFF are not valid and must throw rather than silently vanish
    CHECK_THROWS_AS(xcli::detail::remove_escaped_characters("test\\U00110000"), std::invalid_argument);
    CHECK_THROWS_AS(xcli::detail::remove_escaped_characters("test\\UFFFFFFFF"), std::invalid_argument);
    // the largest valid code point still works
    CHECK(xcli::detail::remove_escaped_characters("test\\U0010FFFF") == from_u8string(u8"test\U0010FFFF"));
}

TEST_CASE("StringTools: close_sequence", "[helpers]") {
    CHECK(xcli::detail::close_sequence("[test]", 0, ']') == 5U);
    CHECK(xcli::detail::close_sequence("[\"test]\"]", 0, ']') == 8U);
    CHECK(xcli::detail::close_sequence("[\"test]\"],[t2]", 0, ']') == 8U);
    CHECK(xcli::detail::close_sequence("[\"test]\"],[t2]", 10, ']') == 13U);
    CHECK(xcli::detail::close_sequence("{\"test]\"],[t2]", 0, '}') == 14U);
    CHECK(xcli::detail::close_sequence("[(),(),{},\"]]52{}\",[],[54],[[],[],()]]", 0, ']') == 37U);
}

TEST_CASE("Trim: Various", "[helpers]") {
    std::string s1{"  sdlfkj sdflk sd s  "};
    std::string a1{"sdlfkj sdflk sd s"};
    xcli::detail::trim(s1);
    CHECK(s1 == a1);

    std::string s2{" a \t"};
    xcli::detail::trim(s2);
    CHECK(s2 == "a");

    std::string s3{" a \n"};
    xcli::detail::trim(s3);
    CHECK(s3 == "a");

    std::string s4{" a b "};
    CHECK(xcli::detail::trim(s4) == "a b");
}

TEST_CASE("Trim: VariousFilters", "[helpers]") {
    std::string s1{"  sdlfkj sdflk sd s  "};
    std::string a1{"sdlfkj sdflk sd s"};
    xcli::detail::trim(s1, " ");
    CHECK(s1 == a1);

    std::string s2{" a \t"};
    xcli::detail::trim(s2, " ");
    CHECK(s2 == "a \t");

    std::string s3{"abdavda"};
    xcli::detail::trim(s3, "a");
    CHECK(s3 == "bdavd");

    std::string s4{"abcabcabc"};
    CHECK(xcli::detail::trim(s4, "ab") == "cabcabc");
}

TEST_CASE("Trim: TrimCopy", "[helpers]") {
    std::string orig{" cabc  "};
    std::string trimmed = xcli::detail::trim_copy(orig);
    CHECK(trimmed == "cabc");
    CHECK(trimmed != orig);
    xcli::detail::trim(orig);
    CHECK(orig == trimmed);

    orig = "abcabcabc";
    trimmed = xcli::detail::trim_copy(orig, "ab");
    CHECK(trimmed == "cabcabc");
    CHECK(trimmed != orig);
    xcli::detail::trim(orig, "ab");
    CHECK(orig == trimmed);
}

TEST_CASE("Validators: FileExists", "[helpers]") {
    std::string myfile{"TestFileNotUsed.txt"};
    CHECK_FALSE(xcli::ExistingFile(myfile).empty());
    bool ok = static_cast<bool>(std::ofstream(myfile.c_str()).put('a'));  // create file
    CHECK(ok);
    CHECK(xcli::ExistingFile(myfile).empty());

    std::remove(myfile.c_str());
    CHECK_FALSE(xcli::ExistingFile(myfile).empty());
}

TEST_CASE("Validators: FileNotExists", "[helpers]") {
    std::string myfile{"TestFileNotUsed.txt"};
    CHECK(xcli::NonexistentPath(myfile).empty());
    bool ok = static_cast<bool>(std::ofstream(myfile.c_str()).put('a'));  // create file
    CHECK(ok);
    CHECK_FALSE(xcli::NonexistentPath(myfile).empty());

    std::remove(myfile.c_str());
    CHECK(xcli::NonexistentPath(myfile).empty());
}

TEST_CASE("Validators: FilePathModifier", "[helpers]") {
    std::string myfile{"../TestFileNotUsed_1.txt"};
    bool ok = static_cast<bool>(std::ofstream(myfile.c_str()).put('a'));  // create file
    CHECK(ok);
    std::string filename = "TestFileNotUsed_1.txt";
    xcli::FileOnDefaultPath defPath("../");
    CHECK(defPath(filename).empty());
    CHECK(filename == myfile);
    std::string filename2 = "nonexistingfile.csv";
    CHECK_FALSE(defPath(filename2).empty());
    // check it didn't modify the string
    CHECK(filename2 == "nonexistingfile.csv");
    CHECK(defPath(filename).empty());
    std::remove(myfile.c_str());
    CHECK_FALSE(defPath(myfile).empty());
    // now test the no error version
    xcli::FileOnDefaultPath defPathNoFail("../", false);
    CHECK(defPathNoFail(filename2).empty());
    CHECK(filename2 == "nonexistingfile.csv");
    // empty default path must not invoke UB via back() on an empty string
    xcli::FileOnDefaultPath defPathEmpty("", false);
    std::string filename3 = "nonexistingfile.csv";
    CHECK(defPathEmpty(filename3).empty());
    xcli::FileOnDefaultPath defPathEmptyErr("", true);
    CHECK_FALSE(defPathEmptyErr(filename3).empty());
}

TEST_CASE("Validators: FileIsDir", "[helpers]") {
    std::string mydir{testing::testing_root()};
    CHECK(!xcli::ExistingFile(mydir).empty());
}

TEST_CASE("Validators: DirectoryExists", "[helpers]") {
    std::string mydir{testing::testing_root()};
    CHECK(xcli::ExistingDirectory(mydir).empty());
}

TEST_CASE("Validators: DirectoryNotExists", "[helpers]") {
    std::string mydir{"nondirectory"};
    CHECK(!xcli::ExistingDirectory(mydir).empty());
}

TEST_CASE("Validators: DirectoryIsFile", "[helpers]") {
    std::string myfile{"TestFileNotUsed.txt"};
    CHECK(xcli::NonexistentPath(myfile).empty());
    bool ok = static_cast<bool>(std::ofstream(myfile.c_str()).put('a'));  // create file
    CHECK(ok);
    CHECK_FALSE(xcli::ExistingDirectory(myfile).empty());

    std::remove(myfile.c_str());
    CHECK(xcli::NonexistentPath(myfile).empty());
}

TEST_CASE("Validators: PathExistsDir", "[helpers]") {
    std::string mydir{testing::testing_root()};
    CHECK(xcli::ExistingPath(mydir).empty());
}

TEST_CASE("Validators: PathExistsFile", "[helpers]") {
    std::string myfile{"TestFileNotUsed.txt"};
    CHECK_FALSE(xcli::ExistingPath(myfile).empty());
    bool ok = static_cast<bool>(std::ofstream(myfile.c_str()).put('a'));  // create file
    CHECK(ok);
    CHECK(xcli::ExistingPath(myfile).empty());

    std::remove(myfile.c_str());
    CHECK_FALSE(xcli::ExistingPath(myfile).empty());
}

TEST_CASE("Validators: PathNotExistsDir", "[helpers]") {
    std::string mydir{"nonpath"};
    CHECK(!xcli::ExistingPath(mydir).empty());
}

TEST_CASE("Validators: PositiveValidator", "[helpers]") {
    std::string num = "1.1.1.1";
    CHECK_FALSE(xcli::PositiveNumber(num).empty());
    num = "1";
    CHECK(xcli::PositiveNumber(num).empty());
    num = "10000";
    CHECK(xcli::PositiveNumber(num).empty());
    num = "0";
    CHECK_FALSE(xcli::PositiveNumber(num).empty());
    num = "+0.5";
    CHECK(xcli::PositiveNumber(num).empty());
    num = "-1";
    CHECK_FALSE(xcli::PositiveNumber(num).empty());
    num = "-1.5";
    CHECK_FALSE(xcli::PositiveNumber(num).empty());
    num = "a";
    CHECK_FALSE(xcli::PositiveNumber(num).empty());
    // subnormal positive doubles are strictly positive and must be accepted
    num = "5e-324";  // std::numeric_limits<double>::denorm_min
    CHECK(xcli::PositiveNumber(num).empty());
}

TEST_CASE("Validators: NonNegativeValidator", "[helpers]") {
    std::string num = "1.1.1.1";
    CHECK_FALSE(xcli::NonNegativeNumber(num).empty());
    num = "1";
    CHECK(xcli::NonNegativeNumber(num).empty());
    num = "10000";
    CHECK(xcli::NonNegativeNumber(num).empty());
    num = "0";
    CHECK(xcli::NonNegativeNumber(num).empty());
    num = "+0.5";
    CHECK(xcli::NonNegativeNumber(num).empty());
    num = "-1";
    CHECK_FALSE(xcli::NonNegativeNumber(num).empty());
    num = "-1.5";
    CHECK_FALSE(xcli::NonNegativeNumber(num).empty());
    num = "a";
    CHECK_FALSE(xcli::NonNegativeNumber(num).empty());
}

TEST_CASE("Validators: CombinedAndRange", "[helpers]") {
    auto crange = xcli::Range(0, 12) & xcli::Range(4, 16);
    CHECK(crange("4").empty());
    CHECK(crange("12").empty());
    CHECK(crange("7").empty());

    CHECK_FALSE(crange("-2").empty());
    CHECK_FALSE(crange("2").empty());
    CHECK_FALSE(crange("15").empty());
    CHECK_FALSE(crange("16").empty());
    CHECK_FALSE(crange("18").empty());
}

TEST_CASE("Validators: CombinedOrRange", "[helpers]") {
    auto crange = xcli::Range(0, 4) | xcli::Range(8, 12);

    CHECK_FALSE(crange("-2").empty());
    CHECK(crange("2").empty());
    CHECK_FALSE(crange("5").empty());
    CHECK(crange("8").empty());
    CHECK(crange("12").empty());
    CHECK_FALSE(crange("16").empty());
}

TEST_CASE("Validators: CombinedPaths", "[helpers]") {
    std::string myfile{"TestFileNotUsed.txt"};
    CHECK_FALSE(xcli::ExistingFile(myfile).empty());
    bool ok = static_cast<bool>(std::ofstream(myfile.c_str()).put('a'));  // create file
    CHECK(ok);

    std::string dir{testing::testing_root()};
    std::string notpath{"nondirectory"};

    auto path_or_dir = xcli::ExistingPath | xcli::ExistingDirectory;
    CHECK(path_or_dir(dir).empty());
    CHECK(path_or_dir(myfile).empty());
    CHECK_FALSE(path_or_dir(notpath).empty());

    auto file_or_dir = xcli::ExistingFile | xcli::ExistingDirectory;
    CHECK(file_or_dir(dir).empty());
    CHECK(file_or_dir(myfile).empty());
    CHECK_FALSE(file_or_dir(notpath).empty());

    auto path_and_dir = xcli::ExistingPath & xcli::ExistingDirectory;
    CHECK(path_and_dir(dir).empty());
    CHECK_FALSE(path_and_dir(myfile).empty());
    CHECK_FALSE(path_and_dir(notpath).empty());

    auto path_and_file = xcli::ExistingFile & xcli::ExistingDirectory;
    CHECK_FALSE(path_and_file(dir).empty());
    CHECK_FALSE(path_and_file(myfile).empty());
    CHECK_FALSE(path_and_file(notpath).empty());

    std::remove(myfile.c_str());
    CHECK_FALSE(xcli::ExistingFile(myfile).empty());
}

TEST_CASE("Validators: ProgramNameSplit", "[helpers]") {
    TempFile myfile{"program_name1.exe"};
    {
        std::ofstream out{myfile};
        out << "useless string doesn't matter" << '\n';
    }
    auto res =
        xcli::detail::split_program_name(std::string("./") + std::string(myfile) + " this is a bunch of extra stuff  ");
    CHECK(std::string("./") + std::string(myfile) == res.first);
    CHECK("this is a bunch of extra stuff" == res.second);

    TempFile myfile2{"program name1.exe"};
    {
        std::ofstream out{myfile2};
        out << "useless string doesn't matter" << '\n';
    }
    res = xcli::detail::split_program_name(std::string("   ") + std::string("./") + std::string(myfile2) +
                                          "      this is a bunch of extra stuff  ");
    CHECK(std::string("./") + std::string(myfile2) == res.first);
    CHECK("this is a bunch of extra stuff" == res.second);

    res = xcli::detail::split_program_name("./program_name    this is a bunch of extra stuff  ");
    CHECK("./program_name" == res.first);
    CHECK("this is a bunch of extra stuff" == res.second);

    res = xcli::detail::split_program_name(std::string("  ./") + std::string(myfile) + "    ");
    CHECK(std::string("./") + std::string(myfile) == res.first);
    CHECK(res.second.empty());

    res = xcli::detail::split_program_name("'odd_program_name.exe --arg --arg2=5");
    CHECK("'odd_program_name.exe" == res.first);
    CHECK_FALSE(res.second.empty());

    // No space in commandline: esp starts as npos, should not increment npos
    res = xcli::detail::split_program_name("not_a_real_program");
    CHECK("not_a_real_program" == res.first);
    CHECK(res.second.empty());

    // Quoted program name only, no trailing args (regression: npos+1 wrap-around)
    res = xcli::detail::split_program_name(R"("C:\example.exe")");
    CHECK(R"(C:\example.exe)" == res.first);
    CHECK(res.second.empty());
}

TEST_CASE("CheckedMultiply: Int", "[helpers]") {
    int a{10};
    int b{-20};
    REQUIRE(xcli::detail::checked_multiply(a, b));
    REQUIRE(-200 == a);

    a = 0;
    b = -20;
    REQUIRE(xcli::detail::checked_multiply(a, b));
    REQUIRE(0 == a);

    a = 20;
    b = 0;
    REQUIRE(xcli::detail::checked_multiply(a, b));
    REQUIRE(0 == a);

    a = (std::numeric_limits<int>::max)();
    b = 1;
    REQUIRE(xcli::detail::checked_multiply(a, b));
    REQUIRE((std::numeric_limits<int>::max)() == a);

    a = (std::numeric_limits<int>::max)();
    b = 2;
    REQUIRE(!xcli::detail::checked_multiply(a, b));
    REQUIRE((std::numeric_limits<int>::max)() == a);

    a = (std::numeric_limits<int>::max)();
    b = -1;
    REQUIRE(xcli::detail::checked_multiply(a, b));
    REQUIRE(-(std::numeric_limits<int>::max)() == a);

    a = (std::numeric_limits<int>::max)();
    b = (std::numeric_limits<int>::max)();
    REQUIRE(!xcli::detail::checked_multiply(a, b));
    REQUIRE((std::numeric_limits<int>::max)() == a);

    a = (std::numeric_limits<int>::min)();
    b = (std::numeric_limits<int>::max)();
    REQUIRE(!xcli::detail::checked_multiply(a, b));
    REQUIRE((std::numeric_limits<int>::min)() == a);

    a = (std::numeric_limits<int>::min)();
    b = 1;
    REQUIRE(xcli::detail::checked_multiply(a, b));
    REQUIRE((std::numeric_limits<int>::min)() == a);

    a = (std::numeric_limits<int>::min)();
    b = -1;
    REQUIRE(!xcli::detail::checked_multiply(a, b));
    REQUIRE((std::numeric_limits<int>::min)() == a);

    b = (std::numeric_limits<int>::min)();
    a = -1;
    REQUIRE(!xcli::detail::checked_multiply(a, b));
    REQUIRE(-1 == a);

    a = (std::numeric_limits<int>::min)() / 100;
    b = 99;
    REQUIRE(xcli::detail::checked_multiply(a, b));
    REQUIRE((std::numeric_limits<int>::min)() / 100 * 99 == a);

    a = (std::numeric_limits<int>::min)() / 100;
    b = -101;
    REQUIRE(!xcli::detail::checked_multiply(a, b));
    REQUIRE((std::numeric_limits<int>::min)() / 100 == a);
    a = 2;
    b = (std::numeric_limits<int>::min)() / 2;
    REQUIRE(xcli::detail::checked_multiply(a, b));
    a = (std::numeric_limits<int>::min)() / 2;
    b = 2;
    REQUIRE(xcli::detail::checked_multiply(a, b));

    a = 4;
    b = (std::numeric_limits<int>::min)() / 4;
    REQUIRE(xcli::detail::checked_multiply(a, b));

    a = 48;
    b = (std::numeric_limits<int>::min)() / 48;
    REQUIRE(xcli::detail::checked_multiply(a, b));
}

TEST_CASE("CheckedMultiply: SizeT", "[helpers]") {
    std::size_t a = 10;
    std::size_t b = 20;
    REQUIRE(xcli::detail::checked_multiply(a, b));
    REQUIRE(200u == a);

    a = 0u;
    b = 20u;
    REQUIRE(xcli::detail::checked_multiply(a, b));
    REQUIRE(0u == a);

    a = 20u;
    b = 0u;
    REQUIRE(xcli::detail::checked_multiply(a, b));
    REQUIRE(0u == a);

    a = (std::numeric_limits<std::size_t>::max)();
    b = 1u;
    REQUIRE(xcli::detail::checked_multiply(a, b));
    REQUIRE((std::numeric_limits<std::size_t>::max)() == a);

    a = (std::numeric_limits<std::size_t>::max)();
    b = 2u;
    REQUIRE(!xcli::detail::checked_multiply(a, b));
    REQUIRE((std::numeric_limits<std::size_t>::max)() == a);

    a = (std::numeric_limits<std::size_t>::max)();
    b = (std::numeric_limits<std::size_t>::max)();
    REQUIRE(!xcli::detail::checked_multiply(a, b));
    REQUIRE((std::numeric_limits<std::size_t>::max)() == a);

    a = (std::numeric_limits<std::size_t>::max)() / 100;
    b = 99u;
    REQUIRE(xcli::detail::checked_multiply(a, b));
    REQUIRE((std::numeric_limits<std::size_t>::max)() / 100u * 99u == a);
}

TEST_CASE("CheckedMultiply: Float", "[helpers]") {
    float a{10.0F};
    float b{20.0F};
    REQUIRE(xcli::detail::checked_multiply(a, b));
    REQUIRE(200 == Approx(a));

    a = 0.0F;
    b = 20.0F;
    REQUIRE(xcli::detail::checked_multiply(a, b));
    REQUIRE(0 == Approx(a));

    a = INFINITY;
    b = 20.0F;
    REQUIRE(xcli::detail::checked_multiply(a, b));
    REQUIRE(INFINITY == Approx(a));

    a = 2.0F;
    b = -INFINITY;
    REQUIRE(xcli::detail::checked_multiply(a, b));
    REQUIRE(-INFINITY == Approx(a));

    a = (std::numeric_limits<float>::max)() / 100.0F;
    b = 1.0F;
    REQUIRE(xcli::detail::checked_multiply(a, b));
    REQUIRE((std::numeric_limits<float>::max)() / 100.0F == Approx(a));

    a = (std::numeric_limits<float>::max)() / 100.0F;
    b = 99.0F;
    REQUIRE(xcli::detail::checked_multiply(a, b));
    REQUIRE((std::numeric_limits<float>::max)() / 100.0F * 99.0F == Approx(a));

    a = (std::numeric_limits<float>::max)() / 100.0F;
    b = 101;
    REQUIRE(!xcli::detail::checked_multiply(a, b));
    REQUIRE((std::numeric_limits<float>::max)() / 100.0F == Approx(a));

    a = (std::numeric_limits<float>::max)() / 100.0F;
    b = -99;
    REQUIRE(xcli::detail::checked_multiply(a, b));
    REQUIRE((std::numeric_limits<float>::max)() / 100.0F * -99.0F == Approx(a));

    a = (std::numeric_limits<float>::max)() / 100.0F;
    b = -101;
    REQUIRE(!xcli::detail::checked_multiply(a, b));
    REQUIRE((std::numeric_limits<float>::max)() / 100.0F == Approx(a));
}

TEST_CASE("CheckedMultiply: Double", "[helpers]") {
    double a{10.0F};
    double b{20.0F};
    REQUIRE(xcli::detail::checked_multiply(a, b));
    REQUIRE(200 == Approx(a));

    a = 0;
    b = 20;
    REQUIRE(xcli::detail::checked_multiply(a, b));
    REQUIRE(0 == Approx(a));

    a = std::numeric_limits<double>::infinity();
    b = 20;
    REQUIRE(xcli::detail::checked_multiply(a, b));
    REQUIRE(std::numeric_limits<double>::infinity() == Approx(a));

    a = 2;
    b = -std::numeric_limits<double>::infinity();
    REQUIRE(xcli::detail::checked_multiply(a, b));
    REQUIRE(-std::numeric_limits<double>::infinity() == Approx(a));

    a = (std::numeric_limits<double>::max)() / 100;
    b = 1;
    REQUIRE(xcli::detail::checked_multiply(a, b));
    REQUIRE((std::numeric_limits<double>::max)() / 100 == Approx(a));

    a = (std::numeric_limits<double>::max)() / 100;
    b = 99;
    REQUIRE(xcli::detail::checked_multiply(a, b));
    REQUIRE((std::numeric_limits<double>::max)() / 100 * 99 == Approx(a));

    a = (std::numeric_limits<double>::max)() / 100;
    b = 101;
    REQUIRE(!xcli::detail::checked_multiply(a, b));
    REQUIRE((std::numeric_limits<double>::max)() / 100 == Approx(a));

    a = (std::numeric_limits<double>::max)() / 100;
    b = -99;
    REQUIRE(xcli::detail::checked_multiply(a, b));
    REQUIRE((std::numeric_limits<double>::max)() / 100 * -99 == Approx(a));

    a = (std::numeric_limits<double>::max)() / 100;
    b = -101;
    REQUIRE(!xcli::detail::checked_multiply(a, b));
    REQUIRE((std::numeric_limits<double>::max)() / 100 == Approx(a));
}

// Yes, this is testing an app_helper :)
TEST_CASE("AppHelper: TempfileCreated", "[helpers]") {
    std::string name = "TestFileNotUsed.txt";
    {
        TempFile myfile{name};

        CHECK_FALSE(xcli::ExistingFile(myfile).empty());

        bool ok = static_cast<bool>(std::ofstream(myfile.c_str()).put('a'));  // create file
        CHECK(ok);
        CHECK(xcli::ExistingFile(name).empty());
        CHECK_THROWS_AS([&]() { TempFile otherfile(name); }(), std::runtime_error);
    }
    CHECK_FALSE(xcli::ExistingFile(name).empty());
}

TEST_CASE("AppHelper: TempfileNotCreated", "[helpers]") {
    std::string name = "TestFileNotUsed.txt";
    {
        TempFile myfile{name};

        CHECK_FALSE(xcli::ExistingFile(myfile).empty());
    }
    CHECK_FALSE(xcli::ExistingFile(name).empty());
}

TEST_CASE("AppHelper: Ofstream", "[helpers]") {

    std::string name = "TestFileNotUsed.txt";
    {
        TempFile myfile(name);

        {
            std::ofstream out{myfile};
            out << "this is output" << '\n';
        }

        CHECK(xcli::ExistingFile(myfile).empty());
    }
    CHECK_FALSE(xcli::ExistingFile(name).empty());
}

TEST_CASE("Split: StringList", "[helpers]") {

    std::vector<std::string> results{"a", "long", "--lone", "-q"};
    CHECK(xcli::detail::split_names("a,long,--lone,-q") == results);
    CHECK(xcli::detail::split_names(" a, long, --lone, -q") == results);
    CHECK(xcli::detail::split_names(" a , long , --lone , -q ") == results);
    CHECK(xcli::detail::split_names("   a  ,  long  ,  --lone  ,    -q  ") == results);

    CHECK(xcli::detail::split_names("one") == std::vector<std::string>({"one"}));
}

TEST_CASE("RegEx: Shorts", "[helpers]") {
    std::string name, value;

    CHECK(xcli::detail::split_short("-a", name, value));
    CHECK(name == "a");
    CHECK(value.empty());

    CHECK(xcli::detail::split_short("-B", name, value));
    CHECK(name == "B");
    CHECK(value.empty());

    CHECK(xcli::detail::split_short("-cc", name, value));
    CHECK(name == "c");
    CHECK(value == "c");

    CHECK(xcli::detail::split_short("-simple", name, value));
    CHECK(name == "s");
    CHECK(value == "imple");

    CHECK_FALSE(xcli::detail::split_short("--a", name, value));
    CHECK_FALSE(xcli::detail::split_short("--thing", name, value));
    CHECK_FALSE(xcli::detail::split_short("--", name, value));
    CHECK_FALSE(xcli::detail::split_short("something", name, value));
    CHECK_FALSE(xcli::detail::split_short("s", name, value));
}

TEST_CASE("RegEx: Longs", "[helpers]") {
    std::string name, value;

    CHECK(xcli::detail::split_long("--a", name, value));
    CHECK(name == "a");
    CHECK(value.empty());

    CHECK(xcli::detail::split_long("--thing", name, value));
    CHECK(name == "thing");
    CHECK(value.empty());

    CHECK(xcli::detail::split_long("--some=thing", name, value));
    CHECK(name == "some");
    CHECK(value == "thing");

    CHECK_FALSE(xcli::detail::split_long("-a", name, value));
    CHECK_FALSE(xcli::detail::split_long("-things", name, value));
    CHECK_FALSE(xcli::detail::split_long("Q", name, value));
    CHECK_FALSE(xcli::detail::split_long("--", name, value));
}

TEST_CASE("RegEx: SplittingNew", "[helpers]") {

    std::vector<std::string> shorts;
    std::vector<std::string> longs;
    std::string pname;

    CHECK_NOTHROW(std::tie(shorts, longs, pname) = xcli::detail::get_names({"--long", "-s", "-q", "--also-long"}));
    CHECK(longs == std::vector<std::string>({"long", "also-long"}));
    CHECK(shorts == std::vector<std::string>({"s", "q"}));
    CHECK(pname.empty());

    std::tie(shorts, longs, pname) = xcli::detail::get_names({"--long", "", "-s", "-q", "", "--also-long"});
    CHECK(longs == std::vector<std::string>({"long", "also-long"}));
    CHECK(shorts == std::vector<std::string>({"s", "q"}));

    CHECK_THROWS_AS([&]() { std::tie(shorts, longs, pname) = xcli::detail::get_names({"-"}); }(), xcli::BadNameString);
    CHECK_THROWS_AS([&]() { std::tie(shorts, longs, pname) = xcli::detail::get_names({"--"}); }(), xcli::BadNameString);
    CHECK_THROWS_AS([&]() { std::tie(shorts, longs, pname) = xcli::detail::get_names({"-hi"}); }(), xcli::BadNameString);
    CHECK_THROWS_AS([&]() { std::tie(shorts, longs, pname) = xcli::detail::get_names({"---hi"}); }(),
                    xcli::BadNameString);
    CHECK_THROWS_AS([&]() { std::tie(shorts, longs, pname) = xcli::detail::get_names({"one", "two"}); }(),
                    xcli::BadNameString);
}

TEST_CASE("String: ToLower", "[helpers]") { CHECK("one and two" == xcli::detail::to_lower("one And TWO")); }

TEST_CASE("Join: Forward", "[helpers]") {
    std::vector<std::string> val{{"one", "two", "three"}};
    CHECK(xcli::detail::join(val) == "one,two,three");
    CHECK(xcli::detail::join(val, ";") == "one;two;three");
}

TEST_CASE("Join: Backward", "[helpers]") {
    std::vector<std::string> val{{"three", "two", "one"}};
    CHECK(xcli::detail::rjoin(val) == "one,two,three");
    CHECK(xcli::detail::rjoin(val, ";") == "one;two;three");
}

TEST_CASE("SplitUp: Simple", "[helpers]") {
    std::vector<std::string> oput = {"one", "\"two three\""};
    std::string orig{R"(one "two three")"};
    std::vector<std::string> result = xcli::detail::split_up(orig);
    CHECK(result == oput);
}

TEST_CASE("SplitUp: SimpleDifferentQuotes", "[helpers]") {
    std::vector<std::string> oput = {"one", "`two three`"};
    std::string orig{R"(one `two three`)"};
    std::vector<std::string> result = xcli::detail::split_up(orig);
    CHECK(result == oput);
}

TEST_CASE("SplitUp: SimpleMissingQuotes", "[helpers]") {
    std::vector<std::string> oput = {"one", "`two three"};
    std::string orig{R"(one `two three)"};
    std::vector<std::string> result = xcli::detail::split_up(orig);
    CHECK(result == oput);
}

TEST_CASE("SplitUp: SimpleMissingQuotesEscaped", "[helpers]") {
    std::vector<std::string> oput = {"one", R"("two three\"")"};
    std::string orig{R"(one "two three\"")"};
    std::vector<std::string> result = xcli::detail::split_up(orig);
    CHECK(result == oput);
}

TEST_CASE("SplitUp: SimpleDifferentQuotes2", "[helpers]") {
    std::vector<std::string> oput = {"one", "'two three'"};
    std::string orig{R"(one 'two three')"};
    std::vector<std::string> result = xcli::detail::split_up(orig);
    CHECK(result == oput);
}

TEST_CASE("SplitUp: Bracket1", "[helpers]") {
    std::vector<std::string> oput = {"one", "[two, three]"};
    std::string orig{"one, [two, three]"};
    std::vector<std::string> result = xcli::detail::split_up(orig, ',');
    CHECK(result == oput);
}

TEST_CASE("SplitUp: Bracket2", "[helpers]") {
    std::vector<std::string> oput = {"one", "<two, three>"};
    std::string orig{"one, <two, three>"};
    std::vector<std::string> result = xcli::detail::split_up(orig, ',');
    CHECK(result == oput);
}

TEST_CASE("SplitUp: Bracket3", "[helpers]") {
    std::vector<std::string> oput = {"one", "(two, three)"};
    std::string orig{"one, (two, three)"};
    std::vector<std::string> result = xcli::detail::split_up(orig, ',');
    CHECK(result == oput);
}

TEST_CASE("SplitUp: Bracket4", "[helpers]") {
    std::vector<std::string> oput = {"one", "{two, three}"};
    std::string orig{"one, {two, three}"};
    std::vector<std::string> result = xcli::detail::split_up(orig, ',');
    CHECK(result == oput);
}

TEST_CASE("SplitUp: Comment", "[helpers]") {
    std::vector<std::string> oput = {R"(["quote1", "#"])"};
    std::string orig{R"(["quote1", "#"])"};
    std::vector<std::string> result = xcli::detail::split_up(orig, '#');
    CHECK(result == oput);
}

TEST_CASE("SplitUp: Layered", "[helpers]") {
    std::vector<std::string> output = {R"("one 'two three'")"};
    std::string orig{R"("one 'two three'")"};
    std::vector<std::string> result = xcli::detail::split_up(orig);
    CHECK(result == output);
}

TEST_CASE("SplitUp: Spaces", "[helpers]") {
    std::vector<std::string> oput = {"one", "\"  two three\""};
    std::string orig{R"(  one  "  two three" )"};
    std::vector<std::string> result = xcli::detail::split_up(orig);
    CHECK(result == oput);
}

TEST_CASE("SplitUp: BadStrings", "[helpers]") {
    std::vector<std::string> oput = {"one", "\"  two three"};
    std::string orig{R"(  one  "  two three )"};
    std::vector<std::string> result = xcli::detail::split_up(orig);
    CHECK(result == oput);

    oput = {"one", "'  two three"};
    orig = R"(  one  '  two three )";
    result = xcli::detail::split_up(orig);
    CHECK(result == oput);
}

TEST_CASE("SplitUp: TrailingAfterQuote", "[helpers]") {
    // a non-delimiter character following a closing quote must not be dropped
    std::vector<std::string> oput = {"\"abc\"", "def"};
    std::string orig{"\"abc\"def"};
    std::vector<std::string> result = xcli::detail::split_up(orig);
    CHECK(result == oput);
}

TEST_CASE("SplitUp: TrailingAfterBracket", "[helpers]") {
    // a non-delimiter character following a closing bracket must not be dropped
    std::vector<std::string> oput = {"[a, b]", "c"};
    std::string orig{"[a, b]c"};
    std::vector<std::string> result = xcli::detail::split_up(orig, ',');
    CHECK(result == oput);
}

TEST_CASE("SplitUp: DelimiterAfterQuote", "[helpers]") {
    // an actual delimiter following a closing quote is still consumed
    std::vector<std::string> oput = {"\"abc\"", "def"};
    std::string orig{"\"abc\",def"};
    std::vector<std::string> result = xcli::detail::split_up(orig, ',');
    CHECK(result == oput);
}

TEST_CASE("Types: TypeName", "[helpers]") {
    std::string int_name = xcli::detail::type_name<int>();
    CHECK(int_name == "INT");

    std::string int2_name = xcli::detail::type_name<std::int16_t>();
    CHECK(int2_name == "INT");

    std::string uint_name = xcli::detail::type_name<unsigned char>();
    CHECK(uint_name == "UINT");

    std::string float_name = xcli::detail::type_name<double>();
    CHECK(float_name == "FLOAT");

    std::string char_name = xcli::detail::type_name<char>();
    CHECK(char_name == "CHAR");

    std::string vector_name = xcli::detail::type_name<std::vector<int>>();
    CHECK(vector_name == "INT");

    vector_name = xcli::detail::type_name<std::vector<double>>();
    CHECK(vector_name == "FLOAT");

    static_assert(xcli::detail::classify_object<std::pair<int, std::string>>::value ==
                      xcli::detail::object_category::tuple_value,
                  "pair<int,string> does not read like a tuple");

    static_assert(xcli::detail::classify_object<std::tuple<std::string, double>>::value ==
                      xcli::detail::object_category::tuple_value,
                  "tuple<string,double> does not read like a tuple");

    std::string pair_name = xcli::detail::type_name<std::vector<std::pair<int, std::string>>>();
    CHECK(pair_name == "[INT,TEXT]");

    vector_name = xcli::detail::type_name<std::vector<std::vector<unsigned char>>>();
    CHECK(vector_name == "UINT");

    auto vclass = xcli::detail::classify_object<std::vector<std::vector<unsigned char>>>::value;
    CHECK(xcli::detail::object_category::container_value == vclass);

    auto tclass = xcli::detail::classify_object<std::tuple<double>>::value;
    CHECK(xcli::detail::object_category::number_constructible == tclass);

    std::string tuple_name = xcli::detail::type_name<std::tuple<double>>();
    CHECK(tuple_name == "FLOAT");

    static_assert(xcli::detail::classify_object<std::tuple<int, std::string>>::value ==
                      xcli::detail::object_category::tuple_value,
                  "tuple<int,string> does not read like a tuple");
    tuple_name = xcli::detail::type_name<std::tuple<int, std::string>>();
    CHECK(tuple_name == "[INT,TEXT]");

    tuple_name = xcli::detail::type_name<std::tuple<const int, std::string>>();
    CHECK(tuple_name == "[INT,TEXT]");

    tuple_name = xcli::detail::type_name<const std::tuple<int, std::string>>();
    CHECK(tuple_name == "[INT,TEXT]");

    tuple_name = xcli::detail::type_name<std::tuple<std::string, double>>();
    CHECK(tuple_name == "[TEXT,FLOAT]");

    tuple_name = xcli::detail::type_name<const std::tuple<std::string, double>>();
    CHECK(tuple_name == "[TEXT,FLOAT]");

    tuple_name = xcli::detail::type_name<std::tuple<int, std::string, double>>();
    CHECK(tuple_name == "[INT,TEXT,FLOAT]");

    tuple_name = xcli::detail::type_name<std::tuple<int, std::string, double, unsigned int>>();
    CHECK(tuple_name == "[INT,TEXT,FLOAT,UINT]");

    tuple_name = xcli::detail::type_name<std::tuple<int, std::string, double, unsigned int, std::string>>();
    CHECK(tuple_name == "[INT,TEXT,FLOAT,UINT,TEXT]");

    tuple_name = xcli::detail::type_name<std::array<int, 10>>();
    CHECK(tuple_name == "[INT,INT,INT,INT,INT,INT,INT,INT,INT,INT]");

    std::string text_name = xcli::detail::type_name<std::string>();
    CHECK(text_name == "TEXT");

    std::string text2_name = xcli::detail::type_name<char *>();
    CHECK(text2_name == "TEXT");

    enum class test : std::uint8_t { test1, test2, test3 };
    std::string enum_name = xcli::detail::type_name<test>();
    CHECK(enum_name == "ENUM");

    vclass = xcli::detail::classify_object<std::tuple<test>>::value;
    CHECK(xcli::detail::object_category::tuple_value == vclass);
    static_assert(xcli::detail::classify_object<std::tuple<test>>::value == xcli::detail::object_category::tuple_value,
                  "tuple<test> does not classify as a tuple");
    std::string enum_name2 = xcli::detail::type_name<std::tuple<test>>();
    CHECK(enum_name2 == "ENUM");
    std::string umapName = xcli::detail::type_name<std::unordered_map<int, std::tuple<std::string, double>>>();
    CHECK(umapName == "[INT,[TEXT,FLOAT]]");

    // On older compilers, this may show up as other/TEXT
    vclass = xcli::detail::classify_object<std::atomic<int>>::value;
    CHECK((xcli::detail::object_category::wrapper_value == vclass || xcli::detail::object_category::other == vclass));

    std::string atomic_name = xcli::detail::type_name<std::atomic<int>>();
    CHECK((atomic_name == "INT" || atomic_name == "TEXT"));
}

TEST_CASE("Types: TypeNameStrings", "[helpers]") {
    auto sclass = xcli::detail::classify_object<std::string>::value;
    CHECK(xcli::detail::object_category::string_assignable == sclass);

    auto wsclass = xcli::detail::classify_object<std::wstring>::value;
    CHECK(xcli::detail::object_category::wstring_assignable == wsclass);

#if defined XCLI_HAS_FILESYSTEM && XCLI_HAS_FILESYSTEM > 0 && defined(_MSC_VER)
    auto fspclass = xcli::detail::classify_object<std::filesystem::path>::value;
    CHECK(xcli::detail::object_category::wstring_assignable == fspclass);
#endif
}

TEST_CASE("Types: OverflowSmall", "[helpers]") {
    signed char x = 0;
    auto strmax = std::to_string((std::numeric_limits<signed char>::max)() + 1);
    CHECK_FALSE(xcli::detail::lexical_cast(strmax, x));

    unsigned char y = 0;
    strmax = std::to_string((std::numeric_limits<unsigned char>::max)() + 1);
    CHECK_FALSE(xcli::detail::lexical_cast(strmax, y));
}

TEST_CASE("Types: LexicalCastInt", "[helpers]") {
    std::string signed_input = "-912";
    int x_signed = 0;
    CHECK(xcli::detail::lexical_cast(signed_input, x_signed));
    CHECK(x_signed == -912);

    std::string unsigned_input = "912";
    unsigned int x_unsigned = 0;
    CHECK(xcli::detail::lexical_cast(unsigned_input, x_unsigned));
    CHECK(x_unsigned == (unsigned int)912);

    CHECK_FALSE(xcli::detail::lexical_cast(signed_input, x_unsigned));

    unsigned char y = 0;
    std::string overflow_input = std::to_string((std::numeric_limits<uint64_t>::max)()) + "0";
    CHECK_FALSE(xcli::detail::lexical_cast(overflow_input, y));

    char y_signed = 0;
    CHECK_FALSE(xcli::detail::lexical_cast(overflow_input, y_signed));

    std::string bad_input = "hello";
    CHECK_FALSE(xcli::detail::lexical_cast(bad_input, y));

    std::string extra_input = "912i";
    CHECK_FALSE(xcli::detail::lexical_cast(extra_input, y));

    extra_input = "true";
    CHECK(xcli::detail::lexical_cast(extra_input, x_signed));
    CHECK(x_signed != 0);

    std::string empty_input{};
    CHECK_FALSE(xcli::detail::lexical_cast(empty_input, x_signed));
    CHECK_FALSE(xcli::detail::lexical_cast(empty_input, x_unsigned));
    CHECK_FALSE(xcli::detail::lexical_cast(empty_input, y_signed));
}

TEST_CASE("Types: LexicalCastUnsignedLeadingWhitespaceNegative", "[helpers]") {
    // strtoull skips leading whitespace and silently wraps a negative value; " -1" must not become UINT64_MAX
    std::uint64_t x_unsigned = 0;
    CHECK_FALSE(xcli::detail::lexical_cast(" -1", x_unsigned));
    CHECK_FALSE(xcli::detail::lexical_cast("\t-5", x_unsigned));
    CHECK_FALSE(xcli::detail::lexical_cast("-1", x_unsigned));

    // a leading-whitespace positive value should still convert correctly
    CHECK(xcli::detail::lexical_cast(" 7", x_unsigned));
    CHECK(x_unsigned == static_cast<std::uint64_t>(7));
}

TEST_CASE("Types: LexicalCastDouble", "[helpers]") {
    std::string input = "9.12";
    long double x = NAN;
    CHECK(xcli::detail::lexical_cast(input, x));
    CHECK((float)x == Approx((float)9.12));

    std::string bad_input = "hello";
    CHECK_FALSE(xcli::detail::lexical_cast(bad_input, x));

    std::string overflow_input = "1" + std::to_string((std::numeric_limits<long double>::max)());
    CHECK(xcli::detail::lexical_cast(overflow_input, x));
    CHECK_FALSE(std::isfinite(x));

    std::string extra_input = "9.12i";
    CHECK_FALSE(xcli::detail::lexical_cast(extra_input, x));

    std::string empty_input{};
    CHECK_FALSE(xcli::detail::lexical_cast(empty_input, x));

    // whitespace-only strings perform no conversion in strtold and must fail rather than report 0.0
    double d = 7.5;
    CHECK_FALSE(xcli::detail::lexical_cast(" ", d));
    CHECK_FALSE(xcli::detail::lexical_cast("   ", d));
    CHECK_FALSE(xcli::detail::lexical_cast("\t\n", d));
}

TEST_CASE("Types: LexicalCastBool", "[helpers]") {
    std::string input = "false";
    bool x = false;
    CHECK(xcli::detail::lexical_cast(input, x));
    CHECK_FALSE(x);

    std::string bad_input = "happy";
    CHECK_FALSE(xcli::detail::lexical_cast(bad_input, x));

    std::string input_true = "EnaBLE";
    CHECK(xcli::detail::lexical_cast(input_true, x));
    CHECK(x);
}

TEST_CASE("Types: LexicalCastString", "[helpers]") {
    std::string input = "one";
    std::string output;
    xcli::detail::lexical_cast(input, output);
    CHECK(output == input);
}

TEST_CASE("Types: LexicalCastParsable", "[helpers]") {
    std::string input = "(4.2,7.3)";
    std::string fail_input = "4.2,7.3";
    std::string extra_input = "(4.2,7.3)e";

    std::complex<double> output;
    CHECK(xcli::detail::lexical_cast(input, output));
    CHECK(4.2 == Approx(output.real()));
    CHECK(7.3 == Approx(output.imag()));

    CHECK(xcli::detail::lexical_cast("2.456", output));
    CHECK(2.456 == Approx(output.real()));
    CHECK(0.0 == Approx(output.imag()));

    CHECK_FALSE(xcli::detail::lexical_cast(fail_input, output));
    CHECK_FALSE(xcli::detail::lexical_cast(extra_input, output));
}

TEST_CASE("Types: LexicalCastEnum", "[helpers]") {
    enum t1 : signed char { v1 = 5, v3 = 7, v5 = -9 };

    t1 output = v1;
    CHECK(xcli::detail::lexical_cast("-9", output));
    CHECK(v5 == output);

    CHECK_FALSE(xcli::detail::lexical_cast("invalid", output));
    enum class t2 : std::uint64_t { enum1 = 65, enum2 = 45667, enum3 = 9999999999999 };
    t2 output2{t2::enum2};
    CHECK(xcli::detail::lexical_cast("65", output2));
    CHECK(t2::enum1 == output2);

    CHECK_FALSE(xcli::detail::lexical_cast("invalid", output2));

    CHECK(xcli::detail::lexical_cast("9999999999999", output2));
    CHECK(t2::enum3 == output2);
}

TEST_CASE("Types: LexicalConversionDouble", "[helpers]") {
    xcli::results_t input = {"9.12"};
    long double x{0.0};
    bool res = xcli::detail::lexical_conversion<long double, double>(input, x);
    CHECK(res);
    CHECK((float)x == Approx((float)9.12));

    xcli::results_t bad_input = {"hello"};
    res = xcli::detail::lexical_conversion<long double, double>(bad_input, x);
    CHECK_FALSE(res);
}

TEST_CASE("Types: LexicalConversionDoubleTuple", "[helpers]") {
    xcli::results_t input = {"9.12"};
    std::tuple<double> x{0.0};
    bool res = xcli::detail::lexical_conversion<decltype(x), decltype(x)>(input, x);
    CHECK(res);
    CHECK(std::get<0>(x) == Approx(9.12));

    xcli::results_t bad_input = {"hello"};
    res = xcli::detail::lexical_conversion<decltype(x), decltype(x)>(bad_input, x);
    CHECK_FALSE(res);
}

TEST_CASE("Types: LexicalConversionVectorDouble", "[helpers]") {
    xcli::results_t input = {"9.12", "10.79", "-3.54"};
    std::vector<double> x;
    bool res = xcli::detail::lexical_conversion<std::vector<double>, double>(input, x);
    CHECK(res);
    CHECK(3u == x.size());
    CHECK(-3.54 == Approx(x[2]));

    res = xcli::detail::lexical_conversion<std::vector<double>, std::vector<double>>(input, x);
    CHECK(res);
    CHECK(3u == x.size());
    CHECK(-3.54 == Approx(x[2]));
}

TEST_CASE("Types: LexicalConversionEmptyVectorDouble", "[helpers]") {
    xcli::results_t input = {};
    std::vector<double> x;
    bool res = xcli::detail::lexical_conversion<std::vector<double>, std::vector<double>>(input, x);
    CHECK(res);
    CHECK(x.empty());
}

TEST_CASE("Types: LexicalConversionPairContainerOddCount", "[helpers]") {
    // an odd element count must fail rather than insert a pair with an uninitialized/defaulted second value
    using map_t = std::vector<std::pair<std::string, int>>;
    xcli::results_t input = {"onlykey"};
    map_t x;
    bool res = xcli::detail::lexical_conversion<map_t, map_t>(input, x);
    CHECK_FALSE(res);

    // a complete set of pairs should still convert successfully
    input = {"a", "1", "b", "2"};
    res = xcli::detail::lexical_conversion<map_t, map_t>(input, x);
    CHECK(res);
    REQUIRE(x.size() == static_cast<std::size_t>(2));
    CHECK(x[0].first == "a");
    CHECK(x[0].second == 1);
    CHECK(x[1].first == "b");
    CHECK(x[1].second == 2);
}

TEST_CASE("Types: SumStringVector", "[helpers]") {
    // large integer sums must stay exact and render as plain integers, not scientific notation
    std::vector<std::string> big = {"5000000000000000", "5000000000000000"};
    CHECK(xcli::detail::sum_string_vector(big) == "10000000000000000");

    // values above 2^53 must not lose precision through a double accumulation
    std::vector<std::string> precise = {"9007199254740993", "0"};
    CHECK(xcli::detail::sum_string_vector(precise) == "9007199254740993");

    // simple integer sums
    std::vector<std::string> small = {"3", "4", "5"};
    CHECK(xcli::detail::sum_string_vector(small) == "12");

    // floating point sums fall back to the double path
    std::vector<std::string> floats = {"1.5", "2.5"};
    CHECK(xcli::detail::sum_string_vector(floats) == "4");

    // non-numeric values fall back to string concatenation
    std::vector<std::string> strings = {"a", "b"};
    CHECK(xcli::detail::sum_string_vector(strings) == "ab");
}

static_assert(!xcli::detail::is_tuple_like<std::vector<double>>::value, "vector should not be like a tuple");
static_assert(xcli::detail::is_tuple_like<std::pair<double, double>>::value, "pair of double should be like a tuple");
static_assert(xcli::detail::is_tuple_like<std::array<double, 4>>::value, "std::array<double,4> should be like a tuple");
static_assert(xcli::detail::is_tuple_like<std::array<int, 10>>::value, "std::array<int,10> should be like a tuple");
static_assert(!xcli::detail::is_tuple_like<std::string>::value, "std::string should not be like a tuple");
static_assert(!xcli::detail::is_tuple_like<double>::value, "double should not be like a tuple");
static_assert(xcli::detail::is_tuple_like<std::tuple<double, int, double>>::value, "tuple should look like a tuple");
static_assert(!xcli::detail::is_tuple_like<std::complex<double>>::value, "std::complex should not be like a tuple");

TEST_CASE("Types: LexicalConversionTuple2", "[helpers]") {
    xcli::results_t input = {"9.12", "19"};

    std::tuple<double, int> x{0.0, 0};
    static_assert(xcli::detail::is_tuple_like<decltype(x)>::value,
                  "tuple type must have is_tuple_like trait to be true");
    bool res = xcli::detail::lexical_conversion<decltype(x), decltype(x)>(input, x);
    CHECK(res);
    CHECK(19 == std::get<1>(x));
    CHECK(9.12 == Approx(std::get<0>(x)));

    input = {"19", "9.12"};
    res = xcli::detail::lexical_conversion<decltype(x), decltype(x)>(input, x);
    CHECK_FALSE(res);
}

TEST_CASE("Types: LexicalConversionTuple3", "[helpers]") {
    xcli::results_t input = {"9.12", "19", "hippo"};
    std::tuple<double, int, std::string> x;
    bool res = xcli::detail::lexical_conversion<decltype(x), decltype(x)>(input, x);
    CHECK(res);
    CHECK(19 == std::get<1>(x));
    CHECK(9.12 == Approx(std::get<0>(x)));
    CHECK("hippo" == std::get<2>(x));

    input = {"19", "9.12"};
    res = xcli::detail::lexical_conversion<decltype(x), decltype(x)>(input, x);
    CHECK_FALSE(res);
}

TEST_CASE("Types: LexicalConversionTuple4", "[helpers]") {
    xcli::results_t input = {"9.12", "19", "18.6", "5.87"};
    std::array<double, 4> x;
    auto tsize = xcli::detail::type_count<decltype(x)>::value;
    CHECK(tsize == 4);
    bool res = xcli::detail::lexical_conversion<decltype(x), decltype(x)>(input, x);
    CHECK(res);
    CHECK(19 == Approx(std::get<1>(x)));
    CHECK(9.12 == Approx(x[0]));
    CHECK(18.6 == Approx(x[2]));
    CHECK(5.87 == Approx(x[3]));

    input = {"19", "9.12", "hippo"};
    res = xcli::detail::lexical_conversion<decltype(x), decltype(x)>(input, x);
    CHECK_FALSE(res);
}

TEST_CASE("Types: LexicalConversionTuple5", "[helpers]") {
    xcli::results_t input = {"9", "19", "18", "5", "235235"};
    std::array<unsigned int, 5> x;
    bool res = xcli::detail::lexical_conversion<decltype(x), decltype(x)>(input, x);
    CHECK(res);
    CHECK(19u == std::get<1>(x));
    CHECK(9u == x[0]);
    CHECK(18u == x[2]);
    CHECK(5u == x[3]);
    CHECK(235235u == x[4]);

    input = {"19", "9.12", "hippo"};
    res = xcli::detail::lexical_conversion<decltype(x), decltype(x)>(input, x);
    CHECK_FALSE(res);
}

TEST_CASE("Types: LexicalConversionTuple10", "[helpers]") {
    xcli::results_t input = {"9", "19", "18", "5", "235235", "9", "19", "18", "5", "235235"};
    std::array<unsigned int, 10> x;
    bool res = xcli::detail::lexical_conversion<decltype(x), decltype(x)>(input, x);
    CHECK(res);
    CHECK(19u == std::get<1>(x));
    CHECK(9u == x[0]);
    CHECK(18u == x[2]);
    CHECK(5u == x[3]);
    CHECK(235235u == x[4]);
    CHECK(235235u == x[9]);
    input[3] = "hippo";
    res = xcli::detail::lexical_conversion<decltype(x), decltype(x)>(input, x);
    CHECK_FALSE(res);
}

TEST_CASE("Types: LexicalConversionTuple10XC", "[helpers]") {
    xcli::results_t input = {"9", "19", "18", "5", "235235", "9", "19", "18", "5", "235235"};
    std::array<double, 10> x;
    bool res = xcli::detail::lexical_conversion<decltype(x), std::array<unsigned int, 10>>(input, x);

    CHECK(res);
    CHECK(19.0 == std::get<1>(x));
    CHECK(9.0 == x[0]);
    CHECK(18.0 == x[2]);
    CHECK(5.0 == x[3]);
    CHECK(235235.0 == x[4]);
    CHECK(235235.0 == x[9]);
    input[3] = "19.7";
    res = xcli::detail::lexical_conversion<decltype(x), std::array<unsigned int, 10>>(input, x);
    CHECK_FALSE(res);
}

TEST_CASE("Types: LexicalConversionComplex", "[helpers]") {
    xcli::results_t input = {"5.1", "3.5"};
    std::complex<double> x;
    bool res = xcli::detail::lexical_conversion<std::complex<double>, std::array<double, 2>>(input, x);
    CHECK(res);
    CHECK(5.1 == x.real());
    CHECK(3.5 == x.imag());
}

static_assert(xcli::detail::is_wrapper<std::vector<double>>::value, "vector double should be a wrapper");
static_assert(xcli::detail::is_wrapper<std::vector<std::string>>::value, "vector string should be a wrapper");
static_assert(xcli::detail::is_wrapper<std::string>::value, "string should be a wrapper");
static_assert(!xcli::detail::is_wrapper<double>::value, "double should not be a wrapper");

static_assert(xcli::detail::is_mutable_container<std::vector<double>>::value, "vector class should be a container");
static_assert(xcli::detail::is_mutable_container<std::vector<std::string>>::value, "vector class should be a container");
static_assert(!xcli::detail::is_mutable_container<std::string>::value, "string should be a container");
static_assert(!xcli::detail::is_mutable_container<double>::value, "double should not be a container");
static_assert(!xcli::detail::is_mutable_container<std::array<double, 5>>::value, "array should not be a container");

static_assert(xcli::detail::is_mutable_container<std::vector<int>>::value, "vector int should be a container");

static_assert(xcli::detail::is_readable_container<std::vector<int> &>::value,
              "vector int & should be a readable container");
static_assert(xcli::detail::is_readable_container<const std::vector<int>>::value,
              "const vector int should be a readable container");
static_assert(xcli::detail::is_readable_container<const std::vector<int> &>::value,
              "const vector int & should be a readable container");

TEST_CASE("FixNewLines: BasicCheck", "[helpers]") {
    std::string input = "one\ntwo";
    std::string output = "one\n; two";
    std::string result = xcli::detail::fix_newlines("; ", input);
    CHECK(output == result);
}

TEST_CASE("FixNewLines: EdgesCheck", "[helpers]") {
    std::string input = "\none\ntwo\n";
    std::string output = "\n; one\n; two\n; ";
    std::string result = xcli::detail::fix_newlines("; ", input);
    CHECK(output == result);
}

TEST_CASE("String: environment", "[helpers]") {
    put_env("TEST1", "TESTS");

    auto value = xcli::detail::get_environment_value("TEST1");
    CHECK(value == "TESTS");
    unset_env("TEST1");

    value = xcli::detail::get_environment_value("TEST2");
    CHECK(value.empty());
}
