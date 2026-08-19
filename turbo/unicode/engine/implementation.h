#ifndef SIMDUTF_IMPLEMENTATION_H
#define SIMDUTF_IMPLEMENTATION_H
#if !defined(SIMDUTF_NO_THREADS)
#include <atomic>
#endif
#ifdef SIMDUTF_INTERNAL_TESTS
#include <vector>
#endif
#include <turbo/unicode/engine/common_defs.h>
#include <turbo/unicode/engine/compiler_check.h>
#include <turbo/unicode/engine/encoding_types.h>
#include <turbo/unicode/engine/error.h>
#include <turbo/unicode/internal/isadetection.h>

#include <string_view>


#ifndef SIMDUTF_FEATURE_DETECT_ENCODING
#define SIMDUTF_FEATURE_DETECT_ENCODING 1
#endif
#ifndef SIMDUTF_FEATURE_ASCII
#define SIMDUTF_FEATURE_ASCII 1
#endif
#ifndef SIMDUTF_FEATURE_LATIN1
#define SIMDUTF_FEATURE_LATIN1 1
#endif
#ifndef SIMDUTF_FEATURE_UTF8
#define SIMDUTF_FEATURE_UTF8 1
#endif
#ifndef SIMDUTF_FEATURE_UTF16
#define SIMDUTF_FEATURE_UTF16 1
#endif
#ifndef SIMDUTF_FEATURE_UTF32
#define SIMDUTF_FEATURE_UTF32 1
#endif
#ifndef SIMDUTF_FEATURE_BASE64
#define SIMDUTF_FEATURE_BASE64 1
#endif

/// helpers placed in namespace detail are not a part of the public API
namespace turbo {
    namespace detail {
        namespace {
            // this is to avoid including <algorithm> just for min
            constexpr std::size_t min(std::size_t a, std::size_t b) {
                return a < b ? a : b;
            }
            template <typename T, typename U>
            constexpr std::size_t min(const T& a, const U& b) = delete;
        } // namespace
    } // namespace detail
} // namespace turbo

#if SIMDUTF_CPLUSPLUS23
#include <turbo/unicode/engine/constexpr_ptr.h>
#endif


// these includes are needed for constexpr support. they are
// not part of the public api.
#include <turbo/unicode/engine/scalar/swap_bytes.h>
#include <turbo/unicode/engine/scalar/ascii.h>
#include <turbo/unicode/engine/scalar/atomic_util.h>
#include <turbo/unicode/engine/scalar/latin1.h>
#include <turbo/unicode/engine/scalar/latin1_to_utf16/latin1_to_utf16.h>
#include <turbo/unicode/engine/scalar/latin1_to_utf32/latin1_to_utf32.h>
#include <turbo/unicode/engine/scalar/latin1_to_utf8/latin1_to_utf8.h>
#include <turbo/unicode/engine/scalar/utf16.h>
#include <turbo/unicode/engine/scalar/utf16_to_latin1/utf16_to_latin1.h>
#include <turbo/unicode/engine/scalar/utf16_to_latin1/valid_utf16_to_latin1.h>
#include <turbo/unicode/engine/scalar/utf16_to_utf32/utf16_to_utf32.h>
#include <turbo/unicode/engine/scalar/utf16_to_utf32/valid_utf16_to_utf32.h>
#include <turbo/unicode/engine/scalar/utf16_to_utf8/utf16_to_utf8.h>
#include <turbo/unicode/engine/scalar/utf16_to_utf8/valid_utf16_to_utf8.h>
#include <turbo/unicode/engine/scalar/utf32.h>
#include <turbo/unicode/engine/scalar/utf32_to_latin1/utf32_to_latin1.h>
#include <turbo/unicode/engine/scalar/utf32_to_latin1/valid_utf32_to_latin1.h>
#include <turbo/unicode/engine/scalar/utf32_to_utf16/utf32_to_utf16.h>
#include <turbo/unicode/engine/scalar/utf32_to_utf16/valid_utf32_to_utf16.h>
#include <turbo/unicode/engine/scalar/utf32_to_utf8/utf32_to_utf8.h>
#include <turbo/unicode/engine/scalar/utf32_to_utf8/valid_utf32_to_utf8.h>
#include <turbo/unicode/engine/scalar/utf8.h>
#include <turbo/unicode/engine/scalar/utf8_to_latin1/utf8_to_latin1.h>
#include <turbo/unicode/engine/scalar/utf8_to_latin1/valid_utf8_to_latin1.h>
#include <turbo/unicode/engine/scalar/utf8_to_utf16/utf8_to_utf16.h>
#include <turbo/unicode/engine/scalar/utf8_to_utf16/valid_utf8_to_utf16.h>
#include <turbo/unicode/engine/scalar/utf8_to_utf32/utf8_to_utf32.h>
#include <turbo/unicode/engine/scalar/utf8_to_utf32/valid_utf8_to_utf32.h>

#include <turbo/unicode/api/detect.h>
#include <turbo/unicode/api/utf8.h>
#include <turbo/unicode/api/ascii.h>
#include <turbo/unicode/api/utf16.h>
#include <turbo/unicode/api/utf32.h>
#include <turbo/unicode/api/latin1.h>
#include <turbo/unicode/api/base64.h>

namespace turbo {



#ifndef SIMDUTF_NEED_TRAILING_ZEROES
#define SIMDUTF_NEED_TRAILING_ZEROES 1
#endif

    /// An implementation of simdutf for a particular CPU architecture.
    ///
    /// Also used to maintain the currently active implementation. The active
    /// implementation is automatically initialized on first use to the most advanced
    /// implementation supported by the host.
    class implementation {
    public:
        /// The name of this implementation.
        ///
        ///     const implementation *impl = turbo::active_implementation;
        ///     cout << "simdutf is optimized for " << impl->name() << "(" <<
        /// impl->description() << ")" << endl;
        ///
        /// @return the name of the implementation, e.g. "haswell", "westmere", "arm64"
        virtual std::string_view name() const noexcept { return _name; }

        /// The description of this implementation.
        ///
        ///     const implementation *impl = turbo::active_implementation;
        ///     cout << "simdutf is optimized for " << impl->name() << "(" <<
        /// impl->description() << ")" << endl;
        ///
        /// @return the name of the implementation, e.g. "haswell", "westmere", "arm64"
        virtual std::string_view description() const noexcept { return _description; }

        /// The instruction sets this implementation is compiled against
        /// and the current CPU match. This function may poll the current CPU/system
        /// and should therefore not be called too often if performance is a concern.
        ///
        ///
        /// @return true if the implementation can be safely used on the current system
        /// (determined at runtime)
        bool supported_by_runtime_system() const;

        /// This function will try to detect the encoding
        /// @param input the string to identify
        /// @param length the length of the string in bytes.
        /// @return the encoding type detected
        virtual encoding_type autodetect_encoding(const char* input,
            size_t length) const noexcept;

        /// This function will try to detect the possible encodings in one pass
        /// @param input the string to identify
        /// @param length the length of the string in bytes.
        /// @return the encoding type detected
        virtual int detect_encodings(const char* input,
            size_t length) const noexcept
            = 0;

        /// @private For internal implementation use
        ///
        /// The instruction sets this implementation is compiled against.
        ///
        /// @return a mask of all required `internal::instruction_set::` values
        virtual uint32_t required_instruction_sets() const {
            return _required_instruction_sets;
        }

        /// Validate the UTF-8 string.
        ///
        /// Overridden by each implementation.
        ///
        /// @param buf the UTF-8 string to validate.
        /// @param len the length of the string in bytes.
        /// @return true if and only if the string is valid UTF-8.
        simdutf_warn_unused virtual bool validate_utf8(const char* buf,
            size_t len) const noexcept
            = 0;

        /// Validate the UTF-8 string and stop on errors.
        ///
        /// Overridden by each implementation.
        ///
        /// @param buf the UTF-8 string to validate.
        /// @param len the length of the string in bytes.
        /// @return a result pair struct (of type turbo::result containing the two
        /// fields error and count) with an error code and either position of the error
        /// (in the input in code units) if any, or the number of code units validated
        /// if successful.
        simdutf_warn_unused virtual result
        validate_utf8_with_errors(const char* buf, size_t len) const noexcept
            = 0;

        /// Validate the ASCII string.
        ///
        /// Overridden by each implementation.
        ///
        /// @param buf the ASCII string to validate.
        /// @param len the length of the string in bytes.
        /// @return true if and only if the string is valid ASCII.
        simdutf_warn_unused virtual bool
        validate_ascii(const char* buf, size_t len) const noexcept
            = 0;

        /// Validate the ASCII string and stop on error.
        ///
        /// Overridden by each implementation.
        ///
        /// @param buf the ASCII string to validate.
        /// @param len the length of the string in bytes.
        /// @return a result pair struct (of type turbo::result containing the two
        /// fields error and count) with an error code and either position of the error
        /// (in the input in code units) if any, or the number of code units validated
        /// if successful.
        simdutf_warn_unused virtual result
        validate_ascii_with_errors(const char* buf, size_t len) const noexcept
            = 0;


        /// Validate the ASCII string as a UTF-16BE sequence.
        /// An UTF-16 sequence is considered an ASCII sequence
        /// if it could be converted to an ASCII string losslessly.
        ///
        /// Overridden by each implementation.
        ///
        /// @param buf the UTF-16BE string to validate.
        /// @param len the length of the string in bytes.
        /// @return true if and only if the string is valid ASCII.
        simdutf_warn_unused virtual bool
        validate_utf16be_as_ascii(const char16_t* buf, size_t len) const noexcept
            = 0;

