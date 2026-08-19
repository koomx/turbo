#include <turbo/unicode/engine/common_defs.h>
#include <turbo/unicode/text_encoding.h>

namespace turbo {
    namespace tests {
        namespace reference {
             [[nodiscard]] bool
            validate_utf16_to_latin1(turbo::endianness utf16_endianness,
                const char16_t* buf, size_t len) noexcept;

        }
    } // namespace tests
} // namespace turbo
