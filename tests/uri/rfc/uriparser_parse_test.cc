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

// From uriparser test/test.cpp UriSuite

#include <optional>
#include <string>
#include <string_view>

#include <gtest/gtest.h>
#include <turbo/strings/ascii.h>
#include <turbo/uri/ip.h>
#include <turbo/uri/rfc/parser.h>
#include <turbo/uri/uri_view.h>
#include <turbo/uri/scheme.h>

namespace {

bool parse_ok(std::string_view input) {
    return turbo::parse_rfc_uri(input).ok();
}

bool parse_ok(std::string_view input, turbo::UriView &uri) {
    uri = turbo::parse_rfc_uri(input);
    return uri.ok();
}

bool parse_ok_with_base(std::string_view input, const turbo::UriView &base) {
    return turbo::parse_rfc_uri(input, base).ok();
}

bool parse_ok_with_base(std::string_view input, const turbo::UriView &base,
    turbo::UriView &uri) {
    uri = turbo::parse_rfc_uri(input, base);
    return uri.ok();
}

// Absolute (scheme:) → no base; relative-ref → with base.
bool parse_ref_ok(std::string_view input, const turbo::UriView &base) {
    if (input.empty()) {
        return parse_ok_with_base(input, base);
    }
    if (!turbo::ascii_isalpha(static_cast<unsigned char>(input[0]))) {
        return parse_ok_with_base(input, base);
    }
    size_t i = 1;
    while (i < input.size() && turbo::is_valid_schema_alnum(input[i])) {
        ++i;
    }
    if (i < input.size() && input[i] == ':') {
        return parse_ok(input);
    }
    return parse_ok_with_base(input, base);
}

bool parse_rel_ok(std::string_view base, std::string_view rel) {
    turbo::UriView b(base);
    if (!parse_ok(base, b)) {
        return false;
    }
    turbo::UriView out(rel);
    return parse_ok_with_base(rel, b, out);
}

std::string view_href(const turbo::UriView &u) {
    std::string out;
    out.append(u.shema());
    out.push_back(':');
    if (u.has_host()) {
        out.append("//");
        if (!u.username().empty() || !u.password().empty()) {
            out.append(u.username());
            if (!u.password().empty()) {
                out.push_back(':');
                out.append(u.password());
            }
            out.push_back('@');
        }
        out.append(u.host());
        if (!u.port().empty()) {
            out.push_back(':');
            out.append(u.port());
        }
    }
    out.append(u.path());
    if (u.has_query()) {
        out.push_back('?');
        out.append(u.query());
    }
    if (u.has_fragment()) {
        out.push_back('#');
        out.append(u.fragment());
    }
    return out;
}

void ExpectMerge(std::string_view base, std::string_view rel,
    std::string_view expected) {
    turbo::UriView b;
    ASSERT_TRUE(parse_ok(base, b)) << base;
    turbo::UriView out = turbo::merge_rfc_uri(rel, b);
    ASSERT_TRUE(out.ok()) << rel;
    EXPECT_EQ(view_href(out), expected) << rel;
}

void ExpectHref(std::string_view base, std::string_view rel,
    std::string_view expected) {
    turbo::UriView b;
    ASSERT_TRUE(parse_ok(base, b)) << base;
    EXPECT_EQ(turbo::get_rfc_href(b, turbo::UriView(rel)), expected) << rel;
}

bool rfc_ipv4_ok(std::string_view input) {
    std::optional<turbo::IpAddr> out;
    auto r = turbo::try_parse_rfc_ip_v4(input, out);
    return r.ok() && r.error_pos == input.size() && out.has_value();
}

bool rfc_ipv6_ok(std::string_view inner) {
    const std::string lit = std::string("[") + std::string(inner) + "]";
    std::optional<turbo::IpAddr> out;
    auto r = turbo::try_parse_rfc_ip_v6(lit, out);
    return r.ok() && r.error_pos == lit.size() && out.has_value();
}

}  // namespace

// UriSuite::TestUri
TEST(UriSuite, TestUri) {
    turbo::UriView base("http://a/b/c/d;p?q");
    ASSERT_TRUE(parse_ok("http://a/b/c/d;p?q", base));

    EXPECT_TRUE(parse_ok_with_base(
        "//user:pass@[::1]:80/segment/index.html?query#frag", base));
    EXPECT_TRUE(parse_ok("http://[::1]:80/segment/index.html?query#frag"));
    EXPECT_TRUE(parse_ok("http://user:pass@[::1]/segment/index.html?query#frag"));
    EXPECT_TRUE(parse_ok("http://user:pass@[::1]:80?query#frag"));
    EXPECT_TRUE(parse_ok("http://user:pass@[::1]:80/segment/index.html#frag"));
    EXPECT_TRUE(parse_ok("http://user:pass@[::1]:80/segment/index.html?query"));

    EXPECT_TRUE(parse_ok("ftp://host:21/gnu/"));

    EXPECT_TRUE(parse_ok_with_base("one/two/three", base));
    EXPECT_TRUE(parse_ok_with_base("/one/two/three", base));
    EXPECT_TRUE(parse_ok_with_base("//user:pass@localhost/one/two/three", base));

    EXPECT_TRUE(parse_ok("http://www.example.com/"));

    EXPECT_TRUE(parse_ok("http://sourceforge.net/projects/uriparser/"));
    EXPECT_TRUE(parse_ok(
        "http://sourceforge.net/project/platformdownload.php?group_id=182840"));
    EXPECT_TRUE(parse_ok("mailto:test@example.com"));
    EXPECT_TRUE(parse_ok_with_base("../../", base));
    EXPECT_TRUE(parse_ok_with_base("/", base));
    EXPECT_TRUE(parse_ok_with_base("", base));
    EXPECT_TRUE(parse_ok("file:///bin/bash"));

    EXPECT_TRUE(parse_ok("http://www.example.com/name%20with%20spaces/"));
    EXPECT_FALSE(parse_ok("http://www.example.com/name with spaces/"));
}

