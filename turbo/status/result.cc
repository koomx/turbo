// Copyright 2020 The Abseil Authors.
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
#include <turbo/status/statusor.h>

#include <cstdlib>
#include <utility>

#include <turbo/base/call_once.h>
#include <turbo/macros/config.h>
#include <turbo/base/internal/raw_logging.h>
#include <turbo/base/nullability.h>
#include <turbo/status/internal/statusor_internal.h>
#include <turbo/status/status.h>
#include <turbo/strings/str_cat.h>

namespace turbo {
    BadResultAccess::BadResultAccess(turbo::Status status)
        : status_(std::move(status)) {
    }

    BadResultAccess::BadResultAccess(const BadResultAccess &other)
        : status_(other.status_) {
    }

    BadResultAccess &BadResultAccess::operator=(
        const BadResultAccess &other) {
        // Ensure assignment is correct regardless of whether this->InitWhat() has
        // already been called.
        other.InitWhat();
        status_ = other.status_;
        what_ = other.what_;
        return *this;
    }

    BadResultAccess &BadResultAccess::operator=(BadResultAccess &&other) {
        // Ensure assignment is correct regardless of whether this->InitWhat() has
        // already been called.
        other.InitWhat();
        status_ = std::move(other.status_);
        what_ = std::move(other.what_);
        return *this;
    }

    BadResultAccess::BadResultAccess(BadResultAccess &&other)
        : status_(std::move(other.status_)) {
    }

    const char * turbo_nonnull BadResultAccess::what() const noexcept {
        InitWhat();
        return what_.c_str();
    }

    const turbo::Status &BadResultAccess::status() const { return status_; }

    void BadResultAccess::InitWhat() const {
        turbo::call_once(init_what_, [this] {
            what_ = turbo::StrCat("Bad Result access: ", status_.ToString());
        });
    }

    namespace internal_statusor {
        void Helper::HandleInvalidStatusCtorArg(turbo::Status * turbo_nonnull status) {
            const char *kMessage =
                    "An OK status is not a valid constructor argument to Result<T>";
#ifdef NDEBUG
            TURBO_INTERNAL_LOG(ERROR, kMessage);
#else
            TURBO_INTERNAL_LOG(FATAL, kMessage);
#endif
            // In optimized builds, we will fall back to internal_error.
            *status = turbo::internal_error(kMessage);
        }

        void Helper::Crash(const turbo::Status &status) {
            TURBO_INTERNAL_LOG(
                FATAL,
                turbo::StrCat("Attempting to fetch value instead of handling error ",
                              status.ToString()));
        }

        void ThrowBadResultAccess(turbo::Status status) {
#if KUMO_HAVE_EXCEPTIONS
            throw turbo::BadResultAccess(std::move(status));
#else
            TURBO_INTERNAL_LOG(
                FATAL,
                turbo::StrCat("Attempting to fetch value instead of handling error ",
                              status.ToString()));
            std::abort();
#endif
        }
    } // namespace internal_statusor
} // namespace turbo
