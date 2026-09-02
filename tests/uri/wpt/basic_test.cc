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

#include <string_view>

#include <gtest/gtest.h>
#include <turbo/uri/uri_common_base.h>
#include <turbo/uri/wpt/parser.h>
#include <turbo/uri/wpt/uri.h>

TEST(basic_tests, insane_url) {
  auto r = turbo::parse_wpt_uri("e:@EEEEEEEEEE");
  ASSERT_TRUE(r.ok());
  ASSERT_EQ(r.get_protocol(), "e:");
  ASSERT_EQ(r.get_username(), "");
  ASSERT_EQ(r.get_password(), "");
  ASSERT_EQ(r.get_hostname(), "");
  ASSERT_EQ(r.get_port(), "");
  ASSERT_EQ(r.get_pathname(), "@EEEEEEEEEE");
  SUCCEED();
}

TEST(basic_tests, bad_percent_encoding) {
  auto r = turbo::parse_wpt_uri("http://www.google.com/%X%");
  ASSERT_TRUE(r.ok());
  ASSERT_EQ(r.get_href(), "http://www.google.com/%X%");
  r = turbo::parse_wpt_uri("http://www.google%X%.com/");
  ASSERT_FALSE(r.ok());
  r = turbo::parse_wpt_uri("http://www.google.com/");
  ASSERT_TRUE(r.ok());
  r.set_href("http://www.google.com/%X%");
  ASSERT_EQ(r.get_href(), "http://www.google.com/%X%");
  ASSERT_FALSE(r.set_host("www.google%X%.com"));
  SUCCEED();
}

TEST(basic_tests, spaces_spaces) {
  auto r = turbo::parse_wpt_uri("http://www.google.com/%37/ /");
  ASSERT_TRUE(r.ok());
  ASSERT_EQ(r.get_href(), "http://www.google.com/%37/%20/");
  r.set_href("http://www.google.com/  /  /+/");
  ASSERT_TRUE(r.ok());
  ASSERT_EQ(r.get_href(), "http://www.google.com/%20%20/%20%20/+/");
  r = turbo::parse_wpt_uri("http://www.google com/");
  ASSERT_FALSE(r.ok());
  SUCCEED();
}

TEST(basic_tests, pluses) {
  auto r = turbo::parse_wpt_uri("http://www.google.com/%37+/");
  ASSERT_TRUE(r.ok());
  ASSERT_EQ(r.get_href(), "http://www.google.com/%37+/");
  r = turbo::parse_wpt_uri("http://www.google+com/");
  ASSERT_TRUE(r.ok());
  ASSERT_EQ(r.get_href(), "http://www.google+com/");
  SUCCEED();
}

TEST(basic_tests, set_host_should_return_false_sometimes) {
  auto r = turbo::parse_wpt_uri("mailto:a@b.com");
  ASSERT_FALSE(r.set_host("something"));
  auto r2 = turbo::parse_wpt_uri("mailto:a@b.com");
  ASSERT_FALSE(r2.set_host("something"));
  SUCCEED();
}

TEST(basic_tests, empty_url_should_return_false) {
  auto r = turbo::parse_wpt_uri("");
  ASSERT_FALSE(r.ok());
  SUCCEED();
}

TEST(basic_tests, set_host_should_return_true_sometimes) {
  auto r = turbo::parse_wpt_uri("https://www.google.com");
  ASSERT_TRUE(r.set_host("something"));
  SUCCEED();
}

TEST(basic_tests, set_hostname_should_return_false_sometimes) {
  auto r = turbo::parse_wpt_uri("mailto:a@b.com");
  ASSERT_FALSE(r.set_hostname("something"));
  SUCCEED();
}

TEST(basic_tests, set_hostname_should_return_true_sometimes) {
  auto r = turbo::parse_wpt_uri("https://www.google.com");
  ASSERT_TRUE(r.set_hostname("something"));
  SUCCEED();
}

