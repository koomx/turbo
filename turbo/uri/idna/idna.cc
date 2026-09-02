/* auto-generated on 2023-09-19 15:58:51 -0400. Do not edit! */
/* begin file src/idna.cpp */
/* begin file src/unicode_transcoding.cpp */

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <turbo/uri/idna/idna.h>

#include "turbo/strings/ascii.h"

#include <turbo/strings/match.h>
#include <turbo/unicode/utf.h>
#include <turbo/uri/idna/tables.h>

namespace turbo::idna {
    /*
    size_t utf8_to_utf32(const char* buf, size_t len, char32_t* utf32_output) {
        const uint8_t* data = reinterpret_cast<const uint8_t*>(buf);
        size_t pos = 0;
        char32_t* start { utf32_output };
        while (pos < len) {
            // try to convert the next block of 16 ASCII bytes
            if (pos + 16 <= len) { // if it is safe to read 16 more
                                   // bytes, check that they are ascii
                uint64_t v1;
                std::memcpy(&v1, data + pos, sizeof(uint64_t));
                uint64_t v2;
                std::memcpy(&v2, data + pos + sizeof(uint64_t), sizeof(uint64_t));
                uint64_t v { v1 | v2 };
                if ((v & 0x8080808080808080) == 0) {
                    size_t final_pos = pos + 16;
                    while (pos < final_pos) {
                        *utf32_output++ = char32_t(buf[pos]);
                        pos++;
                    }
                    continue;
                }
            }
            uint8_t leading_byte = data[pos]; // leading byte
            if (leading_byte < 0b10000000) {
                // converting one ASCII byte !!!
                *utf32_output++ = char32_t(leading_byte);
                pos++;
            } else if ((leading_byte & 0b11100000) == 0b11000000) {
                // We have a two-byte UTF-8
                if (pos + 1 >= len) {
                    return 0;
                } // minimal bound checking
                if ((data[pos + 1] & 0b11000000) != 0b10000000) {
                    return 0;
                }
                // range check
                uint32_t code_point = (leading_byte & 0b00011111) << 6 | (data[pos + 1] & 0b00111111);
                if (code_point < 0x80 || 0x7ff < code_point) {
                    return 0;
                }
                *utf32_output++ = char32_t(code_point);
                pos += 2;
            } else if ((leading_byte & 0b11110000) == 0b11100000) {
                // We have a three-byte UTF-8
                if (pos + 2 >= len) {
                    return 0;
                } // minimal bound checking

                if ((data[pos + 1] & 0b11000000) != 0b10000000) {
                    return 0;
                }
                if ((data[pos + 2] & 0b11000000) != 0b10000000) {
                    return 0;
                }
                // range check
                uint32_t code_point = (leading_byte & 0b00001111) << 12 | (data[pos + 1] & 0b00111111) << 6 | (data[pos + 2] & 0b00111111);
                if (code_point < 0x800 || 0xffff < code_point || (0xd7ff < code_point && code_point < 0xe000)) {
                    return 0;
                }
                *utf32_output++ = char32_t(code_point);
                pos += 3;
            } else if ((leading_byte & 0b11111000) == 0b11110000) { // 0b11110000
                // we have a 4-byte UTF-8 word.
                if (pos + 3 >= len) {
                    return 0;
                } // minimal bound checking
                if ((data[pos + 1] & 0b11000000) != 0b10000000) {
                    return 0;
                }
                if ((data[pos + 2] & 0b11000000) != 0b10000000) {
                    return 0;
                }
                if ((data[pos + 3] & 0b11000000) != 0b10000000) {
                    return 0;
                }

                // range check
                uint32_t code_point = (leading_byte & 0b00000111) << 18 | (data[pos + 1] & 0b00111111) << 12 | (data[pos + 2] & 0b00111111) << 6 | (data[pos + 3] & 0b00111111);
                if (code_point <= 0xffff || 0x10ffff < code_point) {
                    return 0;
                }
                *utf32_output++ = char32_t(code_point);
                pos += 4;
            } else {
                return 0;
            }
        }
        return utf32_output - start;
    }

    size_t utf8_length_from_utf32(const char32_t* buf, size_t len) {
        // We are not BOM aware.
        const uint32_t* p = reinterpret_cast<const uint32_t*>(buf);
        size_t counter { 0 };
        for (size_t i = 0; i != len; ++i) {
            ++counter; // ASCII
            counter += static_cast<size_t>(p[i] > 0x7F); // two-byte
            counter += static_cast<size_t>(p[i] > 0x7FF); // three-byte
            counter += static_cast<size_t>(p[i] > 0xFFFF); // four-bytes
        }
        return counter;
    }

    size_t utf32_length_from_utf8(const char* buf, size_t len) {
        const int8_t* p = reinterpret_cast<const int8_t*>(buf);
        return std::count_if(p, std::next(p, len), [](int8_t c) {
            // -65 is 0b10111111, anything larger in two-complement's
            // should start a new code point.
            return c > -65;
        });
    }

    size_t utf32_to_utf8(const char32_t* buf, size_t len, char* utf8_output) {
        const uint32_t* data = reinterpret_cast<const uint32_t*>(buf);
        size_t pos = 0;
        char* start { utf8_output };
        while (pos < len) {
            // try to convert the next block of 2 ASCII characters
            if (pos + 2 <= len) { // if it is safe to read 8 more
                                  // bytes, check that they are ascii
                uint64_t v;
                std::memcpy(&v, data + pos, sizeof(uint64_t));
                if ((v & 0xFFFFFF80FFFFFF80) == 0) {
                    *utf8_output++ = char(buf[pos]);
                    *utf8_output++ = char(buf[pos + 1]);
                    pos += 2;
                    continue;
                }
            }
            uint32_t word = data[pos];
            if ((word & 0xFFFFFF80) == 0) {
                // will generate one UTF-8 bytes
                *utf8_output++ = char(word);
                pos++;
            } else if ((word & 0xFFFFF800) == 0) {
                // will generate two UTF-8 bytes
                // we have 0b110XXXXX 0b10XXXXXX
                *utf8_output++ = char((word >> 6) | 0b11000000);
                *utf8_output++ = char((word & 0b111111) | 0b10000000);
                pos++;
            } else if ((word & 0xFFFF0000) == 0) {
                // will generate three UTF-8 bytes
                // we have 0b1110XXXX 0b10XXXXXX 0b10XXXXXX
                if (word >= 0xD800 && word <= 0xDFFF) {
                    return 0;
                }
                *utf8_output++ = char((word >> 12) | 0b11100000);
                *utf8_output++ = char(((word >> 6) & 0b111111) | 0b10000000);
                *utf8_output++ = char((word & 0b111111) | 0b10000000);
                pos++;
            } else {
                // will generate four UTF-8 bytes
                // we have 0b11110XXX 0b10XXXXXX 0b10XXXXXX
                // 0b10XXXXXX
                if (word > 0x10FFFF) {
                    return 0;
                }
                *utf8_output++ = char((word >> 18) | 0b11110000);
                *utf8_output++ = char(((word >> 12) & 0b111111) | 0b10000000);
                *utf8_output++ = char(((word >> 6) & 0b111111) | 0b10000000);
                *utf8_output++ = char((word & 0b111111) | 0b10000000);
                pos++;
            }
        }
        return utf8_output - start;
    }
    */