// UriSuite::TestIpFour
TEST(UriSuite, TestIpFour) {
    EXPECT_FALSE(rfc_ipv4_ok("01.0.0.0"));
    EXPECT_FALSE(rfc_ipv4_ok("001.0.0.0"));
    EXPECT_FALSE(rfc_ipv4_ok("00.0.0.0"));
    EXPECT_FALSE(rfc_ipv4_ok("000.0.0.0"));
    EXPECT_FALSE(rfc_ipv4_ok("256.0.0.0"));
    EXPECT_FALSE(rfc_ipv4_ok("300.0.0.0"));
    EXPECT_FALSE(rfc_ipv4_ok("1111.0.0.0"));
    EXPECT_FALSE(rfc_ipv4_ok("-1.0.0.0"));
    EXPECT_FALSE(rfc_ipv4_ok("0.0.0"));
    EXPECT_FALSE(rfc_ipv4_ok("0.0.0."));
    EXPECT_FALSE(rfc_ipv4_ok("0.0.0.0."));
    EXPECT_FALSE(rfc_ipv4_ok("0.0.0.0.0"));
    EXPECT_FALSE(rfc_ipv4_ok("0.0..0"));
    EXPECT_FALSE(rfc_ipv4_ok(".0.0.0"));

    EXPECT_TRUE(rfc_ipv4_ok("255.0.0.0"));
    EXPECT_TRUE(rfc_ipv4_ok("0.0.0.0"));
    EXPECT_TRUE(rfc_ipv4_ok("1.0.0.0"));
    EXPECT_TRUE(rfc_ipv4_ok("2.0.0.0"));
    EXPECT_TRUE(rfc_ipv4_ok("3.0.0.0"));
    EXPECT_TRUE(rfc_ipv4_ok("30.0.0.0"));
}

// UriSuite::TestIpSixPass
TEST(UriSuite, TestIpSixPass) {
    EXPECT_TRUE(rfc_ipv6_ok("abcd::"));

    EXPECT_TRUE(rfc_ipv6_ok("abcd::1"));
    EXPECT_TRUE(rfc_ipv6_ok("abcd::12"));
    EXPECT_TRUE(rfc_ipv6_ok("abcd::123"));
    EXPECT_TRUE(rfc_ipv6_ok("abcd::1234"));

    EXPECT_TRUE(rfc_ipv6_ok("2001:0db8:0100:f101:0210:a4ff:fee3:9566"));
    EXPECT_TRUE(rfc_ipv6_ok("2001:0DB8:0100:F101:0210:A4FF:FEE3:9566"));
    EXPECT_TRUE(rfc_ipv6_ok("2001:db8:100:f101:210:a4ff:fee3:9566"));
    EXPECT_TRUE(rfc_ipv6_ok("2001:0db8:100:f101:0:0:0:1"));
    EXPECT_TRUE(rfc_ipv6_ok("1:2:3:4:5:6:255.255.255.255"));

    EXPECT_TRUE(rfc_ipv6_ok("::1.2.3.4"));
    EXPECT_TRUE(rfc_ipv6_ok("3:4::5:1.2.3.4"));
    EXPECT_TRUE(rfc_ipv6_ok("::ffff:1.2.3.4"));
    EXPECT_TRUE(rfc_ipv6_ok("::0.0.0.0"));
    EXPECT_TRUE(rfc_ipv6_ok("::255.255.255.255"));

    EXPECT_TRUE(rfc_ipv6_ok("::1:2:3:4:5:6:7"));
    EXPECT_TRUE(rfc_ipv6_ok("1::1:2:3:4:5:6"));
    EXPECT_TRUE(rfc_ipv6_ok("1:2::1:2:3:4:5"));
    EXPECT_TRUE(rfc_ipv6_ok("1:2:3::1:2:3:4"));
    EXPECT_TRUE(rfc_ipv6_ok("1:2:3:4::1:2:3"));
    EXPECT_TRUE(rfc_ipv6_ok("1:2:3:4:5::1:2"));
    EXPECT_TRUE(rfc_ipv6_ok("1:2:3:4:5:6::1"));
    EXPECT_TRUE(rfc_ipv6_ok("1:2:3:4:5:6:7::"));

    EXPECT_TRUE(rfc_ipv6_ok("1:1:1::1:1:1:1"));
    EXPECT_TRUE(rfc_ipv6_ok("1:1:1::1:1:1"));
    EXPECT_TRUE(rfc_ipv6_ok("1:1:1::1:1"));
    EXPECT_TRUE(rfc_ipv6_ok("1:1::1:1"));
    EXPECT_TRUE(rfc_ipv6_ok("1:1::1"));
    EXPECT_TRUE(rfc_ipv6_ok("1::1"));
    EXPECT_TRUE(rfc_ipv6_ok("::1"));
    EXPECT_TRUE(rfc_ipv6_ok("::"));

    EXPECT_TRUE(rfc_ipv6_ok("21ff:abcd::1"));
    EXPECT_TRUE(rfc_ipv6_ok("2001:db8:100:f101::1"));
    EXPECT_TRUE(rfc_ipv6_ok("a:b:c::12:1"));
    EXPECT_TRUE(rfc_ipv6_ok("a:b::0:1:2:3"));

    EXPECT_TRUE(rfc_ipv6_ok("::100.1.1.1"));
    EXPECT_TRUE(rfc_ipv6_ok("::1.100.1.1"));
    EXPECT_TRUE(rfc_ipv6_ok("::1.1.100.1"));
    EXPECT_TRUE(rfc_ipv6_ok("::1.1.1.100"));
    EXPECT_TRUE(rfc_ipv6_ok("::100.100.100.100"));
    EXPECT_TRUE(rfc_ipv6_ok("::10.1.1.1"));
    EXPECT_TRUE(rfc_ipv6_ok("::1.10.1.1"));
    EXPECT_TRUE(rfc_ipv6_ok("::1.1.10.1"));
    EXPECT_TRUE(rfc_ipv6_ok("::1.1.1.10"));
    EXPECT_TRUE(rfc_ipv6_ok("::10.10.10.10"));
}

