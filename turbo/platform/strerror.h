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

#pragma once

#include <string>

#include <turbo/macros/config.h>

namespace turbo {

    // A portable and thread-safe alternative to C89's `strerror`.
    //
    // The C89 specification of `strerror` is not suitable for use in a
    // multi-threaded application as the returned string may be changed by calls to
    // `strerror` from another thread.  The many non-stdlib alternatives differ
    // enough in their names, availability, and semantics to justify this wrapper
    // around them.  `errno` will not be modified by a call to `turbo::str_error`.
    std::string str_error(int errnum);


    // `ErrnoSaver` captures the value of `errno` upon construction and restores it
    // upon deletion.  It is used in low-level code and must be super fast.  Do not
    // add instrumentation, even in debug modes.
    class ErrnoSaver {
    public:
        ErrnoSaver() : saved_errno_(errno) {}
        ~ErrnoSaver() { errno = saved_errno_; }
        int operator()() const { return saved_errno_; }

    private:
        const int saved_errno_;
    };


} // namespace turbo
