#include <turbo/unicode/utf.h>

#include <tests/unicode/helpers/test.h>

int main(int argc, char* argv[]) {
#ifdef SIMDUTF_INTERNAL_TESTS
    bool any_added = false;
    for (const auto& implementation : turbo::get_available_implementations()) {
        for (const auto& test : implementation->internal_tests()) {
            turbo::test::test_procedures().push_back(turbo::test::test_entry {
                test.name,
                test.procedure,
            });
            any_added = true;
        }
    }

    if (not any_added) {
        puts("None of implementations provides internal tests, skipping.");
        return 0;
    }

    const auto cmdline = turbo::test::CommandLine::parse(argc, argv);
    turbo::test::run(cmdline);
#endif
    return 0;
}
