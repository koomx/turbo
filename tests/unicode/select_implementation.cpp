#include <turbo/unicode/utf.h>

#include <cstdio>
#include <cstdlib>
#include <string>

int main() {
    // This is just a demonstration, not actual testing required.
    std::string source = "La vie est belle.";
    std::string chosen_implementation;
    for (auto& implementation : turbo::get_available_implementations()) {
        if (!implementation->supported_by_runtime_system()) {
            continue;
        }
        bool validutf8 = implementation->validate_utf8(source.c_str(), source.size());
        if (!validutf8) {
            return EXIT_FAILURE;
        }
        printf("%.*s: %.*s\n", int(implementation->name().size()),
            implementation->name().data(),
            int(implementation->description().size()),
            implementation->description().data());
        chosen_implementation = std::string(implementation->name());
    }
    auto my_implementation = turbo::get_available_implementations()[chosen_implementation];
    if (!my_implementation) {
        return EXIT_FAILURE;
    }
    if (!my_implementation->supported_by_runtime_system()) {
        return EXIT_FAILURE;
    }
    turbo::get_active_implementation() = my_implementation;
    bool validutf8 = turbo::validate_utf8(source.c_str(), source.size());
    if (!validutf8) {
        return EXIT_FAILURE;
    }
    if (turbo::get_active_implementation()->name() != chosen_implementation) {
        return EXIT_FAILURE;
    }
    printf("Manually selected: %.*s\n",
        int(turbo::get_active_implementation()->name().size()),
        turbo::get_active_implementation()->name().data());
    return EXIT_SUCCESS;
}