        /// Validate the ASCII string as a UTF-16LE sequence.
        /// An UTF-16 sequence is considered an ASCII sequence
        /// if it could be converted to an ASCII string losslessly.
        ///
        /// Overridden by each implementation.
        ///
        /// @param buf the UTF-16LE string to validate.
        /// @param len the length of the string in bytes.
        /// @return true if and only if the string is valid ASCII.
        simdutf_warn_unused virtual bool
        validate_utf16le_as_ascii(const char16_t* buf, size_t len) const noexcept
            = 0;

        /// Validate the UTF-16LE string.This function may be best when you expect
        /// the input to be almost always valid. Otherwise, consider using
        /// validate_utf16le_with_errors.
        ///
        /// Overridden by each implementation.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param buf the UTF-16LE string to validate.
        /// @param len the length of the string in number of 2-byte code units
        /// (char16_t).
        /// @return true if and only if the string is valid UTF-16LE.
        simdutf_warn_unused virtual bool
        validate_utf16le(const char16_t* buf, size_t len) const noexcept
            = 0;


        /// Validate the UTF-16BE string. This function may be best when you expect
        /// the input to be almost always valid. Otherwise, consider using
        /// validate_utf16be_with_errors.
        ///
        /// Overridden by each implementation.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param buf the UTF-16BE string to validate.
        /// @param len the length of the string in number of 2-byte code units
        /// (char16_t).
        /// @return true if and only if the string is valid UTF-16BE.
        simdutf_warn_unused virtual bool
        validate_utf16be(const char16_t* buf, size_t len) const noexcept
            = 0;

        /// Validate the UTF-16LE string and stop on error.  It might be faster than
        /// validate_utf16le when an error is expected to occur early.
        ///
        /// Overridden by each implementation.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param buf the UTF-16LE string to validate.
        /// @param len the length of the string in number of 2-byte code units
        /// (char16_t).
        /// @return a result pair struct (of type turbo::result containing the two
        /// fields error and count) with an error code and either position of the error
        /// (in the input in code units) if any, or the number of code units validated
        /// if successful.
        simdutf_warn_unused virtual result
        validate_utf16le_with_errors(const char16_t* buf,
            size_t len) const noexcept
            = 0;

        /// Validate the UTF-16BE string and stop on error. It might be faster than
        /// validate_utf16be when an error is expected to occur early.
        ///
        /// Overridden by each implementation.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param buf the UTF-16BE string to validate.
        /// @param len the length of the string in number of 2-byte code units
        /// (char16_t).
        /// @return a result pair struct (of type turbo::result containing the two
        /// fields error and count) with an error code and either position of the error
        /// (in the input in code units) if any, or the number of code units validated
        /// if successful.
        simdutf_warn_unused virtual result
        validate_utf16be_with_errors(const char16_t* buf,
            size_t len) const noexcept
            = 0;
        /// Copies the UTF-16LE string while replacing mismatched surrogates with the
        /// Unicode replacement character U+FFFD. We allow the input and output to be
        /// the same buffer so that the correction is done in-place.
        ///
        /// Overridden by each implementation.
        ///
        /// @param input the UTF-16LE string to correct.
        /// @param len the length of the string in number of 2-byte code units
        /// (char16_t).
        /// @param output the output buffer.
        virtual void to_well_formed_utf16le(const char16_t* input, size_t len,
            char16_t* output) const noexcept
            = 0;
        /// Copies the UTF-16BE string while replacing mismatched surrogates with the
        /// Unicode replacement character U+FFFD. We allow the input and output to be
        /// the same buffer so that the correction is done in-place.
        ///
        /// Overridden by each implementation.
        ///
        /// @param input the UTF-16BE string to correct.
        /// @param len the length of the string in number of 2-byte code units
        /// (char16_t).
        /// @param output the output buffer.
        virtual void to_well_formed_utf16be(const char16_t* input, size_t len,
            char16_t* output) const noexcept
            = 0;

        /// Validate the UTF-32 string.
        ///
        /// Overridden by each implementation.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param buf the UTF-32 string to validate.
        /// @param len the length of the string in number of 4-byte code units
        /// (char32_t).
        /// @return true if and only if the string is valid UTF-32.
        simdutf_warn_unused virtual bool
        validate_utf32(const char32_t* buf, size_t len) const noexcept
            = 0;

        /// Validate the UTF-32 string and stop on error.
        ///
        /// Overridden by each implementation.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param buf the UTF-32 string to validate.
        /// @param len the length of the string in number of 4-byte code units
        /// (char32_t).
        /// @return a result pair struct (of type turbo::result containing the two
        /// fields error and count) with an error code and either position of the error
        /// (in the input in code units) if any, or the number of code units validated
        /// if successful.
        simdutf_warn_unused virtual result
        validate_utf32_with_errors(const char32_t* buf,
            size_t len) const noexcept
            = 0;

        /// Convert Latin1 string into UTF-8 string.
        ///
        /// This function is suitable to work with inputs from untrusted sources.
        ///
        /// @param input         the Latin1 string to convert
        /// @param length        the length of the string in bytes
        /// @param utf8_output  the pointer to buffer that can hold conversion result
        /// @return the number of written char; 0 if conversion is not possible
        simdutf_warn_unused virtual size_t
        convert_latin1_to_utf8(const char* input, size_t length,
            char* utf8_output) const noexcept
            = 0;

        /// Convert possibly Latin1 string into UTF-16LE string.
        ///
        /// This function is suitable to work with inputs from untrusted sources.
        ///
        /// @param input         the Latin1  string to convert
        /// @param length        the length of the string in bytes
        /// @param utf16_output  the pointer to buffer that can hold conversion result
        /// @return the number of written char16_t; 0 if conversion is not possible
        simdutf_warn_unused virtual size_t
        convert_latin1_to_utf16le(const char* input, size_t length,
            char16_t* utf16_output) const noexcept
            = 0;

        /// Convert Latin1 string into UTF-16BE string.
        ///
        /// This function is suitable to work with inputs from untrusted sources.
        ///
        /// @param input         the Latin1 string to convert
        /// @param length        the length of the string in bytes
        /// @param utf16_output  the pointer to buffer that can hold conversion result
        /// @return the number of written char16_t; 0 if conversion is not possible
        simdutf_warn_unused virtual size_t
        convert_latin1_to_utf16be(const char* input, size_t length,
            char16_t* utf16_output) const noexcept
            = 0;

        /// Convert Latin1 string into UTF-32 string.
        ///
        /// This function is suitable to work with inputs from untrusted sources.
        ///
        /// @param input         the Latin1 string to convert
        /// @param length        the length of the string in bytes
        /// @param utf32_buffer  the pointer to buffer that can hold conversion result
        /// @return the number of written char32_t; 0 if conversion is not possible
        simdutf_warn_unused virtual size_t
        convert_latin1_to_utf32(const char* input, size_t length,
            char32_t* utf32_buffer) const noexcept
            = 0;

        /// Convert possibly broken UTF-8 string into latin1 string.
        ///
        /// During the conversion also validation of the input string is done.
        /// This function is suitable to work with inputs from untrusted sources.
        ///
        /// @param input         the UTF-8 string to convert
        /// @param length        the length of the string in bytes
        /// @param latin1_output  the pointer to buffer that can hold conversion result
        /// @return the number of written char; 0 if the input was not valid UTF-8
        /// string or if it cannot be represented as Latin1
        simdutf_warn_unused virtual size_t
        convert_utf8_to_latin1(const char* input, size_t length,
            char* latin1_output) const noexcept
            = 0;

        /// Convert possibly broken UTF-8 string into latin1 string with errors.
        /// If the string cannot be represented as Latin1, an error
        /// code is returned.
        ///
        /// During the conversion also validation of the input string is done.
        /// This function is suitable to work with inputs from untrusted sources.
        ///
        /// @param input         the UTF-8 string to convert
        /// @param length        the length of the string in bytes
        /// @param latin1_output  the pointer to buffer that can hold conversion result
        /// @return a result pair struct (of type turbo::result containing the two
        /// fields error and count) with an error code and either position of the error
        /// (in the input in code units) if any, or the number of code units validated
        /// if successful.
        simdutf_warn_unused virtual result
        convert_utf8_to_latin1_with_errors(const char* input, size_t length,
            char* latin1_output) const noexcept
            = 0;

        /// Convert valid UTF-8 string into latin1 string.
        ///
        /// This function assumes that the input string is valid UTF-8 and that it can
        /// be represented as Latin1. If you violate this assumption, the result is
        /// implementation defined and may include system-dependent behavior such as
        /// crashes.
        ///
        /// This function is for expert users only and not part of our public API. Use
        /// convert_utf8_to_latin1 instead.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param input         the UTF-8 string to convert
        /// @param length        the length of the string in bytes
        /// @param latin1_output  the pointer to buffer that can hold conversion result
        /// @return the number of written char; 0 if the input was not valid UTF-8
        /// string
        simdutf_warn_unused virtual size_t
        convert_valid_utf8_to_latin1(const char* input, size_t length,
            char* latin1_output) const noexcept
            = 0;

        /// Convert possibly broken UTF-8 string into UTF-16LE string.
        ///
        /// During the conversion also validation of the input string is done.
        /// This function is suitable to work with inputs from untrusted sources.
        ///
        /// @param input         the UTF-8 string to convert
        /// @param length        the length of the string in bytes
        /// @param utf16_output  the pointer to buffer that can hold conversion result
        /// @return the number of written char16_t; 0 if the input was not valid UTF-8
        /// string
        simdutf_warn_unused virtual size_t
        convert_utf8_to_utf16le(const char* input, size_t length,
            char16_t* utf16_output) const noexcept
            = 0;

