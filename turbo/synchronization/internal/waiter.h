// Copyright 2017 The Abseil Authors.
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
//

#ifndef TURBO_SYNCHRONIZATION_INTERNAL_WAITER_H_
#define TURBO_SYNCHRONIZATION_INTERNAL_WAITER_H_

#include <turbo/macros/config.h>
#include <turbo/synchronization/internal/futex_waiter.h>
#include <turbo/synchronization/internal/pthread_waiter.h>
#include <turbo/synchronization/internal/sem_waiter.h>
#include <turbo/synchronization/internal/stdcpp_waiter.h>
#include <turbo/synchronization/internal/win32_waiter.h>

// May be chosen at compile time via -DTURBO_FORCE_WAITER_MODE=<index>
#define TURBO_WAITER_MODE_FUTEX 0
#define TURBO_WAITER_MODE_SEM 1
#define TURBO_WAITER_MODE_CONDVAR 2
#define TURBO_WAITER_MODE_WIN32 3
#define TURBO_WAITER_MODE_STDCPP 4

#if defined(TURBO_FORCE_WAITER_MODE)
#define TURBO_WAITER_MODE TURBO_FORCE_WAITER_MODE
#elif defined(TURBO_INTERNAL_HAVE_WIN32_WAITER)
#define TURBO_WAITER_MODE TURBO_WAITER_MODE_WIN32
#elif defined(TURBO_INTERNAL_HAVE_FUTEX_WAITER)
#define TURBO_WAITER_MODE TURBO_WAITER_MODE_FUTEX
#elif defined(TURBO_INTERNAL_HAVE_SEM_WAITER)
#define TURBO_WAITER_MODE TURBO_WAITER_MODE_SEM
#elif defined(TURBO_INTERNAL_HAVE_PTHREAD_WAITER)
#define TURBO_WAITER_MODE TURBO_WAITER_MODE_CONDVAR
#elif defined(TURBO_INTERNAL_HAVE_STDCPP_WAITER)
#define TURBO_WAITER_MODE TURBO_WAITER_MODE_STDCPP
#else
#error TURBO_WAITER_MODE is undefined
#endif

namespace turbo {

namespace synchronization_internal {

#if TURBO_WAITER_MODE == TURBO_WAITER_MODE_FUTEX
using Waiter = FutexWaiter;
#elif TURBO_WAITER_MODE == TURBO_WAITER_MODE_SEM
using Waiter = SemWaiter;
#elif TURBO_WAITER_MODE == TURBO_WAITER_MODE_CONDVAR
using Waiter = PthreadWaiter;
#elif TURBO_WAITER_MODE == TURBO_WAITER_MODE_WIN32
using Waiter = Win32Waiter;
#elif TURBO_WAITER_MODE == TURBO_WAITER_MODE_STDCPP
using Waiter = StdcppWaiter;
#endif

}  // namespace synchronization_internal

}  // namespace turbo

#endif  // TURBO_SYNCHRONIZATION_INTERNAL_WAITER_H_
