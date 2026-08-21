// Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#pragma once

#include <cstring>

namespace turbo {
    namespace scalar {
        namespace {
            namespace utf8 {

                // credit: based on code from Google Fuchsia (Apache Licensed)
                template <class BytePtr>
                  [[nodiscard]] bool validate(BytePtr data,
                    size_t len) noexcept {
                    static_assert(
                        std::is_same<typename std::decay<decltype(*data)>::type, uint8_t>::value,
                        "dereferencing the data pointer must UnicodeResult in a uint8_t");
                    uint64_t pos = 0;
                    uint32_t code_point = 0;
                    while (pos < len) {
                        uint64_t next_pos;
                        { // check if the next 16 bytes are ascii.
                            next_pos = pos + 16;
                            if (next_pos <= len) { // if it is safe to read 16 more bytes, check
                                                   // that they are ascii
                                uint64_t v1 {};
                                std::memcpy(&v1, data + pos, sizeof(uint64_t));
                                uint64_t v2 {};
                                std::memcpy(&v2, data + pos + sizeof(uint64_t), sizeof(uint64_t));
                                uint64_t v { v1 | v2 };
                                if ((v & 0x8080808080808080) == 0) {
                                    pos = next_pos;
                                    continue;
                                }
                            }
                        }

                        unsigned char byte = data[pos];

                        while (byte < 0b10000000) {
                            if (++pos == len) {
                                return true;
                            }
                            byte = data[pos];
                        }

                        if ((byte & 0b11100000) == 0b11000000) {
                            next_pos = pos + 2;
                            if (next_pos > len) {
                                return false;
                            }
                            if ((data[pos + 1] & 0b11000000) != 0b10000000) {
                                return false;
                            }
                            // range check
                            code_point = (byte & 0b00011111) << 6 | (data[pos + 1] & 0b00111111);
                            if (code_point < 0x80) {
                                return false;
                            }
                        } else if ((byte & 0b11110000) == 0b11100000) {
                            next_pos = pos + 3;
                            if (next_pos > len) {
                                return false;
                            }
                            if ((data[pos + 1] & 0b11000000) != 0b10000000) {
                                return false;
                            }
                            if ((data[pos + 2] & 0b11000000) != 0b10000000) {
                                return false;
                            }
                            // range check
                            code_point = (byte & 0b00001111) << 12 | (data[pos + 1] & 0b00111111) << 6 | (data[pos + 2] & 0b00111111);
                            if ((code_point < 0x800) || (0xd7ff < code_point && code_point < 0xe000)) {
                                return false;
                            }
                        } else if ((byte & 0b11111000) == 0b11110000) { // 0b11110000
                            next_pos = pos + 4;
                            if (next_pos > len) {
                                return false;
                            }
                            if ((data[pos + 1] & 0b11000000) != 0b10000000) {
                                return false;
                            }
                            if ((data[pos + 2] & 0b11000000) != 0b10000000) {
                                return false;
                            }
                            if ((data[pos + 3] & 0b11000000) != 0b10000000) {
                                return false;
                            }
                            // range check
                            code_point = (byte & 0b00000111) << 18 | (data[pos + 1] & 0b00111111) << 12 | (data[pos + 2] & 0b00111111) << 6 | (data[pos + 3] & 0b00111111);
                            if (code_point <= 0xffff || 0x10ffff < code_point) {
                                return false;
                            }
                        } else {
                            // we may have a continuation
                            return false;
                        }
                        pos = next_pos;
                    }
                    return true;
                }

                [[nodiscard]] KUMO_FORCE_INLINE bool validate(const char* buf,
                    size_t len) noexcept {
                    return validate(reinterpret_cast<const uint8_t*>(buf), len);
                }

