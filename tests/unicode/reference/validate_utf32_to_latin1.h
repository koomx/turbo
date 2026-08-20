#include <turbo/unicode/engine/portability.h>

namespace turbo {
    namespace tests {
        namespace reference {

             [[nodiscard]] bool validate_utf32_to_latin1(const char32_t* buf,
                size_t len) noexcept;

        }
    } // namespace tests
} // namespace turbo