        /// Convert possibly broken UTF-8 string into UTF-16BE string.
        ///
        /// During the conversion also validation of the input string is done.
        /// This function is suitable to work with inputs from untrusted sources.
        ///
        /// @param input         the UTF-8 string to convert
        /// @param length        the length of the string in bytes
        /// @param utf16_output  the pointer to buffer that can hold conversion result
        /// @return the number of written char16_t; 0 if the input was not valid UTF-8
        /// string
        simdutf_warn_unused virtual size_t
        convert_utf8_to_utf16be(const char* input, size_t length,
            char16_t* utf16_output) const noexcept
            = 0;

        /// Convert possibly broken UTF-8 string into UTF-16LE string and stop on
        /// error.
        ///
        /// During the conversion also validation of the input string is done.
        /// This function is suitable to work with inputs from untrusted sources.
        ///
        /// @param input         the UTF-8 string to convert
        /// @param length        the length of the string in bytes
        /// @param utf16_output  the pointer to buffer that can hold conversion result
        /// @return a result pair struct (of type turbo::result containing the two
        /// fields error and count) with an error code and either position of the error
        /// (in the input in code units) if any, or the number of code units validated
        /// if successful.
        simdutf_warn_unused virtual result convert_utf8_to_utf16le_with_errors(
            const char* input, size_t length,
            char16_t* utf16_output) const noexcept
            = 0;

        /// Convert possibly broken UTF-8 string into UTF-16BE string and stop on
        /// error.
        ///
        /// During the conversion also validation of the input string is done.
        /// This function is suitable to work with inputs from untrusted sources.
        ///
        /// @param input         the UTF-8 string to convert
        /// @param length        the length of the string in bytes
        /// @param utf16_output  the pointer to buffer that can hold conversion result
        /// @return a result pair struct (of type turbo::result containing the two
        /// fields error and count) with an error code and either position of the error
        /// (in the input in code units) if any, or the number of code units validated
        /// if successful.
        simdutf_warn_unused virtual result convert_utf8_to_utf16be_with_errors(
            const char* input, size_t length,
            char16_t* utf16_output) const noexcept
            = 0;
        /// Compute the number of bytes that this UTF-16LE string would require in
        /// UTF-8 format even when the UTF-16LE content contains mismatched
        /// surrogates that have to be replaced by the replacement character (0xFFFD).
        ///
        /// @param input         the UTF-16LE string to convert
        /// @param length        the length of the string in 2-byte code units
        /// (char16_t)
        /// @return a result pair struct (of type turbo::result containing the two
        /// fields error and count) where the count is the number of bytes required to
        /// encode the UTF-16LE string as UTF-8, and the error code is either SUCCESS
        /// or SURROGATE. The count is correct regardless of the error field.
        /// When SURROGATE is returned, it does not indicate an error in the case of
        /// this function: it indicates that at least one surrogate has been
        /// encountered: the surrogates may be matched or not (thus this function does
        /// not validate). If the returned error code is SUCCESS, then the input
        /// contains no surrogate, is in the Basic Multilingual Plane, and is
        /// necessarily valid.
        virtual simdutf_warn_unused result utf8_length_from_utf16le_with_replacement(
            const char16_t* input, size_t length) const noexcept
            = 0;

        /// Compute the number of bytes that this UTF-16BE string would require in
        /// UTF-8 format even when the UTF-16BE content contains mismatched
        /// surrogates that have to be replaced by the replacement character (0xFFFD).
        ///
        /// @param input         the UTF-16BE string to convert
        /// @param length        the length of the string in 2-byte code units
        /// (char16_t)
        /// @return a result pair struct (of type turbo::result containing the two
        /// fields error and count) where the count is the number of bytes required to
        /// encode the UTF-16BE string as UTF-8, and the error code is either SUCCESS
        /// or SURROGATE. The count is correct regardless of the error field.
        /// When SURROGATE is returned, it does not indicate an error in the case of
        /// this function: it indicates that at least one surrogate has been
        /// encountered: the surrogates may be matched or not (thus this function does
        /// not validate). If the returned error code is SUCCESS, then the input
        /// contains no surrogate, is in the Basic Multilingual Plane, and is
        /// necessarily valid.
        virtual simdutf_warn_unused result utf8_length_from_utf16be_with_replacement(
            const char16_t* input, size_t length) const noexcept
            = 0;


        /// Convert possibly broken UTF-8 string into UTF-32 string.
        ///
        /// During the conversion also validation of the input string is done.
        /// This function is suitable to work with inputs from untrusted sources.
        ///
        /// @param input         the UTF-8 string to convert
        /// @param length        the length of the string in bytes
        /// @param utf32_output  the pointer to buffer that can hold conversion result
        /// @return the number of written char16_t; 0 if the input was not valid UTF-8
        /// string
        simdutf_warn_unused virtual size_t
        convert_utf8_to_utf32(const char* input, size_t length,
            char32_t* utf32_output) const noexcept
            = 0;

        /// Convert possibly broken UTF-8 string into UTF-32 string and stop on error.
        ///
        /// During the conversion also validation of the input string is done.
        /// This function is suitable to work with inputs from untrusted sources.
        ///
        /// @param input         the UTF-8 string to convert
        /// @param length        the length of the string in bytes
        /// @param utf32_output  the pointer to buffer that can hold conversion result
        /// @return a result pair struct (of type turbo::result containing the two
        /// fields error and count) with an error code and either position of the error
        /// (in the input in code units) if any, or the number of char32_t written if
        /// successful.
        simdutf_warn_unused virtual result
        convert_utf8_to_utf32_with_errors(const char* input, size_t length,
            char32_t* utf32_output) const noexcept
            = 0;


        /// Convert valid UTF-8 string into UTF-16LE string.
        ///
        /// This function assumes that the input string is valid UTF-8.
        ///
        /// @param input         the UTF-8 string to convert
        /// @param length        the length of the string in bytes
        /// @param utf16_buffer  the pointer to buffer that can hold conversion result
        /// @return the number of written char16_t
        simdutf_warn_unused virtual size_t
        convert_valid_utf8_to_utf16le(const char* input, size_t length,
            char16_t* utf16_buffer) const noexcept
            = 0;

        /// Convert valid UTF-8 string into UTF-16BE string.
        ///
        /// This function assumes that the input string is valid UTF-8.
        ///
        /// @param input         the UTF-8 string to convert
        /// @param length        the length of the string in bytes
        /// @param utf16_buffer  the pointer to buffer that can hold conversion result
        /// @return the number of written char16_t
        simdutf_warn_unused virtual size_t
        convert_valid_utf8_to_utf16be(const char* input, size_t length,
            char16_t* utf16_buffer) const noexcept
            = 0;

        /// Convert valid UTF-8 string into UTF-32 string.
        ///
        /// This function assumes that the input string is valid UTF-8.
        ///
        /// @param input         the UTF-8 string to convert
        /// @param length        the length of the string in bytes
        /// @param utf32_buffer  the pointer to buffer that can hold conversion result
        /// @return the number of written char32_t
        simdutf_warn_unused virtual size_t
        convert_valid_utf8_to_utf32(const char* input, size_t length,
            char32_t* utf32_buffer) const noexcept
            = 0;

        /// Compute the number of 2-byte code units that this UTF-8 string would
        /// require in UTF-16LE format.
        ///
        /// This function does not validate the input. It is acceptable to pass invalid
        /// UTF-8 strings but in such cases the result is implementation defined.
        ///
        /// @param input         the UTF-8 string to process
        /// @param length        the length of the string in bytes
        /// @return the number of char16_t code units required to encode the UTF-8
        /// string as UTF-16LE
        simdutf_warn_unused virtual size_t
        utf16_length_from_utf8(const char* input, size_t length) const noexcept
            = 0;

        /// Compute the number of 4-byte code units that this UTF-8 string would
        /// require in UTF-32 format.
        ///
        /// This function is equivalent to count_utf8. It is acceptable to pass invalid
        /// UTF-8 strings but in such cases the result is implementation defined.
        ///
        /// This function does not validate the input.
        ///
        /// @param input         the UTF-8 string to process
        /// @param length        the length of the string in bytes
        /// @return the number of char32_t code units required to encode the UTF-8
        /// string as UTF-32
        simdutf_warn_unused virtual size_t
        utf32_length_from_utf8(const char* input, size_t length) const noexcept
            = 0;

        /// Convert possibly broken UTF-16LE string into Latin1 string.
        ///
        /// During the conversion also validation of the input string is done.
        /// This function is suitable to work with inputs from untrusted sources.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param input         the UTF-16LE string to convert
        /// @param length        the length of the string in 2-byte code units
        /// (char16_t)
        /// @param latin1_buffer   the pointer to buffer that can hold conversion
        /// result
        /// @return number of written code units; 0 if input is not a valid UTF-16LE
        /// string or if it cannot be represented as Latin1
        simdutf_warn_unused virtual size_t
        convert_utf16le_to_latin1(const char16_t* input, size_t length,
            char* latin1_buffer) const noexcept
            = 0;

        /// Convert possibly broken UTF-16BE string into Latin1 string.
        ///
        /// During the conversion also validation of the input string is done.
        /// This function is suitable to work with inputs from untrusted sources.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param input         the UTF-16BE string to convert
        /// @param length        the length of the string in 2-byte code units
        /// (char16_t)
        /// @param latin1_buffer   the pointer to buffer that can hold conversion
        /// result
        /// @return number of written code units; 0 if input is not a valid UTF-16BE
        /// string or if it cannot be represented as Latin1
        simdutf_warn_unused virtual size_t
        convert_utf16be_to_latin1(const char16_t* input, size_t length,
            char* latin1_buffer) const noexcept
            = 0;