// UriSuite::TestIpSixFail
TEST(UriSuite, TestIpSixFail) {
    EXPECT_FALSE(rfc_ipv6_ok("::12345"));

    EXPECT_FALSE(rfc_ipv6_ok("abcd::abcd::abcd"));

    EXPECT_FALSE(rfc_ipv6_ok(":::1234"));
    EXPECT_FALSE(rfc_ipv6_ok("1234:::1234:1234"));
    EXPECT_FALSE(rfc_ipv6_ok("1234:1234:::1234"));
    EXPECT_FALSE(rfc_ipv6_ok("1234:::"));

    EXPECT_FALSE(rfc_ipv6_ok("1.2.3.4"));
    EXPECT_FALSE(rfc_ipv6_ok("0001.0002.0003.0004"));

    EXPECT_FALSE(rfc_ipv6_ok("0000:0000:0000:0000:0000:1.2.3.4"));

    EXPECT_FALSE(rfc_ipv6_ok("0:0:0:0:0:0:0"));
    EXPECT_FALSE(rfc_ipv6_ok("0:0:0:0:0:0:0:"));
    EXPECT_FALSE(rfc_ipv6_ok("0:0:0:0:0:0:0:1.2.3.4"));

    EXPECT_FALSE(rfc_ipv6_ok("1:2:3:4:5:6:7:8:9"));
    EXPECT_FALSE(rfc_ipv6_ok("::2:3:4:5:6:7:8:9"));
    EXPECT_FALSE(rfc_ipv6_ok("1:2:3:4::6:7:8:9"));
    EXPECT_FALSE(rfc_ipv6_ok("1:2:3:4:5:6:7:8::"));

    EXPECT_FALSE(rfc_ipv6_ok("::ffff:001.02.03.004"));
    EXPECT_FALSE(rfc_ipv6_ok("::ffff:1.2.3.1111"));
    EXPECT_FALSE(rfc_ipv6_ok("::ffff:1.2.3.256"));
    EXPECT_FALSE(rfc_ipv6_ok("::ffff:311.2.3.4"));
    EXPECT_FALSE(rfc_ipv6_ok("::ffff:1.2.3:4"));
    EXPECT_FALSE(rfc_ipv6_ok("::ffff:1.2.3"));
    EXPECT_FALSE(rfc_ipv6_ok("::ffff:1.2.3."));
    EXPECT_FALSE(rfc_ipv6_ok("::ffff:1.2.3a.4"));
    EXPECT_FALSE(rfc_ipv6_ok("::ffff:1.2.3.4:123"));

    EXPECT_FALSE(rfc_ipv6_ok("g:0:0:0:0:0:0"));

    EXPECT_FALSE(rfc_ipv6_ok("0:0:0:0:0:0:0::1"));

    EXPECT_FALSE(rfc_ipv6_ok(":1::1"));
    EXPECT_FALSE(rfc_ipv6_ok("1::1:"));
    EXPECT_FALSE(rfc_ipv6_ok(":1::1:"));
    EXPECT_FALSE(rfc_ipv6_ok(":0:0:0:0:0:0:0:0"));
    EXPECT_FALSE(rfc_ipv6_ok("0:0:0:0:0:0:0:0:"));
    EXPECT_FALSE(rfc_ipv6_ok(":0:0:0:0:0:0:0:0:"));

    EXPECT_FALSE(rfc_ipv6_ok("1:1:1:1:1:1::1.1.1.1"));
}

// UriSuite::TestUriComponents
TEST(UriSuite, TestUriComponents) {
    constexpr std::string_view kIn =
        "http://sourceforge.net/project/platformdownload.php?group_id=182840";
    turbo::UriView uri(kIn);
    ASSERT_TRUE(parse_ok(kIn, uri));
    EXPECT_EQ(uri.shema(), "http");
    EXPECT_TRUE(uri.username().empty());
    EXPECT_TRUE(uri.password().empty());
    EXPECT_EQ(uri.host(), "sourceforge.net");
    EXPECT_TRUE(uri.port().empty());
    EXPECT_EQ(uri.path(), "/project/platformdownload.php");
    EXPECT_EQ(uri.query(), "group_id=182840");
    EXPECT_TRUE(uri.fragment().empty());
}

// UriSuite::TestUriComponentsBug20070701
TEST(UriSuite, TestUriComponentsBug20070701) {
    turbo::UriView uri("a:b");
    ASSERT_TRUE(parse_ok("a:b", uri));
    EXPECT_EQ(uri.shema(), "a");
    EXPECT_TRUE(uri.host().empty());
    EXPECT_EQ(uri.path(), "b");
    EXPECT_TRUE(uri.query().empty());
    EXPECT_TRUE(uri.fragment().empty());
}

