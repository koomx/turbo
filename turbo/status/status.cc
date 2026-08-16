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
#include <turbo/status/status.h>

#include <errno.h>

#include <cstdint>
#include <memory>
#include <ostream>
#include <string>
#include <type_traits>
#include <utility>

#include <turbo/macros/config.h>
#include <turbo/base/internal/strerror.h>
#include <turbo/base/no_destructor.h>
#include <turbo/base/nullability.h>
#include <turbo/status/internal/status_internal.h>
#include <turbo/strings/str_cat.h>
#include <string_view>
#include <turbo/types/source_location.h>

namespace turbo {
    static_assert(
        alignof(status_internal::StatusRep) >= 4,
        "turbo::Status assumes it can use the bottom 2 bits of a StatusRep*.");

    std::string status_code_to_string(StatusCode code) {
        return std::string(turbo::status_code_to_string_view(code));
    }

    std::string_view status_code_to_string_view(StatusCode code) {
        switch (code) {
            case StatusCode::kOk:
                return "OK";
            case StatusCode::kCancelled:
                return "CANCELLED";
            case StatusCode::kUnknown:
                return "UNKNOWN";
            case StatusCode::kInvalidArgument:
                return "INVALID_ARGUMENT";
            case StatusCode::kDeadlineExceeded:
                return "DEADLINE_EXCEEDED";
            case StatusCode::kNotFound:
                return "NOT_FOUND";
            case StatusCode::kAlreadyExists:
                return "ALREADY_EXISTS";
            case StatusCode::kPermissionDenied:
                return "PERMISSION_DENIED";
            case StatusCode::kUnauthenticated:
                return "UNAUTHENTICATED";
            case StatusCode::kResourceExhausted:
                return "RESOURCE_EXHAUSTED";
            case StatusCode::kFailedPrecondition:
                return "FAILED_PRECONDITION";
            case StatusCode::kAborted:
                return "ABORTED";
            case StatusCode::kOutOfRange:
                return "OUT_OF_RANGE";
            case StatusCode::kUnimplemented:
                return "UNIMPLEMENTED";
            case StatusCode::kInternal:
                return "INTERNAL";
            case StatusCode::kUnavailable:
                return "UNAVAILABLE";
            case StatusCode::kDataLoss:
                return "DATA_LOSS";
            default:
                return "";
        }
    }

    std::ostream &operator<<(std::ostream &os, StatusCode code) {
        return os << status_code_to_string(code);
    }

    const std::string * turbo_nonnull Status::EmptyString() {
        static const turbo::NoDestructor<std::string> kEmpty;
        return kEmpty.get();
    }

    const std::string * turbo_nonnull Status::MovedFromString() {
        static const turbo::NoDestructor<std::string> kMovedFrom(kMovedFromString);
        return kMovedFrom.get();
    }

    turbo::Status turbo::Status::make_non_ok_status_with_ok_code(
        std::string_view message) {
        return turbo::Status(
            turbo::Status::pointer_to_rep(new turbo::status_internal::StatusRep(
                turbo::StatusCode::kOk, message, nullptr)));
    }

    template<typename StringOrView>
    uintptr_t make_status_rep_impl(uintptr_t inlined_rep, StringOrView msg,
                                turbo::SourceLocation loc) {
        static_assert(std::is_same_v<StringOrView, std::string_view> ||
                      std::is_same_v<StringOrView, std::string &&>);
        bool ok = inlined_rep == Status::code_to_inlined_rep(turbo::StatusCode::kOk);
        if (ok) return inlined_rep;
        if (msg.empty()
        ) {
            return inlined_rep;
        }
        auto *rep =
                new status_internal::StatusRep(Status::inlined_rep_to_code(inlined_rep),
                                               std::forward<StringOrView>(msg), nullptr);
        if (loc.file_name()[0] != '\0') {
            rep->add_source_location(loc);
        }
        return Status::pointer_to_rep(rep);
    }

    uintptr_t Status::make_rep_from_string_view(uintptr_t inlined_rep,
                                            std::string_view msg,
                                            turbo::SourceLocation loc) {
        return make_status_rep_impl<std::string_view>(inlined_rep, msg, loc);
    }

    uintptr_t Status::MakeRepFromStringRvalue(uintptr_t inlined_rep,
                                              std::string &&msg,
                                              turbo::SourceLocation loc) {
        return make_status_rep_impl<std::string &&>(inlined_rep, std::move(msg), loc);
    }

    uintptr_t Status::add_source_location_impl(uintptr_t rep,
                                            turbo::SourceLocation loc) {
        if (is_inlined(rep)) return rep;
        if (loc.file_name()[0] == '\0') return rep;
        status_internal::StatusRep *rep_ptr = PrepareToModify(rep);
        rep_ptr->add_source_location(loc);
        return pointer_to_rep(rep_ptr);
    }

    status_internal::StatusRep * turbo_nonnull Status::PrepareToModify(
        uintptr_t rep) {
        if (is_inlined(rep)) {
            return new status_internal::StatusRep(inlined_rep_to_code(rep),
                                                  std::string_view(), nullptr);
        }
        return rep_to_pointer(rep)->CloneAndUnref();
    }

