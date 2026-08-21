// Copyright (c) 2017-2026, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <turbo/cli/cli.h>

#include "app_helper.hpp"
#include <turbo/version.h>
#include "catch.hpp"
#include <fstream>
#include <set>
#include <string>
#include <utility>
#include <vector>

TEST_CASE("THelp: Basic", "[help]") {
    xcli::App app{"My prog"};

    std::string help = app.help();

    CHECK_THAT(help, Contains("My prog"));
    CHECK_THAT(help, Contains("-h,     --help"));
    CHECK_THAT(help, Contains("OPTIONS:"));
    CHECK_THAT(help, Contains("Usage:"));
}

TEST_CASE("THelp: Usage", "[help]") {
    xcli::App app{"My prog"};
    app.usage("use: just use it");

    std::string help = app.help();

    CHECK_THAT(help, Contains("use: just use it"));
}

TEST_CASE("THelp: UsageCallback", "[help]") {
    xcli::App app{"My prog"};
    app.usage([]() { return "use: just use it"; });

    std::string help = app.help();
    CHECK_THAT(help, Contains("use: just use it"));
}

TEST_CASE("THelp: UsageCallbackBoth", "[help]") {
    xcli::App app{"My prog"};
    app.usage([]() { return "use: just use it"; });
    app.usage("like 1, 2, and 3");
    std::string help = app.help();

    CHECK_THAT(help, Contains("use: just use it"));
    CHECK_THAT(help, Contains("like 1, 2, and 3"));
}

TEST_CASE("THelp: Footer", "[help]") {
    xcli::App app{"My prog"};
    app.footer("Report bugs to bugs@example.com");

    std::string help = app.help();

    CHECK_THAT(help, Contains("Report bugs to bugs@example.com"));
}

TEST_CASE("THelp: FooterCallback", "[help]") {
    xcli::App app{"My prog"};
    app.footer([]() { return "Report bugs to bugs@example.com"; });

    std::string help = app.help();

    CHECK_THAT(help, Contains("Report bugs to bugs@example.com"));
}

TEST_CASE("THelp: FooterCallbackBoth", "[help]") {
    xcli::App app{"My prog"};
    app.footer([]() { return "Report bugs to bugs@example.com"; });
    app.footer(" foot!!!!");
    std::string help = app.help();

    CHECK_THAT(help, Contains("Report bugs to bugs@example.com"));
    CHECK_THAT(help, Contains("foot!!!!"));
}

/// @brief from github issue #1156
TEST_CASE("THelp: FooterOptionGroup", "[help]") {
    xcli::App app{"My prog"};

    app.footer("Report bugs to bugs@example.com");

    app.add_option_group("group-a", "");

    app.add_option_group("group-b", "");

    std::string help = app.help();

    auto footer_loc = help.find("bugs@example.com");
    auto footer_loc2 = help.find("bugs@example.com", footer_loc + 10);
    CHECK(footer_loc != std::string::npos);
    // should only see the footer once
    CHECK(footer_loc2 == std::string::npos);
}

/// @brief from github issue #1183
TEST_CASE("THelp: FooterSubcommandHelpAll", "[help]") {
    xcli::App app{"My prog"};

    app.footer("Report bugs to bugs@example.com");
    app.set_help_all_flag("--help-all", "All options of subcommands");
    app.add_subcommand("Subcommand1", "Desc1");
    app.add_subcommand("Subcommand2", "Desc2");

    CHECK_THROWS_AS(app.parse("--help-all"), xcli::CallForAllHelp);

    std::string help = app.help("", xcli::AppFormatMode::All);

    auto footer_loc = help.find("bugs@example.com");
    auto footer_loc2 = help.find("bugs@example.com", footer_loc + 10);
    CHECK(footer_loc != std::string::npos);
    // should only see the footer once
    CHECK(footer_loc2 == std::string::npos);
}

TEST_CASE("THelp: FooterSubcommandHelp", "[help]") {
    xcli::App app{"My prog"};

    app.footer("Report bugs to bugs@example.com");
    app.add_subcommand("Subcommand1", "Desc1");
    app.add_subcommand("Subcommand2", "Desc2");

    CHECK_THROWS_AS(app.parse("Subcommand1 Subcommand2 --help"), xcli::CallForHelp);

    std::string help = app.help();
    auto footer_loc = help.find("bugs@example.com");
    auto footer_loc2 = help.find("bugs@example.com", footer_loc + 10);
    CHECK(footer_loc != std::string::npos);
    // should only see the footer once
    CHECK(footer_loc2 == std::string::npos);
}

TEST_CASE("THelp: Description", "[help]") {
    xcli::App app{"My prog"};
    std::string x;
    app.add_option("--option", x, "option description BD");

    std::string help = app.help();
    CHECK_THAT(help, Contains("option description BD"));
    // there was an issue where an extra space was being generated in the description, so make sure that is not there
    CHECK_THAT(help, !Contains("BD "));
}

TEST_CASE("THelp: OptionalPositional", "[help]") {
    xcli::App app{"My prog", "program"};

    std::string x;
    app.add_option("something", x, "My option here");

    std::string help = app.help();

    CHECK_THAT(help, Contains("OPTIONS:"));
    CHECK_THAT(help, Contains("POSITIONALS:"));
    CHECK_THAT(help, Contains("something TEXT"));
    CHECK_THAT(help, Contains("My option here"));
    CHECK_THAT(help, Contains("program [OPTIONS] [something]"));
}

TEST_CASE("THelp: Hidden", "[help]") {
    xcli::App app{"My prog"};

    std::string x;
    app.add_option("something", x, "My option here")->group("");
    std::string y;
    app.add_option("--another", y)->group("");

    std::string help = app.help();

    CHECK_THAT(help, Contains("OPTIONS:"));
    CHECK_THAT(help, !Contains("[something]"));
    CHECK_THAT(help, !Contains("something "));
    CHECK_THAT(help, !Contains("another"));
}

TEST_CASE("THelp: deprecatedOptions", "[help]") {
    xcli::App app{"My prog"};

    std::string x;
    auto *soption = app.add_option("--something", x, "My option here");
    app.add_option("--something_else", x, "My option here");
    std::string y;
    app.add_option("--another", y);

    xcli::deprecate_option(soption, "something_else");

    std::string help = app.help();

    CHECK_THAT(help, Contains("DEPRECATED"));
    CHECK_THAT(help, Contains("something"));
    CHECK_NOTHROW(app.parse("--something deprecated"));
}

TEST_CASE("THelp: deprecatedOptions2", "[help]") {
    xcli::App app{"My prog"};

    std::string x;
    app.add_option("--something", x, "My option here");
    app.add_option("--something_else", x, "My option here");
    std::string y;
    app.add_option("--another", y);

    xcli::deprecate_option(&app, "--something");

    std::string help = app.help();

    CHECK_THAT(help, Contains("DEPRECATED"));
    CHECK_THAT(help, Contains("something"));
    CHECK_NOTHROW(app.parse("--something deprecated"));
}

