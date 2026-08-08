#include "catch.hpp"
#include <turbo/cli/flags.h>

XDECLARE_string(declared_flag);

TEST_CASE("DECLARE cross-file access", "[flags][declare]") {
    CHECK(XFLAGS_declared_flag == "declared_initial");

    XFLAGS_declared_flag = "modified_via_declare";
    CHECK(XFLAGS_declared_flag == "modified_via_declare");
}

TEST_CASE("DECLARE cross-file via registry", "[flags][declare]") {
    auto *flag = xcli::detail::FlagRegistry::global_registry().find("declared_flag");
    REQUIRE(flag != nullptr);
    CHECK(flag->name() == "declared_flag");
    CHECK(flag->type() == xcli::detail::FlagType::String);
    CHECK(flag->default_value() == "declared_initial");
}