// UriSuite::TestUriUserInfoHostPort1
TEST(UriSuite, TestUriUserInfoHostPort1) {
    turbo::UriView uri("http://abc:def@localhost");
    ASSERT_TRUE(parse_ok("http://abc:def@localhost", uri));
    EXPECT_EQ(uri.username(), "abc");
    EXPECT_EQ(uri.password(), "def");
    EXPECT_EQ(uri.host(), "localhost");
    EXPECT_TRUE(uri.port().empty());
}

// UriSuite::TestUriUserInfoHostPort2
TEST(UriSuite, TestUriUserInfoHostPort2) {
    turbo::UriView uri("http://abc:def@localhost:123");
    ASSERT_TRUE(parse_ok("http://abc:def@localhost:123", uri));
    EXPECT_EQ(uri.username(), "abc");
    EXPECT_EQ(uri.password(), "def");
    EXPECT_EQ(uri.host(), "localhost");
    EXPECT_EQ(uri.port(), "123");
}

// UriSuite::TestUriUserInfoHostPort22Bug1948038
TEST(UriSuite, TestUriUserInfoHostPort22Bug1948038) {
    turbo::UriView uri("http://user:21@host/");
    ASSERT_TRUE(parse_ok("http://user:21@host/", uri));
    EXPECT_EQ(uri.username(), "user");
    EXPECT_EQ(uri.password(), "21");
    EXPECT_EQ(uri.host(), "host");
    EXPECT_TRUE(uri.port().empty());

    EXPECT_TRUE(parse_ok("http://user:1234@192.168.0.1:1234/foo.com"));
    EXPECT_FALSE(parse_ok("http://moo:21@moo:21@moo/"));
    EXPECT_FALSE(parse_ok("http://moo:21@moo:21@moo:21/"));
}

// UriSuite::TestUriUserInfoHostPort23Bug3510198One
TEST(UriSuite, TestUriUserInfoHostPort23Bug3510198One) {
    turbo::UriView uri("http://user:%2F21@host/");
    ASSERT_TRUE(parse_ok("http://user:%2F21@host/", uri));
    EXPECT_EQ(uri.username(), "user");
    EXPECT_EQ(uri.password(), "%2F21");
    EXPECT_EQ(uri.host(), "host");
    EXPECT_TRUE(uri.port().empty());
}

// UriSuite::TestUriUserInfoHostPort23Bug3510198Two
TEST(UriSuite, TestUriUserInfoHostPort23Bug3510198Two) {
    turbo::UriView uri("http://%2Fuser:%2F21@host/");
    ASSERT_TRUE(parse_ok("http://%2Fuser:%2F21@host/", uri));
    EXPECT_EQ(uri.username(), "%2Fuser");
    EXPECT_EQ(uri.password(), "%2F21");
    EXPECT_EQ(uri.host(), "host");
}

// UriSuite::TestUriUserInfoHostPort23Bug3510198Three
TEST(UriSuite, TestUriUserInfoHostPort23Bug3510198Three) {
    turbo::UriView uri("http://user:!$&'()*+,;=@host/");
    ASSERT_TRUE(parse_ok("http://user:!$&'()*+,;=@host/", uri));
    EXPECT_EQ(uri.username(), "user");
    EXPECT_EQ(uri.password(), "!$&'()*+,;=");
    EXPECT_EQ(uri.host(), "host");
}

// UriSuite::TestUriUserInfoHostPort23Bug3510198Four
TEST(UriSuite, TestUriUserInfoHostPort23Bug3510198Four) {
    turbo::UriView uri("http://!$&'()*+,;=:password@host/");
    ASSERT_TRUE(parse_ok("http://!$&'()*+,;=:password@host/", uri));
    EXPECT_EQ(uri.username(), "!$&'()*+,;=");
    EXPECT_EQ(uri.password(), "password");
    EXPECT_EQ(uri.host(), "host");
}

// UriSuite::TestUriUserInfoHostPort23Bug3510198RelatedOne
TEST(UriSuite, TestUriUserInfoHostPort23Bug3510198RelatedOne) {
    turbo::UriView uri("http://@host/");
    ASSERT_TRUE(parse_ok("http://@host/", uri));
    EXPECT_TRUE(uri.username().empty());
    EXPECT_TRUE(uri.password().empty());
    EXPECT_EQ(uri.host(), "host");
}

// UriSuite::TestUriUserInfoHostPort23Bug3510198RelatedOneTwo
TEST(UriSuite, TestUriUserInfoHostPort23Bug3510198RelatedOneTwo) {
    turbo::UriView uri("http://%2Fhost/");
    ASSERT_TRUE(parse_ok("http://%2Fhost/", uri));
    EXPECT_TRUE(uri.username().empty());
    EXPECT_EQ(uri.host(), "%2Fhost");
}

// UriSuite::TestUriUserInfoHostPort23Bug3510198RelatedTwo
TEST(UriSuite, TestUriUserInfoHostPort23Bug3510198RelatedTwo) {
    turbo::UriView uri("http://::@host/");
    ASSERT_TRUE(parse_ok("http://::@host/", uri));
    EXPECT_TRUE(uri.username().empty());
    EXPECT_EQ(uri.password(), ":");
    EXPECT_EQ(uri.host(), "host");
}

// UriSuite::TestUriUserInfoHostPort3
TEST(UriSuite, TestUriUserInfoHostPort3) {
    turbo::UriView uri("http://abcdefg@localhost");
    ASSERT_TRUE(parse_ok("http://abcdefg@localhost", uri));
    EXPECT_EQ(uri.username(), "abcdefg");
    EXPECT_TRUE(uri.password().empty());
    EXPECT_EQ(uri.host(), "localhost");
    EXPECT_TRUE(uri.port().empty());
}