TEST_CASE("THelp: deprecatedOptions3", "[help]") {
    xcli::App app{"My prog"};
    app.get_formatter()->right_column_width(100);
    std::string x;
    app.add_option("--something", x, "Some Description");
    app.add_option("--something_else", x, "Some other description");
    std::string y;
    app.add_option("--another", y);

    xcli::deprecate_option(app, "--something", "--something_else");

    std::string help = app.help();

    CHECK_THAT(help, Contains("DEPRECATED"));
    CHECK_THAT(help, Contains("'--something_else' instead"));
    CHECK_NOTHROW(app.parse("--something deprecated"));
}

TEST_CASE("THelp: retiredOptions", "[help]") {
    xcli::App app{"My prog"};

    std::string x;
    auto *opt1 = app.add_option("--something", x, "My option here");
    app.add_option("--something_else", x, "My option here");
    std::string y;
    app.add_option("--another", y);

    xcli::retire_option(app, opt1);

    std::string help = app.help();

    CHECK_THAT(help, Contains("RETIRED"));
    CHECK_THAT(help, Contains("something"));

    CHECK_NOTHROW(app.parse("--something old"));
}

TEST_CASE("THelp: retiredOptions2", "[help]") {
    xcli::App app{"My prog"};

    std::string x;
    app.add_option("--something_else", x, "My option here");
    std::string y;
    app.add_option("--another", y);

    xcli::retire_option(&app, "--something");

    std::string help = app.help();

    CHECK_THAT(help, Contains("RETIRED"));
    CHECK_THAT(help, Contains("something"));
    CHECK_NOTHROW(app.parse("--something old"));
}

TEST_CASE("THelp: retiredOptions3", "[help]") {
    xcli::App app{"My prog"};

    std::string x;
    app.add_option("--something", x, "My option here");
    app.add_option("--something_else", x, "My option here");
    std::string y;
    app.add_option("--another", y);

    xcli::retire_option(app, "--something");

    std::string help = app.help();

    CHECK_THAT(help, Contains("RETIRED"));
    CHECK_THAT(help, Contains("something"));

    CHECK_NOTHROW(app.parse("--something old"));
}

TEST_CASE("THelp: HiddenGroup", "[help]") {
    xcli::App app{"My prog"};
    // empty option group name should be hidden
    auto *hgroup = app.add_option_group("");
    std::string x;
    hgroup->add_option("something", x, "My option here");
    std::string y;
    hgroup->add_option("--another", y);

    std::string help = app.help();

    CHECK_THAT(help, Contains("OPTIONS:"));
    CHECK_THAT(help, !Contains("[something]"));
    CHECK_THAT(help, !Contains("something "));
    CHECK_THAT(help, !Contains("another"));

    hgroup->group("ghidden");

    help = app.help();

    CHECK_THAT(help, Contains("something "));
    CHECK_THAT(help, Contains("another"));
}

// from https://github.com/CLIUtils/CLI11/issues/1045
TEST_CASE("THelp: multiple_group", "[help]") {
    xcli::App app{"test_group"};
    auto *group1 = app.add_option_group("outGroup");
    auto *group2 = app.add_option_group("inGroup");

    std::string outFile("");
    group1->add_option("--outfile,-o", outFile, "specify the file location of the output")->required();

    std::string inFile("");
    group2->add_option("--infile,-i", inFile, "specify the file location of the input")->required();

    auto help = app.help();
    int inCount = 0;
    int outCount = 0;
    auto iFind = help.find("inGroup");
    while(iFind != std::string::npos) {
        ++inCount;
        iFind = help.find("inGroup", iFind + 6);
    }
    auto oFind = help.find("outGroup");
    while(oFind != std::string::npos) {
        ++outCount;
        oFind = help.find("outGroup", oFind + 6);
    }
    CHECK(inCount == 1);
    CHECK(outCount == 1);
}

TEST_CASE("THelp: OptionalPositionalAndOptions", "[help]") {
    xcli::App app{"My prog", "AnotherProgram"};
    app.add_flag("-q,--quick");

    std::string x;
    app.add_option("something", x, "My option here");

    std::string help = app.help();

    CHECK_THAT(help, Contains("AnotherProgram [OPTIONS] [something]"));
}

TEST_CASE("THelp: NonStandardOptions", "[help]") {
    xcli::App app{"My prog", "nonstandard"};
    app.allow_non_standard_option_names();
    app.add_flag("-q,--quick");
    app.add_flag("-slow");
    app.add_option("--fast,-not-slow", "a description of what is");
    std::string x;
    app.add_option("something", x, "My option here");

    std::string help = app.help();

    CHECK_THAT(help, Contains("-not-slow"));
}

TEST_CASE("THelp: RequiredPositionalAndOptions", "[help]") {
    xcli::App app{"My prog"};
    app.add_flag("-q,--quick");

    std::string x;
    app.add_option("something", x, "My option here")->required();

    std::string help = app.help();

    CHECK_THAT(help, Contains("OPTIONS:"));
    CHECK_THAT(help, Contains("POSITIONALS:"));
    CHECK_THAT(help, Contains("Usage: [OPTIONS] something"));
}

TEST_CASE("THelp: MultiOpts", "[help]") {
    xcli::App app{"My prog"};
    std::vector<int> x, y;
    app.add_option("-q,--quick", x, "Disc")->expected(2);
    app.add_option("-v,--vals", y, "Other");

    std::string help = app.help();

    CHECK_THAT(help, Contains("My prog"));
    CHECK_THAT(help, !Contains("POSITIONALS:"));
    CHECK_THAT(help, Contains("Usage: [OPTIONS]"));
    CHECK_THAT(help, Contains("INT x 2"));
    CHECK_THAT(help, Contains("INT ..."));
}

TEST_CASE("THelp: VectorOpts", "[help]") {
    xcli::App app{"My prog"};
    std::vector<int> x = {1, 2};
    app.add_option("-q,--quick", x)->capture_default_str();

    std::string help = app.help();

    CHECK_THAT(help, Contains("[1,2]"));
    CHECK_THAT(help, Contains(" ..."));
}

TEST_CASE("THelp: MultiPosOpts", "[help]") {
    xcli::App app{"My prog"};
    app.name("program");
    std::vector<int> x, y;
    app.add_option("quick", x, "Disc")->expected(2);
    app.add_option("vals", y, "Other");

    std::string help = app.help();

    CHECK_THAT(help, Contains("My prog"));
    CHECK_THAT(help, Contains("POSITIONALS:"));
    CHECK_THAT(help, Contains("program [OPTIONS]"));
    CHECK_THAT(help, Contains("INT x 2"));
    CHECK_THAT(help, Contains("INT ..."));
    CHECK_THAT(help, Contains("[quick(2x)]"));
    CHECK_THAT(help, Contains("[vals...]"));
}

TEST_CASE("THelp: EnvName", "[help]") {
    xcli::App app{"My prog"};
    std::string input;
    app.add_option("--something", input)->envname("SOME_ENV");

    std::string help = app.help();

    CHECK_THAT(help, Contains("SOME_ENV"));
}

TEST_CASE("THelp: Needs", "[help]") {
    xcli::App app{"My prog"};

    xcli::Option *op1 = app.add_flag("--op1");
    app.add_flag("--op2")->needs(op1);

    std::string help = app.help();

    CHECK_THAT(help, Contains("Needs: --op1"));
}

