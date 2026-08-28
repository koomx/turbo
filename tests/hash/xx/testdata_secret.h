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

#include <cstdint>

#include <tests/hash/xx/testing.h>

namespace turbo::xxtest {

    
    static constexpr  XXHSecrets xxhash_testdata_secret[] = {
        {    0,  192, { 0xE7, 0x8C, 0x77, 0x77, 0x00 } }, ///!< xxhash_testdata_secret[0]
        {    0,  240, { 0xE7, 0x8C, 0x77, 0xAA, 0x00 } }, ///!< xxhash_testdata_secret[1]
        {    0,  277, { 0xE7, 0x8C, 0x77, 0xAA, 0xED } }, ///!< xxhash_testdata_secret[2]
        {    0, 9867, { 0xE7, 0x8C, 0x77, 0xAA, 0xED } }, ///!< xxhash_testdata_secret[3]
        {    1,  192, { 0x2B, 0x3E, 0xDE, 0x67, 0x00 } }, ///!< xxhash_testdata_secret[4]
        {    1,  240, { 0x2B, 0x3E, 0xDE, 0xC1, 0x00 } }, ///!< xxhash_testdata_secret[5]
        {    1,  277, { 0x2B, 0x3E, 0xDE, 0xC1, 0xCC } }, ///!< xxhash_testdata_secret[6]
        {    1, 9867, { 0x2B, 0x3E, 0xDE, 0xC1, 0xCC } }, ///!< xxhash_testdata_secret[7]
        {  135,  192, { 0xE8, 0x39, 0x6C, 0x16, 0x00 } }, ///!< xxhash_testdata_secret[8]
        {  135,  240, { 0xE8, 0x39, 0x6C, 0xCC, 0x00 } }, ///!< xxhash_testdata_secret[9]
        {  135,  277, { 0xE8, 0x39, 0x6C, 0xCC, 0x7B } }, ///!< xxhash_testdata_secret[10]
        {  135, 9867, { 0xE8, 0x39, 0x6C, 0xCC, 0x7B } }, ///!< xxhash_testdata_secret[11]
        {  692,  192, { 0xD6, 0x1C, 0x41, 0x69, 0x00 } }, ///!< xxhash_testdata_secret[12]
        {  692,  240, { 0xD6, 0x1C, 0x41, 0x17, 0x00 } }, ///!< xxhash_testdata_secret[13]
        {  692,  277, { 0xD6, 0x1C, 0x41, 0x17, 0xB3 } }, ///!< xxhash_testdata_secret[14]
        {  692, 9867, { 0xD6, 0x1C, 0x41, 0x17, 0xB3 } }, ///!< xxhash_testdata_secret[15]
    };

    
}  // namespace turbo::xxtest


