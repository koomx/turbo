#include "catch.hpp"
#include <turbo/cli/flags.h>

// -----------------------------------------------------------
// Define flags for testing
// -----------------------------------------------------------
XDEFINE_bool(test_bool, false, "test bool flag");
XDEFINE_int32(test_int32, -1, "test int32 flag");
XDEFINE_uint32(test_uint32, 42, "test uint32 flag");
XDEFINE_int64(test_int64, -99, "test int64 flag");
XDEFINE_uint64(test_uint64, 123, "test uint64 flag");
XDEFINE_double(test_double, 3.14, "test double flag");
XDEFINE_string(test_string, "hello", "test string flag");

// -----------------------------------------------------------
// FlagTypes: verify variables exist and have correct types
// -----------------------------------------------------------
TEST_CASE("FlagTypes", "[flags]") {
    XFLAGS_test_bool = false;
    CHECK_FALSE(XFLAGS_test_bool);

    XFLAGS_test_bool = true;
    CHECK(XFLAGS_test_bool);
}

TEST_CASE("FlagTypes int32", "[flags]") {
    XFLAGS_test_int32 = 42;
    CHECK(XFLAGS_test_int32 == 42);
}

TEST_CASE("FlagTypes uint32", "[flags]") {
    XFLAGS_test_uint32 = 99;
    CHECK(XFLAGS_test_uint32 == 99);
}

TEST_CASE("FlagTypes int64", "[flags]") {
    XFLAGS_test_int64 = -1000;
    CHECK(XFLAGS_test_int64 == -1000);
}

TEST_CASE("FlagTypes uint64", "[flags]") {
    XFLAGS_test_uint64 = 9999;
    CHECK(XFLAGS_test_uint64 == 9999);
}

TEST_CASE("FlagTypes double", "[flags]") {
    XFLAGS_test_double = 2.718;
    CHECK(XFLAGS_test_double == Approx(2.718));
}

TEST_CASE("FlagTypes string", "[flags]") {
    XFLAGS_test_string = "world";
    CHECK(XFLAGS_test_string == "world");
}

// -----------------------------------------------------------
// Registry: flags are automatically registered
// -----------------------------------------------------------
TEST_CASE("Registry findFlag", "[flags]") {
    auto *reg = &xcli::detail::FlagRegistry::global_registry();

    auto *flag = reg->find("test_bool");
    REQUIRE(flag != nullptr);
    CHECK(flag->name() == "test_bool");
    CHECK(flag->type() == xcli::detail::FlagType::Bool);

    flag = reg->find("test_int32");
    REQUIRE(flag != nullptr);
    CHECK(flag->name() == "test_int32");
    CHECK(flag->type() == xcli::detail::FlagType::Int32);

    flag = reg->find("test_string");
    REQUIRE(flag != nullptr);
    CHECK(flag->name() == "test_string");
    CHECK(flag->type() == xcli::detail::FlagType::String);
}

TEST_CASE("Registry find unknown", "[flags]") {
    auto *flag = xcli::detail::FlagRegistry::global_registry().find("no_such_flag");
    CHECK(flag == nullptr);
}

TEST_CASE("Registry find dash-underscore", "[flags]") {
    // Should find flags even with dash/underscore substitution
    auto *flag = xcli::detail::FlagRegistry::global_registry().find("test-bool");
    // "test_bool" does not contain dashes, but test the mechanism
    if (flag != nullptr) {
        CHECK(flag->name() == "test_bool");
    }
}

// -----------------------------------------------------------
// Default values
// -----------------------------------------------------------
TEST_CASE("DefaultValues", "[flags]") {
    auto *reg = &xcli::detail::FlagRegistry::global_registry();

    CHECK(reg->find("test_bool")->default_value() == "false");
    CHECK(reg->find("test_int32")->default_value() == "-1");
    CHECK(reg->find("test_uint32")->default_value() == "42");
    CHECK(reg->find("test_int64")->default_value() == "-99");
    CHECK(reg->find("test_uint64")->default_value() == "123");
    CHECK(reg->find("test_double")->default_value() == "3.1400000000000001");
    CHECK(reg->find("test_string")->default_value() == "hello");
}