TEST_CASE("THelp: NeedsPositional", "[help]") {
    xcli::App app{"My prog"};

    int x{0}, y{0};

    xcli::Option *op1 = app.add_option("op1", x, "one");
    app.add_option("op2", y, "two")->needs(op1);

    std::string help = app.help();

    CHECK_THAT(help, Contains("POSITIONALS:"));
    CHECK_THAT(help, Contains("Needs: op1"));
}

TEST_CASE("THelp: Excludes", "[help]") {
    xcli::App app{"My prog"};

    xcli::Option *op1 = app.add_flag("--op1");
    app.add_flag("--op2")->excludes(op1);

    std::string help = app.help();

    CHECK_THAT(help, Contains("Excludes: --op1"));
}

TEST_CASE("THelp: ExcludesPositional", "[help]") {
    xcli::App app{"My prog"};

    int x{0}, y{0};

    xcli::Option *op1 = app.add_option("op1", x);
    app.add_option("op2", y)->excludes(op1);

    std::string help = app.help();

    CHECK_THAT(help, Contains("POSITIONALS:"));
    CHECK_THAT(help, Contains("Excludes: op1"));
}

TEST_CASE("THelp: ExcludesSymmetric", "[help]") {
    xcli::App app{"My prog"};

    xcli::Option *op1 = app.add_flag("--op1");
    app.add_flag("--op2")->excludes(op1);

    std::string help = app.help();

    CHECK_THAT(help, Contains("Excludes: --op2"));
}

TEST_CASE("THelp: NeedsSortedByName", "[help]") {
    xcli::App app{"My prog"};

    xcli::Option *option_b = app.add_flag("--optionB");
    xcli::Option *option_a = app.add_flag("--optionA");
    app.add_flag("--target")->needs(option_b)->needs(option_a);

    std::string help = app.help();

    CHECK_THAT(help, Contains("Needs: --optionA --optionB"));
}

TEST_CASE("THelp: ExcludesSortedByName", "[help]") {
    xcli::App app{"My prog"};

    xcli::Option *option_b = app.add_flag("--optionB");
    xcli::Option *option_a = app.add_flag("--optionA");
    app.add_flag("--target")->excludes(option_b)->excludes(option_a);

    std::string help = app.help();

    CHECK_THAT(help, Contains("Excludes: --optionA --optionB"));
}

TEST_CASE("THelp: ManualSetters", "[help]") {

    xcli::App app{"My prog"};

    int x{1};

    xcli::Option *op1 = app.add_option("--op", x);
    op1->default_str("12");
    op1->type_name("BIGGLES");
    CHECK(1 == x);

    std::string help = app.help();

    CHECK_THAT(help, Contains("[12]"));
    CHECK_THAT(help, Contains("BIGGLES"));

    op1->default_val("14");
    CHECK(14 == x);
    help = app.help();
    CHECK_THAT(help, Contains("[14]"));

    op1->default_val(12);
    CHECK(12 == x);
    help = app.help();
    CHECK_THAT(help, Contains("[12]"));

    CHECK(op1->get_run_callback_for_default());
    op1->run_callback_for_default(false);
    CHECK(!op1->get_run_callback_for_default());

    op1->default_val(18);
    // x should not be modified in this case
    CHECK(12 == x);
    help = app.help();
    CHECK_THAT(help, Contains("[18]"));
}

TEST_CASE("THelp: Subcom", "[help]") {
    xcli::App app{"My prog"};

    auto *sub1 = app.add_subcommand("sub1");
    app.add_subcommand("sub2");

    std::string help = app.help();
    CHECK_THAT(help, Contains("Usage: [OPTIONS] [SUBCOMMANDS]"));

    app.require_subcommand();

    help = app.help();
    CHECK_THAT(help, Contains("Usage: [OPTIONS] SUBCOMMANDS"));
    app.require_subcommand(1);

    help = app.help();
    CHECK_THAT(help, Contains("Usage: [OPTIONS] SUBCOMMAND"));

    app.require_subcommand(1, 0);
    help = app.help();
    CHECK_THAT(help, Contains("Usage: [OPTIONS] SUBCOMMANDS"));

    help = sub1->help();
    CHECK_THAT(help, Contains("sub1 [OPTIONS]"));

    char x[] = "./myprogram";  // NOLINT(modernize-avoid-c-arrays)
    char y[] = "sub2";         // NOLINT(modernize-avoid-c-arrays)

    std::vector<char *> args = {x, y};
    app.parse(static_cast<int>(args.size()), args.data());

    help = app.help();
    CHECK_THAT(help, Contains("./myprogram sub2"));
}

TEST_CASE("THelp: Subcom_alias", "[help]") {
    xcli::App app{"My prog"};

    auto *sub1 = app.add_subcommand("sub1", "Subcommand1 description test");
    sub1->alias("sub_alias1");
    sub1->alias("sub_alias2");

    app.add_subcommand("sub2", "Subcommand2 description test");

    std::string help = app.help();
    CHECK_THAT(help, Contains("Usage: [OPTIONS] [SUBCOMMANDS]"));
    CHECK_THAT(help, Contains("sub_alias1"));
    CHECK_THAT(help, Contains("sub_alias2"));

    app.require_subcommand(0, 0);
    help = app.help();
    CHECK_THAT(help, Contains("Usage: [OPTIONS] [SUBCOMMANDS]"));
}

TEST_CASE("THelp: Subcom_alias_group", "[help]") {
    xcli::App app{"My prog"};

    auto *sub1 = app.add_subcommand("", "Subcommand1 description test");
    sub1->alias("sub_alias1");
    sub1->alias("sub_alias2");

    app.add_subcommand("sub2", "Subcommand2 description test");

    std::string help = app.help();
    CHECK_THAT(help, Contains("Usage: [OPTIONS] [SUBCOMMANDS]"));
    CHECK_THAT(help, Contains("sub_alias1"));
    CHECK_THAT(help, Contains("sub_alias2"));
}

// from https://github.com/CLIUtils/CLI11/issues/1269
TEST_CASE("THelp: nested_subcommand", "[help]") {
    xcli::App app{"My prog"};

    auto *sub1 = app.add_subcommand("sub1", "Subcommand1 description test");
    int opt1 = 0;
    sub1->add_option("--opt1", opt1, "Option belonging to subc1");

    // Level 2 subcommand under subc1 with fallthrough enabled
    auto *subc2 = sub1->add_subcommand("subc2", "Second level command");
    subc2->fallthrough();  // allow unknown options to fall through to parent
    int opt2 = 0;
    subc2->add_option("--opt2", opt2, "Option belonging to subc2");

    try {
        app.parse("sub1 subc2 --help");
        CHECK(false);
    } catch(const xcli::ParseError &e) {
        std::ostringstream eout;
        auto res = app.exit(e, eout);
        std::string help = eout.str();
        CHECK(res == 0);
        CHECK_THAT(help, Contains("--opt2"));
        CHECK_THAT(help, Contains("--opt1"));
    }
}

TEST_CASE("THelp: MasterName", "[help]") {
    xcli::App app{"My prog", "MyRealName"};

    char x[] = "./myprogram";  // NOLINT(modernize-avoid-c-arrays)

    std::vector<char *> args = {x};
    app.parse(static_cast<int>(args.size()), args.data());

    CHECK_THAT(app.help(), Contains("MyRealName"));
}