        /// Convert possibly broken UTF-16LE string into Latin1 string.
        /// If the string cannot be represented as Latin1, an error
        /// is returned.
        ///
        /// During the conversion also validation of the input string is done.
        /// This function is suitable to work with inputs from untrusted sources.
        /// This function is not BOM-aware.
        ///
        /// @param input         the UTF-16LE string to convert
        /// @param length        the length of the string in 2-byte code units
        /// (char16_t)
        /// @param latin1_buffer   the pointer to buffer that can hold conversion
        /// result
        /// @return a result pair struct (of type turbo::result containing the two
        /// fields error and count) with an error code and either position of the error
        /// (in the input in code units) if any, or the number of char written if
        /// successful.
        simdutf_warn_unused virtual result
        convert_utf16le_to_latin1_with_errors(const char16_t* input, size_t length,
            char* latin1_buffer) const noexcept
            = 0;

        /// Convert possibly broken UTF-16BE string into Latin1 string.
        /// If the string cannot be represented as Latin1, an error
        /// is returned.
        ///
        /// During the conversion also validation of the input string is done.
        /// This function is suitable to work with inputs from untrusted sources.
        /// This function is not BOM-aware.
        ///
        /// @param input         the UTF-16BE string to convert
        /// @param length        the length of the string in 2-byte code units
        /// (char16_t)
        /// @param latin1_buffer   the pointer to buffer that can hold conversion
        /// result
        /// @return a result pair struct (of type turbo::result containing the two
        /// fields error and count) with an error code and either position of the error
        /// (in the input in code units) if any, or the number of char written if
        /// successful.
        simdutf_warn_unused virtual result
        convert_utf16be_to_latin1_with_errors(const char16_t* input, size_t length,
            char* latin1_buffer) const noexcept
            = 0;

        /// Convert valid UTF-16LE string into Latin1 string.
        ///
        /// This function assumes that the input string is valid UTF-L16LE and that it
        /// can be represented as Latin1. If you violate this assumption, the result is
        /// implementation defined and may include system-dependent behavior such as
        /// crashes.
        ///
        /// This function is for expert users only and not part of our public API. Use
        /// convert_utf16le_to_latin1 instead.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param input         the UTF-16LE string to convert
        /// @param length        the length of the string in 2-byte code units
        /// (char16_t)
        /// @param latin1_buffer   the pointer to buffer that can hold conversion
        /// result
        /// @return number of written code units; 0 if conversion is not possible
        simdutf_warn_unused virtual size_t
        convert_valid_utf16le_to_latin1(const char16_t* input, size_t length,
            char* latin1_buffer) const noexcept
            = 0;

        /// Convert valid UTF-16BE string into Latin1 string.
        ///
        /// This function assumes that the input string is valid UTF16-BE and that it
        /// can be represented as Latin1. If you violate this assumption, the result is
        /// implementation defined and may include system-dependent behavior such as
        /// crashes.
        ///
        /// This function is for expert users only and not part of our public API. Use
        /// convert_utf16be_to_latin1 instead.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param input         the UTF-16BE string to convert
        /// @param length        the length of the string in 2-byte code units
        /// (char16_t)
        /// @param latin1_buffer   the pointer to buffer that can hold conversion
        /// result
        /// @return number of written code units; 0 if conversion is not possible
        simdutf_warn_unused virtual size_t
        convert_valid_utf16be_to_latin1(const char16_t* input, size_t length,
            char* latin1_buffer) const noexcept
            = 0;

        /// Convert possibly broken UTF-16LE string into UTF-8 string.
        ///
        /// During the conversion also validation of the input string is done.
        /// This function is suitable to work with inputs from untrusted sources.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param input         the UTF-16LE string to convert
        /// @param length        the length of the string in 2-byte code units
        /// (char16_t)
        /// @param utf8_buffer   the pointer to buffer that can hold conversion result
        /// @return number of written code units; 0 if input is not a valid UTF-16LE
        /// string
        simdutf_warn_unused virtual size_t
        convert_utf16le_to_utf8(const char16_t* input, size_t length,
            char* utf8_buffer) const noexcept
            = 0;

        /// Convert possibly broken UTF-16BE string into UTF-8 string.
        ///
        /// During the conversion also validation of the input string is done.
        /// This function is suitable to work with inputs from untrusted sources.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param input         the UTF-16BE string to convert
        /// @param length        the length of the string in 2-byte code units
        /// (char16_t)
        /// @param utf8_buffer   the pointer to buffer that can hold conversion result
        /// @return number of written code units; 0 if input is not a valid UTF-16BE
        /// string
        simdutf_warn_unused virtual size_t
        convert_utf16be_to_utf8(const char16_t* input, size_t length,
            char* utf8_buffer) const noexcept
            = 0;

        /// Convert possibly broken UTF-16LE string into UTF-8 string and stop on
        /// error.
        ///
        /// During the conversion also validation of the input string is done.
        /// This function is suitable to work with inputs from untrusted sources.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param input         the UTF-16LE string to convert
        /// @param length        the length of the string in 2-byte code units
        /// (char16_t)
        /// @param utf8_buffer   the pointer to buffer that can hold conversion result
        /// @return a result pair struct (of type turbo::result containing the two
        /// fields error and count) with an error code and either position of the error
        /// (in the input in code units) if any, or the number of char written if
        /// successful.
        simdutf_warn_unused virtual result
        convert_utf16le_to_utf8_with_errors(const char16_t* input, size_t length,
            char* utf8_buffer) const noexcept
            = 0;

        /// Convert possibly broken UTF-16BE string into UTF-8 string and stop on
        /// error.
        ///
        /// During the conversion also validation of the input string is done.
        /// This function is suitable to work with inputs from untrusted sources.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param input         the UTF-16BE string to convert
        /// @param length        the length of the string in 2-byte code units
        /// (char16_t)
        /// @param utf8_buffer   the pointer to buffer that can hold conversion result
        /// @return a result pair struct (of type turbo::result containing the two
        /// fields error and count) with an error code and either position of the error
        /// (in the input in code units) if any, or the number of char written if
        /// successful.
        simdutf_warn_unused virtual result
        convert_utf16be_to_utf8_with_errors(const char16_t* input, size_t length,
            char* utf8_buffer) const noexcept
            = 0;

        /// Convert possibly broken UTF-16LE string into UTF-8 string, replacing
        /// unpaired surrogates with the Unicode replacement character U+FFFD.
        ///
        /// This function always succeeds: unpaired surrogates are replaced with
        /// U+FFFD (3 bytes in UTF-8: 0xEF 0xBF 0xBD).
        ///
        /// This function is not BOM-aware.
        ///
        /// @param input         the UTF-16LE string to convert
        /// @param length        the length of the string in 2-byte code units
        /// (char16_t)
        /// @param utf8_buffer   the pointer to buffer that can hold conversion result
        /// @return number of written code units
        simdutf_warn_unused virtual size_t convert_utf16le_to_utf8_with_replacement(
            const char16_t* input, size_t length,
            char* utf8_buffer) const noexcept
            = 0;

        /// Convert possibly broken UTF-16BE string into UTF-8 string, replacing
        /// unpaired surrogates with the Unicode replacement character U+FFFD.
        ///
        /// This function always succeeds: unpaired surrogates are replaced with
        /// U+FFFD (3 bytes in UTF-8: 0xEF 0xBF 0xBD).
        ///
        /// This function is not BOM-aware.
        ///
        /// @param input         the UTF-16BE string to convert
        /// @param length        the length of the string in 2-byte code units
        /// (char16_t)
        /// @param utf8_buffer   the pointer to buffer that can hold conversion result
        /// @return number of written code units
        simdutf_warn_unused virtual size_t convert_utf16be_to_utf8_with_replacement(
            const char16_t* input, size_t length,
            char* utf8_buffer) const noexcept
            = 0;

        /// Convert valid UTF-16LE string into UTF-8 string.
        ///
        /// This function assumes that the input string is valid UTF-16LE.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param input         the UTF-16LE string to convert
        /// @param length        the length of the string in 2-byte code units
        /// (char16_t)
        /// @param utf8_buffer   the pointer to a buffer that can hold the conversion
        /// result
        /// @return number of written code units; 0 if conversion is not possible
        simdutf_warn_unused virtual size_t
        convert_valid_utf16le_to_utf8(const char16_t* input, size_t length,
            char* utf8_buffer) const noexcept
            = 0;

        /// Convert valid UTF-16BE string into UTF-8 string.
        ///
        /// This function assumes that the input string is valid UTF-16BE.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param input         the UTF-16BE string to convert
        /// @param length        the length of the string in 2-byte code units
        /// (char16_t)
        /// @param utf8_buffer   the pointer to a buffer that can hold the conversion
        /// result
        /// @return number of written code units; 0 if conversion is not possible
        simdutf_warn_unused virtual size_t
        convert_valid_utf16be_to_utf8(const char16_t* input, size_t length,
            char* utf8_buffer) const noexcept
            = 0;

        /// Convert possibly broken UTF-16LE string into UTF-32 string.
        ///
        /// During the conversion also validation of the input string is done.
        /// This function is suitable to work with inputs from untrusted sources.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param input         the UTF-16LE string to convert
        /// @param length        the length of the string in 2-byte code units
        /// (char16_t)
        /// @param utf32_buffer   the pointer to buffer that can hold conversion result
        /// @return number of written code units; 0 if input is not a valid UTF-16LE
        /// string
        simdutf_warn_unused virtual size_t
        convert_utf16le_to_utf32(const char16_t* input, size_t length,
            char32_t* utf32_buffer) const noexcept
            = 0;

        /// Convert possibly broken UTF-16BE string into UTF-32 string.
        ///
        /// During the conversion also validation of the input string is done.
        /// This function is suitable to work with inputs from untrusted sources.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param input         the UTF-16BE string to convert
        /// @param length        the length of the string in 2-byte code units
        /// (char16_t)
        /// @param utf32_buffer   the pointer to buffer that can hold conversion result
        /// @return number of written code units; 0 if input is not a valid UTF-16BE
        /// string
        simdutf_warn_unused virtual size_t
        convert_utf16be_to_utf32(const char16_t* input, size_t length,
            char32_t* utf32_buffer) const noexcept
            = 0;

