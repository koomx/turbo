#ifndef UNICODE_VALID_UTF32_TO_UTF16_H
#define UNICODE_VALID_UTF32_TO_UTF16_H

namespace turbo {
    namespace scalar {
        namespace {
            namespace utf32_to_utf16 {

                template <Endian big_endian>
                 size_t convert_valid(const char32_t* data, size_t len,
                    char16_t* utf16_output) {
                    size_t pos = 0;
                    char16_t* start { utf16_output };
                    while (pos < len) {
                        uint32_t word = data[pos];
                        if ((word & 0xFFFF0000) == 0) {
                            // will not generate a surrogate pair
                            *utf16_output++ = !match_system(big_endian)
                                ? char16_t(u16_byteswap(word))
                                : char16_t(word);
                            pos++;
                        } else {
                            // will generate a surrogate pair
                            word -= 0x10000;
                            uint16_t high_surrogate = uint16_t(0xD800 + (word >> 10));
                            uint16_t low_surrogate = uint16_t(0xDC00 + (word & 0x3FF));
                            if constexpr (!match_system(big_endian)) {
                                high_surrogate = u16_byteswap(high_surrogate);
                                low_surrogate = u16_byteswap(low_surrogate);
                            }
                            *utf16_output++ = char16_t(high_surrogate);
                            *utf16_output++ = char16_t(low_surrogate);
                            pos++;
                        }
                    }
                    return utf16_output - start;
                }

            } // namespace utf32_to_utf16
        } // unnamed namespace
    } // namespace scalar
} // namespace turbo

#endif
