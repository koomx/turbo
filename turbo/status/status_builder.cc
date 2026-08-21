// Copyright 2026 The Abseil Authors
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

#include <turbo/status/status_builder.h>

#include <cstdint>
#include <memory>
#include <ostream>
#include <string>
#include <type_traits>
#include <utility>

#include <turbo/macros/config.h>
#include <turbo/status/status.h>
#include <turbo/strings/str_cat.h>
#include <string_view>
#include <turbo/types/source_location.h>

namespace turbo {
    void StatusBuilder::Destroy(std::unique_ptr<Rep>) {
        // nothing to do. The unique_ptr will do the cleanup.
    }

    // These constructors are not-inlined and defined in the .cc file to reduce
    // binary size. See cl/354351433 for a quantification.
    StatusBuilder::StatusBuilder() = default;

    StatusBuilder::StatusBuilder(const turbo::Status &original_status,
                                 turbo::SourceLocation location)
        : loc_(location), rep_(InitRep(original_status)) {
    }

    StatusBuilder::operator turbo::Status() const & {
        if (rep_ == nullptr) return turbo::Status();
        return CreateStatusAndConditionallyLog(loc_, std::make_unique<Rep>(*rep_));
    }

    StatusBuilder::Rep::Rep(const turbo::Status &s) : status(s) {
    }

    StatusBuilder::Rep::Rep(turbo::Status &&s) : status(std::move(s)) {
    }

    StatusBuilder::Rep::~Rep() = default;

    StatusBuilder::Rep *StatusBuilder::InitRepImpl(turbo::Status s) {
        if (s.ok()) {
            return nullptr;
        } else {
            return new Rep(std::move(s));
        }
    }

    StatusBuilder::Rep::Rep(const Rep &r)
        : status(r.status),
          logging_mode(r.logging_mode),
          log_severity(r.log_severity),
          verbose_level(r.verbose_level),
          n(r.n),
          period(r.period),
          stream_message(r.stream_message),
          sink(r.sink),
          message_join_style(r.message_join_style),
          should_log_stack_trace(r.should_log_stack_trace),
          also_send_to_log(r.also_send_to_log) {
        if (r.stream.has_value()) {
            InitStream();
        }
    }

    void StatusBuilder::Rep::InitStream() { stream.emplace(stream_message); }

    bool StatusBuilder::HasPayload() const {
        static constexpr std::string_view kMessageSetUrl =
                "type.googleapis.com/util.MessageSetPayload";
        return rep_ != nullptr && rep_->status.get_payload(kMessageSetUrl).has_value();
    }

    KUMO_ATTRIBUTE_WEAK StatusBuilder &StatusBuilder::SetCode(
        turbo::StatusCode code) & {
        if (rep_ == nullptr) {
            rep_ = std::make_unique<StatusBuilder::Rep>(
                turbo::Status(code, std::string_view(), turbo::SourceLocation()));
        } else {
            turbo::Status status(code, std::string_view(), turbo::SourceLocation());
            rep_->status.for_each_payload(
                [&status](std::string_view type_url, const std::string &payload) {
                    status.set_payload(type_url, payload);
                });
            rep_->status = std::move(status);
        }
        return *this;
    }

    KUMO_ATTRIBUTE_WEAK void TurboInternalSetErrorCode(StatusBuilder &builder,
                                                       turbo::StatusCode code) {
        builder.SetCode(code);
    }

    class status_internal::StatusPrivateAccessorForStatusBuilder {
    public:
        static turbo::Status SetMessage(const turbo::Status &status,
                                        std::string_view message) {
            KUMO_ASSERT(!status.ok());

            if (message.empty()) {
                return turbo::Status(status.code(), status.sub_type(), status.sub_code(),
                    message, turbo::SourceLocation());
            }

            using StatusRep =
                    std::remove_cv_t<std::remove_pointer_t<decltype(Status::rep_to_pointer(
                        std::declval<uintptr_t>()))> >;
            StatusRep *rep;
            if (Status::is_inlined(status.rep_)) {
                rep = new StatusRep(Status::inlined_rep_to_code(status.rep_), message,
                                    nullptr, Status::inlined_rep_to_sub_type(status.rep_),
                                    Status::inlined_rep_to_sub_code(status.rep_));
            } else {
                rep = Status::rep_to_pointer(status.rep_)->Clone(message, true, true);
            }
            return turbo::Status(Status::pointer_to_rep(rep));
        }

        static turbo::Status JoinMessageToStatus(turbo::Status s, std::string_view msg,
                                                 MessageJoinStyle style) {
            if (s.ok() || msg.empty()) return s;
            const std::string_view original_message = s.message();
            switch (style) {
                case MessageJoinStyle::kAnnotate: {
                    std::string annotated;
                    if (!original_message.empty()) {
                        turbo::str_append(&annotated, original_message, "; ", msg);
                        msg = annotated;
                    }
                    return SetMessage(s, msg);
                }
                case MessageJoinStyle::kPrepend:
                    return SetMessage(s, turbo::str_cat(msg, original_message));
                case MessageJoinStyle::kAppend:
                    return SetMessage(s, turbo::str_cat(original_message, msg));
                default:
                    return turbo::internal_error("Unknown MessageJoinStyle");
            }
        }
    };

    KUMO_ATTRIBUTE_WEAK std::string StatusBuilder::CurrentStackTrace() {
        return std::string();
    }

    KUMO_ATTRIBUTE_WEAK turbo::Status StatusBuilder::CreateStatusAndConditionallyLog(
        turbo::SourceLocation loc, std::unique_ptr<Rep> rep) {
        if (rep == nullptr) return turbo::ok_status();
        turbo::Status result = status_internal::StatusPrivateAccessorForStatusBuilder::
                JoinMessageToStatus(std::move(rep->status), rep->stream_message,
                                    rep->message_join_style);
        // Passing in the `loc` last to ensure the sequence of the source locations.
        result.add_source_location(loc);
        return result;
    }

    KUMO_ATTRIBUTE_WEAK std::string StatusBuilder::ToString() const {
        if (rep_ == nullptr) {
            return turbo::ok_status().ToString();
        }

        return status_internal::StatusPrivateAccessorForStatusBuilder::
                JoinMessageToStatus(rep_->status, rep_->stream_message,
                                    rep_->message_join_style)
                .with_source_location(loc_)
                .ToString();
    }

    KUMO_ATTRIBUTE_WEAK std::ostream &operator<<(std::ostream &os,
                                                 const StatusBuilder &builder) {
        return os << static_cast<turbo::Status>(builder);
    }

    KUMO_ATTRIBUTE_WEAK std::ostream &operator<<(std::ostream &os,
                                                 StatusBuilder &&builder) {
        return os << static_cast<turbo::Status>(std::move(builder));
    }
} // namespace turbo