        /// Convert possibly broken UTF-16LE string into UTF-32 string and stop on
        /// error.
        ///
        /// During the conversion also validation of the input string is done.
        /// This function is suitable to work with inputs from untrusted sources.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param input         the UTF-16LE string to convert
        /// @param length        the length of the string in 2-byte code units
        /// (char16_t)
        /// @param utf32_buffer   the pointer to buffer that can hold conversion result
        /// @return a result pair struct (of type turbo::result containing the two
        /// fields error and count) with an error code and either position of the error
        /// (in the input in code units) if any, or the number of char32_t written if
        /// successful.
        simdutf_warn_unused virtual result convert_utf16le_to_utf32_with_errors(
            const char16_t* input, size_t length,
            char32_t* utf32_buffer) const noexcept
            = 0;

        /// Convert possibly broken UTF-16BE string into UTF-32 string and stop on
        /// error.
        ///
        /// During the conversion also validation of the input string is done.
        /// This function is suitable to work with inputs from untrusted sources.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param input         the UTF-16BE string to convert
        /// @param length        the length of the string in 2-byte code units
        /// (char16_t)
        /// @param utf32_buffer   the pointer to buffer that can hold conversion result
        /// @return a result pair struct (of type turbo::result containing the two
        /// fields error and count) with an error code and either position of the error
        /// (in the input in code units) if any, or the number of char32_t written if
        /// successful.
        simdutf_warn_unused virtual result convert_utf16be_to_utf32_with_errors(
            const char16_t* input, size_t length,
            char32_t* utf32_buffer) const noexcept
            = 0;

        /// Convert valid UTF-16LE string into UTF-32 string.
        ///
        /// This function assumes that the input string is valid UTF-16LE.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param input         the UTF-16LE string to convert
        /// @param length        the length of the string in 2-byte code units
        /// (char16_t)
        /// @param utf32_buffer   the pointer to a buffer that can hold the conversion
        /// result
        /// @return number of written code units; 0 if conversion is not possible
        simdutf_warn_unused virtual size_t
        convert_valid_utf16le_to_utf32(const char16_t* input, size_t length,
            char32_t* utf32_buffer) const noexcept
            = 0;

        /// Convert valid UTF-16LE string into UTF-32BE string.
        ///
        /// This function assumes that the input string is valid UTF-16BE.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param input         the UTF-16BE string to convert
        /// @param length        the length of the string in 2-byte code units
        /// (char16_t)
        /// @param utf32_buffer   the pointer to a buffer that can hold the conversion
        /// result
        /// @return number of written code units; 0 if conversion is not possible
        simdutf_warn_unused virtual size_t
        convert_valid_utf16be_to_utf32(const char16_t* input, size_t length,
            char32_t* utf32_buffer) const noexcept
            = 0;

        /// Compute the number of bytes that this UTF-16LE string would require in
        /// UTF-8 format.
        ///
        /// This function does not validate the input. It is acceptable to pass invalid
        /// UTF-16 strings but in such cases the result is implementation defined.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param input         the UTF-16LE string to convert
        /// @param length        the length of the string in 2-byte code units
        /// (char16_t)
        /// @return the number of bytes required to encode the UTF-16LE string as UTF-8
        simdutf_warn_unused virtual size_t
        utf8_length_from_utf16le(const char16_t* input,
            size_t length) const noexcept
            = 0;

        /// Compute the number of bytes that this UTF-16BE string would require in
        /// UTF-8 format.
        ///
        /// This function does not validate the input. It is acceptable to pass invalid
        /// UTF-16 strings but in such cases the result is implementation defined.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param input         the UTF-16BE string to convert
        /// @param length        the length of the string in 2-byte code units
        /// (char16_t)
        /// @return the number of bytes required to encode the UTF-16BE string as UTF-8
        simdutf_warn_unused virtual size_t
        utf8_length_from_utf16be(const char16_t* input,
            size_t length) const noexcept
            = 0;

        /// Convert possibly broken UTF-32 string into Latin1 string.
        ///
        /// During the conversion also validation of the input string is done.
        /// This function is suitable to work with inputs from untrusted sources.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param input         the UTF-32 string to convert
        /// @param length        the length of the string in 4-byte code units
        /// (char32_t)
        /// @param latin1_buffer   the pointer to buffer that can hold conversion
        /// result
        /// @return number of written code units; 0 if input is not a valid UTF-32
        /// string
        simdutf_warn_unused virtual size_t
        convert_utf32_to_latin1(const char32_t* input, size_t length,
            char* latin1_buffer) const noexcept
            = 0;

        /// Convert possibly broken UTF-32 string into Latin1 string and stop on error.
        /// If the string cannot be represented as Latin1, an error is returned.
        ///
        /// During the conversion also validation of the input string is done.
        /// This function is suitable to work with inputs from untrusted sources.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param input         the UTF-32 string to convert
        /// @param length        the length of the string in 4-byte code units
        /// (char32_t)
        /// @param latin1_buffer   the pointer to buffer that can hold conversion
        /// result
        /// @return a result pair struct (of type turbo::result containing the two
        /// fields error and count) with an error code and either position of the error
        /// (in the input in code units) if any, or the number of char written if
        /// successful.
        simdutf_warn_unused virtual result
        convert_utf32_to_latin1_with_errors(const char32_t* input, size_t length,
            char* latin1_buffer) const noexcept
            = 0;

        /// Convert valid UTF-32 string into Latin1 string.
        ///
        /// This function assumes that the input string is valid UTF-32 and can be
        /// represented as Latin1. If you violate this assumption, the result is
        /// implementation defined and may include system-dependent behavior such as
        /// crashes.
        ///
        /// This function is for expert users only and not part of our public API. Use
        /// convert_utf32_to_latin1 instead.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param input         the UTF-32 string to convert
        /// @param length        the length of the string in 4-byte code units
        /// (char32_t)
        /// @param latin1_buffer   the pointer to a buffer that can hold the conversion
        /// result
        /// @return number of written code units; 0 if conversion is not possible
        simdutf_warn_unused virtual size_t
        convert_valid_utf32_to_latin1(const char32_t* input, size_t length,
            char* latin1_buffer) const noexcept
            = 0;

        /// Convert possibly broken UTF-32 string into UTF-8 string.
        ///
        /// During the conversion also validation of the input string is done.
        /// This function is suitable to work with inputs from untrusted sources.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param input         the UTF-32 string to convert
        /// @param length        the length of the string in 4-byte code units
        /// (char32_t)
        /// @param utf8_buffer   the pointer to buffer that can hold conversion result
        /// @return number of written code units; 0 if input is not a valid UTF-32
        /// string
        simdutf_warn_unused virtual size_t
        convert_utf32_to_utf8(const char32_t* input, size_t length,
            char* utf8_buffer) const noexcept
            = 0;

        /// Convert possibly broken UTF-32 string into UTF-8 string and stop on error.
        ///
        /// During the conversion also validation of the input string is done.
        /// This function is suitable to work with inputs from untrusted sources.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param input         the UTF-32 string to convert
        /// @param length        the length of the string in 4-byte code units
        /// (char32_t)
        /// @param utf8_buffer   the pointer to buffer that can hold conversion result
        /// @return a result pair struct (of type turbo::result containing the two
        /// fields error and count) with an error code and either position of the error
        /// (in the input in code units) if any, or the number of char written if
        /// successful.
        simdutf_warn_unused virtual result
        convert_utf32_to_utf8_with_errors(const char32_t* input, size_t length,
            char* utf8_buffer) const noexcept
            = 0;

        /// Convert valid UTF-32 string into UTF-8 string.
        ///
        /// This function assumes that the input string is valid UTF-32.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param input         the UTF-32 string to convert
        /// @param length        the length of the string in 4-byte code units
        /// (char32_t)
        /// @param utf8_buffer   the pointer to a buffer that can hold the conversion
        /// result
        /// @return number of written code units; 0 if conversion is not possible
        simdutf_warn_unused virtual size_t
        convert_valid_utf32_to_utf8(const char32_t* input, size_t length,
            char* utf8_buffer) const noexcept
            = 0;

        /// Return the number of bytes that this UTF-16 string would require in Latin1
        /// format.
        ///
        ///
        /// @param length        the length of the string in 2-byte code units
        /// (char16_t)
        /// @return the number of bytes required to encode the UTF-16 string as Latin1
        simdutf_warn_unused virtual size_t
        utf16_length_from_latin1(size_t length) const noexcept {
            return length;
        }

        /// Convert possibly broken UTF-32 string into UTF-16LE string.
        ///
        /// During the conversion also validation of the input string is done.
        /// This function is suitable to work with inputs from untrusted sources.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param input         the UTF-32 string to convert
        /// @param length        the length of the string in 4-byte code units
        /// (char32_t)
        /// @param utf16_buffer   the pointer to buffer that can hold conversion result
        /// @return number of written code units; 0 if input is not a valid UTF-32
        /// string
        simdutf_warn_unused virtual size_t
        convert_utf32_to_utf16le(const char32_t* input, size_t length,
            char16_t* utf16_buffer) const noexcept
            = 0;

        /// Convert possibly broken UTF-32 string into UTF-16BE string.
        ///
        /// During the conversion also validation of the input string is done.
        /// This function is suitable to work with inputs from untrusted sources.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param input         the UTF-32 string to convert
        /// @param length        the length of the string in 4-byte code units
        /// (char32_t)
        /// @param utf16_buffer   the pointer to buffer that can hold conversion result
        /// @return number of written code units; 0 if input is not a valid UTF-32
        /// string
        simdutf_warn_unused virtual size_t
        convert_utf32_to_utf16be(const char32_t* input, size_t length,
            char16_t* utf16_buffer) const noexcept
            = 0;

