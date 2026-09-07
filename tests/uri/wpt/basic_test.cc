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

#include <string>
#include <string_view>

#include <gtest/gtest.h>
#include <turbo/uri/types.h>
#include <turbo/uri/uri_view.h>
#include <turbo/uri/wpt/parser.h>

namespace {

std::string href(const turbo::UriView &u) {
    return turbo::get_wpt_href(u);
}

std::string protocol(const turbo::UriView &u) {
    if (!u.has_shema()) {
        return {};
    }
    return std::string(u.shema()) + ":";
}

std::string host_with_port(const turbo::UriView &u) {
    if (!u.has_host()) {
        return {};
    }
    if (u.has_port() && !u.port().empty()) {
        return std::string(u.host()) + ":" + std::string(u.port());
    }
    return std::string(u.host());
}

std::string search(const turbo::UriView &u) {
    if (!u.has_query()) {
        return {};
    }
    return "?" + std::string(u.query());
}

std::string hash(const turbo::UriView &u) {
    if (!u.has_fragment()) {
        return {};
    }
    return "#" + std::string(u.fragment());
}

}  // namespace

TEST(basic_tests, insane_url) {
    auto r = turbo::parse_wpt_uri("e:@EEEEEEEEEE");
    ASSERT_TRUE(r.ok());
    ASSERT_EQ(protocol(r), "e:");
    ASSERT_EQ(r.username(), "");
    ASSERT_EQ(r.password(), "");
    ASSERT_FALSE(r.has_host());
    ASSERT_EQ(r.port(), "");
    ASSERT_EQ(r.path(), "@EEEEEEEEEE");
}

TEST(basic_tests, bad_percent_encoding) {
    auto r = turbo::parse_wpt_uri("http://www.google.com/%X%");
    ASSERT_TRUE(r.ok());
    ASSERT_EQ(href(r), "http://www.google.com/%X%");
    r = turbo::parse_wpt_uri("http://www.google%X%.com/");
    ASSERT_FALSE(r.ok());
    r = turbo::parse_wpt_uri("http://www.google.com/");
    ASSERT_TRUE(r.ok());
    r = turbo::parse_wpt_uri("http://www.google.com/%X%");
    ASSERT_TRUE(r.ok());
    ASSERT_EQ(href(r), "http://www.google.com/%X%");
}

TEST(basic_tests, spaces_spaces) {
    auto r = turbo::parse_wpt_uri("http://www.google.com/%37/ /");
    ASSERT_TRUE(r.ok());
    ASSERT_EQ(href(r), "http://www.google.com/%37/%20/");
    r = turbo::parse_wpt_uri("http://www.google.com/  /  /+/");
    ASSERT_TRUE(r.ok());
    ASSERT_EQ(href(r), "http://www.google.com/%20%20/%20%20/+/");
    r = turbo::parse_wpt_uri("http://www.google com/");
    ASSERT_FALSE(r.ok());
}

TEST(basic_tests, pluses) {
    auto r = turbo::parse_wpt_uri("http://www.google.com/%37+/");
    ASSERT_TRUE(r.ok());
    ASSERT_EQ(href(r), "http://www.google.com/%37+/");
    r = turbo::parse_wpt_uri("http://www.google+com/");
    ASSERT_TRUE(r.ok());
    ASSERT_EQ(href(r), "http://www.google+com/");
}

TEST(basic_tests, empty_url_should_return_false) {
    auto r = turbo::parse_wpt_uri("");
    ASSERT_FALSE(r.ok());
}

TEST(basic_tests, readme) {
    auto url = turbo::parse_wpt_uri("https://www.google.com");
    ASSERT_TRUE(url.ok());
}

TEST(basic_tests, readme2) {
    auto url = turbo::parse_wpt_uri("https://username:password@www.google.com/");
    ASSERT_TRUE(url.ok());
    ASSERT_EQ(url.username(), "username");
    ASSERT_EQ(url.password(), "password");
    ASSERT_EQ(href(url), "https://username:password@www.google.com/");
}

