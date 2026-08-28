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

#include <cstdint>
#include <string>

#include "benchmark/benchmark.h"
#include <turbo/hash/hash.h>
#include <turbo/hash/xx/xxh3.h>
#include <turbo/hash/xx/xxhash_scalar.h>

namespace {

uint64_t kInt = 0x9E3779B185EBCA87ULL;

void BM_Int_StdHash(benchmark::State& state) {
    std::hash<uint64_t> h;
    for (auto _ : state) {
        benchmark::DoNotOptimize(kInt);
        benchmark::DoNotOptimize(h(kInt));
    }
}
BENCHMARK(BM_Int_StdHash);

void BM_Int_TurboHash(benchmark::State& state) {
    turbo::Hash<uint64_t> h;
    for (auto _ : state) {
        benchmark::DoNotOptimize(kInt);
        benchmark::DoNotOptimize(h(kInt));
    }
}
BENCHMARK(BM_Int_TurboHash);

void BM_Int_XxHash64(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(kInt);
        benchmark::DoNotOptimize(turbo::xxhash64_scalar(
            reinterpret_cast<const uint8_t*>(&kInt), sizeof(kInt)));
    }
}
BENCHMARK(BM_Int_XxHash64);

void BM_Int_Xxh3_64(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(kInt);
        benchmark::DoNotOptimize(turbo::xxhash64(
            reinterpret_cast<const uint8_t*>(&kInt), sizeof(kInt)));
    }
}
BENCHMARK(BM_Int_Xxh3_64);

void BM_String_StdHash(benchmark::State& state) {
    std::string s(static_cast<size_t>(state.range(0)), 'x');
    std::hash<std::string> h;
    for (auto _ : state) {
        benchmark::DoNotOptimize(s);
        benchmark::DoNotOptimize(h(s));
    }
    state.SetBytesProcessed(state.iterations() * s.size());
}
BENCHMARK(BM_String_StdHash)->Arg(8)->Arg(16)->Arg(64)->Arg(256)->Arg(1024)->Arg(4096);

void BM_String_TurboHash(benchmark::State& state) {
    std::string s(static_cast<size_t>(state.range(0)), 'x');
    turbo::Hash<std::string> h;
    for (auto _ : state) {
        benchmark::DoNotOptimize(s);
        benchmark::DoNotOptimize(h(s));
    }
    state.SetBytesProcessed(state.iterations() * s.size());
}
BENCHMARK(BM_String_TurboHash)->Arg(8)->Arg(16)->Arg(64)->Arg(256)->Arg(1024)->Arg(4096);

void BM_String_XxHash64(benchmark::State& state) {
    std::string s(static_cast<size_t>(state.range(0)), 'x');
    for (auto _ : state) {
        benchmark::DoNotOptimize(s);
        benchmark::DoNotOptimize(turbo::xxhash64_scalar(
            reinterpret_cast<const uint8_t*>(s.data()), s.size()));
    }
    state.SetBytesProcessed(state.iterations() * s.size());
}
BENCHMARK(BM_String_XxHash64)->Arg(8)->Arg(16)->Arg(64)->Arg(256)->Arg(1024)->Arg(4096);

void BM_String_Xxh3_64(benchmark::State& state) {
    std::string s(static_cast<size_t>(state.range(0)), 'x');
    for (auto _ : state) {
        benchmark::DoNotOptimize(s);
        benchmark::DoNotOptimize(turbo::xxhash64(
            reinterpret_cast<const uint8_t*>(s.data()), s.size()));
    }
    state.SetBytesProcessed(state.iterations() * s.size());
}
BENCHMARK(BM_String_Xxh3_64)->Arg(8)->Arg(16)->Arg(64)->Arg(256)->Arg(1024)->Arg(4096);

}  // namespace

BENCHMARK_MAIN();