#if (defined(XCLI_ENABLE_EXTRA_VALIDATORS) && XCLI_ENABLE_EXTRA_VALIDATORS == 1) ||                                  \
    (!defined(XCLI_DISABLE_EXTRA_VALIDATORS) || XCLI_DISABLE_EXTRA_VALIDATORS == 0)

TEST_CASE("THelp: ManualSetterOverFunction", "[help]") {

    xcli::App app{"My prog"};

    int x{1};

    xcli::Option *op1 = app.add_option("--op1", x)->check(xcli::IsMember({1, 2}));
    xcli::Option *op2 = app.add_option("--op2", x)->transform(xcli::IsMember({1, 2}));
    op1->default_str("12");
    op1->type_name("BIGGLES");
    op2->type_name("QUIGGLES");
    CHECK(1 == x);

    std::string help = app.help();
    CHECK_THAT(help, Contains("[12]"));
    CHECK_THAT(help, Contains("BIGGLES"));
    CHECK_THAT(help, Contains("QUIGGLES"));
    CHECK_THAT(help, Contains("{1,2}"));
}

TEST_CASE("THelp: IntDefaults", "[help]") {
    xcli::App app{"My prog"};

    int one{1}, two{2};
    app.add_option("--one", one, "Help for one")->capture_default_str();
    app.add_option("--set", two, "Help for set")->capture_default_str()->check(xcli::IsMember({2, 3, 4}));

    std::string help = app.help();

    CHECK_THAT(help, Contains("--one"));
    CHECK_THAT(help, Contains("--set"));
    CHECK_THAT(help, Contains("1"));
    CHECK_THAT(help, Contains("[2]"));
    CHECK_THAT(help, Contains("2,3,4"));
}

TEST_CASE("THelp: SetLower", "[help]") {
    xcli::App app{"My prog"};
    app.option_defaults()->always_capture_default();

    std::string def{"One"};
    app.add_option("--set", def, "Help for set")->check(xcli::IsMember({"oNe", "twO", "THREE"}));

    std::string help = app.help();

    CHECK_THAT(help, Contains("--set"));
    CHECK_THAT(help, Contains("[One]"));
    CHECK_THAT(help, Contains("oNe"));
    CHECK_THAT(help, Contains("twO"));
    CHECK_THAT(help, Contains("THREE"));
}

#endif

TEST_CASE("THelp: OnlyOneHelp", "[help]") {
    xcli::App app{"My prog"};

    // It is not supported to have more than one help flag, last one wins
    app.set_help_flag("--help", "No short name allowed");
    app.set_help_flag("--yelp", "Alias for help");

    std::vector<std::string> input{"--help"};
    CHECK_THROWS_AS(app.parse(input), xcli::ExtrasError);
}

TEST_CASE("THelp: MultiHelp", "[help]") {
    xcli::App app{"My prog"};

    // It is not supported to have more than one help flag, last one wins
    app.set_help_flag("--help,-h,-?", "No short name allowed");
    app.allow_windows_style_options();

    std::vector<std::string> input{"/?"};
    CHECK_THROWS_AS(app.parse(input), xcli::CallForHelp);
}

TEST_CASE("THelp: OnlyOneAllHelp", "[help]") {
    xcli::App app{"My prog"};

    // It is not supported to have more than one help flag, last one wins
    app.set_help_all_flag("--help-all", "No short name allowed");
    app.set_help_all_flag("--yelp", "Alias for help");

    std::vector<std::string> input{"--help-all"};
    CHECK_THROWS_AS(app.parse(input), xcli::ExtrasError);

    std::vector<std::string> input2{"--yelp"};
    CHECK_THROWS_AS(app.parse(input2), xcli::CallForAllHelp);

    // Remove the flag
    app.set_help_all_flag();
    std::vector<std::string> input3{"--yelp"};
    CHECK_THROWS_AS(app.parse(input3), xcli::ExtrasError);
}

TEST_CASE("THelp: RemoveHelp", "[help]") {
    xcli::App app{"My prog"};
    app.set_help_flag();

    std::string help = app.help();

    CHECK_THAT(help, Contains("My prog"));
    CHECK_THAT(help, !Contains("-h,--help"));
    CHECK_THAT(help, !Contains("OPTIONS:"));
    CHECK_THAT(help, Contains("Usage:"));

    std::vector<std::string> input{"--help"};
    try {
        app.parse(input);
    } catch(const xcli::ParseError &e) {
        CHECK(e.get_exit_code() == static_cast<int>(xcli::ExitCodes::ExtrasError));
    }
}

TEST_CASE("THelp: RemoveOtherMethodHelp", "[help]") {
    xcli::App app{"My prog"};

    // Don't do this. Just in case, let's make sure it works.
    app.remove_option(const_cast<xcli::Option *>(app.get_help_ptr()));

    std::string help = app.help();

    CHECK_THAT(help, Contains("My prog"));
    CHECK_THAT(help, !Contains("-h,--help"));
    CHECK_THAT(help, !Contains("OPTIONS:"));
    CHECK_THAT(help, Contains("Usage:"));

    std::vector<std::string> input{"--help"};
    try {
        app.parse(input);
    } catch(const xcli::ParseError &e) {
        CHECK(e.get_exit_code() == static_cast<int>(xcli::ExitCodes::ExtrasError));
    }
}

TEST_CASE("THelp: RemoveOtherMethodHelpAll", "[help]") {
    xcli::App app{"My prog"};

    app.set_help_all_flag("--help-all");
    // Don't do this. Just in case, let's make sure it works.
    app.remove_option(const_cast<xcli::Option *>(app.get_help_all_ptr()));

    std::string help = app.help();

    CHECK_THAT(help, Contains("My prog"));
    CHECK_THAT(help, !Contains("--help-all"));
    CHECK_THAT(help, Contains("OPTIONS:"));
    CHECK_THAT(help, Contains("Usage:"));

    std::vector<std::string> input{"--help-all"};
    try {
        app.parse(input);
    } catch(const xcli::ParseError &e) {
        CHECK(e.get_exit_code() == static_cast<int>(xcli::ExitCodes::ExtrasError));
    }
}

TEST_CASE("THelp: NoHelp", "[help]") {
    xcli::App app{"My prog"};
    app.set_help_flag();

    std::string help = app.help();

    CHECK_THAT(help, Contains("My prog"));
    CHECK_THAT(help, !Contains("-h,--help"));
    CHECK_THAT(help, !Contains("OPTIONS:"));
    CHECK_THAT(help, Contains("Usage:"));

    std::vector<std::string> input{"--help"};
    try {
        app.parse(input);
    } catch(const xcli::ParseError &e) {
        CHECK(e.get_exit_code() == static_cast<int>(xcli::ExitCodes::ExtrasError));
    }
}

