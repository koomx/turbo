// Copyright 2025 The Abseil Authors.
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

#include <cstdint>
#include <string>
#include <utility>

#include <turbo/macros/config.h>
#include <turbo/status/status.h>
#include <turbo/status/statusor.h>
#include "benchmark/benchmark.h"

namespace {

void BM_StatusOrInt_CtorStatus(benchmark::State& state) {
  for (auto _ : state) {
    turbo::StatusOr<int> status(turbo::CancelledError());
    benchmark::DoNotOptimize(status);
  }
}
BENCHMARK(BM_StatusOrInt_CtorStatus);

void BM_StatusOrInt_CtorStatusWithMessage(benchmark::State& state) {
  for (auto _ : state) {
    turbo::StatusOr<int> status(
        turbo::UnknownError("This string is 28 characters"));
    benchmark::DoNotOptimize(status);
  }
}
BENCHMARK(BM_StatusOrInt_CtorStatusWithMessage);

void BM_StatusOrInt_CopyCtor_Error(benchmark::State& state) {
  for (auto _ : state) {
    turbo::StatusOr<int> original(
        turbo::UnknownError("This string is 28 characters"));
    benchmark::DoNotOptimize(original);
    turbo::StatusOr<int> status(original);
    benchmark::DoNotOptimize(status);
  }
}
BENCHMARK(BM_StatusOrInt_CopyCtor_Error);

void BM_StatusOrInt_CopyCtor_Ok(benchmark::State& state) {
  for (auto _ : state) {
    turbo::StatusOr<int> original(42);
    benchmark::DoNotOptimize(original);
    turbo::StatusOr<int> status(original);
    benchmark::DoNotOptimize(status);
  }
}
BENCHMARK(BM_StatusOrInt_CopyCtor_Ok);

void BM_StatusOrInt_MoveCtor_Error(benchmark::State& state) {
  for (auto _ : state) {
    turbo::StatusOr<int> original(
        turbo::UnknownError("This string is 28 characters"));
    benchmark::DoNotOptimize(original);
    turbo::StatusOr<int> status(std::move(original));
    benchmark::DoNotOptimize(status);
  }
}
BENCHMARK(BM_StatusOrInt_MoveCtor_Error);

void BM_StatusOrInt_MoveCtor_Ok(benchmark::State& state) {
  for (auto _ : state) {
    turbo::StatusOr<int> original(42);
    benchmark::DoNotOptimize(original);
    turbo::StatusOr<int> status(std::move(original));
    benchmark::DoNotOptimize(status);
  }
}
BENCHMARK(BM_StatusOrInt_MoveCtor_Ok);

void BM_StatusOrInt_CopyAssign_Error(benchmark::State& state) {
  for (auto _ : state) {
    turbo::StatusOr<int> original(
        turbo::UnknownError("This string is 28 characters"));
    benchmark::DoNotOptimize(original);
    turbo::StatusOr<int> status(42);
    benchmark::DoNotOptimize(status);
    benchmark::DoNotOptimize(status = original);
    benchmark::DoNotOptimize(status);
  }
}
BENCHMARK(BM_StatusOrInt_CopyAssign_Error);

void BM_StatusOrInt_CopyAssign_Ok(benchmark::State& state) {
  for (auto _ : state) {
    turbo::StatusOr<int> original(42);
    benchmark::DoNotOptimize(original);
    turbo::StatusOr<int> status(42);
    benchmark::DoNotOptimize(status);
    benchmark::DoNotOptimize(status = original);
    benchmark::DoNotOptimize(status);
  }
}
BENCHMARK(BM_StatusOrInt_CopyAssign_Ok);

void BM_StatusOrInt_MoveAssign_Error(benchmark::State& state) {
  for (auto _ : state) {
    turbo::StatusOr<int> original(
        turbo::UnknownError("This string is 28 characters"));
    benchmark::DoNotOptimize(original);
    turbo::StatusOr<int> status(42);
    benchmark::DoNotOptimize(status);
    benchmark::DoNotOptimize(status = std::move(original));
    benchmark::DoNotOptimize(status);
  }
}
BENCHMARK(BM_StatusOrInt_MoveAssign_Error);

void BM_StatusOrInt_MoveAssign_Ok(benchmark::State& state) {
  for (auto _ : state) {
    turbo::StatusOr<int> original(42);
    benchmark::DoNotOptimize(original);
    turbo::StatusOr<int> status(42);
    benchmark::DoNotOptimize(status);
    benchmark::DoNotOptimize(status = std::move(original));
    benchmark::DoNotOptimize(status);
  }
}
BENCHMARK(BM_StatusOrInt_MoveAssign_Ok);

void BM_StatusOrInt_OkMethod_Error(benchmark::State& state) {
  turbo::StatusOr<int> status(
      turbo::UnknownError("This string is 28 characters"));
  for (auto _ : state) {
    benchmark::DoNotOptimize(status);
    benchmark::DoNotOptimize(status.ok());
  }
}
BENCHMARK(BM_StatusOrInt_OkMethod_Error);

void BM_StatusOrInt_OkMethod_Ok(benchmark::State& state) {
  turbo::StatusOr<int> status(42);
  for (auto _ : state) {
    benchmark::DoNotOptimize(status);
    benchmark::DoNotOptimize(status.ok());
  }
}
BENCHMARK(BM_StatusOrInt_OkMethod_Ok);

void BM_StatusOrInt_StatusMethod_Error(benchmark::State& state) {
  for (auto _ : state) {
    turbo::StatusOr<int> status(
        turbo::UnknownError("This string is 28 characters"));
    benchmark::DoNotOptimize(status);
    benchmark::DoNotOptimize(status.status().ok());
  }
}
BENCHMARK(BM_StatusOrInt_StatusMethod_Error);

void BM_StatusOrInt_StatusMethod_Ok(benchmark::State& state) {
  for (auto _ : state) {
    turbo::StatusOr<int> status(42);
    benchmark::DoNotOptimize(status);
    benchmark::DoNotOptimize(std::move(status).status().ok());
  }
}
BENCHMARK(BM_StatusOrInt_StatusMethod_Ok);

void BM_StatusOrInt_StatusMethodRvalue_Error(benchmark::State& state) {
  for (auto _ : state) {
    turbo::StatusOr<int> status(
        turbo::UnknownError("This string is 28 characters"));
    benchmark::DoNotOptimize(status);
    benchmark::DoNotOptimize(std::move(status).status().ok());
  }
}
BENCHMARK(BM_StatusOrInt_StatusMethodRvalue_Error);

void BM_StatusOrInt_StatusMethodRvalue_Ok(benchmark::State& state) {
  for (auto _ : state) {
    turbo::StatusOr<int> status(42);
    benchmark::DoNotOptimize(status);
    benchmark::DoNotOptimize(std::move(status).status());
  }
}
BENCHMARK(BM_StatusOrInt_StatusMethodRvalue_Ok);

void BM_StatusOrString_CtorStatus(benchmark::State& state) {
  for (auto _ : state) {
    turbo::StatusOr<std::string> status(turbo::CancelledError());
    benchmark::DoNotOptimize(status);
  }
}
BENCHMARK(BM_StatusOrString_CtorStatus);

void BM_StatusOrString_CtorStatusWithMessage(benchmark::State& state) {
  for (auto _ : state) {
    turbo::StatusOr<std::string> status(
        turbo::UnknownError("This string is 28 characters"));
    benchmark::DoNotOptimize(status);
  }
}
BENCHMARK(BM_StatusOrString_CtorStatusWithMessage);

void BM_StatusOrString_CopyCtor_Error(benchmark::State& state) {
  for (auto _ : state) {
    turbo::StatusOr<std::string> original(
        turbo::UnknownError("This string is 28 characters"));
    benchmark::DoNotOptimize(original);
    turbo::StatusOr<std::string> status(original);
    benchmark::DoNotOptimize(status);
  }
}
BENCHMARK(BM_StatusOrString_CopyCtor_Error);

void BM_StatusOrString_CopyCtor_Ok(benchmark::State& state) {
  for (auto _ : state) {
    turbo::StatusOr<std::string> original("This string is 28 characters");
    benchmark::DoNotOptimize(original);
    turbo::StatusOr<std::string> status(original);
    benchmark::DoNotOptimize(status);
  }
}
BENCHMARK(BM_StatusOrString_CopyCtor_Ok);

void BM_StatusOrString_MoveCtor_Error(benchmark::State& state) {
  for (auto _ : state) {
    turbo::StatusOr<std::string> original(
        turbo::UnknownError("This string is 28 characters"));
    benchmark::DoNotOptimize(original);
    turbo::StatusOr<std::string> status(std::move(original));
    benchmark::DoNotOptimize(status);
  }
}
BENCHMARK(BM_StatusOrString_MoveCtor_Error);

void BM_StatusOrString_MoveCtor_Ok(benchmark::State& state) {
  for (auto _ : state) {
    turbo::StatusOr<std::string> original("This string is 28 characters");
    benchmark::DoNotOptimize(original);
    turbo::StatusOr<std::string> status(std::move(original));
    benchmark::DoNotOptimize(status);
  }
}
BENCHMARK(BM_StatusOrString_MoveCtor_Ok);

void BM_StatusOrString_CopyAssign_Error(benchmark::State& state) {
  for (auto _ : state) {
    turbo::StatusOr<std::string> original(
        turbo::UnknownError("This string is 28 characters"));
    benchmark::DoNotOptimize(original);
    turbo::StatusOr<std::string> status("This string is 28 characters");
    benchmark::DoNotOptimize(status);
    benchmark::DoNotOptimize(status = original);
    benchmark::DoNotOptimize(status);
  }
}
BENCHMARK(BM_StatusOrString_CopyAssign_Error);

void BM_StatusOrString_CopyAssign_Ok(benchmark::State& state) {
  for (auto _ : state) {
    turbo::StatusOr<std::string> original("This string is 28 characters");
    benchmark::DoNotOptimize(original);
    turbo::StatusOr<std::string> status("This string is 28 characters");
    benchmark::DoNotOptimize(status);
    benchmark::DoNotOptimize(status = original);
    benchmark::DoNotOptimize(status);
  }
}
BENCHMARK(BM_StatusOrString_CopyAssign_Ok);

void BM_StatusOrString_MoveAssign_Error(benchmark::State& state) {
  for (auto _ : state) {
    turbo::StatusOr<std::string> original(
        turbo::UnknownError("This string is 28 characters"));
    benchmark::DoNotOptimize(original);
    turbo::StatusOr<std::string> status("This string is 28 characters");
    benchmark::DoNotOptimize(status);
    benchmark::DoNotOptimize(status = std::move(original));
    benchmark::DoNotOptimize(status);
  }
}
BENCHMARK(BM_StatusOrString_MoveAssign_Error);

void BM_StatusOrString_MoveAssign_Ok(benchmark::State& state) {
  for (auto _ : state) {
    turbo::StatusOr<std::string> original("This string is 28 characters");
    benchmark::DoNotOptimize(original);
    turbo::StatusOr<std::string> status("This string is 28 characters");
    benchmark::DoNotOptimize(status);
    benchmark::DoNotOptimize(status = std::move(original));
    benchmark::DoNotOptimize(status);
  }
}
BENCHMARK(BM_StatusOrString_MoveAssign_Ok);

void BM_StatusOrString_OkMethod_Error(benchmark::State& state) {
  for (auto _ : state) {
    turbo::StatusOr<std::string> status(
        turbo::UnknownError("This string is 28 characters"));
    benchmark::DoNotOptimize(status);
    benchmark::DoNotOptimize(status.ok());
  }
}
BENCHMARK(BM_StatusOrString_OkMethod_Error);

void BM_StatusOrString_OkMethod_Ok(benchmark::State& state) {
  for (auto _ : state) {
    turbo::StatusOr<std::string> status("This string is 28 characters");
    benchmark::DoNotOptimize(status);
    benchmark::DoNotOptimize(status.ok());
  }
}
BENCHMARK(BM_StatusOrString_OkMethod_Ok);

void BM_StatusOrString_StatusMethod_Error(benchmark::State& state) {
  for (auto _ : state) {
    turbo::StatusOr<std::string> status(
        turbo::UnknownError("This string is 28 characters"));
    benchmark::DoNotOptimize(status);
    benchmark::DoNotOptimize(status.status().ok());
  }
}
BENCHMARK(BM_StatusOrString_StatusMethod_Error);

void BM_StatusOrString_StatusMethod_Ok(benchmark::State& state) {
  for (auto _ : state) {
    turbo::StatusOr<std::string> status("This string is 28 characters");
    benchmark::DoNotOptimize(status);
    benchmark::DoNotOptimize(status.status().ok());
  }
}
BENCHMARK(BM_StatusOrString_StatusMethod_Ok);

void BM_StatusOrString_StatusMethodRvalue_Error(benchmark::State& state) {
  for (auto _ : state) {
    turbo::StatusOr<std::string> status(
        turbo::UnknownError("This string is 28 characters"));
    benchmark::DoNotOptimize(status);
    benchmark::DoNotOptimize(std::move(status).status());
  }
}
BENCHMARK(BM_StatusOrString_StatusMethodRvalue_Error);

void BM_StatusOrString_StatusMethodRvalue_Ok(benchmark::State& state) {
  for (auto _ : state) {
    turbo::StatusOr<std::string> status("This string is 28 characters");
    benchmark::DoNotOptimize(status);
    benchmark::DoNotOptimize(std::move(status).status());
  }
}
BENCHMARK(BM_StatusOrString_StatusMethodRvalue_Ok);

// Benchmarks comparing a few alternative ways of structuring an interface
// for returning an int64 on success or an error.  See (a), (b), (c), (d)
// below for the variants.
bool bm_cond = true;

bool SimpleIntInterface(int64_t* v) KUMO_ATTRIBUTE_NOINLINE;
bool SimpleIntInterfaceWithErrorMessage(int64_t* v, std::string* msg)
    KUMO_ATTRIBUTE_NOINLINE;
turbo::Status SimpleIntInterfaceWithErrorStatus(int64_t* v)
    KUMO_ATTRIBUTE_NOINLINE;
turbo::StatusOr<int64_t> SimpleIntStatusOrInterface() KUMO_ATTRIBUTE_NOINLINE;

// (a): Just a boolean return value with an out int64* parameter
bool SimpleIntInterface(int64_t* v) {
  benchmark::DoNotOptimize(bm_cond);
  if (bm_cond) {
    *v = 42;
    return true;
  } else {
    return false;
  }
}

// (b): A boolean return value and a string error message filled in on failure
// and an out int64* parameter filled on success
bool SimpleIntInterfaceWithErrorMessage(int64_t* v, std::string* msg) {
  benchmark::DoNotOptimize(bm_cond);
  if (bm_cond) {
    *v = 42;
    return true;
  } else {
    *msg = "This is an error message";
    return false;
  }
}

// (c): A Status return value with an out int64* parameter on success
turbo::Status SimpleIntInterfaceWithErrorStatus(int64_t* v) {
  benchmark::DoNotOptimize(bm_cond);
  if (bm_cond) {
    *v = 42;
    return turbo::OkStatus();
  } else {
    return turbo::UnknownError("This is an error message");
  }
}

// (d): A StatusOr<int64> return value
turbo::StatusOr<int64_t> SimpleIntStatusOrInterface() {
  benchmark::DoNotOptimize(bm_cond);
  if (bm_cond) {
    return 42;
  } else {
    return turbo::StatusOr<int64_t>(
        turbo::UnknownError("This is an error message"));
  }
}

void SetCondition(benchmark::State& state) {
  bm_cond = (state.range(0) == 0);
  state.SetLabel(bm_cond ? "Success" : "Failure");
}

void BM_SimpleIntInterface(benchmark::State& state) {
  SetCondition(state);
  int64_t sum = 0;
  for (auto s : state) {
    int64_t v;
    if (SimpleIntInterface(&v)) {
      sum += v;
    }
    benchmark::DoNotOptimize(sum);
  }
}

void BM_SimpleIntInterfaceMsg(benchmark::State& state) {
  SetCondition(state);
  int64_t sum = 0;
  std::string msg;
  for (auto s : state) {
    int64_t v;
    if (SimpleIntInterfaceWithErrorMessage(&v, &msg)) {
      sum += v;
    }
    benchmark::DoNotOptimize(sum);
    benchmark::DoNotOptimize(msg);
  }
}

void BM_SimpleIntInterfaceStatus(benchmark::State& state) {
  SetCondition(state);
  int64_t sum = 0;
  for (auto s : state) {
    int64_t v;
    auto result = SimpleIntInterfaceWithErrorStatus(&v);
    if (result.ok()) {
      sum += v;
    }
    benchmark::DoNotOptimize(sum);
  }
}

void BM_SimpleIntStatusOrInterface(benchmark::State& state) {
  SetCondition(state);
  int64_t sum = 0;
  for (auto s : state) {
    auto v_s = SimpleIntStatusOrInterface();
    if (v_s.ok()) {
      sum += *v_s;
    }
    benchmark::DoNotOptimize(sum);
  }
}

// Ordered like this so all the success path benchmarks (Arg(0)) show up,
// then all the failure benchmarks (Arg(1))
BENCHMARK(BM_SimpleIntInterface)->Arg(0);
BENCHMARK(BM_SimpleIntInterfaceMsg)->Arg(0);
BENCHMARK(BM_SimpleIntInterfaceStatus)->Arg(0);
BENCHMARK(BM_SimpleIntStatusOrInterface)->Arg(0);
BENCHMARK(BM_SimpleIntInterface)->Arg(1);
BENCHMARK(BM_SimpleIntInterfaceMsg)->Arg(1);
BENCHMARK(BM_SimpleIntInterfaceStatus)->Arg(1);
BENCHMARK(BM_SimpleIntStatusOrInterface)->Arg(1);

}  // namespace