TEST(basic_tests, readme) {
  auto url = turbo::parse_wpt_uri("https://www.google.com");
  ASSERT_TRUE(url.ok());
  SUCCEED();
}

TEST(basic_tests, readme2) {
  auto url = turbo::parse_wpt_uri("https://www.google.com");
  url.set_username("username");
  url.set_password("password");
  ASSERT_EQ(url.get_href(), "https://username:password@www.google.com/");
  SUCCEED();
}

TEST(basic_tests, readme3) {
  auto url = turbo::parse_wpt_uri("https://www.google.com");
  ASSERT_EQ(url.set_protocol("wss"), true);
  ASSERT_EQ(url.get_protocol(), "wss:");
  ASSERT_EQ(url.get_href(), "wss://www.google.com/");
  SUCCEED();
}

TEST(basic_tests, set_protocol_should_return_false_sometimes) {
  auto url = turbo::parse_wpt_uri("file:");
  ASSERT_EQ(url.set_protocol("https"), false);
  ASSERT_EQ(url.set_host("google.com"), true);
  ASSERT_EQ(url.get_href(), "file://google.com/");
  SUCCEED();
}

TEST(basic_tests, set_protocol_should_return_true_sometimes) {
  auto url = turbo::parse_wpt_uri("file:");
  ASSERT_EQ(url.set_host("google.com"), true);
  ASSERT_EQ(url.set_protocol("https"), true);
  ASSERT_EQ(url.get_href(), "https://google.com/");
  SUCCEED();
}

TEST(basic_tests, readme4) {
  auto url = turbo::parse_wpt_uri("https://www.google.com");
  url.set_host("github.com");
  ASSERT_EQ(url.get_host(), "github.com");
  SUCCEED();
}

TEST(basic_tests, readme5) {
  auto url = turbo::parse_wpt_uri("https://www.google.com");
  url.set_port("8080");
  ASSERT_EQ(url.get_port(), "8080");
  SUCCEED();
}

TEST(basic_tests, readme6) {
  auto url = turbo::parse_wpt_uri("https://www.google.com");
  url.set_pathname("/my-super-long-path");
  ASSERT_EQ(url.get_pathname(), "/my-super-long-path");
  SUCCEED();
}

TEST(basic_tests, readme7) {
  auto url = turbo::parse_wpt_uri("https://www.google.com");
  url.set_search("target=self");
  ASSERT_EQ(url.get_search(), "?target=self");
  SUCCEED();
}

TEST(basic_tests, readme8) {
  auto url = turbo::parse_wpt_uri("https://www.google.com");
  url.set_hash("is-this-the-real-life");
  ASSERT_EQ(url.get_hash(), "#is-this-the-real-life");
  SUCCEED();
}

TEST(basic_tests, nodejs1) {
  auto base = turbo::parse_wpt_uri("http://other.com/");
  ASSERT_TRUE(base.ok());
  auto url = turbo::parse_wpt_uri("http://GOOgoo.com", &base);
  ASSERT_TRUE(url.ok());
  SUCCEED();
}

TEST(basic_tests, nodejs2) {
  auto url = turbo::parse_wpt_uri("data:space    ?test");
  ASSERT_EQ(url.get_search(), "?test");
  url.set_search("");
  ASSERT_EQ(url.get_search(), "");
  ASSERT_EQ(url.get_pathname(), "space");
  ASSERT_EQ(url.get_href(), "data:space");
  SUCCEED();
}

TEST(basic_tests, nodejs3) {
  auto url = turbo::parse_wpt_uri("data:space    ?test#test");
  ASSERT_EQ(url.get_search(), "?test");
  url.set_search("");
  ASSERT_EQ(url.get_search(), "");
  ASSERT_EQ(url.get_pathname(), "space    ");
  ASSERT_EQ(url.get_href(), "data:space    #test");
  SUCCEED();
}