    // This can be greatly accelerated. For now we just use a simply
    // binary search. In practice, you should *not* do that.
    uint32_t find_range_index(uint32_t key) {
        ////////////////
        // This could be implemented with std::lower_bound, but we roll our own
        // because we want to allow further optimizations in the future.
        ////////////////
        uint32_t len = std::size(IdnaTables::table);
        uint32_t low = 0;
        uint32_t high = len - 1;
        while (low <= high) {
            uint32_t middle_index = (low + high) >> 1; // cannot overflow
            uint32_t middle_value = IdnaTables::table[middle_index][0];
            if (middle_value < key) {
                low = middle_index + 1;
            } else if (middle_value > key) {
                high = middle_index - 1;
            } else {
                return middle_index; // perfect match
            }
        }
        return low == 0 ? 0 : low - 1;
    }

    // Map the characters according to IDNA, returning the empty string on error.
    std::u32string map(std::u32string_view input) {
        //  [Map](https://www.unicode.org/reports/tr46/#ProcessingStepMap).
        //  For each code point in the domain_name string, look up the status
        //  value in Section 5, [IDNA Mapping
        //  Table](https://www.unicode.org/reports/tr46/#IDNA_Mapping_Table),
        //  and take the following actions:
        //    * disallowed: Leave the code point unchanged in the string, and
        //    record that there was an error.
        //    * ignored: Remove the code point from the string. This is
        //    equivalent to mapping the code point to an empty string.
        //    * mapped: Replace the code point in the string by the value for
        //    the mapping in Section 5, [IDNA Mapping
        //    Table](https://www.unicode.org/reports/tr46/#IDNA_Mapping_Table).
        //    * valid: Leave the code point unchanged in the string.
        static std::u32string error = U"";
        std::u32string answer;
        answer.reserve(input.size());
        for (char32_t x : input) {
            size_t index = find_range_index(x);
            uint32_t descriptor = IdnaTables::table[index][1];
            uint8_t code = uint8_t(descriptor);
            switch (code) {
            case 0:
                break; // nothing to do, ignored
            case 1:
                answer.push_back(x); // valid, we just copy it to output
                break;
            case 2:
                return error; // disallowed
            // case 3 :
            default:
                // We have a mapping
                {
                    size_t char_count = (descriptor >> 24);
                    uint16_t char_index = uint16_t(descriptor >> 8);
                    for (size_t idx = char_index; idx < char_index + char_count; idx++) {
                        answer.push_back(IdnaTables::mappings[idx]);
                    }
                }
            }
        }
        return answer;
    }