TEST(basic_tests, readme3) {
    auto url = turbo::parse_wpt_uri("wss://www.google.com/");
    ASSERT_TRUE(url.ok());
    ASSERT_EQ(protocol(url), "wss:");
    ASSERT_EQ(href(url), "wss://www.google.com/");
}

TEST(basic_tests, readme4) {
    auto url = turbo::parse_wpt_uri("https://github.com/");
    ASSERT_TRUE(url.ok());
    ASSERT_EQ(host_with_port(url), "github.com");
}

TEST(basic_tests, readme5) {
    auto url = turbo::parse_wpt_uri("https://www.google.com:8080/");
    ASSERT_TRUE(url.ok());
    ASSERT_EQ(url.port(), "8080");
}

TEST(basic_tests, readme6) {
    auto url = turbo::parse_wpt_uri("https://www.google.com/my-super-long-path");
    ASSERT_TRUE(url.ok());
    ASSERT_EQ(url.path(), "/my-super-long-path");
}

TEST(basic_tests, readme7) {
    auto url = turbo::parse_wpt_uri("https://www.google.com/?target=self");
    ASSERT_TRUE(url.ok());
    ASSERT_EQ(search(url), "?target=self");
}

TEST(basic_tests, readme8) {
    auto url = turbo::parse_wpt_uri("https://www.google.com/#is-this-the-real-life");
    ASSERT_TRUE(url.ok());
    ASSERT_EQ(hash(url), "#is-this-the-real-life");
}

TEST(basic_tests, nodejs1) {
    auto base = turbo::parse_wpt_uri("http://other.com/");
    ASSERT_TRUE(base.ok());
    auto url = turbo::parse_wpt_uri("http://GOOgoo.com", base);
    ASSERT_TRUE(url.ok());
    ASSERT_EQ(href(url), "http://googoo.com/");
}

TEST(basic_tests, nodejs2) {
    auto url = turbo::parse_wpt_uri("data:space    ?test");
    ASSERT_TRUE(url.ok());
    ASSERT_EQ(search(url), "?test");
    ASSERT_EQ(url.path(), "space    ");
    ASSERT_EQ(href(url), "data:space    ?test");
}

TEST(basic_tests, nodejs3) {
    auto url = turbo::parse_wpt_uri("data:space    ?test#test");
    ASSERT_TRUE(url.ok());
    ASSERT_EQ(search(url), "?test");
    ASSERT_EQ(url.path(), "space    ");
    ASSERT_EQ(hash(url), "#test");
    ASSERT_EQ(href(url), "data:space    ?test#test");
}

TEST(basic_tests, nodejs4) {
    auto url = turbo::parse_wpt_uri("http://0300.168.0xF0");
    ASSERT_TRUE(url.ok());
    ASSERT_EQ(protocol(url), "http:");
    ASSERT_EQ(href(url), "http://192.168.0.240/");
}

TEST(basic_tests, empty_url) {
    auto url = turbo::parse_wpt_uri("");
    ASSERT_FALSE(url.ok());
}

TEST(basic_tests, just_hash) {
    auto url = turbo::parse_wpt_uri("#x");
    ASSERT_FALSE(url.ok());
}

TEST(basic_tests, empty_host_dash_dash_path) {
    auto url = turbo::parse_wpt_uri("something:/.//");
    ASSERT_TRUE(url.ok());
    ASSERT_EQ(href(url), "something:/.//");
    ASSERT_EQ(url.path(), "//");
    ASSERT_FALSE(url.has_host());
}

TEST(basic_tests, confusing_mess) {
    auto base_url = turbo::parse_wpt_uri("http://example.org/foo/bar");
    ASSERT_TRUE(base_url.ok());
    auto url = turbo::parse_wpt_uri("http://::@c@d:2", base_url);
    ASSERT_TRUE(url.ok());
    ASSERT_EQ(url.host(), "d");
    ASSERT_EQ(host_with_port(url), "d:2");
    ASSERT_EQ(url.path(), "/");
    ASSERT_EQ(href(url), "http://:%3A%40c@d:2/");
}