// https://github.com/nodejs/node/issues/46755
TEST(basic_tests, nodejs4) {
  auto url = turbo::parse_wpt_uri("file:///var/log/system.log");
  url.set_href("http://0300.168.0xF0");
  ASSERT_EQ(url.get_protocol(), "http:");
  ASSERT_EQ(url.get_href(), "http://192.168.0.240/");
  SUCCEED();
}

TEST(basic_tests, empty_url) {
  auto url = turbo::parse_wpt_uri("");
  ASSERT_FALSE(url.ok());
  SUCCEED();
}

TEST(basic_tests, just_hash) {
  auto url = turbo::parse_wpt_uri("#x");
  ASSERT_FALSE(url.ok());
  SUCCEED();
}

TEST(basic_tests, empty_host_dash_dash_path) {
  auto url = turbo::parse_wpt_uri("something:/.//");
  ASSERT_TRUE(url.ok());
  ASSERT_FALSE(url.has_opaque_path());
  ASSERT_EQ(url.get_href(), "something:/.//");
  ASSERT_EQ(url.get_pathname(), "//");
  ASSERT_EQ(url.get_hostname(), "");
  SUCCEED();
}

TEST(basic_tests, confusing_mess) {
  auto base_url = turbo::parse_wpt_uri("http://example.org/foo/bar");
  ASSERT_TRUE(base_url.ok());
  auto url = turbo::parse_wpt_uri("http://::@c@d:2", &base_url);
  ASSERT_TRUE(url.ok());
  ASSERT_FALSE(url.has_opaque_path());
  ASSERT_EQ(url.get_hostname(), "d");
  ASSERT_EQ(url.get_host(), "d:2");
  ASSERT_EQ(url.get_pathname(), "/");
  ASSERT_EQ(url.get_href(), "http://:%3A%40c@d:2/");
  ASSERT_EQ(url.get_origin(), "http://d:2");
  SUCCEED();
}

TEST(basic_tests, standard_file) {
  auto url = turbo::parse_wpt_uri("file:///tmp/mock/path");
  ASSERT_TRUE(url.ok());
  ASSERT_TRUE(url.has_empty_hostname());
  ASSERT_FALSE(url.has_opaque_path());
  ASSERT_EQ(url.get_pathname(), "/tmp/mock/path");
  ASSERT_EQ(url.get_hostname(), "");
  ASSERT_EQ(url.get_host(), "");
  ASSERT_EQ(url.get_href(), "file:///tmp/mock/path");
  SUCCEED();
}

TEST(basic_tests, default_port_should_be_removed) {
  auto url = turbo::parse_wpt_uri("http://www.google.com:443");
  ASSERT_TRUE(url.ok());
  url.set_protocol("https");
  ASSERT_EQ(url.get_port(), "");
  ASSERT_EQ(url.get_host(), "www.google.com");
  SUCCEED();
}

TEST(basic_tests, test_amazon) {
  auto url = turbo::parse_wpt_uri("HTTP://AMAZON.COM");
  ASSERT_TRUE(url.ok());
  ASSERT_EQ(url.get_href(), "http://amazon.com/");
  SUCCEED();
}

TEST(basic_tests, remove_username) {
  auto url = turbo::parse_wpt_uri("http://me@example.net");
  ASSERT_TRUE(url.ok());
  url.set_username("");
  ASSERT_EQ(url.get_username(), "");
  ASSERT_EQ(url.get_href(), "http://example.net/");
  SUCCEED();
}

TEST(basic_tests, remove_password) {
  auto url = turbo::parse_wpt_uri("http://user:pass@example.net");
  ASSERT_TRUE(url.ok());
  url.set_password("");
  ASSERT_EQ(url.get_password(), "");
  ASSERT_EQ(url.get_href(), "http://user@example.net/");
  SUCCEED();
}

