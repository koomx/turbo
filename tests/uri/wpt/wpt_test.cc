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

#include <cstring>
#include <filesystem>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>

#include <gtest/gtest.h>
#include <simdjson.h>
#include <turbo/uri/character_sets.h>
#include <turbo/uri/uri_view.h>
#include <turbo/uri/utility.h>
#include <turbo/uri/wpt/parser.h>

using namespace simdjson;

#ifndef WPT_DATA_DIR
#define WPT_DATA_DIR "wpt/"
#endif
const char *PERCENT_ENCODING_JSON = WPT_DATA_DIR "percent-encoding.json";
const char *TOASCII_JSON = WPT_DATA_DIR "toascii.json";
const char *IDNA_TEST_V2 = WPT_DATA_DIR "IdnaTestV2.json";
const char *URLTESTDATA_JSON = WPT_DATA_DIR "urltestdata.json";
const char *ADA_URLTESTDATA_JSON = WPT_DATA_DIR "ada_extra_urltestdata.json";

namespace {

turbo::UriView wpt_parse(std::string_view view,
    const turbo::UriView *base = nullptr) {
    std::unique_ptr<char[]> buffer(new char[view.size()]);
    memcpy(buffer.get(), view.data(), view.size());
    const std::string_view copy(buffer.get(), view.size());
    if (base != nullptr) {
        return turbo::parse_wpt_uri(copy, *base);
    }
    return turbo::parse_wpt_uri(copy);
}

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
    // urltestdata: empty query → search "" (href still may include '?').
    if (!u.has_query() || u.query().empty()) {
        return {};
    }
    return "?" + std::string(u.query());
}

std::string hash(const turbo::UriView &u) {
    // urltestdata: empty fragment → hash "" (href still may end with '#').
    if (!u.has_fragment() || u.fragment().empty()) {
        return {};
    }
    return "#" + std::string(u.fragment());
}

}  // namespace

std::stringstream error_buffer;

bool file_exists(const char *filename) {
    namespace fs = std::filesystem;
    std::filesystem::path f{filename};
    if (std::filesystem::exists(filename)) {
        std::cout << "  file found: " << filename << std::endl;
        return true;
    }
    std::cerr << "  file missing: " << filename << std::endl;
    error_buffer << "  file missing: " << filename << std::endl;
    return false;
}

TEST(wpt_tests, idna_test_v2_to_ascii) {
    ondemand::parser parser;
    ASSERT_TRUE(file_exists(IDNA_TEST_V2));
    padded_string json = padded_string::load(IDNA_TEST_V2);
    ondemand::document doc = parser.iterate(json);
    try {
        for (auto element : doc.get_array()) {
            if (element.type() == ondemand::json_type::string) {
                continue;
            }

            ondemand::object object = element.get_object();
            std::string_view input = object["input"].get_string();

            std::optional<std::string> output;
            turbo::to_ascii(output, input, input.find('%'));
            auto expected_output = object["output"];
            auto given_output = output.has_value() ? output.value() : "";

            if (expected_output.is_null()) {
                ASSERT_EQ(given_output, "");
            } else if (expected_output.type() == ondemand::json_type::string) {
                std::string_view str_expected_output = expected_output.get_string();
                ASSERT_EQ(str_expected_output, given_output);
            }
        }
    } catch (simdjson::simdjson_error &error) {
        std::cerr << "JSON error: " << error.what() << " near "
                  << doc.current_location() << " in " << TOASCII_JSON << std::endl;
        FAIL();
    }
}

TEST(wpt_tests, percent_encoding) {
    ondemand::parser parser;
    size_t counter{0};

    ASSERT_TRUE(file_exists(PERCENT_ENCODING_JSON));
    padded_string json = padded_string::load(PERCENT_ENCODING_JSON);
    ondemand::document doc = parser.iterate(json);
    try {
        for (auto element : doc.get_array()) {
            if (element.type() == ondemand::json_type::string) {
                std::cout << "   comment: " << element.get_string() << std::endl;
            } else if (element.type() == ondemand::json_type::object) {
                ondemand::object object = element.get_object();
                object.reset();

                auto input_element = object["input"];
                std::string_view input;
                bool allow_replacement_characters = true;
                EXPECT_FALSE(
                    input_element.get_string(allow_replacement_characters).get(input));
                std::string my_input_encoded = turbo::percent_encode(
                    input, turbo::uri_charsets::QUERY_PERCENT_ENCODE);
                ondemand::object outputs = object["output"].get_object();
                std::string_view expected_view;
                ASSERT_FALSE(outputs["utf-8"].get(expected_view));
                ASSERT_EQ(my_input_encoded, expected_view);
                counter++;
            }
        }
    } catch (simdjson::simdjson_error &error) {
        std::cerr << "JSON error: " << error.what() << " near "
                  << doc.current_location() << " in " << TOASCII_JSON << std::endl;
        FAIL();
    }
    std::cout << "Tests executed = " << counter << std::endl;
}

// UriView has no setters; old WptUri setter suite not applicable.
TEST(wpt_tests, DISABLED_setters_tests_encoding) {
    GTEST_SKIP() << "setters moved off UriView; reintroduce via Uri builder later";
}