TEST(basic_tests, standard_file) {
    auto url = turbo::parse_wpt_uri("file:///tmp/mock/path");
    ASSERT_TRUE(url.ok());
    ASSERT_EQ(url.path(), "/tmp/mock/path");
    ASSERT_TRUE(url.has_host());
    ASSERT_EQ(url.host(), "");
    ASSERT_EQ(host_with_port(url), "");
    ASSERT_EQ(href(url), "file:///tmp/mock/path");
}

TEST(basic_tests, default_port_should_be_removed) {
    auto url = turbo::parse_wpt_uri("http://www.google.com:80");
    ASSERT_TRUE(url.ok());
    ASSERT_EQ(url.port(), "");
    ASSERT_EQ(host_with_port(url), "www.google.com");
}

TEST(basic_tests, test_amazon) {
    auto url = turbo::parse_wpt_uri("HTTP://AMAZON.COM");
    ASSERT_TRUE(url.ok());
    ASSERT_EQ(href(url), "http://amazon.com/");
}

TEST(basic_tests, username_present) {
    auto url = turbo::parse_wpt_uri("http://me@example.net");
    ASSERT_TRUE(url.ok());
    ASSERT_EQ(url.username(), "me");
    ASSERT_EQ(href(url), "http://me@example.net/");
}

TEST(basic_tests, password_present) {
    auto url = turbo::parse_wpt_uri("http://user:pass@example.net");
    ASSERT_TRUE(url.ok());
    ASSERT_EQ(url.password(), "pass");
    ASSERT_EQ(href(url), "http://user:pass@example.net/");
}

TEST(basic_tests, empty_username_with_password) {
    auto url = turbo::parse_wpt_uri("http://:pass@example.net");
    ASSERT_TRUE(url.ok());
    ASSERT_EQ(url.username(), "");
    ASSERT_EQ(url.password(), "pass");
    ASSERT_EQ(href(url), "http://:pass@example.net/");
}

TEST(basic_tests, should_update_password_correctly) {
    auto url = turbo::parse_wpt_uri(
        "https://username:test@host:8000/path?query#fragment");
    ASSERT_TRUE(url.ok());
    ASSERT_EQ(url.password(), "test");
    ASSERT_EQ(href(url),
        "https://username:test@host:8000/path?query#fragment");
}

TEST(basic_tests, node_issue_47889) {
    auto urlbase = turbo::parse_wpt_uri("a:b");
    ASSERT_TRUE(urlbase.ok());
    ASSERT_EQ(href(urlbase), "a:b");
    ASSERT_EQ(protocol(urlbase), "a:");
    ASSERT_EQ(urlbase.path(), "b");
    auto expected_url = turbo::parse_wpt_uri("a:b#");
    ASSERT_TRUE(expected_url.ok());
    ASSERT_EQ(href(expected_url), "a:b#");
    ASSERT_EQ(expected_url.path(), "b");
    // opaque base + non-# relative is failure (WHATWG)
    auto url = turbo::parse_wpt_uri("..#", urlbase);
    ASSERT_FALSE(url.ok());
    auto only_hash = turbo::parse_wpt_uri("#", urlbase);
    ASSERT_TRUE(only_hash.ok());
    ASSERT_EQ(href(only_hash), "a:b#");
}

TEST(basic_tests, can_parse) {
    ASSERT_TRUE(turbo::parse_wpt_uri("https://www.yagiz.co").ok());
    auto base = turbo::parse_wpt_uri("https://yagiz.co");
    ASSERT_TRUE(base.ok());
    ASSERT_TRUE(turbo::parse_wpt_uri("/hello", base).ok());
    ASSERT_EQ(href(turbo::parse_wpt_uri("/hello", base)), "https://yagiz.co/hello");

    auto invalid_base = turbo::parse_wpt_uri("!!!!!!!1");
    ASSERT_FALSE(invalid_base.ok());
    ASSERT_FALSE(turbo::parse_wpt_uri("!!!").ok());
}