TEST(basic_tests, remove_password_with_empty_username) {
  auto url = turbo::parse_wpt_uri("http://:pass@example.net");
  ASSERT_TRUE(url.ok());
  url.set_password("");
  ASSERT_EQ(url.get_username(), "");
  ASSERT_EQ(url.get_password(), "");
  ASSERT_EQ(url.get_href(), "http://example.net/");
  SUCCEED();
}

TEST(basic_tests, should_remove_dash_dot) {
  auto url = turbo::parse_wpt_uri("non-spec:/.//p");
  ASSERT_TRUE(url.ok());
  ASSERT_FALSE(url.has_empty_hostname());
  ASSERT_FALSE(url.has_hostname());
  url.set_hostname("h");
  ASSERT_TRUE(url.has_hostname());
  ASSERT_FALSE(url.has_empty_hostname());
  ASSERT_EQ(url.get_pathname(), "//p");
  ASSERT_EQ(url.get_href(), "non-spec://h//p");
  SUCCEED();
}

TEST(basic_tests, should_remove_dash_dot_with_empty_hostname) {
  auto url = turbo::parse_wpt_uri("non-spec:/.//p");
  ASSERT_TRUE(url.ok());
  ASSERT_EQ(url.get_pathname(), "//p");
  ASSERT_FALSE(url.has_empty_hostname());
  ASSERT_FALSE(url.has_hostname());
  url.set_hostname("");
  ASSERT_TRUE(url.has_hostname());
  ASSERT_TRUE(url.has_empty_hostname());
  ASSERT_EQ(url.get_pathname(), "//p");
  ASSERT_EQ(url.get_href(), "non-spec:////p");
  SUCCEED();
}

TEST(basic_tests, should_add_dash_dot_on_pathname) {
  auto url = turbo::parse_wpt_uri("non-spec:/");
  ASSERT_TRUE(url.ok());
  url.set_pathname("//p");
  ASSERT_EQ(url.get_pathname(), "//p");
  ASSERT_EQ(url.get_href(), "non-spec:/.//p");
  SUCCEED();
}

TEST(basic_tests, should_update_password_correctly) {
  auto url = turbo::parse_wpt_uri(
      "https://username:password@host:8000/path?query#fragment");
  ASSERT_TRUE(url.ok());
  ASSERT_TRUE(url.set_password("test"));
  ASSERT_EQ(url.get_password(), "test");
  ASSERT_EQ(url.get_href(),
            "https://username:test@host:8000/path?query#fragment");
  SUCCEED();
}

// https://github.com/nodejs/node/issues/47889
TEST(basic_tests, node_issue_47889) {
  auto urlbase = turbo::parse_wpt_uri("a:b");
  ASSERT_EQ(urlbase.get_href(), "a:b");
  ASSERT_EQ(urlbase.get_protocol(), "a:");
  ASSERT_EQ(urlbase.get_pathname(), "b");
  ASSERT_TRUE(urlbase.has_opaque_path());
  ASSERT_TRUE(urlbase.ok());
  auto expected_url = turbo::parse_wpt_uri("a:b#");
  ASSERT_TRUE(expected_url.ok());
  ASSERT_TRUE(expected_url.has_opaque_path());
  ASSERT_EQ(expected_url.get_href(), "a:b#");
  ASSERT_EQ(expected_url.get_pathname(), "b");
  auto url = turbo::parse_wpt_uri("..#", &urlbase);
  ASSERT_TRUE(url.ok());
  ASSERT_TRUE(url.has_opaque_path());
  ASSERT_EQ(url.get_href(), "a:b#");
  ASSERT_EQ(url.get_pathname(), "b");
  SUCCEED();
}

