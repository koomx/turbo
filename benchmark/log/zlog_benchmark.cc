// Copyright 2024 The Abseil Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <string>

#include <turbo/log/klog.h>
#include <turbo/log/zlog.h>
#include <turbo/log/globals.h>
#include <turbo/log/initialize.h>
#include <turbo/log/log_sink.h>
#include <turbo/log/log_sink_registry.h>
#include "benchmark/benchmark.h"

namespace {

void EnsureLogInitialized() {
    static const bool once = [] {
        turbo::initialize_log();
        return true;
    }();
    (void)once;
}

class NullLogSink : public turbo::LogSink {
public:
    NullLogSink() { turbo::add_log_sink(this); }
    ~NullLogSink() override { turbo::remove_log_sink(this); }
    void send(const turbo::LogEntry&) override {}
};

constexpr int kIntVal = 42;
constexpr double kDoubleVal = 3.14159;
const char* kStrVal = "hello_world";

// ---------------------------------------------------------------------------
// Enabled logging — stream vs printf
// ---------------------------------------------------------------------------

// Baseline: stream-based LOG with int + string
void BM_StreamEnabled(benchmark::State& state) {
    EnsureLogInitialized();
    turbo::ScopedStderrThreshold stderr_logging(
        turbo::LogSeverityAtLeast::kInfinity);
    turbo::log_internal::ScopedMinLogLevel scoped_min_log_level(
        turbo::LogSeverityAtLeast::kInfo);
    KUMO_ATTRIBUTE_UNUSED NullLogSink null_sink;

    for (auto _ : state) {
        KLOG(INFO) << "x=" << kIntVal << " y=" << kStrVal;
    }
}
BENCHMARK(BM_StreamEnabled);

// printf-style ZLOG with %d + %s
void BM_ZlogEnabled(benchmark::State& state) {
    EnsureLogInitialized();
    turbo::ScopedStderrThreshold stderr_logging(
        turbo::LogSeverityAtLeast::kInfinity);
    turbo::log_internal::ScopedMinLogLevel scoped_min_log_level(
        turbo::LogSeverityAtLeast::kInfo);
    KUMO_ATTRIBUTE_UNUSED NullLogSink null_sink;

    for (auto _ : state) {
        ZLOG(INFO, "x=%d y=%s", kIntVal, kStrVal);
    }
}
BENCHMARK(BM_ZlogEnabled);

// Stream: int + double
void BM_StreamEnabledNum(benchmark::State& state) {
    EnsureLogInitialized();
    turbo::ScopedStderrThreshold stderr_logging(
        turbo::LogSeverityAtLeast::kInfinity);
    turbo::log_internal::ScopedMinLogLevel scoped_min_log_level(
        turbo::LogSeverityAtLeast::kInfo);
    KUMO_ATTRIBUTE_UNUSED NullLogSink null_sink;

    for (auto _ : state) {
        KLOG(INFO) << "pi=" << kDoubleVal << " val=" << kIntVal;
    }
}
BENCHMARK(BM_StreamEnabledNum);

// printf: double + int
void BM_ZlogEnabledNum(benchmark::State& state) {
    EnsureLogInitialized();
    turbo::ScopedStderrThreshold stderr_logging(
        turbo::LogSeverityAtLeast::kInfinity);
    turbo::log_internal::ScopedMinLogLevel scoped_min_log_level(
        turbo::LogSeverityAtLeast::kInfo);
    KUMO_ATTRIBUTE_UNUSED NullLogSink null_sink;

    for (auto _ : state) {
        ZLOG(INFO, "pi=%.5f val=%d", kDoubleVal, kIntVal);
    }
}
BENCHMARK(BM_ZlogEnabledNum);

// ---------------------------------------------------------------------------
// Disabled logging — stream vs printf (min_log_level = kInfinity)
// ---------------------------------------------------------------------------

void BM_StreamDisabled(benchmark::State& state) {
    EnsureLogInitialized();
    turbo::ScopedStderrThreshold disable_stderr(
        turbo::LogSeverityAtLeast::kInfinity);
    turbo::log_internal::ScopedMinLogLevel scoped_min_log_level(
        turbo::LogSeverityAtLeast::kInfinity);

    for (auto _ : state) {
        KLOG(INFO) << "x=" << kIntVal << " y=" << kStrVal;
    }
}
BENCHMARK(BM_StreamDisabled);

void BM_ZlogDisabled(benchmark::State& state) {
    EnsureLogInitialized();
    turbo::ScopedStderrThreshold disable_stderr(
        turbo::LogSeverityAtLeast::kInfinity);
    turbo::log_internal::ScopedMinLogLevel scoped_min_log_level(
        turbo::LogSeverityAtLeast::kInfinity);

    for (auto _ : state) {
        ZLOG(INFO, "x=%d y=%s", kIntVal, kStrVal);
    }
}
BENCHMARK(BM_ZlogDisabled);

// ---------------------------------------------------------------------------
// Warm call: enabled, but condition always false (ZFATAL/ERROR)
// ---------------------------------------------------------------------------

void BM_StreamWarmDisabled(benchmark::State& state) {
    EnsureLogInitialized();
    turbo::ScopedStderrThreshold disable_stderr(
        turbo::LogSeverityAtLeast::kInfinity);

    for (auto _ : state) {
        KLOG(INFO) << "x=" << kIntVal;
    }
}
BENCHMARK(BM_StreamWarmDisabled);

void BM_ZlogWarmDisabled(benchmark::State& state) {
    EnsureLogInitialized();
    turbo::ScopedStderrThreshold disable_stderr(
        turbo::LogSeverityAtLeast::kInfinity);

    for (auto _ : state) {
        ZLOG(INFO, "x=%d", kIntVal);
    }
}
BENCHMARK(BM_ZlogWarmDisabled);

}  // namespace

BENCHMARK_MAIN();