                template <class BytePtr>
                  [[nodiscard]] UnicodeResult
                validate_with_errors(BytePtr data, size_t len) noexcept {
                    static_assert(
                        std::is_same<typename std::decay<decltype(*data)>::type, uint8_t>::value,
                        "dereferencing the data pointer must UnicodeResult in a uint8_t");
                    size_t pos = 0;
                    uint32_t code_point = 0;
                    while (pos < len) {
                        // check of the next 16 bytes are ascii.
                        size_t next_pos = pos + 16;
                        if (next_pos <= len) { // if it is safe to read 16 more bytes, check that they are ascii
                            uint64_t v1;
                            std::memcpy(&v1, data + pos, sizeof(uint64_t));
                            uint64_t v2;
                            std::memcpy(&v2, data + pos + sizeof(uint64_t), sizeof(uint64_t));
                            uint64_t v { v1 | v2 };
                            if ((v & 0x8080808080808080) == 0) {
                                pos = next_pos;
                                continue;
                            }
                        }
                        unsigned char byte = data[pos];

                        while (byte < 0b10000000) {
                            if (++pos == len) {
                                return UnicodeResult(UnicodeError::SUCCESS, len);
                            }
                            byte = data[pos];
                        }

                        if ((byte & 0b11100000) == 0b11000000) {
                            next_pos = pos + 2;
                            if (next_pos > len) {
                                return UnicodeResult(UnicodeError::TOO_SHORT, pos);
                            }
                            if ((data[pos + 1] & 0b11000000) != 0b10000000) {
                                return UnicodeResult(UnicodeError::TOO_SHORT, pos);
                            }
                            // range check
                            code_point = (byte & 0b00011111) << 6 | (data[pos + 1] & 0b00111111);
                            if (code_point < 0x80) {
                                return UnicodeResult(UnicodeError::OVERLONG, pos);
                            }
                        } else if ((byte & 0b11110000) == 0b11100000) {
                            next_pos = pos + 3;
                            if (next_pos > len) {
                                return UnicodeResult(UnicodeError::TOO_SHORT, pos);
                            }
                            if ((data[pos + 1] & 0b11000000) != 0b10000000) {
                                return UnicodeResult(UnicodeError::TOO_SHORT, pos);
                            }
                            if ((data[pos + 2] & 0b11000000) != 0b10000000) {
                                return UnicodeResult(UnicodeError::TOO_SHORT, pos);
                            }
                            // range check
                            code_point = (byte & 0b00001111) << 12 | (data[pos + 1] & 0b00111111) << 6 | (data[pos + 2] & 0b00111111);
                            if (code_point < 0x800) {
                                return UnicodeResult(UnicodeError::OVERLONG, pos);
                            }
                            if (0xd7ff < code_point && code_point < 0xe000) {
                                return UnicodeResult(UnicodeError::SURROGATE, pos);
                            }
                        } else if ((byte & 0b11111000) == 0b11110000) { // 0b11110000
                            next_pos = pos + 4;
                            if (next_pos > len) {
                                return UnicodeResult(UnicodeError::TOO_SHORT, pos);
                            }
                            if ((data[pos + 1] & 0b11000000) != 0b10000000) {
                                return UnicodeResult(UnicodeError::TOO_SHORT, pos);
                            }
                            if ((data[pos + 2] & 0b11000000) != 0b10000000) {
                                return UnicodeResult(UnicodeError::TOO_SHORT, pos);
                            }
                            if ((data[pos + 3] & 0b11000000) != 0b10000000) {
                                return UnicodeResult(UnicodeError::TOO_SHORT, pos);
                            }
                            // range check
                            code_point = (byte & 0b00000111) << 18 | (data[pos + 1] & 0b00111111) << 12 | (data[pos + 2] & 0b00111111) << 6 | (data[pos + 3] & 0b00111111);
                            if (code_point <= 0xffff) {
                                return UnicodeResult(UnicodeError::OVERLONG, pos);
                            }
                            if (0x10ffff < code_point) {
                                return UnicodeResult(UnicodeError::TOO_LARGE, pos);
                            }
                        } else {
                            // we either have too many continuation bytes or an invalid leading byte
                            if ((byte & 0b11000000) == 0b10000000) {
                                return UnicodeResult(UnicodeError::TOO_LONG, pos);
                            } else {
                                return UnicodeResult(UnicodeError::HEADER_BITS, pos);
                            }
                        }
                        pos = next_pos;
                    }
                    return UnicodeResult(UnicodeError::SUCCESS, len);
                }

