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

#include <string_view>
#include <cstdint>

namespace turbo {

    uint16_t crc16_redis_extend(uint16_t crc, const uint8_t* data, size_t count);

    inline uint16_t crc16_redis(const uint8_t* data, size_t count) {
        return crc16_redis_extend(0, data,count);
    }

    inline uint16_t crc16_redis_slot(const uint8_t* data, size_t count) {
        return static_cast<uint16_t>(crc16_redis(data, count) & 0x3FFFu);
    }

    inline uint16_t crc16_redis_extend(uint16_t crc, std::string_view data) {
        return crc16_redis_extend(crc, reinterpret_cast<const uint8_t*>(data.data()),data.size());
    }

    inline uint16_t crc16_redis(std::string_view data) {
        return crc16_redis_extend(0,reinterpret_cast<const uint8_t*>(data.data()),data.size());
    }

    inline uint16_t crc16_redis_slot(std::string_view data) {
        return static_cast<uint16_t>(crc16_redis(reinterpret_cast<const uint8_t*>(data.data()), data.size()) & 0x3FFFu);
    }
}  // namespace turbo
