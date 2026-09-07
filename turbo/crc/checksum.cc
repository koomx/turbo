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

#include <turbo/crc/checksum.h>
#include <array>
#include <cstdio>

namespace turbo {

    static constexpr uint16_t kCrcPoly16 = 0xA001;
    static constexpr uint16_t kCrcPolyCcitt = 0x1021;
    static constexpr uint16_t kCrcPolyKermit = 0x8408;
    static constexpr uint16_t kCrcPolyDnp = 0xA6BC;
    static constexpr uint16_t kCrcPolySick = 0x8005;
    static constexpr uint32_t kCrcPoly32 = 0xEDB88320u;
    static constexpr uint64_t kCrcPoly64 = 0x42F0E1EBA9EA3693ull;

    static uint8_t sht75_crc_table[] = {

        0,   49,  98,  83,  196, 245, 166, 151, 185, 136, 219, 234, 125, 76,  31,  46,
        67,  114, 33,  16,  135, 182, 229, 212, 250, 203, 152, 169, 62,  15,  92,  109,
        134, 183, 228, 213, 66,  115, 32,  17,  63,  14,  93,  108, 251, 202, 153, 168,
        197, 244, 167, 150, 1,   48,  99,  82,  124, 77,  30,  47,  184, 137, 218, 235,
        61,  12,  95,  110, 249, 200, 155, 170, 132, 181, 230, 215, 64,  113, 34,  19,
        126, 79,  28,  45,  186, 139, 216, 233, 199, 246, 165, 148, 3,   50,  97,  80,
        187, 138, 217, 232, 127, 78,  29,  44,  2,   51,  96,  81,  198, 247, 164, 149,
        248, 201, 154, 171, 60,  13,  94,  111, 65,  112, 35,  18,  133, 180, 231, 214,
        122, 75,  24,  41,  190, 143, 220, 237, 195, 242, 161, 144, 7,   54,  101, 84,
        57,  8,   91,  106, 253, 204, 159, 174, 128, 177, 226, 211, 68,  117, 38,  23,
        252, 205, 158, 175, 56,  9,   90,  107, 69,  116, 39,  22,  129, 176, 227, 210,
        191, 142, 221, 236, 123, 74,  25,  40,  6,   55,  100, 85,  194, 243, 160, 145,
        71,  118, 37,  20,  131, 178, 225, 208, 254, 207, 156, 173, 58,  11,  88,  105,
        4,   53,  102, 87,  192, 241, 162, 147, 189, 140, 223, 238, 121, 72,  27,  42,
        193, 240, 163, 146, 5,   52,  103, 86,  120, 73,  26,  43,  188, 141, 222, 239,
        130, 179, 224, 209, 70,  119, 36,  21,  59,  10,  89,  104, 255, 206, 157, 172
    };

    uint8_t crc8_extend(uint8_t crc, const uint8_t* data, size_t count) {
        auto end = data + count;
        while (data != end) {
           crc = sht75_crc_table[*data++ ^ crc];
        }
        return crc;
    }

    static const std::array<uint16_t, 256>& get_crc16_table() {
        static std::array<uint16_t, 256> ins = [] {
            std::array<uint16_t, 256> ret;
            uint16_t i;
            uint16_t j;
            uint16_t crc;
            uint16_t c;
            for (i = 0; i < 256; i++) {
                crc = 0;
                c = i;
                for (j = 0; j < 8; j++) {
                    if ((crc ^ c) & 0x0001) crc = (crc >> 1) ^ kCrcPoly16;
                    else crc = crc >> 1;
                    c = c >> 1;
                }
                ret[i] = crc;
            }
            return ret;
        }();
        return ins;
    }

    uint16_t crc16_extend(uint16_t crc, const uint8_t* data, size_t count) {
        auto& table = get_crc16_table();
        auto end = data + count;
        while (data != end) {
            crc = (crc >> 8) ^ table[(crc ^ *data++) & 0x00FF];
        }
        return crc;
    }

    static const std::array<uint16_t, 256>& get_crc_ccitt_table() {
        static std::array<uint16_t, 256> ins = [] {
            std::array<uint16_t, 256> ret;
            uint16_t i;
            uint16_t j;
            uint16_t crc;
            uint16_t c;
            for (i = 0; i < 256; i++) {
                crc = 0;
                c = static_cast<uint16_t>(i << 8);
                for (j = 0; j < 8; j++) {
                    if ((crc ^ c) & 0x8000) crc = (crc << 1) ^ kCrcPolyCcitt;
                    else crc = static_cast<uint16_t>(crc << 1);
                    c = static_cast<uint16_t>(c << 1);
                }
                ret[i] = crc;
            }
            return ret;
        }();
        return ins;
    }

    uint16_t crc_ccitt_extend(uint16_t crc, const uint8_t* data, size_t count) {
        auto& table = get_crc_ccitt_table();
        auto end = data + count;
        while (data != end) {
            crc = static_cast<uint16_t>((crc << 8) ^ table[((crc >> 8) ^ *data++) & 0x00FF]);
        }
        return crc;
    }

