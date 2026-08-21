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

#ifndef TURBO_BASE_INTERNAL_TRACING_H_
#define TURBO_BASE_INTERNAL_TRACING_H_

#include <turbo/macros/config.h>

namespace turbo {

    namespace base_internal {

        // Well known Abseil object types that have causality.
        enum class ObjectKind { kUnknown,
            kBlockingCounter,
            kNotification };

        // `TraceWait` and `TraceContinue` record the start and end of a potentially
        // blocking wait operation on `object`. `object` typically represents a higher
        // level synchronization object such as `turbo::Notification`.
        void TraceWait(const void* object, ObjectKind kind);
        void TraceContinue(const void* object, ObjectKind kind);

        // `TraceSignal` records a signal on `object`.
        void TraceSignal(const void* object, ObjectKind kind);

        // `TraceObserved` records the non-blocking observation of a signaled object.
        void TraceObserved(const void* object, ObjectKind kind);

        // ---------------------------------------------------------------------------
        // Weak implementation detail:
        //
        // We define the weak API as extern "C": in some build configurations we pass
        // `--detect-odr-violations` to the gold linker. This causes it to flag weak
        // symbol overrides as ODR violations. Because ODR only applies to C++ and not
        // C, `--detect-odr-violations` ignores symbols not mangled with C++ names.
        // By changing our extension points to be extern "C", we dodge this check.
        // ---------------------------------------------------------------------------
        extern "C" {

        void TURBO_INTERNAL_C_SYMBOL(TurboInternalTraceWait)(const void* object,
            ObjectKind kind);
        void TURBO_INTERNAL_C_SYMBOL(TurboInternalTraceContinue)(const void* object,
            ObjectKind kind);
        void TURBO_INTERNAL_C_SYMBOL(TurboInternalTraceSignal)(const void* object,
            ObjectKind kind);
        void TURBO_INTERNAL_C_SYMBOL(TurboInternalTraceObserved)(const void* object,
            ObjectKind kind);

        } // extern "C"

        inline void TraceWait(const void* object, ObjectKind kind) {
            TURBO_INTERNAL_C_SYMBOL(TurboInternalTraceWait)(object, kind);
        }

        inline void TraceContinue(const void* object, ObjectKind kind) {
            TURBO_INTERNAL_C_SYMBOL(TurboInternalTraceContinue)(object, kind);
        }

        inline void TraceSignal(const void* object, ObjectKind kind) {
            TURBO_INTERNAL_C_SYMBOL(TurboInternalTraceSignal)(object, kind);
        }

        inline void TraceObserved(const void* object, ObjectKind kind) {
            TURBO_INTERNAL_C_SYMBOL(TurboInternalTraceObserved)(object, kind);
        }

    } // namespace base_internal

} // namespace turbo

#endif // TURBO_BASE_INTERNAL_TRACING_H_