TEST(basic_tests, node_issue_48254) {
    auto base_url = turbo::parse_wpt_uri("localhost:80");
    ASSERT_TRUE(base_url.ok());
    ASSERT_FALSE(base_url.has_host());
    ASSERT_EQ(host_with_port(base_url), "");
    ASSERT_EQ(base_url.path(), "80");
    ASSERT_EQ(href(base_url), "localhost:80");
    auto url = turbo::parse_wpt_uri("", base_url);
    ASSERT_FALSE(url.ok());
}

TEST(basic_tests, url_host_type) {
    ASSERT_EQ(turbo::parse_wpt_uri("http://localhost:3000").host_type(),
        turbo::UriHostType::DEFAULT);
    ASSERT_EQ(turbo::parse_wpt_uri("http://0.0.0.0").host_type(),
        turbo::UriHostType::IPV4);
    ASSERT_EQ(
        turbo::parse_wpt_uri("http://[2001:db8:3333:4444:5555:6666:7777:8888]")
            .host_type(),
        turbo::UriHostType::IPV6);
}

TEST(basic_tests, nodejs_49650) {
    auto out = turbo::parse_wpt_uri("http://foo");
    ASSERT_TRUE(out.ok());
    ASSERT_EQ(href(out), "http://foo/");
}

TEST(basic_tests, nodejs_50235) {
    auto out = turbo::parse_wpt_uri("http://test.com:5/path?param=1");
    ASSERT_TRUE(out.ok());
    ASSERT_EQ(href(out), "http://test.com:5/path?param=1");
}

TEST(basic_tests, nodejs_51514) {
    auto out = turbo::parse_wpt_uri("http://1.1.1.256");
    ASSERT_FALSE(out.ok());
}

TEST(basic_tests, nodejs_51593) {
    auto out = turbo::parse_wpt_uri("http://\u200b123.123.123.123");
    ASSERT_TRUE(out.ok());
    ASSERT_EQ(href(out), "http://123.123.123.123/");
}

TEST(basic_tests, nodejs_51619) {
    auto out = turbo::parse_wpt_uri("https://0.0.0.0x100/");
    ASSERT_FALSE(out.ok());
}

TEST(basic_tests, nodejs_undici_2971) {
    std::string_view base =
        "https://non-ascii-location-header.sys.workers.dev/redirect";
    auto base_url = turbo::parse_wpt_uri(base);
    ASSERT_TRUE(base_url.ok());
    auto out = turbo::parse_wpt_uri("/\xec\x95\x88\xeb\x85\x95", base_url);
    ASSERT_TRUE(out.ok());
    ASSERT_EQ(
        href(out),
        R"(https://non-ascii-location-header.sys.workers.dev/%EC%95%88%EB%85%95)");
}

TEST(basic_tests, merge_encode_decode_href) {
    auto base = turbo::parse_wpt_uri("https://example.com/a/b");
    ASSERT_TRUE(base.ok());
    auto merged = turbo::merge_wpt_uri("../c", base);
    ASSERT_TRUE(merged.ok());
    ASSERT_EQ(href(merged), "https://example.com/c");

    auto enc = turbo::encode_wpt_uri(merged);
    ASSERT_TRUE(enc.ok());
    ASSERT_EQ(enc.encode_type(), turbo::EnodeType::PRECENT);
    auto dec = turbo::decode_wpt_uri(enc);
    ASSERT_TRUE(dec.ok());
    ASSERT_EQ(dec.encode_type(), turbo::EnodeType::PLAIN);

    ASSERT_EQ(turbo::get_wpt_href(base, turbo::UriView("../c")),
        "https://example.com/c");
}