        /// Convert possibly broken UTF-32 string into UTF-16LE string and stop on
        /// error.
        ///
        /// During the conversion also validation of the input string is done.
        /// This function is suitable to work with inputs from untrusted sources.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param input         the UTF-32 string to convert
        /// @param length        the length of the string in 4-byte code units
        /// (char32_t)
        /// @param utf16_buffer   the pointer to buffer that can hold conversion result
        /// @return a result pair struct (of type turbo::result containing the two
        /// fields error and count) with an error code and either position of the error
        /// (in the input in code units) if any, or the number of char16_t written if
        /// successful.
        simdutf_warn_unused virtual result convert_utf32_to_utf16le_with_errors(
            const char32_t* input, size_t length,
            char16_t* utf16_buffer) const noexcept
            = 0;

        /// Convert possibly broken UTF-32 string into UTF-16BE string and stop on
        /// error.
        ///
        /// During the conversion also validation of the input string is done.
        /// This function is suitable to work with inputs from untrusted sources.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param input         the UTF-32 string to convert
        /// @param length        the length of the string in 4-byte code units
        /// (char32_t)
        /// @param utf16_buffer   the pointer to buffer that can hold conversion result
        /// @return a result pair struct (of type turbo::result containing the two
        /// fields error and count) with an error code and either position of the error
        /// (in the input in code units) if any, or the number of char16_t written if
        /// successful.
        simdutf_warn_unused virtual result convert_utf32_to_utf16be_with_errors(
            const char32_t* input, size_t length,
            char16_t* utf16_buffer) const noexcept
            = 0;

        /// Convert valid UTF-32 string into UTF-16LE string.
        ///
        /// This function assumes that the input string is valid UTF-32.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param input         the UTF-32 string to convert
        /// @param length        the length of the string in 4-byte code units
        /// (char32_t)
        /// @param utf16_buffer   the pointer to a buffer that can hold the conversion
        /// result
        /// @return number of written code units; 0 if conversion is not possible
        simdutf_warn_unused virtual size_t
        convert_valid_utf32_to_utf16le(const char32_t* input, size_t length,
            char16_t* utf16_buffer) const noexcept
            = 0;

        /// Convert valid UTF-32 string into UTF-16BE string.
        ///
        /// This function assumes that the input string is valid UTF-32.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param input         the UTF-32 string to convert
        /// @param length        the length of the string in 4-byte code units
        /// (char32_t)
        /// @param utf16_buffer   the pointer to a buffer that can hold the conversion
        /// result
        /// @return number of written code units; 0 if conversion is not possible
        simdutf_warn_unused virtual size_t
        convert_valid_utf32_to_utf16be(const char32_t* input, size_t length,
            char16_t* utf16_buffer) const noexcept
            = 0;

        /// Change the endianness of the input. Can be used to go from UTF-16LE to
        /// UTF-16BE or from UTF-16BE to UTF-16LE.
        ///
        /// This function does not validate the input.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param input         the UTF-16 string to process
        /// @param length        the length of the string in 2-byte code units
        /// (char16_t)
        /// @param output        the pointer to a buffer that can hold the conversion
        /// result
        virtual void change_endianness_utf16(const char16_t* input, size_t length,
            char16_t* output) const noexcept
            = 0;

        /// Return the number of bytes that this Latin1 string would require in UTF-8
        /// format.
        ///
        /// @param input         the Latin1 string to convert
        /// @param length        the length of the string bytes
        /// @return the number of bytes required to encode the Latin1 string as UTF-8
        simdutf_warn_unused virtual size_t
        utf8_length_from_latin1(const char* input, size_t length) const noexcept
            = 0;

        /// Compute the number of bytes that this UTF-32 string would require in UTF-8
        /// format.
        ///
        /// This function does not validate the input. It is acceptable to pass invalid
        /// UTF-32 strings but in such cases the result is implementation defined.
        ///
        /// @param input         the UTF-32 string to convert
        /// @param length        the length of the string in 4-byte code units
        /// (char32_t)
        /// @return the number of bytes required to encode the UTF-32 string as UTF-8
        simdutf_warn_unused virtual size_t
        utf8_length_from_utf32(const char32_t* input,
            size_t length) const noexcept
            = 0;

        /// Compute the number of bytes that this UTF-32 string would require in Latin1
        /// format.
        ///
        /// This function does not validate the input. It is acceptable to pass invalid
        /// UTF-32 strings but in such cases the result is implementation defined.
        ///
        /// @param length        the length of the string in 4-byte code units
        /// (char32_t)
        /// @return the number of bytes required to encode the UTF-32 string as Latin1
        simdutf_warn_unused virtual size_t
        latin1_length_from_utf32(size_t length) const noexcept {
            return length;
        }

        /// Compute the number of bytes that this UTF-8 string would require in Latin1
        /// format.
        ///
        /// This function does not validate the input. It is acceptable to pass invalid
        /// UTF-8 strings but in such cases the result is implementation defined.
        ///
        /// @param input         the UTF-8 string to convert
        /// @param length        the length of the string in byte
        /// @return the number of bytes required to encode the UTF-8 string as Latin1
        simdutf_warn_unused virtual size_t
        latin1_length_from_utf8(const char* input, size_t length) const noexcept
            = 0;

        /// Compute the number of bytes that this UTF-16LE/BE string would require in
        /// Latin1 format.
        ///
        /// This function does not validate the input. It is acceptable to pass invalid
        /// UTF-16 strings but in such cases the result is implementation defined.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param length        the length of the string in 2-byte code units
        /// (char16_t)
        /// @return the number of bytes required to encode the UTF-16LE string as
        /// Latin1
        simdutf_warn_unused virtual size_t
        latin1_length_from_utf16(size_t length) const noexcept {
            return length;
        }

        /// Compute the number of two-byte code units that this UTF-32 string would
        /// require in UTF-16 format.
        ///
        /// This function does not validate the input. It is acceptable to pass invalid
        /// UTF-32 strings but in such cases the result is implementation defined.
        ///
        /// @param input         the UTF-32 string to convert
        /// @param length        the length of the string in 4-byte code units
        /// (char32_t)
        /// @return the number of bytes required to encode the UTF-32 string as UTF-16
        simdutf_warn_unused virtual size_t
        utf16_length_from_utf32(const char32_t* input,
            size_t length) const noexcept
            = 0;

        /// Return the number of bytes that this UTF-32 string would require in Latin1
        /// format.
        ///
        /// @param length        the length of the string in 4-byte code units
        /// (char32_t)
        /// @return the number of bytes required to encode the UTF-32 string as Latin1
        simdutf_warn_unused virtual size_t
        utf32_length_from_latin1(size_t length) const noexcept {
            return length;
        }

        /// Compute the number of bytes that this UTF-16LE string would require in
        /// UTF-32 format.
        ///
        /// This function is equivalent to count_utf16le.
        ///
        /// This function does not validate the input. It is acceptable to pass invalid
        /// UTF-16 strings but in such cases the result is implementation defined.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param input         the UTF-16LE string to convert
        /// @param length        the length of the string in 2-byte code units
        /// (char16_t)
        /// @return the number of bytes required to encode the UTF-16LE string as
        /// UTF-32
        simdutf_warn_unused virtual size_t
        utf32_length_from_utf16le(const char16_t* input,
            size_t length) const noexcept
            = 0;

        /// Compute the number of bytes that this UTF-16BE string would require in
        /// UTF-32 format.
        ///
        /// This function is equivalent to count_utf16be.
        ///
        /// This function does not validate the input. It is acceptable to pass invalid
        /// UTF-16 strings but in such cases the result is implementation defined.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param input         the UTF-16BE string to convert
        /// @param length        the length of the string in 2-byte code units
        /// (char16_t)
        /// @return the number of bytes required to encode the UTF-16BE string as
        /// UTF-32
        simdutf_warn_unused virtual size_t
        utf32_length_from_utf16be(const char16_t* input,
            size_t length) const noexcept
            = 0;

        /// Count the number of code points (characters) in the string assuming that
        /// it is valid.
        ///
        /// This function assumes that the input string is valid UTF-16LE.
        /// It is acceptable to pass invalid UTF-16 strings but in such cases
        /// the result is implementation defined.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param input         the UTF-16LE string to process
        /// @param length        the length of the string in 2-byte code units
        /// (char16_t)
        /// @return number of code points
        simdutf_warn_unused virtual size_t
        count_utf16le(const char16_t* input, size_t length) const noexcept
            = 0;

        /// Count the number of code points (characters) in the string assuming that
        /// it is valid.
        ///
        /// This function assumes that the input string is valid UTF-16BE.
        /// It is acceptable to pass invalid UTF-16 strings but in such cases
        /// the result is implementation defined.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param input         the UTF-16BE string to process
        /// @param length        the length of the string in 2-byte code units
        /// (char16_t)
        /// @return number of code points
        simdutf_warn_unused virtual size_t
        count_utf16be(const char16_t* input, size_t length) const noexcept
            = 0;

        /// Count the number of code points (characters) in the string assuming that
        /// it is valid.
        ///
        /// This function assumes that the input string is valid UTF-8.
        /// It is acceptable to pass invalid UTF-8 strings but in such cases
        /// the result is implementation defined.
        ///
        /// @param input         the UTF-8 string to process
        /// @param length        the length of the string in bytes
        /// @return number of code points
        simdutf_warn_unused virtual size_t
        count_utf8(const char* input, size_t length) const noexcept
            = 0;