TEST_CASE("THelp: CustomHelp", "[help]") {
    xcli::App app{"My prog"};

    xcli::Option *help_option = app.set_help_flag("--yelp", "display help and exit");
    CHECK(help_option == app.get_help_ptr());

    std::string help = app.help();

    CHECK_THAT(help, Contains("My prog"));
    CHECK_THAT(help, !Contains("-h,--help"));
    CHECK_THAT(help, Contains("--yelp"));
    CHECK_THAT(help, Contains("OPTIONS:"));
    CHECK_THAT(help, Contains("Usage:"));

    std::vector<std::string> input{"--yelp"};
    try {
        app.parse(input);
    } catch(const xcli::CallForHelp &e) {
        CHECK(e.get_exit_code() == static_cast<int>(xcli::ExitCodes::Success));
    }
}

TEST_CASE("THelp: HelpSubcommandPriority", "[help]") {
    xcli::App app{"My prog"};

    app.set_help_flag("-h", "display help and exit");

    auto *sub1 = app.add_subcommand("sub1");
    std::string someFile = "";

    put_env("SOME_FILE", "NOT_A_FILE");
    sub1->add_option("-f,--file", someFile)->envname("SOME_FILE")->required()->expected(1)->check(xcli::ExistingFile);

    std::string input{"sub1 -h"};
    CHECK_THROWS_AS(app.parse(input), xcli::CallForHelp);
    unset_env("SOME_FILE");
}

TEST_CASE("THelp: NextLineShouldBeAlignmentInMultilineDescription", "[help]") {
    xcli::App app;
    int i{0};
    const std::string first{"first line"};
    const std::string second{"second line"};
    app.add_option("-i,--int", i, first + "\n" + second);

    const std::string help = app.help();
    const auto width = app.get_formatter()->get_column_width();
    auto first_loc = help.find("first");
    auto first_new_line = help.find_last_of('\n', first_loc);
    auto second_loc = help.find("second");
    auto second_new_line = help.find_last_of('\n', second_loc);
    CHECK(first_loc - first_new_line - 1 == width);
    CHECK(second_loc - second_new_line - 1 == width);
    CHECK(second_new_line > first_loc);
}

TEST_CASE("THelp: CheckRightWidth", "[help]") {
    xcli::App app;
    int i{0};
    const std::string first{"first line"};
    const std::string second{"second line"};
    app.add_option("-i,--int", i, first + "\n" + second);
    app.get_formatter()->column_width(24);
    CHECK(app.get_formatter()->get_column_width() == 24);
    const std::string help = app.help();
    auto first_loc = help.find("first");
    auto first_new_line = help.find_last_of('\n', first_loc);
    auto second_loc = help.find("second");
    auto second_new_line = help.find_last_of('\n', second_loc);
    CHECK(first_loc - first_new_line - 1 == 24);
    CHECK(second_loc - second_new_line - 1 == 24);
    CHECK(second_new_line > first_loc);
}

static const std::string long_string{
    "AAARG this is a long line description that will span across multiple lines and still go on and on.  This is meant "
    "to test how the help handler handles things like this"};

TEST_CASE("THelp: longLineAlignment", "[help]") {
    xcli::App app;
    int i{0};

    app.add_option("-i,--int,--int_very_long_option_name_that_just_keeps_going_on_and_on_and_on_and_on_and_on_possibly_"
                   "to_infinity,--and_another_long_name_just_for_fun",
                   i,
                   long_string);

    std::string help = app.help();
    auto width = app.get_formatter()->get_right_column_width();
    auto first_loc = help.find("AAARG");
    auto first_new_line = help.find_first_of('\n', first_loc);

    CHECK(first_new_line - first_loc - 1 < width);
    app.get_formatter()->right_column_width(30);
    width = app.get_formatter()->get_right_column_width();
    CHECK(width == 30);
    help = app.help();
    first_loc = help.find("AAARG");
    first_new_line = help.find_first_of('\n', first_loc);

    CHECK(first_new_line - first_loc - 1 < width);
}

TEST_CASE("THelp: longPositional", "[help]") {
    xcli::App app;
    int i{0};

    app.add_option("int_very_long_option_name_that_just_keeps_going_on_and_on_and_on_and_on_and_on_possibly_"
                   "to_infinity",
                   i,
                   long_string);

    std::string help = app.help();
    auto width = app.get_formatter()->get_right_column_width();
    auto first_loc = help.find("AAARG");
    auto first_new_line = help.find_first_of('\n', first_loc);

    CHECK(first_new_line - first_loc - 1 < width);
}

TEST_CASE("THelp: SubcommandNewLineDescription", "[help]") {

    const std::string nl_description{"this is a description with aX \n X\\n in it and just for fun \n\t another"};

    xcli::App app;
    int i{0};
    app.add_option("-i,--int", i);
    app.add_subcommand("subcom1", nl_description);
    std::string help = app.help();
    auto width = app.get_formatter()->get_column_width();
    auto first_X = help.find_first_of('X');
    auto first_new_line = help.find_first_of('\n', first_X);
    auto second_X = help.find_first_of('X', first_new_line);
    CHECK(second_X - first_new_line > width);
}

TEST_CASE("THelp: longDescription", "[help]") {

    xcli::App app(long_string, "long_desc");
    int i{0};

    app.add_option("-i,--int", i);

    std::string help = app.help();
    auto width = app.get_formatter()->get_description_paragraph_width();
    auto first_loc = help.find("AAARG");
    auto first_new_line = help.find_first_of('\n', first_loc);

    CHECK(first_new_line - first_loc - 1 < width);
    app.get_formatter()->description_paragraph_width(30);
    width = app.get_formatter()->get_description_paragraph_width();
    CHECK(width == 30);
    help = app.help();
    first_loc = help.find("AAARG");
    first_new_line = help.find_first_of('\n', first_loc);

    CHECK(first_new_line - first_loc - 1 < width);
}

TEST_CASE("THelp: longSubcommandDescription", "[help]") {

    xcli::App app;
    int i{0};

    app.add_option("-i,--int", i);
    app.add_subcommand("test1", long_string);
    std::string help = app.help();
    auto width = app.get_formatter()->get_right_column_width();
    auto first_loc = help.find("AAARG");
    auto first_new_line = help.find_first_of('\n', first_loc);

    CHECK(first_new_line - first_loc - 1 < width);
    app.get_formatter()->right_column_width(30);
    width = 30;
    help = app.help();
    first_loc = help.find("AAARG");
    first_new_line = help.find_first_of('\n', first_loc);

    CHECK(first_new_line - first_loc - 1 < width);
}

TEST_CASE("THelp: longSubcommandDescriptionExpanded", "[help]") {

    xcli::App app;
    int i{0};

    app.add_option("-i,--int", i);
    app.add_subcommand("test1", long_string);

    auto help = app.help("", xcli::AppFormatMode::All);
    auto width = app.get_formatter()->get_description_paragraph_width();
    auto first_loc = help.find("AAARG");
    auto first_new_line = help.find_first_of('\n', first_loc);

    CHECK(first_new_line - first_loc - 1 < width);
}