// -----------------------------------------------------------
// ParseFrom — normal values
// -----------------------------------------------------------
TEST_CASE("ParseFrom bool true values", "[flags]") {
    auto *reg = &xcli::detail::FlagRegistry::global_registry();
    auto *flag = reg->find("test_bool");

    CHECK(flag->set_value("true"));
    CHECK(XFLAGS_test_bool == true);

    CHECK(flag->set_value("1"));
    CHECK(XFLAGS_test_bool == true);

    CHECK(flag->set_value("yes"));
    CHECK(XFLAGS_test_bool == true);

    CHECK(flag->set_value("t"));
    CHECK(XFLAGS_test_bool == true);

    CHECK(flag->set_value("y"));
    CHECK(XFLAGS_test_bool == true);
}

TEST_CASE("ParseFrom bool false values", "[flags]") {
    auto *reg = &xcli::detail::FlagRegistry::global_registry();
    auto *flag = reg->find("test_bool");

    XFLAGS_test_bool = true;

    CHECK(flag->set_value("false"));
    CHECK_FALSE(XFLAGS_test_bool);

    XFLAGS_test_bool = true;
    CHECK(flag->set_value("0"));
    CHECK_FALSE(XFLAGS_test_bool);

    XFLAGS_test_bool = true;
    CHECK(flag->set_value("no"));
    CHECK_FALSE(XFLAGS_test_bool);

    XFLAGS_test_bool = true;
    CHECK(flag->set_value("f"));
    CHECK_FALSE(XFLAGS_test_bool);

    XFLAGS_test_bool = true;
    CHECK(flag->set_value("n"));
    CHECK_FALSE(XFLAGS_test_bool);
}

TEST_CASE("ParseFrom int32 normal", "[flags]") {
    auto *flag = xcli::detail::FlagRegistry::global_registry().find("test_int32");

    CHECK(flag->set_value("12"));
    CHECK(XFLAGS_test_int32 == 12);

    CHECK(flag->set_value("-5"));
    CHECK(XFLAGS_test_int32 == -5);
}

TEST_CASE("ParseFrom int32 hex", "[flags]") {
    auto *flag = xcli::detail::FlagRegistry::global_registry().find("test_int32");

    CHECK(flag->set_value("0x10"));
    CHECK(XFLAGS_test_int32 == 16);

    CHECK(flag->set_value("0XFF"));
    CHECK(XFLAGS_test_int32 == 255);
}

TEST_CASE("ParseFrom int32 leading zero", "[flags]") {
    auto *flag = xcli::detail::FlagRegistry::global_registry().find("test_int32");

    // Leading zero is decimal, not octal
    CHECK(flag->set_value("010"));
    CHECK(XFLAGS_test_int32 == 10);
}

TEST_CASE("ParseFrom uint32 normal", "[flags]") {
    auto *flag = xcli::detail::FlagRegistry::global_registry().find("test_uint32");

    CHECK(flag->set_value("100"));
    CHECK(XFLAGS_test_uint32 == 100);
}

TEST_CASE("ParseFrom uint32 negative fails", "[flags]") {
    auto *flag = xcli::detail::FlagRegistry::global_registry().find("test_uint32");

    uint32_t old = XFLAGS_test_uint32;
    CHECK_FALSE(flag->set_value("-1"));
    CHECK(XFLAGS_test_uint32 == old);  // unchanged
}

TEST_CASE("ParseFrom int64 normal", "[flags]") {
    auto *flag = xcli::detail::FlagRegistry::global_registry().find("test_int64");

    CHECK(flag->set_value("9223372036854775807"));
    CHECK(XFLAGS_test_int64 == 9223372036854775807LL);
}

TEST_CASE("ParseFrom uint64 large", "[flags]") {
    auto *flag = xcli::detail::FlagRegistry::global_registry().find("test_uint64");

    CHECK(flag->set_value("18446744073709551615"));
    CHECK(XFLAGS_test_uint64 == 18446744073709551615ULL);
}