    static const std::array<uint16_t, 256>& get_crc_kermit_table() {
        static std::array<uint16_t, 256> ins = [] {
            std::array<uint16_t, 256> ret;
            uint16_t i;
            uint16_t j;
            uint16_t crc;
            uint16_t c;
            for (i = 0; i < 256; i++) {
                crc = 0;
                c = i;
                for (j = 0; j < 8; j++) {
                    if ((crc ^ c) & 0x0001) crc = (crc >> 1) ^ kCrcPolyKermit;
                    else crc = crc >> 1;
                    c = c >> 1;
                }
                ret[i] = crc;
            }
            return ret;
        }();
        return ins;
    }

    uint16_t crc_kermit_extend(uint16_t crc, const uint8_t* data, size_t count) {
        auto& table = get_crc_kermit_table();
        auto end = data + count;
        while (data != end) {
            crc = (crc >> 8) ^ table[(crc ^ *data++) & 0x00FF];
        }
        return crc;
    }

    static const std::array<uint16_t, 256>& get_crc_dnp_table() {
        static std::array<uint16_t, 256> ins = [] {
            std::array<uint16_t, 256> ret;
            uint16_t i;
            uint16_t j;
            uint16_t crc;
            uint16_t c;
            for (i = 0; i < 256; i++) {
                crc = 0;
                c = i;
                for (j = 0; j < 8; j++) {
                    if ((crc ^ c) & 0x0001) crc = (crc >> 1) ^ kCrcPolyDnp;
                    else crc = crc >> 1;
                    c = c >> 1;
                }
                ret[i] = crc;
            }
            return ret;
        }();
        return ins;
    }

    uint16_t crc_dnp_extend(uint16_t crc, const uint8_t* data, size_t count) {
        auto& table = get_crc_dnp_table();
        auto end = data + count;
        while (data != end) {
            crc = (crc >> 8) ^ table[(crc ^ *data++) & 0x00FF];
        }
        return crc;
    }

    uint16_t crc_sick_extend(uint16_t crc, const uint8_t* data, size_t count) {
        uint16_t short_p = 0;
        auto end = data + count;
        while (data != end) {
            uint16_t short_c = 0x00FF & static_cast<uint16_t>(*data);
            if (crc & 0x8000) crc = (crc << 1) ^ kCrcPolySick;
            else crc = static_cast<uint16_t>(crc << 1);
            crc = static_cast<uint16_t>(crc ^ (short_c | short_p));
            short_p = static_cast<uint16_t>(short_c << 8);
            data++;
        }
        return crc;
    }

    static const std::array<uint32_t, 256>& get_crc32_table() {
        static std::array<uint32_t, 256> ins = [] {
            std::array<uint32_t, 256> ret;
            uint32_t i;
            uint32_t j;
            uint32_t crc;
            for (i = 0; i < 256; i++) {
                crc = i;
                for (j = 0; j < 8; j++) {
                    if (crc & 0x00000001u) crc = (crc >> 1) ^ kCrcPoly32;
                    else crc = crc >> 1;
                }
                ret[i] = crc;
            }
            return ret;
        }();
        return ins;
    }

    uint32_t crc32_extend(uint32_t crc, const uint8_t* data, size_t count) {
        auto& table = get_crc32_table();
        auto end = data + count;
        while (data != end) {
            crc = (crc >> 8) ^ table[(crc ^ *data++) & 0x000000FFu];
        }
        return crc;
    }

    static const std::array<uint64_t, 256>& get_crc64_table() {
        static std::array<uint64_t, 256> ins = [] {
            std::array<uint64_t, 256> ret;
            uint64_t i;
            uint64_t j;
            uint64_t c;
            uint64_t crc;
            for (i = 0; i < 256; i++) {
                crc = 0;
                c = i << 56;
                for (j = 0; j < 8; j++) {
                    if ((crc ^ c) & 0x8000000000000000ull) crc = (crc << 1) ^ kCrcPoly64;
                    else crc = crc << 1;
                    c = c << 1;
                }
                ret[i] = crc;
            }
            return ret;
        }();
        return ins;
    }

    uint64_t crc64_extend(uint64_t crc, const uint8_t* data, size_t count) {
        auto& table = get_crc64_table();
        auto end = data + count;
        while (data != end) {
            crc = (crc << 8) ^ table[((crc >> 56) ^ *data++) & 0xFFull];
        }
        return crc;
    }

    char* checksum_nmea(const char* input, char* result) {
        if (input == nullptr || result == nullptr) {
            return nullptr;
        }
        uint8_t checksum = 0;
        const char* ptr = input;
        if (*ptr == '$') {
            ptr++;
        }
        while (*ptr && *ptr != '\r' && *ptr != '\n' && *ptr != '*') {
            checksum = static_cast<uint8_t>(checksum ^ static_cast<uint8_t>(*ptr++));
        }
        std::snprintf(result, 3, "%02hhX", checksum);
        return result;
    }

}  // namespace turbo