// UriSuite::TestUriUserInfoHostPort4
TEST(UriSuite, TestUriUserInfoHostPort4) {
    turbo::UriView uri("http://abcdefg@localhost:123");
    ASSERT_TRUE(parse_ok("http://abcdefg@localhost:123", uri));
    EXPECT_EQ(uri.username(), "abcdefg");
    EXPECT_TRUE(uri.password().empty());
    EXPECT_EQ(uri.host(), "localhost");
    EXPECT_EQ(uri.port(), "123");
}

// UriSuite::TestUriUserInfoHostPort5
TEST(UriSuite, TestUriUserInfoHostPort5) {
    turbo::UriView uri("http://localhost");
    ASSERT_TRUE(parse_ok("http://localhost", uri));
    EXPECT_TRUE(uri.username().empty());
    EXPECT_EQ(uri.host(), "localhost");
    EXPECT_TRUE(uri.port().empty());
}

// UriSuite::TestUriUserInfoHostPort6
TEST(UriSuite, TestUriUserInfoHostPort6) {
    turbo::UriView uri("http://localhost:123");
    ASSERT_TRUE(parse_ok("http://localhost:123", uri));
    EXPECT_TRUE(uri.username().empty());
    EXPECT_EQ(uri.host(), "localhost");
    EXPECT_EQ(uri.port(), "123");
}

// UriSuite::TestUriHostRegname
TEST(UriSuite, TestUriHostRegname) {
    turbo::UriView uri("http://example.com");
    ASSERT_TRUE(parse_ok("http://example.com", uri));
    EXPECT_EQ(uri.host(), "example.com");
    EXPECT_EQ(uri.host_type(), turbo::UriHostType::DEFAULT);
}

// UriSuite::TestUriHostIpFour1
TEST(UriSuite, TestUriHostIpFour1) {
    turbo::UriView uri("http://1.2.3.4:80");
    ASSERT_TRUE(parse_ok("http://1.2.3.4:80", uri));
    EXPECT_EQ(uri.host(), "1.2.3.4");
    EXPECT_EQ(uri.host_type(), turbo::UriHostType::IPV4);
    EXPECT_EQ(uri.port(), "80");
}

// UriSuite::TestUriHostIpFour2
TEST(UriSuite, TestUriHostIpFour2) {
    turbo::UriView uri("http://1.2.3.4");
    ASSERT_TRUE(parse_ok("http://1.2.3.4", uri));
    EXPECT_EQ(uri.host(), "1.2.3.4");
    EXPECT_EQ(uri.host_type(), turbo::UriHostType::IPV4);
}

// UriSuite::TestUriHostIpSix1
TEST(UriSuite, TestUriHostIpSix1) {
    turbo::UriView uri("http://[::1]:80");
    ASSERT_TRUE(parse_ok("http://[::1]:80", uri));
    EXPECT_EQ(uri.host(), "[::1]");
    EXPECT_EQ(uri.host_type(), turbo::UriHostType::IPV6);
    EXPECT_EQ(uri.port(), "80");
}

// UriSuite::TestUriHostIpSix2
TEST(UriSuite, TestUriHostIpSix2) {
    turbo::UriView uri("http://[::1]");
    ASSERT_TRUE(parse_ok("http://[::1]", uri));
    EXPECT_EQ(uri.host(), "[::1]");
    EXPECT_EQ(uri.host_type(), turbo::UriHostType::IPV6);
}

// UriSuite::TestUriHostEmpty
TEST(UriSuite, TestUriHostEmpty) {
    turbo::UriView uri("http://:123");
    ASSERT_TRUE(parse_ok("http://:123", uri));
    EXPECT_TRUE(uri.host().empty());
    EXPECT_EQ(uri.port(), "123");
}