    // See
    // https://github.com/uni-algo/uni-algo/blob/c612968c5ed3ace39bde4c894c24286c5f2c7fe2/include/uni_algo/impl/impl_norm.h#L467
    constexpr char32_t hangul_sbase = 0xAC00;
    constexpr char32_t hangul_tbase = 0x11A7;
    constexpr char32_t hangul_vbase = 0x1161;
    constexpr char32_t hangul_lbase = 0x1100;
    constexpr char32_t hangul_lcount = 19;
    constexpr char32_t hangul_vcount = 21;
    constexpr char32_t hangul_tcount = 28;
    constexpr char32_t hangul_ncount = hangul_vcount * hangul_tcount;
    constexpr char32_t hangul_scount = hangul_lcount * hangul_vcount * hangul_tcount;

    std::pair<bool, size_t> compute_decomposition_length(
        const std::u32string_view input) noexcept {
        bool decomposition_needed { false };
        size_t additional_elements { 0 };
        for (char32_t current_character : input) {
            size_t decomposition_length { 0 };

            if (current_character >= hangul_sbase && current_character < hangul_sbase + hangul_scount) {
                decomposition_length = 2;
                if ((current_character - hangul_sbase) % hangul_tcount) {
                    decomposition_length = 3;
                }
            } else if (current_character < 0x110000) {
                const uint8_t di = IdnaTables::decomposition_index[current_character >> 8];
                const uint16_t* const decomposition = IdnaTables::decomposition_block[di] + (current_character % 256);
                decomposition_length = (decomposition[1] >> 2) - (decomposition[0] >> 2);
                if ((decomposition_length > 0) && (decomposition[0] & 1)) {
                    decomposition_length = 0;
                }
            }
            if (decomposition_length != 0) {
                decomposition_needed = true;
                additional_elements += decomposition_length - 1;
            }
        }
        return { decomposition_needed, additional_elements };
    }

    void decompose(std::u32string& input, size_t additional_elements) {
        input.resize(input.size() + additional_elements);
        for (size_t descending_idx = input.size(),
                    input_count = descending_idx - additional_elements;
            input_count--;) {
            if (input[input_count] >= hangul_sbase && input[input_count] < hangul_sbase + hangul_scount) {
                // Hangul decomposition.
                char32_t s_index = input[input_count] - hangul_sbase;
                if (s_index % hangul_tcount != 0) {
                    input[--descending_idx] = hangul_tbase + s_index % hangul_tcount;
                }
                input[--descending_idx] = hangul_vbase + (s_index % hangul_ncount) / hangul_tcount;
                input[--descending_idx] = hangul_lbase + s_index / hangul_ncount;
            } else if (input[input_count] < 0x110000) {
                // Check decomposition_data.
                const uint16_t* decomposition = IdnaTables::decomposition_block[IdnaTables::decomposition_index[input[input_count] >> 8]] + (input[input_count] % 256);
                uint16_t decomposition_length = (decomposition[1] >> 2) - (decomposition[0] >> 2);
                if (decomposition_length > 0 && (decomposition[0] & 1)) {
                    decomposition_length = 0;
                }
                if (decomposition_length > 0) {
                    // Non-recursive decomposition.
                    while (decomposition_length-- > 0) {
                        input[--descending_idx] = IdnaTables::decomposition_data[(decomposition[0] >> 2) + decomposition_length];
                    }
                } else {
                    // No decomposition.
                    input[--descending_idx] = input[input_count];
                }
            } else {
                // Non-Unicode character.
                input[--descending_idx] = input[input_count];
            }
        }
    }

