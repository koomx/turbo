#include <turbo/unicode/utf.h>
#include <turbo/unicode/engine/isa_select.h>

#include <cstdio>
#include <cstdlib>
#include <string>

int main() {
    std::string source = "La vie est belle.";
    for (auto* implementation : turbo::UnicodeRegistry::get_avail_isa()) {
        bool validutf8 = implementation->validate_utf8(source.c_str(), source.size());
        if (!validutf8) {
            return EXIT_FAILURE;
        }
        printf("%.*s: %.*s\n", int(implementation->name().size()),
            implementation->name().data(),
            int(implementation->description().size()),
            implementation->description().data());
    }
    bool validutf8 = turbo::validate_utf8(source.c_str(), source.size());
    if (!validutf8) {
        return EXIT_FAILURE;
    }
    auto* best = turbo::UnicodeRegistry::get_best_isa();
    printf("best: %.*s\n", int(best->name().size()), best->name().data());
    return EXIT_SUCCESS;
}