                [[nodiscard]] KUMO_FORCE_INLINE UnicodeResult
                validate_with_errors(const char* buf, size_t len) noexcept {
                    return validate_with_errors(reinterpret_cast<const uint8_t*>(buf), len);
                }

                // Finds the previous leading byte starting backward from buf and validates with
                // errors from there Used to pinpoint the location of an error when an invalid
                // chunk is detected We assume that the stream starts with a leading byte, and
                // to check that it is the case, we ask that you pass a pointer to the start of
                // the stream (start). Note that the resulting count is underflowed if an error
                // is encountered in the rewinded segment.
                [[nodiscard]] inline UnicodeResult rewind_and_validate_with_errors(
                    const char* start, const char* buf, size_t len) noexcept {
                    // First check that we start with a leading byte
                    if ((*start & 0b11000000) == 0b10000000) {
                        return UnicodeResult(UnicodeError::TOO_LONG, 0);
                    }
                    size_t extra_len { 0 };
                    // A leading byte cannot be further than 4 bytes away
                    for (int i = 0; i < 5; i++) {
                        unsigned char byte = *buf;
                        if ((byte & 0b11000000) != 0b10000000) {
                            break;
                        } else {
                            buf--;
                            extra_len++;
                        }
                    }

                    UnicodeResult res = validate_with_errors(buf, len + extra_len);
                    res.count -= extra_len; // Might underflow
                    return res;
                }

                template <typename InputPtr>
                 size_t count_code_points(InputPtr data, size_t len) {
                    size_t counter { 0 };
                    for (size_t i = 0; i < len; i++) {
                        // -65 is 0b10111111, anything larger in two-complement's should start a new
                        // code point.
                        if (int8_t(data[i]) > -65) {
                            counter++;
                        }
                    }
                    return counter;
                }

                template <typename InputPtr>
                 size_t utf16_length_from_utf8(InputPtr data, size_t len) {
                    size_t counter { 0 };
                    for (size_t i = 0; i < len; i++) {
                        if (int8_t(data[i]) > -65) {
                            counter++;
                        }
                        if (uint8_t(data[i]) >= 240) {
                            counter++;
                        }
                    }
                    return counter;
                }

                template <typename InputPtr>
                 [[nodiscard]]  size_t
                trim_partial_utf8(InputPtr input, size_t length) {
                    if (length < 3) {
                        switch (length) {
                        case 2:
                            if (uint8_t(input[length - 1]) >= 0xc0) {
                                return length - 1;
                            } // 2-, 3- and 4-byte characters with only 1 byte left
                            if (uint8_t(input[length - 2]) >= 0xe0) {
                                return length - 2;
                            } // 3- and 4-byte characters with only 2 bytes left
                            return length;
                        case 1:
                            if (uint8_t(input[length - 1]) >= 0xc0) {
                                return length - 1;
                            } // 2-, 3- and 4-byte characters with only 1 byte left
                            return length;
                        case 0:
                            return length;
                        }
                    }
                    if (uint8_t(input[length - 1]) >= 0xc0) {
                        return length - 1;
                    } // 2-, 3- and 4-byte characters with only 1 byte left
                    if (uint8_t(input[length - 2]) >= 0xe0) {
                        return length - 2;
                    } // 3- and 4-byte characters with only 1 byte left
                    if (uint8_t(input[length - 3]) >= 0xf0) {
                        return length - 3;
                    } // 4-byte characters with only 3 bytes left
                    return length;
                }

            } // namespace utf8
        } // unnamed namespace
    } // namespace scalar
} // namespace turbo