TEST(wpt_tests, toascii_encoding) {
    ondemand::parser parser;
    ASSERT_TRUE(file_exists(TOASCII_JSON));
    padded_string json = padded_string::load(TOASCII_JSON);
    ondemand::document doc = parser.iterate(json);
    try {
        for (auto element : doc.get_array()) {
            if (element.type() == ondemand::json_type::string) {
                std::cout << "   comment: " << element.get_string() << std::endl;
            } else if (element.type() == ondemand::json_type::object) {
                ondemand::object object = element.get_object();

                // ondemand: copy fields before further access invalidates views.
                std::string input(object["input"].get_string().value());
                std::string expected_str;
                bool expect_null = false;
                {
                    auto expected_output = object["output"];
                    if (expected_output.type() == ondemand::json_type::string) {
                        expected_str = std::string(expected_output.get_string().value());
                    } else if (expected_output.is_null()) {
                        expect_null = true;
                    }
                }

                auto current =
                    turbo::parse_wpt_uri("https://" + input + "/x");

                if (expect_null) {
                    ASSERT_FALSE(current.ok()) << "input='" << input << "'";
                } else {
                    ASSERT_EQ(host_with_port(current), expected_str)
                        << "input='" << input << "'";
                    ASSERT_EQ(current.host(), expected_str);
                    ASSERT_EQ(current.path(), "/x");
                    ASSERT_EQ(href(current), "https://" + expected_str + "/x");
                }
            }
        }
    } catch (simdjson::simdjson_error &error) {
        std::cerr << "JSON error: " << error.what() << " near "
                  << doc.current_location() << " in " << TOASCII_JSON << std::endl;
        FAIL();
    }
}

TEST(wpt_tests, urltestdata_encoding) {
    for (auto source : {URLTESTDATA_JSON, ADA_URLTESTDATA_JSON}) {
        ondemand::parser parser;
        size_t counter{};
        ASSERT_TRUE(file_exists(source));
        padded_string json = padded_string::load(source);
        ondemand::document doc = parser.iterate(json);
        try {
            for (auto element : doc.get_array()) {
                if (element.type() == ondemand::json_type::string) {
                    std::string_view comment = element.get_string().value();
                    std::cout << comment << std::endl;
                } else if (element.type() == ondemand::json_type::object) {
                    ondemand::object object = element.get_object();
                    object.reset();

                    std::string_view input{};
                    bool allow_replacement_characters = true;
                    ASSERT_FALSE(object["input"]
                                     .get_string(allow_replacement_characters)
                                     .get(input));
                    std::cout << "input='" << input << "' [" << input.size()
                              << " bytes]" << std::endl;
                    std::string_view base;
                    turbo::UriView base_url;
                    if (!object["base"].get(base)) {
                        std::cout << "base=" << base << std::endl;
                        base_url = wpt_parse(base);
                        if (!base_url.ok()) {
                            bool failure = false;
                            if (!object["failure"].get(failure) && failure == true) {
                                continue;
                            }
                            ASSERT_TRUE(base_url.ok());
                        }
                    }
                    bool failure = false;
                    auto input_url = (!object["base"].get(base))
                                         ? wpt_parse(input, &base_url)
                                         : wpt_parse(input);
                    if (!object["failure"].get(failure) && failure == true) {
                        ASSERT_EQ(input_url.ok(), !failure);
                    } else {
                        ASSERT_TRUE(input_url.ok());

                        std::string_view expected_protocol =
                            object["protocol"].get_string();
                        ASSERT_EQ(protocol(input_url), expected_protocol);

                        std::string_view username = object["username"].get_string();
                        ASSERT_EQ(input_url.username(), username);

                        std::string_view password = object["password"].get_string();
                        ASSERT_EQ(input_url.password(), password);

                        std::string_view host = object["host"].get_string();
                        ASSERT_EQ(host_with_port(input_url), host);

                        std::string_view hostname = object["hostname"].get_string();
                        ASSERT_EQ(input_url.host(), hostname);

                        std::string_view port = object["port"].get_string();
                        ASSERT_EQ(input_url.port(), port);

                        std::string_view pathname = object["pathname"].get_string();
                        ASSERT_EQ(input_url.path(), pathname);

                        std::string_view expected_search =
                            object["search"].get_string();
                        ASSERT_EQ(search(input_url), expected_search);

                        std::string_view expected_hash = object["hash"].get_string();
                        ASSERT_EQ(hash(input_url), expected_hash);

                        std::string_view expected_href = object["href"].get_string();
                        ASSERT_EQ(href(input_url), expected_href);

                        counter++;
                    }
                }
            }
        } catch (simdjson::simdjson_error &error) {
            std::cerr << "JSON error: " << error.what() << " near "
                      << doc.current_location() << " in " << source << std::endl;
            FAIL();
        }
        std::cout << "Tests executed = " << counter << std::endl;
    }
}

// UriView has no has_valid_domain(); old WptUri-only check.
TEST(wpt_tests, DISABLED_verify_dns_length) {
    GTEST_SKIP() << "has_valid_domain not on UriView";
}
