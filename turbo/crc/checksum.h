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

#include <cstddef>
#include <cstdint>

namespace turbo {

    static constexpr uint8_t kCrc8Start = 0;
    static constexpr uint16_t kCrc16Start = 0;
    static constexpr uint16_t kCrc16ModbusStart = 0xFFFF;
    static constexpr uint16_t kCrcXmodemStart = 0;
    static constexpr uint16_t kCrcCcitt1d0fStart = 0x1D0F;
    static constexpr uint16_t kCrcCcittFfffStart = 0xFFFF;
    static constexpr uint16_t kCrcKermitStart = 0;
    static constexpr uint16_t kCrcDnpStart = 0;
    static constexpr uint16_t kCrcSickStart = 0;
    static constexpr uint32_t kCrc32Start = 0xFFFFFFFFu;
    static constexpr uint64_t kCrc64EcmaStart = 0;
    static constexpr uint64_t kCrc64WeStart = 0xFFFFFFFFFFFFFFFFull;

    uint8_t crc8_extend(uint8_t crc, const uint8_t* data, size_t count);

    inline uint8_t crc8(const uint8_t* data, size_t count) {
        return crc8_extend(kCrc8Start, data, count);
    }

    uint16_t crc16_extend(uint16_t crc, const uint8_t* data, size_t count);

    inline uint16_t crc16(const uint8_t* data, size_t count) {
        return crc16_extend(kCrc16Start, data, count);
    }

    inline uint16_t crc16_modbus(const uint8_t* data, size_t count) {
        return crc16_extend(kCrc16ModbusStart, data, count);
    }

    uint16_t crc_ccitt_extend(uint16_t crc, const uint8_t* data, size_t count);

    inline uint16_t crc_xmodem(const uint8_t* data, size_t count) {
        return crc_ccitt_extend(kCrcXmodemStart, data, count);
    }

    inline uint16_t crc_ccitt_1d0f(const uint8_t* data, size_t count) {
        return crc_ccitt_extend(kCrcCcitt1d0fStart, data, count);
    }

    inline uint16_t crc_ccitt_ffff(const uint8_t* data, size_t count) {
        return crc_ccitt_extend(kCrcCcittFfffStart, data, count);
    }

    uint16_t crc_kermit_extend(uint16_t crc, const uint8_t* data, size_t count);

    inline uint16_t crc_kermit(const uint8_t* data, size_t count) {
        uint16_t crc = crc_kermit_extend(kCrcKermitStart, data, count);
        return static_cast<uint16_t>((crc >> 8) | (crc << 8));
    }

    uint16_t crc_dnp_extend(uint16_t crc, const uint8_t* data, size_t count);

    inline uint16_t crc_dnp(const uint8_t* data, size_t count) {
        uint16_t crc = static_cast<uint16_t>(~crc_dnp_extend(kCrcDnpStart, data, count));
        return static_cast<uint16_t>((crc >> 8) | (crc << 8));
    }

    uint16_t crc_sick_extend(uint16_t crc, const uint8_t* data, size_t count);

    inline uint16_t crc_sick(const uint8_t* data, size_t count) {
        uint16_t crc = crc_sick_extend(kCrcSickStart, data, count);
        return static_cast<uint16_t>((crc >> 8) | (crc << 8));
    }

    uint32_t crc32_extend(uint32_t crc, const uint8_t* data, size_t count);

    inline uint32_t crc32(const uint8_t* data, size_t count) {
        return crc32_extend(kCrc32Start, data, count) ^ 0xFFFFFFFFu;
    }

    uint64_t crc64_extend(uint64_t crc, const uint8_t* data, size_t count);

    inline uint64_t crc64_ecma(const uint8_t* data, size_t count) {
        return crc64_extend(kCrc64EcmaStart, data, count);
    }

    inline uint64_t crc64_we(const uint8_t* data, size_t count) {
        return crc64_extend(kCrc64WeStart, data, count) ^ 0xFFFFFFFFFFFFFFFFull;
    }

    char* checksum_nmea(const char* input, char* result);

}  // namespace turbo
