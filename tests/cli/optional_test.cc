// Copyright (c) 2017-2026, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <complex>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "app_helper.hpp"
#include <optional>

TEST_CASE("OptionalNoEmpty") { CHECK(1 == 1); }

#ifdef _MSC_VER
// this warning suppresses double to int conversions that are inherent in the test
// on windows.  This may be able to removed in the future as the add_option capability
// improves
#pragma warning(disable : 4244)
#endif

TEST_CASE_METHOD(TApp, "StdOptionalTest", "[optional]") {
    std::optional<int> opt;
    app.add_option("-c,--count", opt);
    run();
    CHECK(!opt);

    args = {"-c", "1"};
    run();
    CHECK((opt && (1 == *opt)));

    args = {"--count", "3"};
    run();
    CHECK((opt && (3 == *opt)));
}

TEST_CASE_METHOD(TApp, "StdOptionalNulloptDefaultValConsistency", "[optional]") {
    std::optional<int> opt_int;
    app.add_option("-a", opt_int)->default_val(std::nullopt);

    std::optional<std::string> opt_string;

    CHECK_FALSE(opt_int.has_value());
    CHECK_FALSE(opt_string.has_value());

    auto *optargs = app.add_option("-b", opt_string);
    optargs->default_val(std::nullopt);

    run();

    CHECK_FALSE(opt_int.has_value());
    CHECK_FALSE(opt_string.has_value());
}

TEST_CASE_METHOD(TApp, "StdOptionalEmptyOptConsistency", "[optional]") {
    std::optional<int> opt_int;
    app.add_option("-a", opt_int)->default_val(opt_int);

    std::optional<std::string> opt_string;

    CHECK_FALSE(opt_int.has_value());
    CHECK_FALSE(opt_string.has_value());

    auto *optargs = app.add_option("-b", opt_string);
    optargs->default_val(opt_string);

    run();

    CHECK_FALSE(opt_int.has_value());
    CHECK_FALSE(opt_string.has_value());
}

TEST_CASE_METHOD(TApp, "StdOptionalVectorEmptyDirect", "[optional]") {
    std::optional<std::vector<int>> opt;
    app.add_option("-v,--vec", opt)->expected(0, 3)->allow_extra_args();
    // app.add_option("-v,--vec", opt)->expected(0, 3)->allow_extra_args();
    run();
    CHECK(!opt);
    args = {"-v"};
    opt = std::vector<int>{4, 3};
    run();
    CHECK(!opt);
    args = {"-v", "1", "4", "5"};
    run();
    REQUIRE(opt);
    std::vector<int> expV{1, 4, 5};
    CHECK(expV == *opt);
}

TEST_CASE_METHOD(TApp, "StdOptionalComplexDirect", "[optional]") {
    std::optional<std::complex<double>> opt;
    app.add_option("-c,--complex", opt)->type_size(0, 2);
    run();
    CHECK(!opt);
    args = {"-c"};
    opt = std::complex<double>{4.0, 3.0};
    run();
    CHECK(!opt);
    args = {"-c", "1+2j"};
    run();
    CHECK(opt);
    std::complex<double> val{1, 2};
    CHECK(val == *opt);
    args = {"-c", "3", "-4"};
    run();
    CHECK(opt);
    std::complex<double> val2{3, -4};
    CHECK(val2 == *opt);
}

TEST_CASE_METHOD(TApp, "StdOptionalUint", "[optional]") {
    std::optional<std::uint64_t> opt;
    app.add_option("-i,--int", opt);
    run();
    CHECK(!opt);

    args = {"-i", "15"};
    run();
    CHECK((opt && (15U == *opt)));
    static_assert(xcli::detail::classify_object<std::optional<std::uint64_t>>::value ==
                  xcli::detail::object_category::wrapper_value);
}

TEST_CASE_METHOD(TApp, "StdOptionalbool", "[optional]") {
    std::optional<bool> opt{};
    CHECK(!opt);
    app.add_flag("--opt,!--no-opt", opt);
    CHECK(!opt);
    run();
    CHECK(!opt);

    args = {"--opt"};
    run();
    CHECK((opt && *opt));

    args = {"--no-opt"};
    run();
    REQUIRE(opt);
    if(opt) {
        CHECK_FALSE(*opt);
    }
    static_assert(xcli::detail::classify_object<std::optional<bool>>::value ==
                  xcli::detail::object_category::wrapper_value);
}

#ifdef _MSC_VER
#pragma warning(default : 4244)
#endif

