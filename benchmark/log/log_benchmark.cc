// Copyright 2022 The Abseil Authors.
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

#include <turbo/macros/config.h>
#include <turbo/base/log_severity.h>
#include <turbo/log/kcheck.h>
#include <turbo/log/globals.h>
#include <turbo/log/initialize.h>
#include <turbo/log/klog.h>
#include <turbo/log/log_entry.h>
#include <turbo/log/log_sink.h>
#include <turbo/log/log_sink_registry.h>
#include <turbo/log/vlog_is_on.h>
#include "benchmark/benchmark.h"

#include <cstdlib>
#include <thread>

namespace {

// Local: ThreadRange up to 2*N. CI (GITHUB_ACTIONS): single-thread only — ARM
// runners hang/oversubscribe under multi-thread log benches.
int MaxBenchThreads() {
  if (const char* ga = std::getenv("GITHUB_ACTIONS");
      ga != nullptr && ga[0] != '\0') {
    return 1;
  }
  unsigned hc = std::thread::hardware_concurrency();
  if (hc == 0) {
    hc = 1;
  }
  return static_cast<int>(2 * hc);
}

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

constexpr int x = -1;

void BM_SuccessfulBinaryCheck(benchmark::State& state) {
  int n = 0;
  while (state.KeepRunningBatch(8)) {
    KCHECK_GE(n, x);
    KCHECK_GE(n, x);
    KCHECK_GE(n, x);
    KCHECK_GE(n, x);
    KCHECK_GE(n, x);
    KCHECK_GE(n, x);
    KCHECK_GE(n, x);
    KCHECK_GE(n, x);
    ++n;
  }
  benchmark::DoNotOptimize(n);
}
BENCHMARK(BM_SuccessfulBinaryCheck);

static void BM_SuccessfulUnaryCheck(benchmark::State& state) {
  int n = 0;
  while (state.KeepRunningBatch(8)) {
    KCHECK(n >= x);
    KCHECK(n >= x);
    KCHECK(n >= x);
    KCHECK(n >= x);
    KCHECK(n >= x);
    KCHECK(n >= x);
    KCHECK(n >= x);
    KCHECK(n >= x);
    ++n;
  }
  benchmark::DoNotOptimize(n);
}
BENCHMARK(BM_SuccessfulUnaryCheck);

static void BM_DisabledLogOverhead(benchmark::State& state) {
  EnsureLogInitialized();
  turbo::ScopedStderrThreshold disable_stderr_logging(
      turbo::LogSeverityAtLeast::kInfinity);
  turbo::log_internal::ScopedMinLogLevel scoped_min_log_level(
      turbo::LogSeverityAtLeast::kInfinity);
  for (auto _ : state) {
    KLOG(INFO);
  }
}
BENCHMARK(BM_DisabledLogOverhead);

static void BM_EnabledLogOverhead(benchmark::State& state) {
  EnsureLogInitialized();
  turbo::ScopedStderrThreshold stderr_logging(
      turbo::LogSeverityAtLeast::kInfinity);
  turbo::log_internal::ScopedMinLogLevel scoped_min_log_level(
      turbo::LogSeverityAtLeast::kInfo);
  KUMO_ATTRIBUTE_UNUSED NullLogSink null_sink;
  for (auto _ : state) {
    KLOG(INFO);
  }
}
BENCHMARK(BM_EnabledLogOverhead);

static void BM_VlogIsOnOverhead(benchmark::State& state) {
  EnsureLogInitialized();
  // It would make sense to do this only when state.thread_index == 0,
  // but thread_index is an int on some platforms (e.g. Android) and a
  // function returning an int on others. So we just do it on all threads.
  // TODO(b/152609127): set only if thread_index == 0.
  turbo::set_global_vlog_level(0);

  while (state.KeepRunningBatch(10)) {
    benchmark::DoNotOptimize(VKLOG_IS_ON(0));  // 1
    benchmark::DoNotOptimize(VKLOG_IS_ON(0));  // 2
    benchmark::DoNotOptimize(VKLOG_IS_ON(0));  // 3
    benchmark::DoNotOptimize(VKLOG_IS_ON(0));  // 4
    benchmark::DoNotOptimize(VKLOG_IS_ON(0));  // 5
    benchmark::DoNotOptimize(VKLOG_IS_ON(0));  // 6
    benchmark::DoNotOptimize(VKLOG_IS_ON(0));  // 7
    benchmark::DoNotOptimize(VKLOG_IS_ON(0));  // 8
    benchmark::DoNotOptimize(VKLOG_IS_ON(0));  // 9
    benchmark::DoNotOptimize(VKLOG_IS_ON(0));  // 10
  }
}
BENCHMARK(BM_VlogIsOnOverhead)->ThreadRange(1, MaxBenchThreads());

static void BM_VlogIsNotOnOverhead(benchmark::State& state) {
  EnsureLogInitialized();
  // It would make sense to do this only when state.thread_index == 0,
  // but thread_index is an int on some platforms (e.g. Android) and a
  // function returning an int on others. So we just do it on all threads.
  // TODO(b/152609127): set only if thread_index == 0.
  turbo::set_global_vlog_level(0);

  while (state.KeepRunningBatch(10)) {
    benchmark::DoNotOptimize(VKLOG_IS_ON(1));  // 1
    benchmark::DoNotOptimize(VKLOG_IS_ON(1));  // 2
    benchmark::DoNotOptimize(VKLOG_IS_ON(1));  // 3
    benchmark::DoNotOptimize(VKLOG_IS_ON(1));  // 4
    benchmark::DoNotOptimize(VKLOG_IS_ON(1));  // 5
    benchmark::DoNotOptimize(VKLOG_IS_ON(1));  // 6
    benchmark::DoNotOptimize(VKLOG_IS_ON(1));  // 7
    benchmark::DoNotOptimize(VKLOG_IS_ON(1));  // 8
    benchmark::DoNotOptimize(VKLOG_IS_ON(1));  // 9
    benchmark::DoNotOptimize(VKLOG_IS_ON(1));  // 10
  }
}
BENCHMARK(BM_VlogIsNotOnOverhead)->ThreadRange(1, MaxBenchThreads());

static void BM_LogEveryNOverhead(benchmark::State& state) {
  EnsureLogInitialized();
  turbo::ScopedStderrThreshold disable_stderr_logging(
      turbo::LogSeverityAtLeast::kInfinity);
  turbo::set_min_log_level(turbo::LogSeverityAtLeast::kInfinity);
  KUMO_ATTRIBUTE_UNUSED NullLogSink null_sink;

  while (state.KeepRunningBatch(10)) {
    KLOG_EVERY_N_SEC(INFO, 10);
    KLOG_EVERY_N_SEC(INFO, 20);
    KLOG_EVERY_N_SEC(INFO, 30);
    KLOG_EVERY_N_SEC(INFO, 40);
    KLOG_EVERY_N_SEC(INFO, 50);
    KLOG_EVERY_N_SEC(INFO, 60);
    KLOG_EVERY_N_SEC(INFO, 70);
    KLOG_EVERY_N_SEC(INFO, 80);
    KLOG_EVERY_N_SEC(INFO, 90);
    KLOG_EVERY_N_SEC(INFO, 100);
  }
}
BENCHMARK(BM_LogEveryNOverhead)->ThreadRange(1, MaxBenchThreads());

}  // namespace

BENCHMARK_MAIN();

