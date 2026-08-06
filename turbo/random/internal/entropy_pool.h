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

#ifndef TURBO_RANDOM_INTERNAL_ENTROPY_POOL_H_
#define TURBO_RANDOM_INTERNAL_ENTROPY_POOL_H_

#include <cstddef>

#include <turbo/macros/config.h>

namespace turbo {

namespace random_internal {

// GetEntropyFromRandenPool() is a helper function that fills a memory region
// with random bytes from the RandenPool.  This is used by the turbo::BitGen
// implementation to fill the internal buffer.
void GetEntropyFromRandenPool(void* dest, size_t bytes);

}  // namespace random_internal

}  // namespace turbo

#endif  // TURBO_RANDOM_INTERNAL_ENTROPY_POOL_H_
