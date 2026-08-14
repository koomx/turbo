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

#include <filesystem>
#include <string>

#include <benchmark/benchmark.h>
#include <turbo/log/globals.h>
#include <turbo/log/initialize.h>
#include <turbo/log/klog.h>
#include <turbo/log/sinks/daily_file_sink.h>

namespace {

void EnsureLogInitialized() {
    static const bool once = [] {
        turbo::initialize_log();
        return true;
    }();
    (void)once;
}

std::filesystem::path MakeTmp(const char *name) {
    auto dir = std::filesystem::temp_directory_path() / name;
    std::filesystem::remove_all(dir);
    std::filesystem::create_directories(dir);
    return dir;
}

void BM_DailySync(benchmark::State &state) {
    EnsureLogInitialized();
    turbo::ScopedStderrThreshold stderr_logging(
        turbo::LogSeverityAtLeast::kInfinity);
    const auto dir = MakeTmp("turbo_bm_daily_sync");
    turbo::DailyFileSink sink((dir / "app.log").string(), 7, 600, true);

    for (auto _ : state) {
        KLOG(INFO).to_sink_only(&sink) << "benchmark daily sync line";
    }
    sink.flush();
    state.SetItemsProcessed(state.iterations());
    std::filesystem::remove_all(dir);
}

void BM_DailyAsync(benchmark::State &state) {
    EnsureLogInitialized();
    turbo::ScopedStderrThreshold stderr_logging(
        turbo::LogSeverityAtLeast::kInfinity);
    const auto dir = MakeTmp("turbo_bm_daily_async");
    turbo::DailyFileSink sink((dir / "app.log").string(), 7, 600, true);
    sink.start();

    for (auto _ : state) {
        KLOG(INFO).to_sink_only(&sink) << "benchmark daily async line";
    }
    sink.stop();
    state.SetItemsProcessed(state.iterations());
    std::filesystem::remove_all(dir);
}

BENCHMARK(BM_DailySync)->UseRealTime();
BENCHMARK(BM_DailyAsync)->UseRealTime();

}  // namespace

BENCHMARK_MAIN();