TEST_CASE("THelp: longFooter", "[help]") {
    xcli::App app("test long footer", "long_desc");
    int i{0};
    app.footer(long_string);
    app.add_option("-i,--int", i);

    std::string help = app.help();
    auto width = app.get_formatter()->get_footer_paragraph_width();
    auto first_loc = help.find("AAARG");
    auto first_new_line = help.find_first_of('\n', first_loc);

    CHECK(first_new_line - first_loc - 1 < width);
    app.get_formatter()->footer_paragraph_width(30);
    width = app.get_formatter()->get_footer_paragraph_width();
    CHECK(width == 30);
    help = app.help();
    first_loc = help.find("AAARG");
    first_new_line = help.find_first_of('\n', first_loc);

    CHECK(first_new_line - first_loc - 1 < width);
}

TEST_CASE("THelp: NiceName", "[help]") {
    xcli::App app;

    int x{0};
    auto *long_name = app.add_option("-s,--long,-q,--other,that", x);
    auto *short_name = app.add_option("more,-x,-y", x);
    auto *positional = app.add_option("posit", x);

    CHECK("--long" == long_name->get_name());
    CHECK("-x" == short_name->get_name());
    CHECK("posit" == positional->get_name());
}

TEST_CASE("Exit: ErrorWithHelp", "[help]") {
    xcli::App app{"My prog"};

    std::vector<std::string> input{"-h"};
    try {
        app.parse(input);
    } catch(const xcli::CallForHelp &e) {
        CHECK(e.get_exit_code() == static_cast<int>(xcli::ExitCodes::Success));
    }
}

TEST_CASE("Exit: ErrorWithAllHelp", "[help]") {
    xcli::App app{"My prog"};
    app.set_help_all_flag("--help-all", "All help");

    std::vector<std::string> input{"--help-all"};
    try {
        app.parse(input);
    } catch(const xcli::CallForAllHelp &e) {
        CHECK(e.get_exit_code() == static_cast<int>(xcli::ExitCodes::Success));
    }
}

TEST_CASE("Exit: ErrorWithoutHelp", "[help]") {
    xcli::App app{"My prog"};

    std::vector<std::string> input{"--none"};
    try {
        app.parse(input);
    } catch(const xcli::ParseError &e) {
        CHECK(e.get_exit_code() == static_cast<int>(xcli::ExitCodes::ExtrasError));
    }
}

TEST_CASE("Exit: ExitCodes", "[help]") {
    xcli::App app;

    auto i = static_cast<int>(xcli::ExitCodes::ExtrasError);
    CHECK(app.exit(xcli::Success()) == 0);
    CHECK(app.exit(xcli::CallForHelp()) == 0);
    CHECK(app.exit(xcli::ExtrasError({"Thing"})) == i);
    CHECK(app.exit(xcli::RuntimeError(42)) == 42);
    CHECK(app.exit(xcli::RuntimeError()) == 1);
}

struct CapturedHelp {
    xcli::App app{"My Test Program"};
    std::stringstream out{};
    std::stringstream err{};

    int run(const xcli::Error &e) { return app.exit(e, out, err); }

    void reset() {
        out.clear();
        err.clear();
    }
};

TEST_CASE_METHOD(CapturedHelp, "Successful", "[help]") {
    CHECK(0 == run(xcli::Success()));
    CHECK(out.str().empty());
    CHECK(err.str().empty());
}

TEST_CASE_METHOD(CapturedHelp, "JustAnError", "[help]") {
    CHECK(42 == run(xcli::RuntimeError(42)));
    CHECK(out.str().empty());
    CHECK(err.str().empty());
}

TEST_CASE_METHOD(CapturedHelp, "CallForHelp", "[help]") {
    CHECK(0 == run(xcli::CallForHelp()));
    CHECK(app.help() == out.str());
    CHECK(err.str().empty());
}
TEST_CASE_METHOD(CapturedHelp, "CallForAllHelp", "[help]") {
    CHECK(0 == run(xcli::CallForAllHelp()));
    CHECK(app.help("", xcli::AppFormatMode::All) == out.str());
    CHECK(err.str().empty());
}
TEST_CASE_METHOD(CapturedHelp, "CallForAllHelpOutput", "[help]") {
    app.set_help_all_flag("--help-all", "Help all");
    app.add_subcommand("one", "One description");
    xcli::App *sub = app.add_subcommand("two");
    sub->add_flag("--three");

    CHECK(0 == run(xcli::CallForAllHelp()));
    CHECK(app.help("", xcli::AppFormatMode::All) == out.str());
    CHECK(err.str().empty());
    CHECK_THAT(out.str(), Contains("one"));
    CHECK_THAT(out.str(), Contains("two"));
    CHECK_THAT(out.str(), Contains("--three"));
    CHECK_THAT(out.str(), Contains("SUBCOMMANDS:"));
    CHECK_THAT(out.str(), Contains("--help-all"));
    CHECK_THAT(out.str(), Contains("My Test Program"));
}
TEST_CASE_METHOD(CapturedHelp, "NewFormattedHelp", "[help]") {
    app.formatter_fn([](const xcli::App *, std::string, xcli::AppFormatMode) { return "New Help"; });
    CHECK(0 == run(xcli::CallForHelp()));
    CHECK("New Help" == out.str());
    CHECK(err.str().empty());
}

TEST_CASE_METHOD(CapturedHelp, "NormalError", "[help]") {
    CHECK(static_cast<int>(xcli::ExitCodes::ExtrasError) == run(xcli::ExtrasError({"Thing"})));
    CHECK(out.str().empty());
    CHECK_THAT(err.str(), Contains("for more information"));
    CHECK_THAT(err.str(), !Contains("ExtrasError"));
    CHECK_THAT(err.str(), Contains("Thing"));
    CHECK_THAT(err.str(), !Contains(" or "));
    CHECK_THAT(err.str(), !Contains("Usage"));
}

TEST_CASE_METHOD(CapturedHelp, "DoubleError", "[help]") {
    app.set_help_all_flag("--help-all");
    CHECK(static_cast<int>(xcli::ExitCodes::ExtrasError) == run(xcli::ExtrasError({"Thing"})));
    CHECK(out.str().empty());
    CHECK_THAT(err.str(), Contains("for more information"));
    CHECK_THAT(err.str(), Contains(" --help "));
    CHECK_THAT(err.str(), Contains(" --help-all "));
    CHECK_THAT(err.str(), Contains(" or "));
    CHECK_THAT(err.str(), !Contains("ExtrasError"));
    CHECK_THAT(err.str(), Contains("Thing"));
    CHECK_THAT(err.str(), !Contains("Usage"));
}

TEST_CASE_METHOD(CapturedHelp, "AllOnlyError", "[help]") {
    app.set_help_all_flag("--help-all");
    app.set_help_flag();
    CHECK(static_cast<int>(xcli::ExitCodes::ExtrasError) == run(xcli::ExtrasError({"Thing"})));
    CHECK(out.str().empty());
    CHECK_THAT(err.str(), Contains("for more information"));
    CHECK_THAT(err.str(), !Contains(" --help "));
    CHECK_THAT(err.str(), Contains(" --help-all "));
    CHECK_THAT(err.str(), !Contains(" or "));
    CHECK_THAT(err.str(), !Contains("ExtrasError"));
    CHECK_THAT(err.str(), Contains("Thing"));
    CHECK_THAT(err.str(), !Contains("Usage"));
}

