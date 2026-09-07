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

#include <optional>
#include <string_view>

#include <gtest/gtest.h>
#include <turbo/uri/ip.h>

namespace {

std::string_view sv(const char* s) { return s; }

}  // namespace

TEST(IpTryParse, WptIpv4Basic) {
    std::optional<turbo::IpAddr> out;
    auto r = turbo::try_parse_wpt_ip_v4(sv("192.168.1.1"), out);
    ASSERT_TRUE(r.ok());
    EXPECT_EQ(r.error_pos, 11u);
    ASSERT_TRUE(out.has_value());
    EXPECT_EQ(out->type, turbo::IpType::IP_V4);
    EXPECT_EQ(out->get_ipv4(), 0xc0a80101ull);
    EXPECT_EQ(out->normalized, "192.168.1.1");
}

TEST(IpTryParse, WptIpv4ConsumesPrefix) {
    std::optional<turbo::IpAddr> out;
    auto r = turbo::try_parse_wpt_ip_v4(sv("192.168.1.1x"), out);
    ASSERT_TRUE(r.ok());
    EXPECT_EQ(r.error_pos, 11u);
    ASSERT_TRUE(out.has_value());
    EXPECT_EQ(out->get_ipv4(), 0xc0a80101ull);
}

TEST(IpTryParse, WptIpv4HexAndOctalCutoff) {
    std::optional<turbo::IpAddr> out;
    auto r = turbo::try_parse_wpt_ip_v4(sv("0x7f.0.0.1/path"), out);
    ASSERT_TRUE(r.ok());
    EXPECT_EQ(r.error_pos, 10u);
    ASSERT_TRUE(out.has_value());
    EXPECT_EQ(out->get_ipv4(), 0x7f000001ull);
    EXPECT_EQ(out->normalized, "127.0.0.1");

    out.reset();
    r = turbo::try_parse_wpt_ip_v4(sv("0177.0.0.1:443"), out);
    ASSERT_TRUE(r.ok());
    EXPECT_EQ(r.error_pos, 10u);
    ASSERT_TRUE(out.has_value());
    EXPECT_EQ(out->get_ipv4(), 0x7f000001ull);
}

TEST(IpTryParse, WptIpv4Illegal) {
    std::optional<turbo::IpAddr> out;
    EXPECT_FALSE(turbo::try_parse_wpt_ip_v4(sv("256.1.1.1"), out).ok());
    out.reset();
    EXPECT_FALSE(turbo::try_parse_wpt_ip_v4(sv("1.2.3"), out).ok());
}

TEST(IpTryParse, WptIpv4CutoffMore) {
    std::optional<turbo::IpAddr> out;
    auto r = turbo::try_parse_wpt_ip_v4(sv("127.0.0.1]"), out);
    ASSERT_TRUE(r.ok());
    EXPECT_EQ(r.error_pos, 9u);

    out.reset();
    r = turbo::try_parse_wpt_ip_v4(sv("8.8.8.8?q=1"), out);
    ASSERT_TRUE(r.ok());
    EXPECT_EQ(r.error_pos, 7u);
    ASSERT_TRUE(out.has_value());
    EXPECT_EQ(out->get_ipv4(), 0x08080808ull);
}

TEST(IpTryParse, RfcIpv4Basic) {
    std::optional<turbo::IpAddr> out;
    auto r = turbo::try_parse_rfc_ip_v4(sv("192.168.1.1"), out);
    ASSERT_TRUE(r.ok());
    EXPECT_EQ(r.error_pos, 11u);
    ASSERT_TRUE(out.has_value());
    EXPECT_EQ(out->type, turbo::IpType::IP_V4);
    EXPECT_EQ(out->get_ipv4(), 0xc0a80101ull);
    EXPECT_EQ(out->normalized, "192.168.1.1");
}

TEST(IpTryParse, RfcIpv4NotIp) {
    std::optional<turbo::IpAddr> out;
    auto r = turbo::try_parse_rfc_ip_v4(sv("example.com"), out);
    ASSERT_TRUE(r.ok());
    EXPECT_EQ(r.error_pos, 0u);
    EXPECT_FALSE(out.has_value());

    out.reset();
    r = turbo::try_parse_rfc_ip_v4(sv(""), out);
    ASSERT_TRUE(r.ok());
    EXPECT_EQ(r.error_pos, 0u);
}

TEST(IpTryParse, RfcIpv4LeadingZeroIllegal) {
    std::optional<turbo::IpAddr> out;
    EXPECT_FALSE(turbo::try_parse_rfc_ip_v4(sv("01.2.3.4"), out).ok());
    out.reset();
    EXPECT_FALSE(turbo::try_parse_rfc_ip_v4(sv("1.02.3.4"), out).ok());
}

TEST(IpTryParse, RfcIpv4Overflow) {
    std::optional<turbo::IpAddr> out;
    EXPECT_FALSE(turbo::try_parse_rfc_ip_v4(sv("256.1.1.1"), out).ok());
}