// UriSuite::TestAddBase — RFC 3986 §5.4 merge_rfc_uri
TEST(UriSuite, TestAddBase) {
    constexpr std::string_view kBase = "http://a/b/c/d;p?q";

    // §5.4.1 Normal Examples

    ExpectMerge(kBase, "g:h", "g:h");
    ExpectMerge(kBase, "g", "http://a/b/c/g");
    ExpectMerge(kBase, "./g", "http://a/b/c/g");
    ExpectMerge(kBase, "g/", "http://a/b/c/g/");
    ExpectMerge(kBase, "/g", "http://a/g");
    ExpectMerge(kBase, "//g", "http://g");
    ExpectMerge(kBase, "?y", "http://a/b/c/d;p?y");
    ExpectMerge(kBase, "g?y", "http://a/b/c/g?y");
    ExpectMerge(kBase, "#s", "http://a/b/c/d;p?q#s");
    ExpectMerge(kBase, "g#s", "http://a/b/c/g#s");
    ExpectMerge(kBase, "g?y#s", "http://a/b/c/g?y#s");
    ExpectMerge(kBase, ";x", "http://a/b/c/;x");
    ExpectMerge(kBase, "g;x", "http://a/b/c/g;x");
    ExpectMerge(kBase, "g;x?y#s", "http://a/b/c/g;x?y#s");
    ExpectMerge(kBase, "", "http://a/b/c/d;p?q");
    ExpectMerge(kBase, ".", "http://a/b/c/");
    ExpectMerge(kBase, "./", "http://a/b/c/");
    ExpectMerge(kBase, "..", "http://a/b/");
    ExpectMerge(kBase, "../", "http://a/b/");
    ExpectMerge(kBase, "../g", "http://a/b/g");
    ExpectMerge(kBase, "../..", "http://a/");
    ExpectMerge(kBase, "../../", "http://a/");
    ExpectMerge(kBase, "../../g", "http://a/g");

    // §5.4.2 Abnormal Examples
    ExpectMerge(kBase, "../../../g", "http://a/g");
    ExpectMerge(kBase, "../../../../g", "http://a/g");
    ExpectMerge(kBase, "/./g", "http://a/g");
    ExpectMerge(kBase, "/../g", "http://a/g");
    ExpectMerge(kBase, "g.", "http://a/b/c/g.");
    ExpectMerge(kBase, ".g", "http://a/b/c/.g");
    ExpectMerge(kBase, "g..", "http://a/b/c/g..");
    ExpectMerge(kBase, "..g", "http://a/b/c/..g");
    ExpectMerge(kBase, "./../g", "http://a/b/g");
    ExpectMerge(kBase, "./g/.", "http://a/b/c/g/");
    ExpectMerge(kBase, "g/./h", "http://a/b/c/g/h");
    ExpectMerge(kBase, "g/../h", "http://a/b/c/h");
    ExpectMerge(kBase, "g;x=1/./y", "http://a/b/c/g;x=1/y");
    ExpectMerge(kBase, "g;x=1/../y", "http://a/b/c/y");
    ExpectMerge(kBase, "g?y/./x", "http://a/b/c/g?y/./x");
    ExpectMerge(kBase, "g?y/../x", "http://a/b/c/g?y/../x");
    ExpectMerge(kBase, "g#s/./x", "http://a/b/c/g#s/./x");
    ExpectMerge(kBase, "g#s/../x", "http://a/b/c/g#s/../x");
    ExpectMerge(kBase, "http:g", "http:g");

    ExpectMerge(kBase, "/", "http://a/");
    ExpectMerge(kBase, "/g/", "http://a/g/");
}

TEST(UriSuite, TestEncodeDecodeRfcUri) {
    turbo::UriView none;
    EXPECT_FALSE(turbo::encode_rfc_uri(none).ok());
    EXPECT_FALSE(turbo::decode_rfc_uri(none).ok());

    turbo::UriView uri;
    ASSERT_TRUE(parse_ok("http://user:pass@a/b%20c?x=%79#f%61", uri));
    EXPECT_EQ(uri.standard(), turbo::StandType::STD_RFC);
    EXPECT_EQ(uri.encode_type(), turbo::EnodeType::PRECENT);

    turbo::UriView enc = turbo::encode_rfc_uri(uri);
    ASSERT_TRUE(enc.ok());
    EXPECT_EQ(enc.encode_type(), turbo::EnodeType::PRECENT);
    EXPECT_EQ(view_href(enc), view_href(uri));

    turbo::UriView dec = turbo::decode_rfc_uri(uri);
    ASSERT_TRUE(dec.ok());
    EXPECT_EQ(dec.encode_type(), turbo::EnodeType::PLAIN);
    EXPECT_EQ(dec.username(), "user");
    EXPECT_EQ(dec.password(), "pass");
    EXPECT_EQ(dec.host(), "a");
    EXPECT_EQ(dec.path(), "/b c");
    EXPECT_EQ(dec.query(), "x=y");
    EXPECT_EQ(dec.fragment(), "fa");

    turbo::UriView dec2 = turbo::decode_rfc_uri(dec);
    ASSERT_TRUE(dec2.ok());
    EXPECT_EQ(dec2.encode_type(), turbo::EnodeType::PLAIN);
    EXPECT_EQ(view_href(dec2), view_href(dec));

    turbo::UriView enc2 = turbo::encode_rfc_uri(dec);
    ASSERT_TRUE(enc2.ok());
    EXPECT_EQ(enc2.encode_type(), turbo::EnodeType::PRECENT);
    EXPECT_EQ(enc2.path(), "/b%20c");
    EXPECT_EQ(enc2.query(), "x=y");
    EXPECT_EQ(enc2.fragment(), "fa");
}