        /// Provide the maximal binary length in bytes given the base64 input.
        /// As long as the input does not contain ignorable characters (e.g., ASCII
        /// spaces or linefeed characters), the result is exact. In particular, the
        /// function checks for padding characters.
        ///
        /// The function is fast (constant time). It checks up to two characters at
        /// the end of the string. The input is not otherwise validated or read..
        ///
        /// @param input         the base64 input to process
        /// @param length        the length of the base64 input in bytes
        /// @return maximal number of binary bytes
        simdutf_warn_unused size_t maximal_binary_length_from_base64(
            const char* input, size_t length) const noexcept;

        /// Provide the maximal binary length in bytes given the base64 input.
        /// As long as the input does not contain ignorable characters (e.g., ASCII
        /// spaces or linefeed characters), the result is exact. In particular, the
        /// function checks for padding characters.
        ///
        /// The function is fast (constant time). It checks up to two characters at
        /// the end of the string. The input is not otherwise validated or read.
        ///
        /// @param input         the base64 input to process, in ASCII stored as 16-bit
        /// units
        /// @param length        the length of the base64 input in 16-bit units
        /// @return maximal number of binary bytes
        simdutf_warn_unused size_t maximal_binary_length_from_base64(
            const char16_t* input, size_t length) const noexcept;

        /// Compute the binary length from a base64 input with ASCII spaces.
        /// This function is useful for well-formed base64 inputs that may contain
        /// ASCII spaces (such as line breaks). For such inputs, the result is exact.
        ///
        /// The function counts non-whitespace characters (ASCII value > 0x20) and
        /// subtracts padding characters ('=') found at the end.
        ///
        /// @param input         the base64 input to process
        /// @param length        the length of the base64 input in bytes
        /// @return number of binary bytes
        simdutf_warn_unused virtual size_t
        binary_length_from_base64(const char* input, size_t length) const noexcept;

        /// Compute the binary length from a base64 input with ASCII spaces.
        /// This function is useful for well-formed base64 inputs that may contain
        /// ASCII spaces (such as line breaks). For such inputs, the result is exact.
        ///
        /// The function counts non-whitespace characters (ASCII value > 0x20) and
        /// subtracts padding characters ('=') found at the end.
        ///
        /// @param input         the base64 input to process, in ASCII stored as 16-bit
        /// units
        /// @param length        the length of the base64 input in 16-bit units
        /// @return number of binary bytes
        simdutf_warn_unused virtual size_t
        binary_length_from_base64(const char16_t* input,
            size_t length) const noexcept;

        /// Convert a base64 input to a binary output.
        ///
        /// This function follows the WHATWG forgiving-base64 format, which means that
        /// it will ignore any ASCII spaces in the input. You may provide a padded
        /// input (with one or two equal signs at the end) or an unpadded input
        /// (without any equal signs at the end).
        ///
        /// See https://infra.spec.whatwg.org/#forgiving-base64-decode
        ///
        /// This function will fail in case of invalid input. When last_chunk_options =
        /// loose, there are two possible reasons for failure: the input contains a
        /// number of base64 characters that when divided by 4, leaves a single
        /// remainder character (BASE64_INPUT_REMAINDER), or the input contains a
        /// character that is not a valid base64 character (INVALID_BASE64_CHARACTER).
        ///
        /// You should call this function with a buffer that is at least
        /// maximal_binary_length_from_base64(input, length) bytes long. If you fail to
        /// provide that much space, the function may cause a buffer overflow.
        ///
        /// @param input         the base64 string to process
        /// @param length        the length of the string in bytes
        /// @param output        the pointer to a buffer that can hold the conversion
        /// result (should be at least maximal_binary_length_from_base64(input, length)
        /// bytes long).
        /// @param options       the base64 options to use, can be base64_default or
        /// base64_url, is base64_default by default.
        /// @param last_chunk_options the handling of the last chunk (default: loose)
        /// @return a result pair struct (of type turbo::result containing the two
        /// fields error and count) with an error code and either position of the error
        /// (in the input in bytes) if any, or the number of bytes written if
        /// successful.
        simdutf_warn_unused virtual result
        base64_to_binary(const char* input, size_t length, char* output,
            base64_options options = base64_default,
            last_chunk_handling_options last_chunk_options = last_chunk_handling_options::loose) const noexcept
            = 0;

        /// Convert a base64 input to a binary output while returning more details
        /// than base64_to_binary.
        ///
        /// This function follows the WHATWG forgiving-base64 format, which means that
        /// it will ignore any ASCII spaces in the input. You may provide a padded
        /// input (with one or two equal signs at the end) or an unpadded input
        /// (without any equal signs at the end).
        ///
        /// See https://infra.spec.whatwg.org/#forgiving-base64-decode
        ///
        /// This function will fail in case of invalid input. When last_chunk_options =
        /// loose, there are two possible reasons for failure: the input contains a
        /// number of base64 characters that when divided by 4, leaves a single
        /// remainder character (BASE64_INPUT_REMAINDER), or the input contains a
        /// character that is not a valid base64 character (INVALID_BASE64_CHARACTER).
        ///
        /// You should call this function with a buffer that is at least
        /// maximal_binary_length_from_base64(input, length) bytes long. If you fail to
        /// provide that much space, the function may cause a buffer overflow.
        ///
        /// @param input         the base64 string to process
        /// @param length        the length of the string in bytes
        /// @param output        the pointer to a buffer that can hold the conversion
        /// result (should be at least maximal_binary_length_from_base64(input, length)
        /// bytes long).
        /// @param options       the base64 options to use, can be base64_default or
        /// base64_url, is base64_default by default.
        /// @param last_chunk_options the handling of the last chunk (default: loose)
        /// @return a full_result pair struct (of type turbo::result containing the
        /// three fields error, input_count and output_count).
        simdutf_warn_unused virtual full_result base64_to_binary_details(
            const char* input, size_t length, char* output,
            base64_options options = base64_default,
            last_chunk_handling_options last_chunk_options = last_chunk_handling_options::loose) const noexcept
            = 0;

        /// Convert a base64 input to a binary output.
        ///
        /// This function follows the WHATWG forgiving-base64 format, which means that
        /// it will ignore any ASCII spaces in the input. You may provide a padded
        /// input (with one or two equal signs at the end) or an unpadded input
        /// (without any equal signs at the end).
        ///
        /// See https://infra.spec.whatwg.org/#forgiving-base64-decode
        ///
        /// This function will fail in case of invalid input. When last_chunk_options =
        /// loose, there are two possible reasons for failure: the input contains a
        /// number of base64 characters that when divided by 4, leaves a single
        /// remainder character (BASE64_INPUT_REMAINDER), or the input contains a
        /// character that is not a valid base64 character (INVALID_BASE64_CHARACTER).
        ///
        /// You should call this function with a buffer that is at least
        /// maximal_binary_length_from_base64(input, length) bytes long. If you
        /// fail to provide that much space, the function may cause a buffer overflow.
        ///
        /// @param input         the base64 string to process, in ASCII stored as
        /// 16-bit units
        /// @param length        the length of the string in 16-bit units
        /// @param output        the pointer to a buffer that can hold the conversion
        /// result (should be at least maximal_binary_length_from_base64(input, length)
        /// bytes long).
        /// @param options       the base64 options to use, can be base64_default or
        /// base64_url, is base64_default by default.
        /// @param last_chunk_options the handling of the last chunk (default: loose)
        /// @return a result pair struct (of type turbo::result containing the two
        /// fields error and count) with an error code and position of the
        /// INVALID_BASE64_CHARACTER error (in the input in units) if any, or the
        /// number of bytes written if successful.
        simdutf_warn_unused virtual result
        base64_to_binary(const char16_t* input, size_t length, char* output,
            base64_options options = base64_default,
            last_chunk_handling_options last_chunk_options = last_chunk_handling_options::loose) const noexcept
            = 0;

        /// Convert a base64 input to a binary output while returning more details
        /// than base64_to_binary.
        ///
        /// This function follows the WHATWG forgiving-base64 format, which means that
        /// it will ignore any ASCII spaces in the input. You may provide a padded
        /// input (with one or two equal signs at the end) or an unpadded input
        /// (without any equal signs at the end).
        ///
        /// See https://infra.spec.whatwg.org/#forgiving-base64-decode
        ///
        /// This function will fail in case of invalid input. When last_chunk_options =
        /// loose, there are two possible reasons for failure: the input contains a
        /// number of base64 characters that when divided by 4, leaves a single
        /// remainder character (BASE64_INPUT_REMAINDER), or the input contains a
        /// character that is not a valid base64 character (INVALID_BASE64_CHARACTER).
        ///
        /// You should call this function with a buffer that is at least
        /// maximal_binary_length_from_base64(input, length) bytes long. If you fail to
        /// provide that much space, the function may cause a buffer overflow.
        ///
        /// @param input         the base64 string to process
        /// @param length        the length of the string in bytes
        /// @param output        the pointer to a buffer that can hold the conversion
        /// result (should be at least maximal_binary_length_from_base64(input, length)
        /// bytes long).
        /// @param options       the base64 options to use, can be base64_default or
        /// base64_url, is base64_default by default.
        /// @param last_chunk_options the handling of the last chunk (default: loose)
        /// @return a full_result pair struct (of type turbo::result containing the
        /// three fields error, input_count and output_count).
        simdutf_warn_unused virtual full_result base64_to_binary_details(
            const char16_t* input, size_t length, char* output,
            base64_options options = base64_default,
            last_chunk_handling_options last_chunk_options = last_chunk_handling_options::loose) const noexcept
            = 0;

        /// Provide the base64 length in bytes given the length of a binary input.
        ///
        /// @param length        the length of the input in bytes
        /// @param options       the base64 options to use, can be base64_default or
        /// base64_url, is base64_default by default.
        /// @return number of base64 bytes
        simdutf_warn_unused size_t base64_length_from_binary(
            size_t length, base64_options options = base64_default) const noexcept;