TEST_CASE_METHOD(CapturedHelp, "ReplacedError", "[help]") {
    app.failure_message(xcli::FailureMessage::help);

    CHECK(static_cast<int>(xcli::ExitCodes::ExtrasError) == run(xcli::ExtrasError({"Thing"})));
    CHECK(out.str().empty());
    CHECK_THAT(err.str(), !Contains("for more information"));
    CHECK_THAT(err.str(), Contains("ERROR: ExtrasError"));
    CHECK_THAT(err.str(), Contains("Thing"));
    CHECK_THAT(err.str(), Contains("Usage"));
}

// #87
TEST_CASE("THelp: CustomDoubleOption", "[help]") {

    std::pair<int, double> custom_opt;

    xcli::App app;

    auto *opt = app.add_option("posit", [&custom_opt](xcli::results_t vals) {
        custom_opt = {stol(vals.at(0)), stod(vals.at(1))};
        return true;
    });
    opt->type_name("INT FLOAT")->type_size(2);

    CHECK_THAT(app.help(), !Contains("x 2"));
}

TEST_CASE("THelp: CheckEmptyTypeName", "[help]") {
    xcli::App app;

    auto *opt = app.add_flag("-f,--flag");
    std::string name = opt->get_type_name();
    CHECK(name.empty());
}

TEST_CASE("THelp: FlagDefaults", "[help]") {
    xcli::App app;

    app.add_flag("-t,--not{false}")->default_str("false");
    auto str = app.help();
    CHECK_THAT(str, Contains("--not{false}"));
}

TEST_CASE("THelp: AccessDescription", "[help]") {
    xcli::App app{"My description goes here"};

    CHECK("My description goes here" == app.get_description());
}

TEST_CASE("THelp: SetDescriptionAfterCreation", "[help]") {
    xcli::App app{""};

    app.description("My description goes here");

    CHECK("My description goes here" == app.get_description());
    CHECK_THAT(app.help(), Contains("My description goes here"));
}

TEST_CASE("THelp: AccessOptionDescription", "[help]") {
    xcli::App app{};

    int x{0};
    auto *opt = app.add_option("-a,--alpha", x, "My description goes here");

    CHECK("My description goes here" == opt->get_description());
}

TEST_CASE("THelp: SetOptionDescriptionAfterCreation", "[help]") {
    xcli::App app{};

    int x{0};
    auto *opt = app.add_option("-a,--alpha", x);
    opt->description("My description goes here");

    CHECK("My description goes here" == opt->get_description());
    CHECK_THAT(app.help(), Contains("My description goes here"));
}

TEST_CASE("THelp: CleanNeeds", "[help]") {
    xcli::App app;

    int x{0};
    auto *a_name = app.add_option("-a,--alpha", x);
    app.add_option("-b,--boo", x)->needs(a_name);

    CHECK_THAT(app.help(), !Contains("Requires"));
    CHECK_THAT(app.help(), !Contains("Needs: -a,--alpha"));
    CHECK_THAT(app.help(), Contains("Needs: --alpha"));
}

TEST_CASE("THelp: RequiredPrintout", "[help]") {
    xcli::App app;

    int x{0};
    app.add_option("-a,--alpha", x)->required();

    CHECK_THAT(app.help(), Contains(" REQUIRED"));
}

TEST_CASE("THelp: GroupOrder", "[help]") {
    xcli::App app;

    app.add_flag("--one")->group("zee");
    app.add_flag("--two")->group("aee");

    std::string help = app.help();

    auto zee_loc = help.find("zee");
    auto aee_loc = help.find("aee");

    CHECK(std::string::npos != zee_loc);
    CHECK(std::string::npos != aee_loc);
    CHECK(aee_loc > zee_loc);
}

TEST_CASE("THelp: GroupNameError", "[help]") {
    xcli::App app;

    auto *f1 = app.add_flag("--one");
    auto *f2 = app.add_flag("--two");

    CHECK_THROWS_AS(f1->group("evil group name\non two lines"), xcli::IncorrectConstruction);
    CHECK_THROWS_AS(f2->group(std::string(5, '\0')), xcli::IncorrectConstruction);
}

TEST_CASE("THelp: ValidatorsText", "[help]") {
    xcli::App app;

    std::string filename;
    int x{0};
    unsigned int y{0};
    app.add_option("--f1", filename)->check(xcli::ExistingFile);
    app.add_option("--f3", x)->check(xcli::Range(1, 4));
    app.add_option("--f4", y)->check(xcli::Range(12));

    std::string help = app.help();
    CHECK_THAT(help, Contains("TEXT:FILE"));
    CHECK_THAT(help, Contains("INT in [1 - 4]"));
    CHECK_THAT(help, Contains("UINT:INT in [0 - 12]"));
}

TEST_CASE("THelp: ValidatorsTextCustom", "[help]") {
    xcli::App app;

    std::string filename;
    app.add_option("--f1", filename)->check(xcli::ExistingFile.description("Existing file"));

    std::string help = app.help();
    CHECK_THAT(help, Contains("Existing file"));
}

TEST_CASE("THelp: ValidatorsNonPathText", "[help]") {
    xcli::App app;

    std::string filename;
    app.add_option("--f2", filename)->check(xcli::NonexistentPath);

    std::string help = app.help();
    CHECK_THAT(help, Contains("TEXT:PATH"));
}

TEST_CASE("THelp: ValidatorsDirText", "[help]") {
    xcli::App app;

    std::string filename;
    app.add_option("--f2", filename)->check(xcli::ExistingDirectory);

    std::string help = app.help();
    CHECK_THAT(help, Contains("TEXT:DIR"));
}

TEST_CASE("THelp: ValidatorsPathText", "[help]") {
    xcli::App app;

    std::string filename;
    app.add_option("--f2", filename)->check(xcli::ExistingPath);

    std::string help = app.help();
    CHECK_THAT(help, Contains("TEXT:PATH"));
}

TEST_CASE("THelp: CombinedValidatorsText", "[help]") {
    xcli::App app;

    std::string filename;
    app.add_option("--f1", filename)->check(xcli::ExistingFile | xcli::ExistingDirectory);

    // This would be nice if it put something other than string, but would it be path or file?
    // Can't programmatically tell!
    // (Users can use ExistingPath, by the way)
    std::string help = app.help();
    CHECK_THAT(help, Contains("TEXT:(FILE) OR (DIR)"));
    CHECK_THAT(help, !Contains("PATH"));
}

// Don't do this in real life, please
TEST_CASE("THelp: CombinedValidatorsPathyText", "[help]") {
    xcli::App app;

    std::string filename;
    app.add_option("--f1", filename)->check(xcli::ExistingPath | xcli::NonexistentPath);

    // Combining validators with the same type string is OK
    std::string help = app.help();
    CHECK_THAT(help, Contains("TEXT:"));
    CHECK_THAT(help, Contains("PATH"));
}

// Don't do this in real life, please (and transform does nothing here)
TEST_CASE("THelp: CombinedValidatorsPathyTextAsTransform", "[help]") {
    xcli::App app;

    std::string filename;
    app.add_option("--f1", filename)->transform(xcli::ExistingPath | xcli::NonexistentPath);

    // Combining validators with the same type string is OK
    std::string help = app.help();
    CHECK_THAT(help, Contains("TEXT:(PATH(existing)) OR (PATH"));
}