    std::string Status::to_string_slow(uintptr_t rep, StatusToStringMode mode) {
        if (is_inlined(rep)) {
            return turbo::str_cat(turbo::status_code_to_string(inlined_rep_to_code(rep)), ": ");
        }
        return rep_to_pointer(rep)->ToString(mode);
    }

    std::ostream &operator<<(std::ostream &os, const Status &x) {
        os << x.ToString(StatusToStringMode::kWithEverything);
        return os;
    }

    namespace status_internal {
        // We use an int in the template parameter to shorten mangled names.
        template<int error_code>
        Status make_error_impl(std::string_view message, turbo::SourceLocation loc) {
            return Status(static_cast<StatusCode>(error_code), message, loc);
        }

        // Explicit instantiation for all the error codes.
        // If we add more error code, we need to add their values on this list.
        // Using ints here instead of static_cast<int>(StatusCode::kFoo) makes it easier
        // to see that the list is complete.
        template Status make_error_impl<0>(std::string_view, turbo::SourceLocation);

        template Status make_error_impl<1>(std::string_view, turbo::SourceLocation);

        template Status make_error_impl<2>(std::string_view, turbo::SourceLocation);

        template Status make_error_impl<3>(std::string_view, turbo::SourceLocation);

        template Status make_error_impl<4>(std::string_view, turbo::SourceLocation);

        template Status make_error_impl<5>(std::string_view, turbo::SourceLocation);

        template Status make_error_impl<6>(std::string_view, turbo::SourceLocation);

        template Status make_error_impl<7>(std::string_view, turbo::SourceLocation);

        template Status make_error_impl<8>(std::string_view, turbo::SourceLocation);

        template Status make_error_impl<9>(std::string_view, turbo::SourceLocation);

        template Status make_error_impl<10>(std::string_view, turbo::SourceLocation);

        template Status make_error_impl<11>(std::string_view, turbo::SourceLocation);

        template Status make_error_impl<12>(std::string_view, turbo::SourceLocation);

        template Status make_error_impl<13>(std::string_view, turbo::SourceLocation);

        template Status make_error_impl<14>(std::string_view, turbo::SourceLocation);

        template Status make_error_impl<15>(std::string_view, turbo::SourceLocation);

        template Status make_error_impl<16>(std::string_view, turbo::SourceLocation);
    } // namespace status_internal

    bool is_aborted(const Status &status) {
        return status.code() == turbo::StatusCode::kAborted;
    }

    bool is_already_exists(const Status &status) {
        return status.code() == turbo::StatusCode::kAlreadyExists;
    }

    bool is_cancelled(const Status &status) {
        return status.code() == turbo::StatusCode::kCancelled;
    }

    bool is_data_loss(const Status &status) {
        return status.code() == turbo::StatusCode::kDataLoss;
    }

    bool is_deadline_exceeded(const Status &status) {
        return status.code() == turbo::StatusCode::kDeadlineExceeded;
    }

    bool is_failed_precondition(const Status &status) {
        return status.code() == turbo::StatusCode::kFailedPrecondition;
    }

    bool is_internal(const Status &status) {
        return status.code() == turbo::StatusCode::kInternal;
    }

    bool is_invalid_argument(const Status &status) {
        return status.code() == turbo::StatusCode::kInvalidArgument;
    }

    bool is_not_found(const Status &status) {
        return status.code() == turbo::StatusCode::kNotFound;
    }

    bool is_out_of_range(const Status &status) {
        return status.code() == turbo::StatusCode::kOutOfRange;
    }

    bool is_permission_denied(const Status &status) {
        return status.code() == turbo::StatusCode::kPermissionDenied;
    }

    bool is_resource_exhausted(const Status &status) {
        return status.code() == turbo::StatusCode::kResourceExhausted;
    }

    bool is_unauthenticated(const Status &status) {
        return status.code() == turbo::StatusCode::kUnauthenticated;
    }

    bool is_unavailable(const Status &status) {
        return status.code() == turbo::StatusCode::kUnavailable;
    }

    bool is_unimplemented(const Status &status) {
        return status.code() == turbo::StatusCode::kUnimplemented;
    }

    bool is_unknown(const Status &status) {
        return status.code() == turbo::StatusCode::kUnknown;
    }