    uint8_t get_ccc(char32_t c) noexcept {
        return c < 0x110000 ? IdnaTables::canonical_combining_class_block
                                  [IdnaTables::canonical_combining_class_index[c >> 8]][c % 256]
                            : 0;
    }

    void sort_marks(std::u32string& input) {
        for (size_t idx = 1; idx < input.size(); idx++) {
            uint8_t ccc = get_ccc(input[idx]);
            if (ccc == 0) {
                continue;
            } // Skip non-combining characters.
            auto current_character = input[idx];
            size_t back_idx = idx;
            while (back_idx != 0 && get_ccc(input[back_idx - 1]) > ccc) {
                input[back_idx] = input[back_idx - 1];
                back_idx--;
            }
            input[back_idx] = current_character;
        }
    }

    void decompose_nfc(std::u32string& input) {
        /**
         * Decompose the domain_name string to Unicode Normalization Form C.
         * @see https://www.unicode.org/reports/tr46/#ProcessingStepDecompose
         */
        auto [decomposition_needed, additional_elements] = compute_decomposition_length(input);
        if (decomposition_needed) {
            decompose(input, additional_elements);
        }
        sort_marks(input);
    }

    void compose(std::u32string& input) {
        /**
         * Compose the domain_name string to Unicode Normalization Form C.
         * @see https://www.unicode.org/reports/tr46/#ProcessingStepCompose
         */
        size_t input_count { 0 };
        size_t composition_count { 0 };
        for (; input_count < input.size(); input_count++, composition_count++) {
            input[composition_count] = input[input_count];
            if (input[input_count] >= hangul_lbase && input[input_count] < hangul_lbase + hangul_lcount) {
                if (input_count + 1 < input.size() && input[input_count + 1] >= hangul_vbase && input[input_count + 1] < hangul_vbase + hangul_vcount) {
                    input[composition_count] = hangul_sbase + ((input[input_count] - hangul_lbase) * hangul_vcount + input[input_count + 1] - hangul_vbase) * hangul_tcount;
                    input_count++;
                    if (input_count + 1 < input.size() && input[input_count + 1] > hangul_tbase && input[input_count + 1] < hangul_tbase + hangul_tcount) {
                        input[composition_count] += input[++input_count] - hangul_tbase;
                    }
                }
            } else if (input[input_count] >= hangul_sbase && input[input_count] < hangul_sbase + hangul_scount) {
                if ((input[input_count] - hangul_sbase) % hangul_tcount && input_count + 1 < input.size() && input[input_count + 1] > hangul_tbase && input[input_count + 1] < hangul_tbase + hangul_tcount) {
                    input[composition_count] += input[++input_count] - hangul_tbase;
                }
            } else if (input[input_count] < 0x110000) {
                const uint16_t* composition = &IdnaTables::composition_block[IdnaTables::composition_index[input[input_count] >> 8]]
                                                                [input[input_count] % 256];
                size_t initial_composition_count = composition_count;
                for (int32_t previous_ccc = -1; input_count + 1 < input.size();
                    input_count++) {
                    uint8_t ccc = get_ccc(input[input_count + 1]);

                    if (composition[1] != composition[0] && previous_ccc < ccc) {
                        // Try finding a composition.
                        uint16_t left = composition[0];
                        uint16_t right = composition[1];
                        while (left + 2 < right) {
                            // mean without overflow
                            uint16_t middle = left + (((right - left) >> 1) & ~1);
                            if (IdnaTables::composition_data[middle] <= input[input_count + 1]) {
                                left = middle;
                            }
                            if (IdnaTables::composition_data[middle] >= input[input_count + 1]) {
                                right = middle;
                            }
                        }
                        if (IdnaTables::composition_data[left] == input[input_count + 1]) {
                            input[initial_composition_count] = IdnaTables::composition_data[left + 1];
                            composition = &IdnaTables::composition_block
                                              [IdnaTables::composition_index[IdnaTables::composition_data[left + 1] >> 8]]
                                              [IdnaTables::composition_data[left + 1] % 256];
                            continue;
                        }
                    }

                    if (ccc == 0) {
                        break;
                    } // Not a combining character.
                    previous_ccc = ccc;
                    input[++composition_count] = input[input_count + 1];
                }
            }
        }

        if (composition_count < input_count) {
            input.resize(composition_count);
        }
    }