TEST(UriSuite, TestGetRfcHref) {
    constexpr std::string_view kBase = "http://a/b/c/d;p?q";

    turbo::UriView bad;
    EXPECT_TRUE(turbo::get_rfc_href(bad).empty());

    turbo::UriView abs;
    ASSERT_TRUE(parse_ok("http://a/b/c/g?y#s", abs));
    EXPECT_EQ(turbo::get_rfc_href(abs), "http://a/b/c/g?y#s");

    turbo::UriView base;
    ASSERT_TRUE(parse_ok(kBase, base));
    turbo::UriView rel;
    ASSERT_TRUE(parse_ok_with_base("g", base, rel));
    EXPECT_TRUE(turbo::get_rfc_href(rel).empty());

    // §5.4.1
    ExpectHref(kBase, "g:h", "g:h");
    ExpectHref(kBase, "g", "http://a/b/c/g");
    ExpectHref(kBase, "./g", "http://a/b/c/g");
    ExpectHref(kBase, "g/", "http://a/b/c/g/");
    ExpectHref(kBase, "/g", "http://a/g");
    ExpectHref(kBase, "//g", "http://g");
    ExpectHref(kBase, "?y", "http://a/b/c/d;p?y");
    ExpectHref(kBase, "g?y", "http://a/b/c/g?y");
    ExpectHref(kBase, "#s", "http://a/b/c/d;p?q#s");
    ExpectHref(kBase, "g#s", "http://a/b/c/g#s");
    ExpectHref(kBase, "g?y#s", "http://a/b/c/g?y#s");
    ExpectHref(kBase, ";x", "http://a/b/c/;x");
    ExpectHref(kBase, "g;x", "http://a/b/c/g;x");
    ExpectHref(kBase, "g;x?y#s", "http://a/b/c/g;x?y#s");
    ExpectHref(kBase, "", "http://a/b/c/d;p?q");
    ExpectHref(kBase, ".", "http://a/b/c/");
    ExpectHref(kBase, "./", "http://a/b/c/");
    ExpectHref(kBase, "..", "http://a/b/");
    ExpectHref(kBase, "../", "http://a/b/");
    ExpectHref(kBase, "../g", "http://a/b/g");
    ExpectHref(kBase, "../..", "http://a/");
    ExpectHref(kBase, "../../", "http://a/");
    ExpectHref(kBase, "../../g", "http://a/g");

    // §5.4.2
    ExpectHref(kBase, "../../../g", "http://a/g");
    ExpectHref(kBase, "../../../../g", "http://a/g");
    ExpectHref(kBase, "/./g", "http://a/g");
    ExpectHref(kBase, "/../g", "http://a/g");
    ExpectHref(kBase, "g.", "http://a/b/c/g.");
    ExpectHref(kBase, ".g", "http://a/b/c/.g");
    ExpectHref(kBase, "g..", "http://a/b/c/g..");
    ExpectHref(kBase, "..g", "http://a/b/c/..g");
    ExpectHref(kBase, "./../g", "http://a/b/g");
    ExpectHref(kBase, "./g/.", "http://a/b/c/g/");
    ExpectHref(kBase, "g/./h", "http://a/b/c/g/h");
    ExpectHref(kBase, "g/../h", "http://a/b/c/h");
    ExpectHref(kBase, "g;x=1/./y", "http://a/b/c/g;x=1/y");
    ExpectHref(kBase, "g;x=1/../y", "http://a/b/c/y");
    ExpectHref(kBase, "g?y/./x", "http://a/b/c/g?y/./x");
    ExpectHref(kBase, "g?y/../x", "http://a/b/c/g?y/../x");
    ExpectHref(kBase, "g#s/./x", "http://a/b/c/g#s/./x");
    ExpectHref(kBase, "g#s/../x", "http://a/b/c/g#s/../x");
    ExpectHref(kBase, "http:g", "http:g");
    ExpectHref(kBase, "/", "http://a/");
    ExpectHref(kBase, "/g/", "http://a/g/");
}