#if (defined(XCLI_ENABLE_EXTRA_VALIDATORS) && XCLI_ENABLE_EXTRA_VALIDATORS == 1) ||                                  \
    (!defined(XCLI_DISABLE_EXTRA_VALIDATORS) || XCLI_DISABLE_EXTRA_VALIDATORS == 0)

// #113 Part 2
TEST_CASE("THelp: ChangingSet", "[help]") {
    xcli::App app;

    std::set<int> vals{1, 2, 3};
    int val{0};
    app.add_option("--val", val)->check(xcli::IsMember(&vals));

    std::string help = app.help();

    CHECK_THAT(help, Contains("1"));
    CHECK_THAT(help, !Contains("4"));

    vals.insert(4);
    vals.erase(1);

    help = app.help();

    CHECK_THAT(help, !Contains("1"));
    CHECK_THAT(help, Contains("4"));
}

TEST_CASE("THelp: ChangingSetDefaulted", "[help]") {
    xcli::App app;

    std::set<int> vals{1, 2, 3};
    int val{2};
    app.add_option("--val", val, "")->check(xcli::IsMember(&vals))->capture_default_str();

    std::string help = app.help();

    CHECK_THAT(help, Contains("1"));
    CHECK_THAT(help, !Contains("4"));

    vals.insert(4);
    vals.erase(1);

    help = app.help();

    CHECK_THAT(help, !Contains("1"));
    CHECK_THAT(help, Contains("4"));
}

TEST_CASE("THelp: ChangingCaselessSet", "[help]") {
    xcli::App app;

    std::set<std::string> vals{"1", "2", "3"};
    std::string val;
    app.add_option("--val", val)->check(xcli::IsMember(&vals, xcli::ignore_case));

    std::string help = app.help();

    CHECK_THAT(help, Contains("1"));
    CHECK_THAT(help, !Contains("4"));

    vals.insert("4");
    vals.erase("1");

    help = app.help();

    CHECK_THAT(help, !Contains("1"));
    CHECK_THAT(help, Contains("4"));
}

TEST_CASE("THelp: ChangingCaselessSetDefaulted", "[help]") {
    xcli::App app;
    app.option_defaults()->always_capture_default();

    std::set<std::string> vals{"1", "2", "3"};
    std::string val = "2";
    app.add_option("--val", val)->check(xcli::IsMember(&vals, xcli::ignore_case));

    std::string help = app.help();

    CHECK_THAT(help, Contains("1"));
    CHECK_THAT(help, !Contains("4"));

    vals.insert("4");
    vals.erase("1");

    help = app.help();

    CHECK_THAT(help, !Contains("1"));
    CHECK_THAT(help, Contains("4"));
}

#endif

// New defaults tests (1.8)

TEST_CASE("THelp: ChangingDefaults", "[help]") {

    xcli::App app;

    std::vector<int> x = {1, 2};
    xcli::Option *opt = app.add_option("-q,--quick", x);
    x = {3, 4};
    CHECK(x[0] == 3);

    opt->capture_default_str();

    x = {5, 6};
    std::string help = app.help();

    CHECK_THAT(help, Contains("[[3,4]]"));
    CHECK_THAT(help, Contains("..."));
    CHECK_THAT(help, Contains("INT"));
    CHECK(x[0] == 5);
}

TEST_CASE("THelp: ChangingDefaultsWithAutoCapture", "[help]") {

    xcli::App app;
    app.option_defaults()->always_capture_default();

    std::vector<int> x = {1, 2};
    CHECK(x[0] == 1);
    app.add_option("-q,--quick", x);
    x = {3, 4};
    CHECK(x[0] == 3);

    std::string help = app.help();

    CHECK_THAT(help, Contains("[[1,2]]"));
    CHECK_THAT(help, Contains("..."));
}

TEST_CASE("THelp: FunctionDefaultString", "[help]") {

    xcli::App app;

    std::vector<int> x = {1, 2};
    xcli::Option *opt = app.add_option("-q,--quick", x);

    opt->default_function([]() { return std::string("Powerful"); });
    opt->capture_default_str();

    std::string help = app.help();

    CHECK_THAT(help, Contains("[Powerful]"));
}

TEST_CASE("TVersion: simple_flag", "[help]") {

    xcli::App app;

    app.set_version_flag("-v,--version", "VERSION " TURBO_VERSION_STRING);

    auto vers = app.version();
    CHECK_THAT(vers, Contains("VERSION"));

    app.set_version_flag();
    CHECK(app.version().empty());
}

TEST_CASE("TVersion: callback_flag", "[help]") {

    xcli::App app;

    app.set_version_flag("-v,--version", []() { return std::string("VERSION " TURBO_VERSION_STRING); });

    auto vers = app.version();
    CHECK_THAT(vers, Contains("VERSION"));

    app.set_version_flag("-v", []() { return std::string("VERSION2 " TURBO_VERSION_STRING); });
    vers = app.version();
    CHECK_THAT(vers, Contains("VERSION"));
}

TEST_CASE("TVersion: help", "[help]") {

    xcli::App app;

    app.set_version_flag("-v,--version", "version_string", "help_for_version");

    auto hvers = app.help();
    CHECK_THAT(hvers, Contains("help_for_version"));

    app.set_version_flag("-v", []() { return std::string("VERSION2 " TURBO_VERSION_STRING); }, "help_for_version2");
    hvers = app.help();
    CHECK_THAT(hvers, Contains("help_for_version2"));
}

TEST_CASE("TVersion: parse_throw", "[help]") {

    xcli::App app;

    app.set_version_flag("--version", TURBO_VERSION_STRING);

    CHECK_THROWS_AS(app.parse("--version"), xcli::CallForVersion);
    CHECK_THROWS_AS(app.parse("--version --arg2 5"), xcli::CallForVersion);

    auto *ptr = app.get_version_ptr();

    ptr->ignore_case();
    try {
        app.parse("--Version");
    } catch(const xcli::CallForVersion &v) {
        CHECK_THAT(TURBO_VERSION_STRING, Equals(v.what()));
        CHECK(0 == v.get_exit_code());
        const auto &appc = app;
        const auto *cptr = appc.get_version_ptr();
        CHECK(1U == cptr->count());
    }
}

TEST_CASE("TVersion: exit", "[help]") {

    xcli::App app;

    app.set_version_flag("--version", TURBO_VERSION_STRING);

    try {
        app.parse("--version");
    } catch(const xcli::CallForVersion &v) {
        std::ostringstream out;
        auto ret = app.exit(v, out);
        CHECK_THAT(out.str(), Contains(TURBO_VERSION_STRING));
        CHECK(0 == ret);
    }
}

TEST_CASE("TVersion: exit_with_required", "[help]") {
    // test that the version flag works even if there are required options
    xcli::App app;

    app.set_version_flag("--version", TURBO_VERSION_STRING);
    app.add_option("--req")->required();

    try {
        app.parse("--version");
    } catch(const xcli::CallForVersion &v) {
        std::ostringstream out;
        auto ret = app.exit(v, out);
        CHECK_THAT(out.str(), Contains(TURBO_VERSION_STRING));
        CHECK(0 == ret);
    }
}