    void normalize(std::u32string& input) {
        /**
         * Normalize the domain_name string to Unicode Normalization Form C.
         * @see https://www.unicode.org/reports/tr46/#ProcessingStepNormalize
         */
        decompose_nfc(input);
        compose(input);
    }

    constexpr int32_t base = 36;
    constexpr int32_t tmin = 1;
    constexpr int32_t tmax = 26;
    constexpr int32_t skew = 38;
    constexpr int32_t damp = 700;
    constexpr int32_t initial_bias = 72;
    constexpr uint32_t initial_n = 128;

    static constexpr int32_t char_to_digit_value(char value) {
        if (value >= 'a' && value <= 'z')
            return value - 'a';
        if (value >= '0' && value <= '9')
            return value - '0' + 26;
        return -1;
    }

    static constexpr char digit_to_char(int32_t digit) {
        return digit < 26 ? char(digit + 97) : char(digit + 22);
    }

    static constexpr int32_t adapt(int32_t d, int32_t n, bool firsttime) {
        if (firsttime) {
            d = d / damp;
        } else {
            d = d / 2;
        }
        d += d / n;
        int32_t k = 0;
        while (d > ((base - tmin) * tmax) / 2) {
            d /= base - tmin;
            k += base;
        }
        return k + (((base - tmin + 1) * d) / (d + skew));
    }

    bool punycode_to_utf32(std::string_view input, std::u32string& out) {
        int32_t written_out { 0 };
        out.reserve(out.size() + input.size());
        uint32_t n = initial_n;
        int32_t i = 0;
        int32_t bias = initial_bias;
        // grab ascii content
        size_t end_of_ascii = input.find_last_of('-');
        if (end_of_ascii != std::string_view::npos) {
            for (uint8_t c : input.substr(0, end_of_ascii)) {
                if (c >= 0x80) {
                    return false;
                }
                out.push_back(c);
                written_out++;
            }
            input.remove_prefix(end_of_ascii + 1);
        }
        while (!input.empty()) {
            int32_t oldi = i;
            int32_t w = 1;
            for (int32_t k = base;; k += base) {
                if (input.empty()) {
                    return false;
                }
                uint8_t code_point = input.front();
                input.remove_prefix(1);
                int32_t digit = char_to_digit_value(code_point);
                if (digit < 0) {
                    return false;
                }
                if (digit > (0x7fffffff - i) / w) {
                    return false;
                }
                i = i + digit * w;
                int32_t t = k <= bias ? tmin : k >= bias + tmax ? tmax
                                                                : k - bias;
                if (digit < t) {
                    break;
                }
                if (w > 0x7fffffff / (base - t)) {
                    return false;
                }
                w = w * (base - t);
            }
            bias = adapt(i - oldi, written_out + 1, oldi == 0);
            if (i / (written_out + 1) > int32_t(0x7fffffff - n)) {
                return false;
            }
            n = n + i / (written_out + 1);
            i = i % (written_out + 1);
            if (n < 0x80) {
                return false;
            }
            out.insert(out.begin() + i, n);
            written_out++;
            ++i;
        }

        return true;
    }

    bool verify_punycode(std::string_view input) {
        size_t written_out { 0 };
        uint32_t n = initial_n;
        int32_t i = 0;
        int32_t bias = initial_bias;
        // grab ascii content
        size_t end_of_ascii = input.find_last_of('-');
        if (end_of_ascii != std::string_view::npos) {
            for (uint8_t c : input.substr(0, end_of_ascii)) {
                if (c >= 0x80) {
                    return false;
                }
                written_out++;
            }
            input.remove_prefix(end_of_ascii + 1);
        }
        while (!input.empty()) {
            int32_t oldi = i;
            int32_t w = 1;
            for (int32_t k = base;; k += base) {
                if (input.empty()) {
                    return false;
                }
                uint8_t code_point = input.front();
                input.remove_prefix(1);
                int32_t digit = char_to_digit_value(code_point);
                if (digit < 0) {
                    return false;
                }
                if (digit > (0x7fffffff - i) / w) {
                    return false;
                }
                i = i + digit * w;
                int32_t t = k <= bias ? tmin : k >= bias + tmax ? tmax
                                                                : k - bias;
                if (digit < t) {
                    break;
                }
                if (w > 0x7fffffff / (base - t)) {
                    return false;
                }
                w = w * (base - t);
            }
            bias = adapt(i - oldi, int32_t(written_out + 1), oldi == 0);
            if (i / (written_out + 1) > 0x7fffffff - n) {
                return false;
            }
            n = n + i / int32_t(written_out + 1);
            i = i % int32_t(written_out + 1);
            if (n < 0x80) {
                return false;
            }
            written_out++;
            ++i;
        }

        return true;
    }