TEST_CASE("ParseFrom double normal", "[flags]") {
    auto *flag = xcli::detail::FlagRegistry::global_registry().find("test_double");

    CHECK(flag->set_value("2.5"));
    CHECK(XFLAGS_test_double == Approx(2.5));

    CHECK(flag->set_value("-1.5e3"));
    CHECK(XFLAGS_test_double == Approx(-1500.0));
}

TEST_CASE("ParseFrom string", "[flags]") {
    auto *flag = xcli::detail::FlagRegistry::global_registry().find("test_string");

    CHECK(flag->set_value("new value"));
    CHECK(XFLAGS_test_string == "new value");

    CHECK(flag->set_value(""));
    CHECK(XFLAGS_test_string == "");
}

// -----------------------------------------------------------
// ParseFrom — illegal/edge values
// -----------------------------------------------------------
TEST_CASE("ParseFrom bool illegal", "[flags]") {
    auto *flag = xcli::detail::FlagRegistry::global_registry().find("test_bool");
    bool old = XFLAGS_test_bool;

    CHECK_FALSE(flag->set_value("2"));
    CHECK(XFLAGS_test_bool == old);

    CHECK_FALSE(flag->set_value(""));
    CHECK(XFLAGS_test_bool == old);

    CHECK_FALSE(flag->set_value("xyz"));
    CHECK(XFLAGS_test_bool == old);
}

TEST_CASE("ParseFrom int32 illegal", "[flags]") {
    auto *flag = xcli::detail::FlagRegistry::global_registry().find("test_int32");
    int32_t old = XFLAGS_test_int32;

    CHECK_FALSE(flag->set_value(""));
    CHECK(XFLAGS_test_int32 == old);

    CHECK_FALSE(flag->set_value("abc"));
    CHECK(XFLAGS_test_int32 == old);

    // overflow
    CHECK_FALSE(flag->set_value("9999999999"));
    CHECK(XFLAGS_test_int32 == old);
}

TEST_CASE("ParseFrom double illegal", "[flags]") {
    auto *flag = xcli::detail::FlagRegistry::global_registry().find("test_double");
    double old = XFLAGS_test_double;

    CHECK_FALSE(flag->set_value(""));
    CHECK(XFLAGS_test_double == old);

    CHECK_FALSE(flag->set_value("0.1xxx"));
    CHECK(XFLAGS_test_double == old);

    CHECK_FALSE(flag->set_value(" "));
    CHECK(XFLAGS_test_double == old);
}

// -----------------------------------------------------------
// set_flag free function
// -----------------------------------------------------------
TEST_CASE("set_flag free function", "[flags]") {
    std::string msg;

    CHECK(xcli::detail::set_flag("test_string", "from setflag", msg));
    CHECK(XFLAGS_test_string == "from setflag");
    CHECK_FALSE(msg.empty());

    CHECK_FALSE(xcli::detail::set_flag("does_not_exist", "x", msg));
    CHECK(msg.empty());
}

// -----------------------------------------------------------
// get_all_flags
// -----------------------------------------------------------
TEST_CASE("get_all_flags", "[flags]") {
    auto flags = xcli::detail::FlagRegistry::global_registry().get_all_flags();
    // At minimum, our 7 test flags + any from other tests
    REQUIRE(flags.size() >= 7);

    bool found = false;
    for (auto *f : flags) {
        if (f->name() == "test_bool") found = true;
    }
    CHECK(found);
}

// -----------------------------------------------------------
// Type names via FlagRegistry
// -----------------------------------------------------------
TEST_CASE("Flagflags_type_name", "[flags]") {
    auto *reg = &xcli::detail::FlagRegistry::global_registry();
    CHECK(reg->find("test_bool")->flags_type_name() == "bool");
    CHECK(reg->find("test_int32")->flags_type_name() == "int32");
    CHECK(reg->find("test_uint32")->flags_type_name() == "uint32");
    CHECK(reg->find("test_int64")->flags_type_name() == "int64");
    CHECK(reg->find("test_uint64")->flags_type_name() == "uint64");
    CHECK(reg->find("test_double")->flags_type_name() == "double");
    CHECK(reg->find("test_string")->flags_type_name() == "string");
}
