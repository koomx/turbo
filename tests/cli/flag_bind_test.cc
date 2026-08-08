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

#include "app_helper.hpp"

#include <turbo/flags/argv.h>
#include <turbo/flags/flag.h>
#include <turbo/flags/reflection.h>

TURBO_FLAG(int, cli_bind_port, 8080, "listening port");
TURBO_FLAG(bool, cli_bind_verbose, false, "enable verbose");
TURBO_FLAG(std::string, cli_bind_name, "world", "display name");
TURBO_FLAG(int, cli_bind_limited, 10, "limited")
    .on_validate([](const int &v, std::string *e) noexcept {
        if(v > 100) {
            if(e) {
                *e = "too big";
            }
            return false;
        }
        return true;
    });

TEST_CASE("TurboFlag: OptionBind", "[flag_bind]") {
    TApp tapp;
    tapp.app.add_option("--port,-p", FLAGS_cli_bind_port);
    tapp.app.add_option("--name,-n", FLAGS_cli_bind_name);

    REQUIRE(turbo::SetFlag(&FLAGS_cli_bind_port, 8080));
    REQUIRE(turbo::SetFlag(&FLAGS_cli_bind_name, std::string("world")));

    tapp.args = {"--port", "9090", "-n", "turbo"};
    tapp.run();

    CHECK(turbo::GetFlag(FLAGS_cli_bind_port) == 9090);
    CHECK(turbo::GetFlag(FLAGS_cli_bind_name) == "turbo");
}

TEST_CASE("TurboFlag: FlagBind", "[flag_bind]") {
    TApp tapp;
    tapp.app.add_flag("--verbose,-v", FLAGS_cli_bind_verbose);

    REQUIRE(turbo::SetFlag(&FLAGS_cli_bind_verbose, false));

    tapp.args = {"-v"};
    tapp.run();

    CHECK(turbo::GetFlag(FLAGS_cli_bind_verbose));
}

TEST_CASE("TurboFlag: DescriptionFromFlag", "[flag_bind]") {
    TApp tapp;
    auto *opt = tapp.app.add_option("--port", FLAGS_cli_bind_port);
    CHECK(opt->get_description() == "listening port");
    CHECK(opt->get_default_str() == "8080");
}

TEST_CASE("TurboFlag: DescOverride", "[flag_bind]") {
    TApp tapp;
    auto *opt = tapp.app.add_option("--port", FLAGS_cli_bind_port, "custom help");
    CHECK(opt->get_description() == "custom help");
}

TEST_CASE("TurboFlag: ValidateRejects", "[flag_bind]") {
    REQUIRE(turbo::SetFlag(&FLAGS_cli_bind_limited, 10));

    xcli::App app;
    app.add_option("--limited", FLAGS_cli_bind_limited);
    input_t args = {"--limited", "50"};
    std::reverse(args.begin(), args.end());
    app.parse(args);
    CHECK(turbo::GetFlag(FLAGS_cli_bind_limited) == 50);

    xcli::App app2;
    app2.add_option("--limited", FLAGS_cli_bind_limited);
    input_t bad = {"--limited", "101"};
    std::reverse(bad.begin(), bad.end());
    CHECK_THROWS(app2.parse(bad));
    CHECK(turbo::GetFlag(FLAGS_cli_bind_limited) == 50);
}

TEST_CASE("TurboFlag: ParseSetsArgv", "[flag_bind]") {
    turbo::FlagSaver fs;
    xcli::App app{"tool"};
    const char *argv[] = {"/tmp/mytool", "--port", "1"};
    app.add_option("--port", FLAGS_cli_bind_port);
    app.parse(3, argv);

    const auto got = turbo::GetFlag(FLAGS_argv);
    REQUIRE(got.size() == 3);
    CHECK(got[0] == "/tmp/mytool");
    CHECK(got[1] == "--port");
    CHECK(got[2] == "1");
    CHECK(turbo::flags_internal::ShortProgramInvocationName() == "mytool");
}