// FourSuite::GoodUriReferences
TEST(FourSuite, GoodUriReferences) {
    turbo::UriView base("http://a/b/c/d;p?q");
    ASSERT_TRUE(parse_ok("http://a/b/c/d;p?q", base));

    EXPECT_TRUE(parse_ref_ok("file:///foo/bar", base));
    EXPECT_TRUE(parse_ref_ok("mailto:user@host?subject=blah", base));
    EXPECT_TRUE(parse_ref_ok("dav:", base));
    EXPECT_TRUE(parse_ref_ok("about:", base));

    EXPECT_TRUE(parse_ref_ok("http://www.yahoo.com", base));
    EXPECT_TRUE(parse_ref_ok("http://www.yahoo.com/", base));
    EXPECT_TRUE(parse_ref_ok("http://1.2.3.4/", base));
    EXPECT_TRUE(parse_ref_ok("http://www.yahoo.com/stuff", base));
    EXPECT_TRUE(parse_ref_ok("http://www.yahoo.com/stuff/", base));
    EXPECT_TRUE(parse_ref_ok("http://www.yahoo.com/hello%20world/", base));
    EXPECT_TRUE(parse_ref_ok("http://www.yahoo.com?name=obi", base));
    EXPECT_TRUE(parse_ref_ok("http://www.yahoo.com?name=obi+wan&status=jedi", base));
    EXPECT_TRUE(parse_ref_ok("http://www.yahoo.com?onery", base));
    EXPECT_TRUE(parse_ref_ok("http://www.yahoo.com#bottom", base));
    EXPECT_TRUE(parse_ref_ok("http://www.yahoo.com/yelp.html#bottom", base));
    EXPECT_TRUE(parse_ref_ok("https://www.yahoo.com/", base));
    EXPECT_TRUE(parse_ref_ok("ftp://www.yahoo.com/", base));
    EXPECT_TRUE(parse_ref_ok("ftp://www.yahoo.com/hello", base));
    EXPECT_TRUE(parse_ref_ok("demo.txt", base));
    EXPECT_TRUE(parse_ref_ok("demo/hello.txt", base));
    EXPECT_TRUE(parse_ref_ok("demo/hello.txt?query=hello#fragment", base));
    EXPECT_TRUE(parse_ref_ok("/cgi-bin/query?query=hello#fragment", base));
    EXPECT_TRUE(parse_ref_ok("/demo.txt", base));
    EXPECT_TRUE(parse_ref_ok("/hello/demo.txt", base));
    EXPECT_TRUE(parse_ref_ok("hello/demo.txt", base));
    EXPECT_TRUE(parse_ref_ok("/", base));
    EXPECT_TRUE(parse_ref_ok("", base));
    EXPECT_TRUE(parse_ref_ok("#", base));
    EXPECT_TRUE(parse_ref_ok("#here", base));

    EXPECT_TRUE(parse_ref_ok("http://www.yahoo.com?name=%00%01", base));
    EXPECT_TRUE(parse_ref_ok("http://www.yaho%6f.com", base));
    EXPECT_TRUE(parse_ref_ok("http://www.yahoo.com/hello%00world/", base));
    EXPECT_TRUE(parse_ref_ok("http://www.yahoo.com/hello+world/", base));
    EXPECT_TRUE(parse_ref_ok("http://www.yahoo.com?name=obi&", base));
    EXPECT_TRUE(parse_ref_ok("http://www.yahoo.com?name=obi&type=", base));
    EXPECT_TRUE(parse_ref_ok("http://www.yahoo.com/yelp.html#", base));

    EXPECT_TRUE(parse_ref_ok("//", base));

    EXPECT_TRUE(parse_ref_ok("http://example.org/aaa/bbb#ccc", base));
    EXPECT_TRUE(parse_ref_ok("mailto:local@domain.org", base));
    EXPECT_TRUE(parse_ref_ok("mailto:local@domain.org#frag", base));
    EXPECT_TRUE(parse_ref_ok("HTTP://EXAMPLE.ORG/AAA/BBB#CCC", base));
    EXPECT_TRUE(parse_ref_ok("//example.org/aaa/bbb#ccc", base));
    EXPECT_TRUE(parse_ref_ok("/aaa/bbb#ccc", base));
    EXPECT_TRUE(parse_ref_ok("bbb#ccc", base));
    EXPECT_TRUE(parse_ref_ok("#ccc", base));
    EXPECT_TRUE(parse_ref_ok("A'C", base));

    EXPECT_TRUE(parse_ref_ok("http://example.org/aaa%2fbbb#ccc", base));
    EXPECT_TRUE(parse_ref_ok("http://example.org/aaa%2Fbbb#ccc", base));
    EXPECT_TRUE(parse_ref_ok("%2F", base));
    EXPECT_TRUE(parse_ref_ok("aaa%2Fbbb", base));

    EXPECT_TRUE(parse_ref_ok("http://example.org:80/aaa/bbb#ccc", base));
    EXPECT_TRUE(parse_ref_ok("http://example.org:/aaa/bbb#ccc", base));
    EXPECT_TRUE(parse_ref_ok("http://example.org./aaa/bbb#ccc", base));
    EXPECT_TRUE(parse_ref_ok("http://example.123./aaa/bbb#ccc", base));
    EXPECT_TRUE(parse_ref_ok("http://example.org", base));

    EXPECT_TRUE(parse_ref_ok(
        "http://[FEDC:BA98:7654:3210:FEDC:BA98:7654:3210]:80/index.html", base));
    EXPECT_TRUE(parse_ref_ok("http://[1080:0:0:0:8:800:200C:417A]/index.html", base));
    EXPECT_TRUE(parse_ref_ok("http://[3ffe:2a00:100:7031::1]", base));
    EXPECT_TRUE(parse_ref_ok("http://[1080::8:800:200C:417A]/foo", base));
    EXPECT_TRUE(parse_ref_ok("http://[::192.9.5.5]/ipng", base));
    EXPECT_TRUE(parse_ref_ok("http://[::FFFF:129.144.52.38]:80/index.html", base));
    EXPECT_TRUE(parse_ref_ok("http://[2010:836B:4179::836B:4179]", base));
    EXPECT_TRUE(parse_ref_ok("//[2010:836B:4179::836B:4179]", base));

    EXPECT_TRUE(parse_ref_ok("http://example/Andr&#567;", base));
    EXPECT_TRUE(parse_ref_ok(
        "file:///C:/DEV/Haskell/lib/HXmlToolbox-3.01/examples/", base));

}

// FourSuite::BadUriReferences（uriparser：只 parse，不带 base）
TEST(FourSuite, BadUriReferences) {
    EXPECT_FALSE(parse_ok("beepbeep\x07\x07"));
    EXPECT_FALSE(parse_ok("\n"));
    EXPECT_FALSE(parse_ok("::"));

    EXPECT_FALSE(parse_ok("http://www yahoo.com"));
    EXPECT_FALSE(parse_ok("http://www.yahoo.com/hello world/"));
    EXPECT_FALSE(parse_ok("http://www.yahoo.com/yelp.html#\""));

    EXPECT_FALSE(parse_ok("[2010:836B:4179::836B:4179]"));
    EXPECT_FALSE(parse_ok(" "));
    EXPECT_FALSE(parse_ok("%"));
    EXPECT_FALSE(parse_ok("A%Z"));
    EXPECT_FALSE(parse_ok("%ZZ"));
    EXPECT_FALSE(parse_ok("%AZ"));
    EXPECT_FALSE(parse_ok("A C"));
    EXPECT_FALSE(parse_ok("A\\'C"));
    EXPECT_FALSE(parse_ok("A`C"));
    EXPECT_FALSE(parse_ok("A<C"));
    EXPECT_FALSE(parse_ok("A>C"));
    EXPECT_FALSE(parse_ok("A^C"));
    EXPECT_FALSE(parse_ok("A\\\\C"));
    EXPECT_FALSE(parse_ok("A{C"));
    EXPECT_FALSE(parse_ok("A|C"));
    EXPECT_FALSE(parse_ok("A}C"));
    EXPECT_FALSE(parse_ok("A[C"));
    EXPECT_FALSE(parse_ok("A]C"));
    EXPECT_FALSE(parse_ok("A[**]C"));
    EXPECT_FALSE(parse_ok("http://[xyz]/"));
    EXPECT_FALSE(parse_ok("http://]/"));
    EXPECT_FALSE(parse_ok("http://example.org/[2010:836B:4179::836B:4179]"));
    EXPECT_FALSE(parse_ok("http://example.org/abc#[2010:836B:4179::836B:4179]"));
    EXPECT_FALSE(parse_ok("http://example.org/xxx/[qwerty]#a[b]"));
    EXPECT_FALSE(parse_ok("http://w3c.org:80path1/path2"));
}
