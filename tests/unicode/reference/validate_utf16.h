#include <turbo/unicode/engine/common_defs.h>
#include <turbo/unicode/text_encoding.h>

namespace turbo {
    namespace tests {
        namespace reference {
            // validate UTF-16
            simdutf_warn_unused bool validate_utf16(endianness utf16_endianness,
                const char16_t* buf,
                size_t len) noexcept;

        } // namespace reference
    } // namespace tests
} // namespace turbo
