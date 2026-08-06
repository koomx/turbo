// Copyright 2024 The Abseil Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include <tuple>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <turbo/macros/config.h>
#include <turbo/base/internal/tracing.h>

#if TURBO_HAVE_ATTRIBUTE_WEAK

namespace {

using ::testing::ElementsAre;

using ::turbo::base_internal::ObjectKind;

enum Function { kWait, kContinue, kSignal, kObserved };

using Record = std::tuple<Function, const void*, ObjectKind>;

thread_local std::vector<Record>* tls_records = nullptr;

}  // namespace

namespace turbo {

namespace base_internal {

// Strong extern "C" implementation.
extern "C" {

void TURBO_INTERNAL_C_SYMBOL(TurboInternalTraceWait)(const void* object,
                                                   ObjectKind kind) {
  if (tls_records != nullptr) {
    tls_records->push_back({kWait, object, kind});
  }
}

void TURBO_INTERNAL_C_SYMBOL(TurboInternalTraceContinue)(const void* object,
                                                       ObjectKind kind) {
  if (tls_records != nullptr) {
    tls_records->push_back({kContinue, object, kind});
  }
}

void TURBO_INTERNAL_C_SYMBOL(TurboInternalTraceSignal)(const void* object,
                                                     ObjectKind kind) {
  if (tls_records != nullptr) {
    tls_records->push_back({kSignal, object, kind});
  }
}

void TURBO_INTERNAL_C_SYMBOL(TurboInternalTraceObserved)(const void* object,
                                                       ObjectKind kind) {
  if (tls_records != nullptr) {
    tls_records->push_back({kObserved, object, kind});
  }
}

}  // extern "C"

}  // namespace base_internal

}  // namespace turbo

namespace {

TEST(TracingInternal, InvokesStrongFunctionWithNullptr) {
  std::vector<Record> records;
  tls_records = &records;
  auto kind = turbo::base_internal::ObjectKind::kUnknown;
  turbo::base_internal::TraceWait(nullptr, kind);
  turbo::base_internal::TraceContinue(nullptr, kind);
  turbo::base_internal::TraceSignal(nullptr, kind);
  turbo::base_internal::TraceObserved(nullptr, kind);
  tls_records = nullptr;

  EXPECT_THAT(records, ElementsAre(Record{kWait, nullptr, kind},
                                   Record{kContinue, nullptr, kind},
                                   Record{kSignal, nullptr, kind},
                                   Record{kObserved, nullptr, kind}));
}

TEST(TracingInternal, InvokesStrongFunctionWithObjectAddress) {
  int object = 0;
  std::vector<Record> records;
  tls_records = &records;
  auto kind = turbo::base_internal::ObjectKind::kUnknown;
  turbo::base_internal::TraceWait(&object, kind);
  turbo::base_internal::TraceContinue(&object, kind);
  turbo::base_internal::TraceSignal(&object, kind);
  turbo::base_internal::TraceObserved(&object, kind);
  tls_records = nullptr;

  EXPECT_THAT(records, ElementsAre(Record{kWait, &object, kind},
                                   Record{kContinue, &object, kind},
                                   Record{kSignal, &object, kind},
                                   Record{kObserved, &object, kind}));
}

}  // namespace

#endif  // TURBO_HAVE_ATTRIBUTE_WEAK