    StatusCode ErrnoToStatusCode(int error_number) {
        switch (error_number) {
            case 0:
                return StatusCode::kOk;
            case EINVAL: // Invalid argument
            case ENAMETOOLONG: // Filename too long
            case E2BIG: // Argument list too long
            case EDESTADDRREQ: // Destination address required
            case EDOM: // Mathematics argument out of domain of function
            case EFAULT: // Bad address
            case EILSEQ: // Illegal byte sequence
            case ENOPROTOOPT: // Protocol not available
            case ENOTSOCK: // Not a socket
            case ENOTTY: // Inappropriate I/O control operation
            case EPROTOTYPE: // Protocol wrong type for socket
            case ESPIPE: // Invalid seek
                return StatusCode::kInvalidArgument;
            case ETIMEDOUT: // Connection timed out
                return StatusCode::kDeadlineExceeded;
            case ENODEV: // No such device
            case ENOENT: // No such file or directory
#ifdef ENOMEDIUM
            case ENOMEDIUM: // No medium found
#endif
            case ENXIO: // No such device or address
            case ESRCH: // No such process
                return StatusCode::kNotFound;
            case EEXIST: // File exists
            case EADDRNOTAVAIL: // Address not available
            case EALREADY: // Connection already in progress
#ifdef ENOTUNIQ
            case ENOTUNIQ: // Name not unique on network
#endif
                return StatusCode::kAlreadyExists;
            case EPERM: // Operation not permitted
            case EACCES: // Permission denied
#ifdef ENOKEY
            case ENOKEY: // Required key not available
#endif
            case EROFS: // Read only file system
                return StatusCode::kPermissionDenied;
            case ENOTEMPTY: // Directory not empty
            case EISDIR: // Is a directory
            case ENOTDIR: // Not a directory
            case EADDRINUSE: // Address already in use
            case EBADF: // Invalid file descriptor
#ifdef EBADFD
            case EBADFD: // File descriptor in bad state
#endif
            case EBUSY: // Device or resource busy
            case ECHILD: // No child processes
            case EISCONN: // Socket is connected
#ifdef EISNAM
            case EISNAM: // Is a named type file
#endif
#ifdef ENOTBLK
            case ENOTBLK: // Block device required
#endif
            case ENOTCONN: // The socket is not connected
            case EPIPE: // Broken pipe
#ifdef ESHUTDOWN
            case ESHUTDOWN: // Cannot send after transport endpoint shutdown
#endif
            case ETXTBSY: // Text file busy
#ifdef EUNATCH
            case EUNATCH: // Protocol driver not attached
#endif
                return StatusCode::kFailedPrecondition;
            case ENOSPC: // No space left on device
#ifdef EDQUOT
            case EDQUOT: // Disk quota exceeded
#endif
            case EMFILE: // Too many open files
            case EMLINK: // Too many links
            case ENFILE: // Too many open files in system
            case ENOBUFS: // No buffer space available
            case ENOMEM: // Not enough space
#ifdef EUSERS
            case EUSERS: // Too many users
#endif
                return StatusCode::kResourceExhausted;
#ifdef ECHRNG
            case ECHRNG: // Channel number out of range
#endif
            case EFBIG: // File too large
            case EOVERFLOW: // Value too large to be stored in data type
            case ERANGE: // Result too large
                return StatusCode::kOutOfRange;
#ifdef ENOPKG
            case ENOPKG: // Package not installed
#endif
            case ENOSYS: // Function not implemented
            case ENOTSUP: // Operation not supported
            case EAFNOSUPPORT: // Address family not supported
#ifdef EPFNOSUPPORT
            case EPFNOSUPPORT: // Protocol family not supported
#endif
            case EPROTONOSUPPORT: // Protocol not supported
#ifdef ESOCKTNOSUPPORT
            case ESOCKTNOSUPPORT: // Socket type not supported
#endif
            case EXDEV: // Improper link
                return StatusCode::kUnimplemented;
            case EAGAIN: // Resource temporarily unavailable
#ifdef ECOMM
            case ECOMM: // Communication error on send
#endif
            case ECONNREFUSED: // Connection refused
            case ECONNABORTED: // Connection aborted
            case ECONNRESET: // Connection reset
            case EINTR: // Interrupted function call
#ifdef EHOSTDOWN
            case EHOSTDOWN: // Host is down
#endif
            case EHOSTUNREACH: // Host is unreachable
            case ENETDOWN: // Network is down
            case ENETRESET: // Connection aborted by network
            case ENETUNREACH: // Network unreachable
            case ENOLCK: // No locks available
            case ENOLINK: // Link has been severed
#ifdef ENONET
            case ENONET: // Machine is not on the network
#endif
                return StatusCode::kUnavailable;
            case EDEADLK: // Resource deadlock avoided
#ifdef ESTALE
            case ESTALE: // Stale file handle
#endif
                return StatusCode::kAborted;
            case ECANCELED: // Operation cancelled
                return StatusCode::kCancelled;
            default:
                return StatusCode::kUnknown;
        }
    }

    namespace {
        std::string MessageForErrnoToStatus(int error_number,
                                            std::string_view message) {
            return turbo::str_cat(message, ": ",
                                 turbo::base_internal::StrError(error_number));
        }
    } // namespace

    Status ErrnoToStatus(int error_number, std::string_view message,
                         turbo::SourceLocation loc) {
        return Status(ErrnoToStatusCode(error_number),
                      MessageForErrnoToStatus(error_number, message), loc);
    }

    const char * turbo_nonnull status_message_as_cstr(const Status &status) {
        // As an internal implementation detail, we guarantee that if status.message()
        // is non-empty, then the resulting std::string_view is null terminated.
        auto sv_message = status.message();
        return sv_message.empty() ? "" : sv_message.data();
    }
} // namespace turbo