        /// Convert a binary input to a base64 output.
        ///
        /// The default option (turbo::base64_default) uses the characters `+` and
        /// `/` as part of its alphabet. Further, it adds padding (`=`) at the end of
        /// the output to ensure that the output length is a multiple of four.
        ///
        /// The URL option (turbo::base64_url) uses the characters `-` and `_` as
        /// part of its alphabet. No padding is added at the end of the output.
        ///
        /// This function always succeeds.
        ///
        /// @param input         the binary to process
        /// @param length        the length of the input in bytes
        /// @param output        the pointer to a buffer that can hold the conversion
        /// result (should be at least base64_length_from_binary(length) bytes long)
        /// @param options       the base64 options to use, can be base64_default or
        /// base64_url, is base64_default by default.
        /// @return number of written bytes, will be equal to
        /// base64_length_from_binary(length, options)
        virtual size_t
        binary_to_base64(const char* input, size_t length, char* output,
            base64_options options = base64_default) const noexcept
            = 0;

        /// Convert a binary input to a base64 output with lines of given length.
        /// Lines are separated by a single linefeed character.
        ///
        /// The default option (turbo::base64_default) uses the characters `+` and
        /// `/` as part of its alphabet. Further, it adds padding (`=`) at the end of
        /// the output to ensure that the output length is a multiple of four.
        ///
        /// The URL option (turbo::base64_url) uses the characters `-` and `_` as
        /// part of its alphabet. No padding is added at the end of the output.
        ///
        /// This function always succeeds.
        ///
        /// @param input         the binary to process
        /// @param length        the length of the input in bytes
        /// @param output        the pointer to a buffer that can hold the conversion
        /// result (should be at least base64_length_from_binary_with_lines(length,
        /// options, line_length) bytes long)
        /// @param line_length   the length of each line, values smaller than 4 are
        /// interpreted as 4
        /// @param options       the base64 options to use, can be base64_default or
        /// base64_url, is base64_default by default.
        /// @return number of written bytes, will be equal to
        /// base64_length_from_binary_with_lines(length, options, line_length)
        virtual size_t binary_to_base64_with_lines(
            const char* input, size_t length, char* output,
            size_t line_length = turbo::default_line_length,
            base64_options options = base64_default) const noexcept
            = 0;

        /// Find the first occurrence of a character in a string. If the character is
        /// not found, return a pointer to the end of the string.
        /// @param start        the start of the string
        /// @param end          the end of the string
        /// @param character    the character to find
        /// @return a pointer to the first occurrence of the character in the string,
        /// or a pointer to the end of the string if the character is not found.
        virtual const char* find(const char* start, const char* end,
            char character) const noexcept
            = 0;
        virtual const char16_t* find(const char16_t* start, const char16_t* end,
            char16_t character) const noexcept
            = 0;

#ifdef SIMDUTF_INTERNAL_TESTS
        // This method is exported only in developer mode, its purpose
        // is to expose some internal test procedures from the given
        // implementation and then use them through our standard test
        // framework.
        //
        // Regular users should not use it, the tests of the public
        // API are enough.

        struct TestProcedure {
            // display name
            std::string_view name;

            // procedure should return whether given test pass or not
            void (*procedure)(const implementation&);
        };

        virtual std::vector<TestProcedure> internal_tests() const;
#endif

    protected:
        /// @private Construct an implementation with the given name and description.
        /// For subclasses.
        /// @param name the name of this implementation
        /// @param description a description of this implementation
        /// @param required_instruction_sets the instruction sets this implementation
        /// requires
        simdutf_really_inline implementation(const char* name,
            const char* description,
            uint32_t required_instruction_sets)
            : _name(name)
            , _description(description)
            , _required_instruction_sets(required_instruction_sets) { }

    protected:
        ~implementation() = default;

    private:
        /// The name of this implementation.
        const char* _name;

        /// The description of this implementation.
        const char* _description;

        /// Instruction sets required for this implementation.
        const uint32_t _required_instruction_sets;
    };

    /// @private
    namespace internal {

        /// The list of available implementations compiled into simdutf.
        class AvailableImplementationList {
        public:
            /// Get the list of available implementations compiled into simdutf
            simdutf_really_inline AvailableImplementationList() { }
            /// Number of implementations
            size_t size() const noexcept;
            /// STL const begin() iterator
            const implementation* const* begin() const noexcept;
            /// STL const end() iterator
            const implementation* const* end() const noexcept;

            /// Get the implementation with the given name.
            ///
            /// Case sensitive.
            ///
            ///     const implementation *impl =
            /// turbo::available_implementations["westmere"]; if (!impl) { exit(1); } if
            /// (!imp->supported_by_runtime_system()) { exit(1); }
            ///     turbo::active_implementation = impl;
            ///
            /// @param name the implementation to find, e.g. "westmere", "haswell", "arm64"
            /// @return the implementation, or nullptr if the parse failed.
            const implementation* operator[](std::string_view name) const noexcept {
                for (const implementation* impl : *this) {
                    if (impl->name() == name) {
                        return impl;
                    }
                }
                return nullptr;
            }

            /// Detect the most advanced implementation supported by the current host.
            ///
            /// This is used to initialize the implementation on startup.
            ///
            ///     const implementation *impl =
            /// turbo::available_implementation::detect_best_supported();
            ///     turbo::active_implementation = impl;
            ///
            /// @return the most advanced supported implementation for the current host, or
            /// an implementation that returns UNSUPPORTED_ARCHITECTURE if there is no
            /// supported implementation. Will never return nullptr.
            const implementation* detect_best_supported() const noexcept;
        };

        template <typename T>
        class atomic_ptr {
        public:
            atomic_ptr(T* _ptr)
                : ptr { _ptr } { }

#if defined(SIMDUTF_NO_THREADS)
            operator const T*() const { return ptr; }
            const T& operator*() const { return *ptr; }
            const T* operator->() const { return ptr; }

            operator T*() { return ptr; }
            T& operator*() { return *ptr; }
            T* operator->() { return ptr; }
            atomic_ptr& operator=(T* _ptr) {
                ptr = _ptr;
                return *this;
            }

#else
            operator const T*() const { return ptr.load(); }
            const T& operator*() const { return *ptr; }
            const T* operator->() const { return ptr.load(); }

            operator T*() { return ptr.load(); }
            T& operator*() { return *ptr; }
            T* operator->() { return ptr.load(); }
            atomic_ptr& operator=(T* _ptr) {
                ptr = _ptr;
                return *this;
            }

#endif

        private:
#if defined(SIMDUTF_NO_THREADS)
            T* ptr;
#else
            std::atomic<T*> ptr;
#endif
        };

        class detect_best_supported_implementation_on_first_use;

    } // namespace internal

    /// The list of available implementations compiled into simdutf.
    extern SIMDUTF_DLLIMPORTEXPORT const internal::AvailableImplementationList&
    get_available_implementations();

    /// The active implementation.
    ///
    /// Automatically initialized on first use to the most advanced implementation
    /// supported by this hardware.
    extern SIMDUTF_DLLIMPORTEXPORT internal::atomic_ptr<const implementation>&
    get_active_implementation();

} // namespace turbo

  // this header is not part of the public api
#include <turbo/unicode/engine/base64_implementation.h>

#if SIMDUTF_CPLUSPLUS23

namespace turbo {
    namespace literals {

        namespace detail {

            // the detail namespace is not part of the public api

            template <std::size_t N>
            struct base64_literal_helper {
                std::array<char, N - 1> storage {};
                static constexpr std::size_t size() noexcept { return N - 1; }
                consteval base64_literal_helper(const char (&str)[N]) {
                    for (std::size_t i = 0; i < size(); i++) {
                        storage[i] = str[i];
                    }
                }
            };

            template <std::size_t InputLen>
            struct base64_decode_result {
                static constexpr std::size_t max_out = (InputLen + 3) / 4 * 3;
                std::array<char, max_out> buffer {};
                std::size_t output_count {};
            };

            template <std::size_t InputLen>
            consteval auto base64_decode_literal(const char* str) {
                base64_decode_result<InputLen> result {};
                auto r = scalar::base64::base64_to_binary_details_impl(
                    str, InputLen, result.buffer.data(), base64_default, loose);
                if (r.error != error_code::SUCCESS) {
#if __cpp_lib_unreachable >= 202202L
                    std::unreachable(); // invalid base64 input in _base64 literal
#else
                    // workaround for older stdlib
                    throw "invalid base64 input in _base64 literal";
#endif
                }
                result.output_count = r.output_count;
                return result;
            }

            template <base64_literal_helper a>
            consteval auto base64_make_array() {
                constexpr auto decoded = base64_decode_literal<a.size()>(a.storage.data());
                std::array<char, decoded.output_count> ret {};
                for (std::size_t i = 0; i < decoded.output_count; i++) {
                    ret[i] = decoded.buffer[i];
                }
                return ret;
            }

        } // namespace detail

        /// User-defined literal for compile-time base64 decoding.
        ///
        /// Usage:
        ///   using namespace turbo::literals;
        ///   constexpr auto decoded = "SGVsbG8gV29ybGQh"_base64;
        ///   // decoded is a std::array<char, 12> containing "Hello World!"
        ///
        /// The input must be valid base64. Whitepace is allowed and ignored.
        /// A compilation error occurs if the input is invalid.
        template <detail::base64_literal_helper a>
        consteval auto operator""_base64() {
            return detail::base64_make_array<a>();
        }

    } // namespace literals
} // namespace turbo

#endif // SIMDUTF_CPLUSPLUS23

#endif // SIMDUTF_IMPLEMENTATION_H