    bool utf32_to_punycode(std::u32string_view input, std::string& out) {
        out.reserve(input.size() + out.size());
        uint32_t n = initial_n;
        int32_t d = 0;
        int32_t bias = initial_bias;
        size_t h = 0;
        // first push the ascii content
        for (uint32_t c : input) {
            if (c < 0x80) {
                ++h;
                out.push_back(char(c));
            }
            if (c > 0x10ffff || (c >= 0xd880 && c < 0xe000)) {
                return false;
            }
        }
        size_t b = h;
        if (b > 0) {
            out.push_back('-');
        }
        while (h < input.size()) {
            uint32_t m = 0x10FFFF;
            for (auto code_point : input) {
                if (code_point >= n && code_point < m)
                    m = code_point;
            }

            if ((m - n) > (0x7fffffff - d) / (h + 1)) {
                return false;
            }
            d = d + int32_t((m - n) * (h + 1));
            n = m;
            for (auto c : input) {
                if (c < n) {
                    if (d == 0x7fffffff) {
                        return false;
                    }
                    ++d;
                }
                if (c == n) {
                    int32_t q = d;
                    for (int32_t k = base;; k += base) {
                        int32_t t = k <= bias ? tmin : k >= bias + tmax ? tmax
                                                                        : k - bias;

                        if (q < t) {
                            break;
                        }
                        out.push_back(digit_to_char(t + ((q - t) % (base - t))));
                        q = (q - t) / (base - t);
                    }
                    out.push_back(digit_to_char(q));
                    bias = adapt(d, int32_t(h + 1), h == b);
                    d = 0;
                    ++h;
                }
            }
            ++d;
            ++n;
        }
        return true;
    }


    // CheckJoiners and CheckBidi are true for URL specification.

    // We return "" on error.
    static std::string from_ascii_to_ascii(std::string_view ut8_string) {
        static const std::string error = "";
        // copy and map
        // we could be more efficient by avoiding the copy when unnecessary.
        std::string mapped_string = turbo::str_to_lower(ut8_string);
        std::string out;
        size_t label_start = 0;

        while (label_start != mapped_string.size()) {
            size_t loc_dot = mapped_string.find('.', label_start);
            bool is_last_label = (loc_dot == std::string_view::npos);
            size_t label_size = is_last_label ? mapped_string.size() - label_start
                                              : loc_dot - label_start;
            size_t label_size_with_dot = is_last_label ? label_size : label_size + 1;
            std::string_view label_view(mapped_string.data() + label_start, label_size);
            label_start += label_size_with_dot;
            if (label_size == 0) {
                // empty label? Nothing to do.
            } else if (starts_with(label_view, "xn--")) {
                // The xn-- part is the expensive game.
                out.append(label_view);
                std::string_view puny_segment_ascii(
                    out.data() + out.size() - label_view.size() + 4,
                    label_view.size() - 4);
                std::u32string tmp_buffer;
                bool is_ok = turbo::idna::punycode_to_utf32(puny_segment_ascii, tmp_buffer);
                if (!is_ok) {
                    return error;
                }
                std::u32string post_map = turbo::idna::map(tmp_buffer);
                if (tmp_buffer != post_map) {
                    return error;
                }
                std::u32string pre_normal = post_map;
                normalize(post_map);
                if (post_map != pre_normal) {
                    return error;
                }
                if (post_map.empty()) {
                    return error;
                }
                if (!is_label_valid(post_map)) {
                    return error;
                }
            } else {
                out.append(label_view);
            }
            if (!is_last_label) {
                out.push_back('.');
            }
        }
        return out;
    }

