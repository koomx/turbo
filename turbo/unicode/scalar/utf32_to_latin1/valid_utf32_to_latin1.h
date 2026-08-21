#ifndef UNICODE_VALID_UTF32_TO_LATIN1_H
#define UNICODE_VALID_UTF32_TO_LATIN1_H

#include <cstring>

namespace turbo {
    namespace scalar {
        namespace {
            namespace utf32_to_latin1 {

                template <typename ReadPtr, typename WritePtr>
                 size_t convert_valid(ReadPtr data, size_t len,
                    WritePtr latin1_output) {
                    static_assert(
                        std::is_same<typename std::decay<decltype(*data)>::type, uint32_t>::value,
                        "dereferencing the data pointer must result in a uint32_t");
                    auto start = latin1_output;
                    uint32_t utf32_char;
                    size_t pos = 0;

                    while (pos < len) {
                        utf32_char = data[pos];
                            if (pos + 2 <= len) {
                                // if it is safe to read 8 more bytes, check that they are Latin1
                                uint64_t v;
                                std::memcpy(&v, data + pos, sizeof(uint64_t));
                                if ((v & 0xFFFFFF00FFFFFF00) == 0) {
                                    *latin1_output++ = char(data[pos]);
                                    *latin1_output++ = char(data[pos + 1]);
                                    pos += 2;
                                    continue;
                                } else {
                                    // output can not be represented in latin1
                                    return 0;
                                }
                            }
                        if ((utf32_char & 0xFFFFFF00) == 0) {
                            *latin1_output++ = char(utf32_char);
                        } else {
                            // output can not be represented in latin1
                            return 0;
                        }
                        pos++;
                    }
                    return latin1_output - start;
                }

                KUMO_FORCE_INLINE size_t convert_valid(const char32_t* buf, size_t len,
                    char* latin1_output) {
                    return convert_valid(reinterpret_cast<const uint32_t*>(buf), len,
                        latin1_output);
                }

            } // namespace utf32_to_latin1
        } // unnamed namespace
    } // namespace scalar
} // namespace turbo

#endif
