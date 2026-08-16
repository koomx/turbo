// Copyright 2019 The Abseil Authors.
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
#ifndef TURBO_STATUS_INTERNAL_STATUS_INTERNAL_H_
#define TURBO_STATUS_INTERNAL_STATUS_INTERNAL_H_

// IWYU pragma: private, include "turbo/status/status.h"

#include <atomic>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <turbo/types/source_location.h>
#include <string_view>
#include <turbo/base/nullability.h>
#include <turbo/functional/function_ref.h>
#include <turbo/macros/config.h>
#include <turbo/types/inlined_vector.h>
#include <turbo/types/optional_ref.h>
#include <turbo/types/span.h>

#ifndef SWIG
// Disabled for SWIG as it doesn't parse attributes correctly.
namespace turbo {
    // Returned Status objects may not be ignored. Codesearch doesn't handle ifdefs
    // as part of a class definitions (b/6995610), so we use a forward declaration.
    //
    // TODO(b/176172494): KUMO_MUST_USE_RESULT should expand to the more strict
    // [[nodiscard]]. For now, just use [[nodiscard]] directly when it is available.
#if KUMO_HAVE_CPP_ATTRIBUTE(nodiscard)
    class [[nodiscard]] KUMO_ATTRIBUTE_TRIVIAL_ABI
        Status;
#else
    class KUMO_MUST_USE_RESULT KUMO_ATTRIBUTE_TRIVIAL_ABI
        Status;
#endif
} // namespace turbo
#endif // !SWIG

namespace turbo {
    enum class StatusCode : int;
    enum class StatusToStringMode : int;

    // Forward declaration of Result for Status friendship.
    template <typename T>
    class Result;

    namespace status_internal {
#ifndef SWIG
        class StatusPrivateAccessor;
        class StatusPrivateAccessorForStatusBuilder;
#endif // !SWIG

        // Container for status payloads.
        struct Payload {
            std::string type_url;
            std::string payload;
        };

        using Payloads = turbo::InlinedVector<Payload, 1>;

        // Reference-counted representation of Status data.
        class StatusRep {
        public:
            StatusRep(turbo::StatusCode code_arg, std::string_view message_arg,
                std::unique_ptr<status_internal::Payloads> payloads_arg)
                : ref_(int32_t { 1 })
                , code_(code_arg)
                , message_(message_arg)
                , payloads_(std::move(payloads_arg)) {
            }

            template <typename String,
                typename = std::enable_if_t<std::is_same_v<String, std::string>>>
            StatusRep(turbo::StatusCode code_arg, String&& message_arg,
                std::unique_ptr<status_internal::Payloads> payloads_arg)
                : ref_(int32_t { 1 })
                , code_(code_arg)
                , message_(std::forward<String>(message_arg))
                , payloads_(std::move(payloads_arg)) {
            }

            turbo::StatusCode code() const { return code_; }
            const std::string& message() const { return message_; }

            // Ref and unref are const to allow access through a const pointer, and are
            // used during copying operations.
            void Ref() const { ref_.fetch_add(1, std::memory_order_relaxed); }

            void Unref() const;

            // Payload methods correspond to the same methods in turbo::Status.
            std::optional<std::string> get_payload(std::string_view type_url) const;

            void set_payload(std::string_view type_url, std::string payload);

            struct EraseResult {
                bool erased;
                uintptr_t new_rep;
            };

            EraseResult erase_payload(std::string_view type_url);

            void for_each_payload(
                turbo::FunctionRef<void(std::string_view, const std::string&)> visitor)
                const;

            turbo::Span<const turbo::SourceLocation> GetSourceLocations() const;

            void add_source_location(turbo::SourceLocation loc);

            std::string ToString(StatusToStringMode mode) const;

            bool operator==(const StatusRep& other) const;

            bool operator!=(const StatusRep& other) const { return !(*this == other); }

            // Returns an equivalent heap allocated StatusRep with refcount 1.
            //
            // If `new_message` is provided, the message will be replaced with the new
            // message.
            StatusRep* turbo_nonnull Clone(
                turbo::optional_ref<std::string_view> new_message, bool include_payloads,
                bool include_source_locations) const;

            // Same as Clone(), but also removes a reference to `this`. `this` is not safe
            // to be used after calling as it may have been deleted.
            StatusRep* turbo_nonnull CloneAndUnref(
                turbo::optional_ref<std::string_view> new_message, bool include_payloads,
                bool include_source_locations) const;

            StatusRep* turbo_nonnull CloneAndUnref() const;

        private:
            mutable std::atomic<int32_t> ref_;
            turbo::StatusCode code_;

            // As an internal implementation detail, we guarantee that if status.message()
            // is non-empty, then the resulting std::string_view is null terminated.
            // This is required to implement 'status_message_as_cstr(...)'
            //
            // NOTE: if most statuses are constructed with messages that are either empty
            // or so long they don't fit in the std::string's local storage (small string
            // optimization), replacing std::string with an entirely heap-allocated
            // string might save memory at scale.
            std::string message_;

            turbo::InlinedVector<turbo::SourceLocation, 1> source_locations_;
            std::unique_ptr<status_internal::Payloads> payloads_;
        };

        turbo::StatusCode MapToLocalCode(int value);

        // Returns a pointer to a newly-allocated string with the given `prefix`,
        // suitable for output as an error message in assertion/`KCHECK()` failures.
        //
        // This is an internal implementation detail for Abseil logging.
        KUMO_ATTRIBUTE_PURE_FUNCTION
        const char* turbo_nonnull make_check_fail_string(
            const turbo::Status* turbo_nonnull status, const char* turbo_nonnull prefix);
    } // namespace status_internal
} // namespace turbo

#endif // TURBO_STATUS_INTERNAL_STATUS_INTERNAL_H_
