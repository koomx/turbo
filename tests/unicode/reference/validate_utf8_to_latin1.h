#include <turbo/unicode/engine/portability.h>

namespace turbo {
    namespace tests {
        namespace reference {

             [[nodiscard]] bool validate_utf8_to_latin1(const char* buf,
                size_t len) noexcept;

        } // namespace reference
    } // namespace tests
} // namespace turbo