    // We return "" on error.
    std::string to_ascii(std::string_view ut8_string) {
        if (validate_ascii(ut8_string)) {
            return from_ascii_to_ascii(ut8_string);
        }
        static const std::string error = "";
        // We convert to UTF-32
        size_t utf32_length = turbo::utf32_length_from_utf8(ut8_string.data(), ut8_string.size());
        std::u32string utf32(utf32_length, '\0');
        size_t actual_utf32_length = turbo::convert_utf8_to_utf32(
            ut8_string.data(), ut8_string.size(), utf32.data());
        if (actual_utf32_length == 0) {
            return error;
        }
        // mapping
        utf32 = turbo::idna::map(utf32);
        normalize(utf32);
        std::string out;
        size_t label_start = 0;

        while (label_start != utf32.size()) {
            size_t loc_dot = utf32.find('.', label_start);
            bool is_last_label = (loc_dot == std::string_view::npos);
            size_t label_size = is_last_label ? utf32.size() - label_start : loc_dot - label_start;
            size_t label_size_with_dot = is_last_label ? label_size : label_size + 1;
            std::u32string_view label_view(utf32.data() + label_start, label_size);
            label_start += label_size_with_dot;
            if (label_size == 0) {
                // empty label? Nothing to do.
            } else if (starts_with(label_view, U"xn--")) {
                // we do not need to check, e.g., Xn-- because mapping goes to lower case
                for (char32_t c : label_view) {
                    if (c >= 0x80) {
                        return error;
                    }
                    out += (unsigned char)(c);
                }
                std::string_view puny_segment_ascii(
                    out.data() + out.size() - label_view.size() + 4,
                    label_view.size() - 4);
                std::u32string tmp_buffer;
                bool is_ok = turbo::idna::punycode_to_utf32(puny_segment_ascii, tmp_buffer);
                if (!is_ok) {
                    return error;
                }
                std::u32string post_map = turbo::idna::map(tmp_buffer);
                if (tmp_buffer != post_map) {
                    return error;
                }
                std::u32string pre_normal = post_map;
                normalize(post_map);
                if (post_map != pre_normal) {
                    return error;
                }
                if (post_map.empty()) {
                    return error;
                }
                if (!is_label_valid(post_map)) {
                    return error;
                }
            } else {
                // The fast path here is an ascii label.
                if (validate_ascii(label_view)) {
                    // no validation needed.
                    for (char32_t c : label_view) {
                        out += (unsigned char)(c);
                    }
                } else {
                    // slow path.
                    // first check validity.
                    if (!is_label_valid(label_view)) {
                        return error;
                    }
                    // It is valid! So now we must encode it as punycode...
                    out.append("xn--");
                    bool is_ok = turbo::idna::utf32_to_punycode(label_view, out);
                    if (!is_ok) {
                        return error;
                    }
                }
            }
            if (!is_last_label) {
                out.push_back('.');
            }
        }
        return out;
    }

    std::string to_unicode(std::string_view input) {
        std::string output;
        output.reserve(input.size());

        size_t label_start = 0;
        while (label_start < input.size()) {
            size_t loc_dot = input.find('.', label_start);
            bool is_last_label = (loc_dot == std::string_view::npos);
            size_t label_size = is_last_label ? input.size() - label_start : loc_dot - label_start;
            auto label_view = std::string_view(input.data() + label_start, label_size);

            if (starts_with(label_view, "xn--") && validate_ascii(label_view)) {
                label_view.remove_prefix(4);
                if (turbo::idna::verify_punycode(label_view)) {
                    std::u32string tmp_buffer;
                    if (turbo::idna::punycode_to_utf32(label_view, tmp_buffer)) {
                        auto utf8_size = turbo::utf8_length_from_utf32(tmp_buffer.data(),
                            tmp_buffer.size());
                        std::string final_utf8(utf8_size, '\0');
                        auto n =turbo::convert_utf32_to_utf8(tmp_buffer.data(), tmp_buffer.size(),
                            final_utf8.data());
                        KUMO_UNUSED(n);
                        output.append(final_utf8);
                    } else {
                        // ToUnicode never fails.  If any step fails, then the original input
                        // sequence is returned immediately in that step.
                        output.append(
                            std::string_view(input.data() + label_start, label_size));
                    }
                } else {
                    output.append(std::string_view(input.data() + label_start, label_size));
                }
            } else {
                output.append(label_view);
            }

            if (!is_last_label) {
                output.push_back('.');
            }

            label_start += label_size + 1;
        }

        return output;
    }
} // namespace turbo::idna