TEST(IpTryParse, RfcIpv4ConsumesPrefix) {
    std::optional<turbo::IpAddr> out;
    auto r = turbo::try_parse_rfc_ip_v4(sv("10.0.0.1:8080"), out);
    ASSERT_TRUE(r.ok());
    EXPECT_EQ(r.error_pos, 8u);
    ASSERT_TRUE(out.has_value());
    EXPECT_EQ(out->get_ipv4(), 0x0a000001ull);

    out.reset();
    r = turbo::try_parse_rfc_ip_v4(sv("192.168.0.1/24"), out);
    ASSERT_TRUE(r.ok());
    EXPECT_EQ(r.error_pos, 11u);
    ASSERT_TRUE(out.has_value());
    EXPECT_EQ(out->get_ipv4(), 0xc0a80001ull);

    out.reset();
    r = turbo::try_parse_rfc_ip_v4(sv("1.2.3.4#frag"), out);
    ASSERT_TRUE(r.ok());
    EXPECT_EQ(r.error_pos, 7u);
}

TEST(IpTryParse, WptIpv6Basic) {
    std::optional<turbo::IpAddr> out;
    auto r = turbo::try_parse_wpt_ip_v6(sv("[::1]"), out);
    ASSERT_TRUE(r.ok());
    EXPECT_EQ(r.error_pos, 5u);
    ASSERT_TRUE(out.has_value());
    EXPECT_EQ(out->type, turbo::IpType::IP_V6);
    EXPECT_EQ(out->data[7], 1u);
    EXPECT_EQ(out->normalized, "[::1]");
}

TEST(IpTryParse, WptIpv6NotIp) {
    std::optional<turbo::IpAddr> out;
    auto r = turbo::try_parse_wpt_ip_v6(sv("::1"), out);
    ASSERT_TRUE(r.ok());
    EXPECT_EQ(r.error_pos, 0u);
}

TEST(IpTryParse, WptIpv6Illegal) {
    std::optional<turbo::IpAddr> out;
    EXPECT_FALSE(turbo::try_parse_wpt_ip_v6(sv("[::1"), out).ok());
    out.reset();
    EXPECT_FALSE(turbo::try_parse_wpt_ip_v6(sv("[]"), out).ok());
}

TEST(IpTryParse, WptIpv6Cutoff) {
    std::optional<turbo::IpAddr> out;
    auto r = turbo::try_parse_wpt_ip_v6(sv("[::1]:80"), out);
    ASSERT_TRUE(r.ok());
    EXPECT_EQ(r.error_pos, 5u);
    ASSERT_TRUE(out.has_value());
    EXPECT_EQ(out->type, turbo::IpType::IP_V6);

    out.reset();
    r = turbo::try_parse_wpt_ip_v6(sv("[::1]/"), out);
    ASSERT_TRUE(r.ok());
    EXPECT_EQ(r.error_pos, 5u);

    out.reset();
    r = turbo::try_parse_wpt_ip_v6(sv("[2001:db8::1]/path"), out);
    ASSERT_TRUE(r.ok());
    EXPECT_EQ(r.error_pos, 13u);
    ASSERT_TRUE(out.has_value());
    EXPECT_EQ(out->type, turbo::IpType::IP_V6);
}

TEST(IpTryParse, RfcIpv6Cutoff) {
    std::optional<turbo::IpAddr> out;
    auto r = turbo::try_parse_rfc_ip_v6(sv("[2001:db8::1]:443"), out);
    ASSERT_TRUE(r.ok());
    EXPECT_EQ(r.error_pos, 13u);
    ASSERT_TRUE(out.has_value());
    EXPECT_EQ(out->type, turbo::IpType::IP_V6);
}

TEST(IpTryParse, DispatchCutoff) {
    std::optional<turbo::IpAddr> out;
    auto r = turbo::try_parse_wpt_ip(sv("[::1]/"), out);
    ASSERT_TRUE(r.ok());
    EXPECT_EQ(r.error_pos, 5u);
    EXPECT_EQ(out->type, turbo::IpType::IP_V6);

    out.reset();
    r = turbo::try_parse_wpt_ip(sv("127.0.0.1/x"), out);
    ASSERT_TRUE(r.ok());
    EXPECT_EQ(r.error_pos, 9u);
    EXPECT_EQ(out->type, turbo::IpType::IP_V4);

    out.reset();
    r = turbo::try_parse_rfc_ip(sv("[::1]:80"), out);
    ASSERT_TRUE(r.ok());
    EXPECT_EQ(r.error_pos, 5u);
    EXPECT_EQ(out->type, turbo::IpType::IP_V6);

    out.reset();
    r = turbo::try_parse_rfc_ip(sv("127.0.0.1:80"), out);
    ASSERT_TRUE(r.ok());
    EXPECT_EQ(r.error_pos, 9u);
    EXPECT_EQ(out->type, turbo::IpType::IP_V4);

    out.reset();
    r = turbo::try_parse_rfc_ip(sv("example.com"), out);
    ASSERT_TRUE(r.ok());
    EXPECT_EQ(r.error_pos, 0u);
}