TEST(basic_tests, can_parse) {
  ASSERT_TRUE(turbo::parse_wpt_uri("https://www.yagiz.co").ok());
  auto base = turbo::parse_wpt_uri("https://yagiz.co");
  ASSERT_TRUE(base.ok());
  ASSERT_TRUE(turbo::parse_wpt_uri("/hello", &base).ok());

  auto invalid_base = turbo::parse_wpt_uri("!!!!!!!1");
  ASSERT_FALSE(invalid_base.ok());
  ASSERT_FALSE(turbo::parse_wpt_uri("!!!").ok());
  SUCCEED();
}

TEST(basic_tests, node_issue_48254) {
  auto base_url = turbo::parse_wpt_uri("localhost:80");
  ASSERT_TRUE(base_url.ok());
  ASSERT_EQ(base_url.get_hostname(), "");
  ASSERT_EQ(base_url.get_host(), "");
  ASSERT_EQ(base_url.get_pathname(), "80");
  ASSERT_EQ(base_url.get_href(), "localhost:80");
  ASSERT_EQ(base_url.get_origin(), "null");
  ASSERT_EQ(base_url.has_opaque_path(), true);
  auto url = turbo::parse_wpt_uri("", &base_url);
  ASSERT_FALSE(url.ok());
  SUCCEED();
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
  SUCCEED();
}

// https://github.com/nodejs/node/issues/49650
TEST(basic_tests, nodejs_49650) {
  auto out = turbo::parse_wpt_uri("http://foo");
  ASSERT_TRUE(out.ok());
  ASSERT_FALSE(out.set_host("::"));
  ASSERT_EQ(out.get_href(), "http://foo/");
  SUCCEED();
}

// https://github.com/nodejs/node/issues/50235
TEST(basic_tests, nodejs_50235) {
  auto out = turbo::parse_wpt_uri("http://test.com:5/?param=1");
  ASSERT_TRUE(out.ok());
  ASSERT_TRUE(out.set_pathname("path"));
  ASSERT_EQ(out.get_href(), "http://test.com:5/path?param=1");
  SUCCEED();
}

// https://github.com/nodejs/node/issues/51514
TEST(basic_tests, nodejs_51514) {
  auto out = turbo::parse_wpt_uri("http://1.1.1.256");
  ASSERT_FALSE(out.ok());
}

// https://github.com/nodejs/node/issues/51593
TEST(basic_tests, nodejs_51593) {
  auto out = turbo::parse_wpt_uri("http://\u200b123.123.123.123");
  ASSERT_TRUE(out.ok());
  ASSERT_EQ(out.get_href(), "http://123.123.123.123/");
  SUCCEED();
}

// https://github.com/nodejs/node/issues/51619
TEST(basic_tests, nodejs_51619) {
  auto out = turbo::parse_wpt_uri("https://0.0.0.0x100/");
  ASSERT_FALSE(out.ok());
  SUCCEED();
}

// https://github.com/nodejs/undici/pull/2971
TEST(basic_tests, nodejs_undici_2971) {
  std::string_view base =
      "https://non-ascii-location-header.sys.workers.dev/redirect";
  auto base_url = turbo::parse_wpt_uri(base);
  ASSERT_TRUE(base_url.ok());
  auto out = turbo::parse_wpt_uri("/\xec\x95\x88\xeb\x85\x95", &base_url);
  ASSERT_TRUE(out.ok());
  ASSERT_EQ(
      out.get_href(),
      R"(https://non-ascii-location-header.sys.workers.dev/%EC%95%88%EB%85%95)");
  SUCCEED();
}

TEST(basic_tests, path_setter_bug) {
  std::string_view base = "blob:/?";
  auto base_url = turbo::parse_wpt_uri(base);
  ASSERT_TRUE(base_url.ok());
  ASSERT_TRUE(base_url.set_pathname("//.."));
  ASSERT_TRUE(base_url.ok());
  SUCCEED();
}

TEST(basic_tests, negativeport) {
  auto url = turbo::parse_wpt_uri("https://www.google.com");
  ASSERT_FALSE(url.set_port("-1"));
  SUCCEED();
}