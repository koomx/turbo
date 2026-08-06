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

#ifdef __cplusplus
#error This is a C compile test
#endif

// Ensures the macros umbrella and commonly used macros are C-compatible.
#include <turbo/macros/macros.h>

static int use_macros(int x) {
  int buf[4];
  KUMO_ASSERT(x >= 0);
  KUMO_DASSERT(x != -1);
  KUMO_HARDENING_ASSERT(x >= 0);
  KUMO_ASSUME(x >= 0);
  KUMO_BLOCK_TAIL_CALL_OPTIMIZATION();
  if (KUMO_LIKELY(x > 0)) {
    return (int)KUMO_ARRAYSIZE(buf) + x;
  }
  if (KUMO_UNLIKELY(x < 0)) {
    return -1;
  }
  return 0;
}

static void never_called_unreachable(void) {
  KUMO_UNREACHABLE();
}

int main(void) {
  (void)never_called_unreachable;
  return use_macros(1) > 0 ? 0 : 1;
}
