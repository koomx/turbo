#pragma once

#if !UNICODE_NO_THREADS
#include <atomic>
#endif
#ifdef UNICODE_INTERNAL_TESTS
#include <vector>
#endif
#include <turbo/unicode/engine/portability.h>
#include <turbo/unicode/text_encoding.h>
#include <turbo/unicode/error.h>
#include <turbo/arch/isadetection.h>
#include <turbo/unicode/api/base64.h>
#include <string_view>

namespace turbo {



#ifndef UNICODE_NEED_TRAILING_ZEROES
#define UNICODE_NEED_TRAILING_ZEROES 1
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
        virtual TextEncoding autodetect_encoding(const char* input,
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
         [[nodiscard]] virtual bool validate_utf8(const char* buf,
            size_t len) const noexcept
            = 0;

        /// Validate the UTF-8 string and stop on errors.
        ///
        /// Overridden by each implementation.
        ///
        /// @param buf the UTF-8 string to validate.
        /// @param len the length of the string in bytes.
        /// @return a UnicodeResult pair struct (of type turbo::UnicodeResult containing the two
        /// fields error and count) with an error code and either position of the error
        /// (in the input in code units) if any, or the number of code units validated
        /// if successful.
         [[nodiscard]] virtual UnicodeResult
        validate_utf8_with_errors(const char* buf, size_t len) const noexcept
            = 0;

        /// Validate the ASCII string.
        ///
        /// Overridden by each implementation.
        ///
        /// @param buf the ASCII string to validate.
        /// @param len the length of the string in bytes.
        /// @return true if and only if the string is valid ASCII.
         [[nodiscard]] virtual bool
        validate_ascii(const char* buf, size_t len) const noexcept
            = 0;

        /// Validate the ASCII string and stop on error.
        ///
        /// Overridden by each implementation.
        ///
        /// @param buf the ASCII string to validate.
        /// @param len the length of the string in bytes.
        /// @return a UnicodeResult pair struct (of type turbo::UnicodeResult containing the two
        /// fields error and count) with an error code and either position of the error
        /// (in the input in code units) if any, or the number of code units validated
        /// if successful.
         [[nodiscard]] virtual UnicodeResult
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
         [[nodiscard]] virtual bool
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
         [[nodiscard]] virtual bool
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
         [[nodiscard]] virtual bool
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
         [[nodiscard]] virtual bool
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
        /// @return a UnicodeResult pair struct (of type turbo::UnicodeResult containing the two
        /// fields error and count) with an error code and either position of the error
        /// (in the input in code units) if any, or the number of code units validated
        /// if successful.
         [[nodiscard]] virtual UnicodeResult
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
        /// @return a UnicodeResult pair struct (of type turbo::UnicodeResult containing the two
        /// fields error and count) with an error code and either position of the error
        /// (in the input in code units) if any, or the number of code units validated
        /// if successful.
         [[nodiscard]] virtual UnicodeResult
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
         [[nodiscard]] virtual bool
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
        /// @return a UnicodeResult pair struct (of type turbo::UnicodeResult containing the two
        /// fields error and count) with an error code and either position of the error
        /// (in the input in code units) if any, or the number of code units validated
        /// if successful.
         [[nodiscard]] virtual UnicodeResult
        validate_utf32_with_errors(const char32_t* buf,
            size_t len) const noexcept
            = 0;

        /// Convert Latin1 string into UTF-8 string.
        ///
        /// This function is suitable to work with inputs from untrusted sources.
        ///
        /// @param input         the Latin1 string to convert
        /// @param length        the length of the string in bytes
        /// @param utf8_output  the pointer to buffer that can hold conversion UnicodeResult
        /// @return the number of written char; 0 if conversion is not possible
         [[nodiscard]] virtual size_t
        convert_latin1_to_utf8(const char* input, size_t length,
            char* utf8_output) const noexcept
            = 0;

        /// Convert possibly Latin1 string into UTF-16LE string.
        ///
        /// This function is suitable to work with inputs from untrusted sources.
        ///
        /// @param input         the Latin1  string to convert
        /// @param length        the length of the string in bytes
        /// @param utf16_output  the pointer to buffer that can hold conversion UnicodeResult
        /// @return the number of written char16_t; 0 if conversion is not possible
         [[nodiscard]] virtual size_t
        convert_latin1_to_utf16le(const char* input, size_t length,
            char16_t* utf16_output) const noexcept
            = 0;

        /// Convert Latin1 string into UTF-16BE string.
        ///
        /// This function is suitable to work with inputs from untrusted sources.
        ///
        /// @param input         the Latin1 string to convert
        /// @param length        the length of the string in bytes
        /// @param utf16_output  the pointer to buffer that can hold conversion UnicodeResult
        /// @return the number of written char16_t; 0 if conversion is not possible
         [[nodiscard]] virtual size_t
        convert_latin1_to_utf16be(const char* input, size_t length,
            char16_t* utf16_output) const noexcept
            = 0;

        /// Convert Latin1 string into UTF-32 string.
        ///
        /// This function is suitable to work with inputs from untrusted sources.
        ///
        /// @param input         the Latin1 string to convert
        /// @param length        the length of the string in bytes
        /// @param utf32_buffer  the pointer to buffer that can hold conversion UnicodeResult
        /// @return the number of written char32_t; 0 if conversion is not possible
         [[nodiscard]] virtual size_t
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
        /// @param latin1_output  the pointer to buffer that can hold conversion UnicodeResult
        /// @return the number of written char; 0 if the input was not valid UTF-8
        /// string or if it cannot be represented as Latin1
         [[nodiscard]] virtual size_t
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
        /// @param latin1_output  the pointer to buffer that can hold conversion UnicodeResult
        /// @return a UnicodeResult pair struct (of type turbo::UnicodeResult containing the two
        /// fields error and count) with an error code and either position of the error
        /// (in the input in code units) if any, or the number of code units validated
        /// if successful.
         [[nodiscard]] virtual UnicodeResult
        convert_utf8_to_latin1_with_errors(const char* input, size_t length,
            char* latin1_output) const noexcept
            = 0;

        /// Convert valid UTF-8 string into latin1 string.
        ///
        /// This function assumes that the input string is valid UTF-8 and that it can
        /// be represented as Latin1. If you violate this assumption, the UnicodeResult is
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
        /// @param latin1_output  the pointer to buffer that can hold conversion UnicodeResult
        /// @return the number of written char; 0 if the input was not valid UTF-8
        /// string
         [[nodiscard]] virtual size_t
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
        /// @param utf16_output  the pointer to buffer that can hold conversion UnicodeResult
        /// @return the number of written char16_t; 0 if the input was not valid UTF-8
        /// string
         [[nodiscard]] virtual size_t
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
        /// @param utf16_output  the pointer to buffer that can hold conversion UnicodeResult
        /// @return the number of written char16_t; 0 if the input was not valid UTF-8
        /// string
         [[nodiscard]] virtual size_t
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
        /// @param utf16_output  the pointer to buffer that can hold conversion UnicodeResult
        /// @return a UnicodeResult pair struct (of type turbo::UnicodeResult containing the two
        /// fields error and count) with an error code and either position of the error
        /// (in the input in code units) if any, or the number of code units validated
        /// if successful.
         [[nodiscard]] virtual UnicodeResult convert_utf8_to_utf16le_with_errors(
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
        /// @param utf16_output  the pointer to buffer that can hold conversion UnicodeResult
        /// @return a UnicodeResult pair struct (of type turbo::UnicodeResult containing the two
        /// fields error and count) with an error code and either position of the error
        /// (in the input in code units) if any, or the number of code units validated
        /// if successful.
         [[nodiscard]] virtual UnicodeResult convert_utf8_to_utf16be_with_errors(
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
        /// @return a UnicodeResult pair struct (of type turbo::UnicodeResult containing the two
        /// fields error and count) where the count is the number of bytes required to
        /// encode the UTF-16LE string as UTF-8, and the error code is either SUCCESS
        /// or SURROGATE. The count is correct regardless of the error field.
        /// When SURROGATE is returned, it does not indicate an error in the case of
        /// this function: it indicates that at least one surrogate has been
        /// encountered: the surrogates may be matched or not (thus this function does
        /// not validate). If the returned error code is SUCCESS, then the input
        /// contains no surrogate, is in the Basic Multilingual Plane, and is
        /// necessarily valid.
        [[nodiscard]] virtual UnicodeResult utf8_length_from_utf16le_with_replacement(
            const char16_t* input, size_t length) const noexcept
            = 0;

        /// Compute the number of bytes that this UTF-16BE string would require in
        /// UTF-8 format even when the UTF-16BE content contains mismatched
        /// surrogates that have to be replaced by the replacement character (0xFFFD).
        ///
        /// @param input         the UTF-16BE string to convert
        /// @param length        the length of the string in 2-byte code units
        /// (char16_t)
        /// @return a UnicodeResult pair struct (of type turbo::UnicodeResult containing the two
        /// fields error and count) where the count is the number of bytes required to
        /// encode the UTF-16BE string as UTF-8, and the error code is either SUCCESS
        /// or SURROGATE. The count is correct regardless of the error field.
        /// When SURROGATE is returned, it does not indicate an error in the case of
        /// this function: it indicates that at least one surrogate has been
        /// encountered: the surrogates may be matched or not (thus this function does
        /// not validate). If the returned error code is SUCCESS, then the input
        /// contains no surrogate, is in the Basic Multilingual Plane, and is
        /// necessarily valid.
        [[nodiscard]] virtual UnicodeResult utf8_length_from_utf16be_with_replacement(
            const char16_t* input, size_t length) const noexcept
            = 0;


        /// Convert possibly broken UTF-8 string into UTF-32 string.
        ///
        /// During the conversion also validation of the input string is done.
        /// This function is suitable to work with inputs from untrusted sources.
        ///
        /// @param input         the UTF-8 string to convert
        /// @param length        the length of the string in bytes
        /// @param utf32_output  the pointer to buffer that can hold conversion UnicodeResult
        /// @return the number of written char16_t; 0 if the input was not valid UTF-8
        /// string
         [[nodiscard]] virtual size_t
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
        /// @param utf32_output  the pointer to buffer that can hold conversion UnicodeResult
        /// @return a UnicodeResult pair struct (of type turbo::UnicodeResult containing the two
        /// fields error and count) with an error code and either position of the error
        /// (in the input in code units) if any, or the number of char32_t written if
        /// successful.
         [[nodiscard]] virtual UnicodeResult
        convert_utf8_to_utf32_with_errors(const char* input, size_t length,
            char32_t* utf32_output) const noexcept
            = 0;


        /// Convert valid UTF-8 string into UTF-16LE string.
        ///
        /// This function assumes that the input string is valid UTF-8.
        ///
        /// @param input         the UTF-8 string to convert
        /// @param length        the length of the string in bytes
        /// @param utf16_buffer  the pointer to buffer that can hold conversion UnicodeResult
        /// @return the number of written char16_t
         [[nodiscard]] virtual size_t
        convert_valid_utf8_to_utf16le(const char* input, size_t length,
            char16_t* utf16_buffer) const noexcept
            = 0;

        /// Convert valid UTF-8 string into UTF-16BE string.
        ///
        /// This function assumes that the input string is valid UTF-8.
        ///
        /// @param input         the UTF-8 string to convert
        /// @param length        the length of the string in bytes
        /// @param utf16_buffer  the pointer to buffer that can hold conversion UnicodeResult
        /// @return the number of written char16_t
         [[nodiscard]] virtual size_t
        convert_valid_utf8_to_utf16be(const char* input, size_t length,
            char16_t* utf16_buffer) const noexcept
            = 0;

        /// Convert valid UTF-8 string into UTF-32 string.
        ///
        /// This function assumes that the input string is valid UTF-8.
        ///
        /// @param input         the UTF-8 string to convert
        /// @param length        the length of the string in bytes
        /// @param utf32_buffer  the pointer to buffer that can hold conversion UnicodeResult
        /// @return the number of written char32_t
         [[nodiscard]] virtual size_t
        convert_valid_utf8_to_utf32(const char* input, size_t length,
            char32_t* utf32_buffer) const noexcept
            = 0;

        /// Compute the number of 2-byte code units that this UTF-8 string would
        /// require in UTF-16LE format.
        ///
        /// This function does not validate the input. It is acceptable to pass invalid
        /// UTF-8 strings but in such cases the UnicodeResult is implementation defined.
        ///
        /// @param input         the UTF-8 string to process
        /// @param length        the length of the string in bytes
        /// @return the number of char16_t code units required to encode the UTF-8
        /// string as UTF-16LE
         [[nodiscard]] virtual size_t
        utf16_length_from_utf8(const char* input, size_t length) const noexcept
            = 0;

        /// Compute the number of 4-byte code units that this UTF-8 string would
        /// require in UTF-32 format.
        ///
        /// This function is equivalent to count_utf8. It is acceptable to pass invalid
        /// UTF-8 strings but in such cases the UnicodeResult is implementation defined.
        ///
        /// This function does not validate the input.
        ///
        /// @param input         the UTF-8 string to process
        /// @param length        the length of the string in bytes
        /// @return the number of char32_t code units required to encode the UTF-8
        /// string as UTF-32
         [[nodiscard]] virtual size_t
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
        /// UnicodeResult
        /// @return number of written code units; 0 if input is not a valid UTF-16LE
        /// string or if it cannot be represented as Latin1
         [[nodiscard]] virtual size_t
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
        /// UnicodeResult
        /// @return number of written code units; 0 if input is not a valid UTF-16BE
        /// string or if it cannot be represented as Latin1
         [[nodiscard]] virtual size_t
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
        /// UnicodeResult
        /// @return a UnicodeResult pair struct (of type turbo::UnicodeResult containing the two
        /// fields error and count) with an error code and either position of the error
        /// (in the input in code units) if any, or the number of char written if
        /// successful.
         [[nodiscard]] virtual UnicodeResult
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
        /// UnicodeResult
        /// @return a UnicodeResult pair struct (of type turbo::UnicodeResult containing the two
        /// fields error and count) with an error code and either position of the error
        /// (in the input in code units) if any, or the number of char written if
        /// successful.
         [[nodiscard]] virtual UnicodeResult
        convert_utf16be_to_latin1_with_errors(const char16_t* input, size_t length,
            char* latin1_buffer) const noexcept
            = 0;

        /// Convert valid UTF-16LE string into Latin1 string.
        ///
        /// This function assumes that the input string is valid UTF-L16LE and that it
        /// can be represented as Latin1. If you violate this assumption, the UnicodeResult is
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
        /// UnicodeResult
        /// @return number of written code units; 0 if conversion is not possible
         [[nodiscard]] virtual size_t
        convert_valid_utf16le_to_latin1(const char16_t* input, size_t length,
            char* latin1_buffer) const noexcept
            = 0;

        /// Convert valid UTF-16BE string into Latin1 string.
        ///
        /// This function assumes that the input string is valid UTF16-BE and that it
        /// can be represented as Latin1. If you violate this assumption, the UnicodeResult is
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
        /// UnicodeResult
        /// @return number of written code units; 0 if conversion is not possible
         [[nodiscard]] virtual size_t
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
        /// @param utf8_buffer   the pointer to buffer that can hold conversion UnicodeResult
        /// @return number of written code units; 0 if input is not a valid UTF-16LE
        /// string
         [[nodiscard]] virtual size_t
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
        /// @param utf8_buffer   the pointer to buffer that can hold conversion UnicodeResult
        /// @return number of written code units; 0 if input is not a valid UTF-16BE
        /// string
         [[nodiscard]] virtual size_t
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
        /// @param utf8_buffer   the pointer to buffer that can hold conversion UnicodeResult
        /// @return a UnicodeResult pair struct (of type turbo::UnicodeResult containing the two
        /// fields error and count) with an error code and either position of the error
        /// (in the input in code units) if any, or the number of char written if
        /// successful.
         [[nodiscard]] virtual UnicodeResult
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
        /// @param utf8_buffer   the pointer to buffer that can hold conversion UnicodeResult
        /// @return a UnicodeResult pair struct (of type turbo::UnicodeResult containing the two
        /// fields error and count) with an error code and either position of the error
        /// (in the input in code units) if any, or the number of char written if
        /// successful.
         [[nodiscard]] virtual UnicodeResult
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
        /// @param utf8_buffer   the pointer to buffer that can hold conversion UnicodeResult
        /// @return number of written code units
         [[nodiscard]] virtual size_t convert_utf16le_to_utf8_with_replacement(
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
        /// @param utf8_buffer   the pointer to buffer that can hold conversion UnicodeResult
        /// @return number of written code units
         [[nodiscard]] virtual size_t convert_utf16be_to_utf8_with_replacement(
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
        /// UnicodeResult
        /// @return number of written code units; 0 if conversion is not possible
         [[nodiscard]] virtual size_t
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
        /// UnicodeResult
        /// @return number of written code units; 0 if conversion is not possible
         [[nodiscard]] virtual size_t
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
        /// @param utf32_buffer   the pointer to buffer that can hold conversion UnicodeResult
        /// @return number of written code units; 0 if input is not a valid UTF-16LE
        /// string
         [[nodiscard]] virtual size_t
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
        /// @param utf32_buffer   the pointer to buffer that can hold conversion UnicodeResult
        /// @return number of written code units; 0 if input is not a valid UTF-16BE
        /// string
         [[nodiscard]] virtual size_t
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
        /// @param utf32_buffer   the pointer to buffer that can hold conversion UnicodeResult
        /// @return a UnicodeResult pair struct (of type turbo::UnicodeResult containing the two
        /// fields error and count) with an error code and either position of the error
        /// (in the input in code units) if any, or the number of char32_t written if
        /// successful.
         [[nodiscard]] virtual UnicodeResult convert_utf16le_to_utf32_with_errors(
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
        /// @param utf32_buffer   the pointer to buffer that can hold conversion UnicodeResult
        /// @return a UnicodeResult pair struct (of type turbo::UnicodeResult containing the two
        /// fields error and count) with an error code and either position of the error
        /// (in the input in code units) if any, or the number of char32_t written if
        /// successful.
         [[nodiscard]] virtual UnicodeResult convert_utf16be_to_utf32_with_errors(
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
        /// UnicodeResult
        /// @return number of written code units; 0 if conversion is not possible
         [[nodiscard]] virtual size_t
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
        /// UnicodeResult
        /// @return number of written code units; 0 if conversion is not possible
         [[nodiscard]] virtual size_t
        convert_valid_utf16be_to_utf32(const char16_t* input, size_t length,
            char32_t* utf32_buffer) const noexcept
            = 0;

        /// Compute the number of bytes that this UTF-16LE string would require in
        /// UTF-8 format.
        ///
        /// This function does not validate the input. It is acceptable to pass invalid
        /// UTF-16 strings but in such cases the UnicodeResult is implementation defined.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param input         the UTF-16LE string to convert
        /// @param length        the length of the string in 2-byte code units
        /// (char16_t)
        /// @return the number of bytes required to encode the UTF-16LE string as UTF-8
         [[nodiscard]] virtual size_t
        utf8_length_from_utf16le(const char16_t* input,
            size_t length) const noexcept
            = 0;

        /// Compute the number of bytes that this UTF-16BE string would require in
        /// UTF-8 format.
        ///
        /// This function does not validate the input. It is acceptable to pass invalid
        /// UTF-16 strings but in such cases the UnicodeResult is implementation defined.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param input         the UTF-16BE string to convert
        /// @param length        the length of the string in 2-byte code units
        /// (char16_t)
        /// @return the number of bytes required to encode the UTF-16BE string as UTF-8
         [[nodiscard]] virtual size_t
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
        /// UnicodeResult
        /// @return number of written code units; 0 if input is not a valid UTF-32
        /// string
         [[nodiscard]] virtual size_t
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
        /// UnicodeResult
        /// @return a UnicodeResult pair struct (of type turbo::UnicodeResult containing the two
        /// fields error and count) with an error code and either position of the error
        /// (in the input in code units) if any, or the number of char written if
        /// successful.
         [[nodiscard]] virtual UnicodeResult
        convert_utf32_to_latin1_with_errors(const char32_t* input, size_t length,
            char* latin1_buffer) const noexcept
            = 0;

        /// Convert valid UTF-32 string into Latin1 string.
        ///
        /// This function assumes that the input string is valid UTF-32 and can be
        /// represented as Latin1. If you violate this assumption, the UnicodeResult is
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
        /// UnicodeResult
        /// @return number of written code units; 0 if conversion is not possible
         [[nodiscard]] virtual size_t
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
        /// @param utf8_buffer   the pointer to buffer that can hold conversion UnicodeResult
        /// @return number of written code units; 0 if input is not a valid UTF-32
        /// string
         [[nodiscard]] virtual size_t
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
        /// @param utf8_buffer   the pointer to buffer that can hold conversion UnicodeResult
        /// @return a UnicodeResult pair struct (of type turbo::UnicodeResult containing the two
        /// fields error and count) with an error code and either position of the error
        /// (in the input in code units) if any, or the number of char written if
        /// successful.
         [[nodiscard]] virtual UnicodeResult
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
        /// UnicodeResult
        /// @return number of written code units; 0 if conversion is not possible
         [[nodiscard]] virtual size_t
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
         [[nodiscard]] virtual size_t
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
        /// @param utf16_buffer   the pointer to buffer that can hold conversion UnicodeResult
        /// @return number of written code units; 0 if input is not a valid UTF-32
        /// string
         [[nodiscard]] virtual size_t
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
        /// @param utf16_buffer   the pointer to buffer that can hold conversion UnicodeResult
        /// @return number of written code units; 0 if input is not a valid UTF-32
        /// string
         [[nodiscard]] virtual size_t
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
        /// @param utf16_buffer   the pointer to buffer that can hold conversion UnicodeResult
        /// @return a UnicodeResult pair struct (of type turbo::UnicodeResult containing the two
        /// fields error and count) with an error code and either position of the error
        /// (in the input in code units) if any, or the number of char16_t written if
        /// successful.
         [[nodiscard]] virtual UnicodeResult convert_utf32_to_utf16le_with_errors(
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
        /// @param utf16_buffer   the pointer to buffer that can hold conversion UnicodeResult
        /// @return a UnicodeResult pair struct (of type turbo::UnicodeResult containing the two
        /// fields error and count) with an error code and either position of the error
        /// (in the input in code units) if any, or the number of char16_t written if
        /// successful.
         [[nodiscard]] virtual UnicodeResult convert_utf32_to_utf16be_with_errors(
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
        /// UnicodeResult
        /// @return number of written code units; 0 if conversion is not possible
         [[nodiscard]] virtual size_t
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
        /// UnicodeResult
        /// @return number of written code units; 0 if conversion is not possible
         [[nodiscard]] virtual size_t
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
        /// UnicodeResult
        virtual void change_endianness_utf16(const char16_t* input, size_t length,
            char16_t* output) const noexcept
            = 0;

        /// Return the number of bytes that this Latin1 string would require in UTF-8
        /// format.
        ///
        /// @param input         the Latin1 string to convert
        /// @param length        the length of the string bytes
        /// @return the number of bytes required to encode the Latin1 string as UTF-8
         [[nodiscard]] virtual size_t
        utf8_length_from_latin1(const char* input, size_t length) const noexcept
            = 0;

        /// Compute the number of bytes that this UTF-32 string would require in UTF-8
        /// format.
        ///
        /// This function does not validate the input. It is acceptable to pass invalid
        /// UTF-32 strings but in such cases the UnicodeResult is implementation defined.
        ///
        /// @param input         the UTF-32 string to convert
        /// @param length        the length of the string in 4-byte code units
        /// (char32_t)
        /// @return the number of bytes required to encode the UTF-32 string as UTF-8
         [[nodiscard]] virtual size_t
        utf8_length_from_utf32(const char32_t* input,
            size_t length) const noexcept
            = 0;

        /// Compute the number of bytes that this UTF-32 string would require in Latin1
        /// format.
        ///
        /// This function does not validate the input. It is acceptable to pass invalid
        /// UTF-32 strings but in such cases the UnicodeResult is implementation defined.
        ///
        /// @param length        the length of the string in 4-byte code units
        /// (char32_t)
        /// @return the number of bytes required to encode the UTF-32 string as Latin1
         [[nodiscard]] virtual size_t
        latin1_length_from_utf32(size_t length) const noexcept {
            return length;
        }

        /// Compute the number of bytes that this UTF-8 string would require in Latin1
        /// format.
        ///
        /// This function does not validate the input. It is acceptable to pass invalid
        /// UTF-8 strings but in such cases the UnicodeResult is implementation defined.
        ///
        /// @param input         the UTF-8 string to convert
        /// @param length        the length of the string in byte
        /// @return the number of bytes required to encode the UTF-8 string as Latin1
         [[nodiscard]] virtual size_t
        latin1_length_from_utf8(const char* input, size_t length) const noexcept
            = 0;

        /// Compute the number of bytes that this UTF-16LE/BE string would require in
        /// Latin1 format.
        ///
        /// This function does not validate the input. It is acceptable to pass invalid
        /// UTF-16 strings but in such cases the UnicodeResult is implementation defined.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param length        the length of the string in 2-byte code units
        /// (char16_t)
        /// @return the number of bytes required to encode the UTF-16LE string as
        /// Latin1
         [[nodiscard]] virtual size_t
        latin1_length_from_utf16(size_t length) const noexcept {
            return length;
        }

        /// Compute the number of two-byte code units that this UTF-32 string would
        /// require in UTF-16 format.
        ///
        /// This function does not validate the input. It is acceptable to pass invalid
        /// UTF-32 strings but in such cases the UnicodeResult is implementation defined.
        ///
        /// @param input         the UTF-32 string to convert
        /// @param length        the length of the string in 4-byte code units
        /// (char32_t)
        /// @return the number of bytes required to encode the UTF-32 string as UTF-16
         [[nodiscard]] virtual size_t
        utf16_length_from_utf32(const char32_t* input,
            size_t length) const noexcept
            = 0;

        /// Return the number of bytes that this UTF-32 string would require in Latin1
        /// format.
        ///
        /// @param length        the length of the string in 4-byte code units
        /// (char32_t)
        /// @return the number of bytes required to encode the UTF-32 string as Latin1
         [[nodiscard]] virtual size_t
        utf32_length_from_latin1(size_t length) const noexcept {
            return length;
        }

        /// Compute the number of bytes that this UTF-16LE string would require in
        /// UTF-32 format.
        ///
        /// This function is equivalent to count_utf16le.
        ///
        /// This function does not validate the input. It is acceptable to pass invalid
        /// UTF-16 strings but in such cases the UnicodeResult is implementation defined.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param input         the UTF-16LE string to convert
        /// @param length        the length of the string in 2-byte code units
        /// (char16_t)
        /// @return the number of bytes required to encode the UTF-16LE string as
        /// UTF-32
         [[nodiscard]] virtual size_t
        utf32_length_from_utf16le(const char16_t* input,
            size_t length) const noexcept
            = 0;

        /// Compute the number of bytes that this UTF-16BE string would require in
        /// UTF-32 format.
        ///
        /// This function is equivalent to count_utf16be.
        ///
        /// This function does not validate the input. It is acceptable to pass invalid
        /// UTF-16 strings but in such cases the UnicodeResult is implementation defined.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param input         the UTF-16BE string to convert
        /// @param length        the length of the string in 2-byte code units
        /// (char16_t)
        /// @return the number of bytes required to encode the UTF-16BE string as
        /// UTF-32
         [[nodiscard]] virtual size_t
        utf32_length_from_utf16be(const char16_t* input,
            size_t length) const noexcept
            = 0;

        /// Count the number of code points (characters) in the string assuming that
        /// it is valid.
        ///
        /// This function assumes that the input string is valid UTF-16LE.
        /// It is acceptable to pass invalid UTF-16 strings but in such cases
        /// the UnicodeResult is implementation defined.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param input         the UTF-16LE string to process
        /// @param length        the length of the string in 2-byte code units
        /// (char16_t)
        /// @return number of code points
         [[nodiscard]] virtual size_t
        count_utf16le(const char16_t* input, size_t length) const noexcept
            = 0;

        /// Count the number of code points (characters) in the string assuming that
        /// it is valid.
        ///
        /// This function assumes that the input string is valid UTF-16BE.
        /// It is acceptable to pass invalid UTF-16 strings but in such cases
        /// the UnicodeResult is implementation defined.
        ///
        /// This function is not BOM-aware.
        ///
        /// @param input         the UTF-16BE string to process
        /// @param length        the length of the string in 2-byte code units
        /// (char16_t)
        /// @return number of code points
         [[nodiscard]] virtual size_t
        count_utf16be(const char16_t* input, size_t length) const noexcept
            = 0;

        /// Count the number of code points (characters) in the string assuming that
        /// it is valid.
        ///
        /// This function assumes that the input string is valid UTF-8.
        /// It is acceptable to pass invalid UTF-8 strings but in such cases
        /// the UnicodeResult is implementation defined.
        ///
        /// @param input         the UTF-8 string to process
        /// @param length        the length of the string in bytes
        /// @return number of code points
         [[nodiscard]] virtual size_t
        count_utf8(const char* input, size_t length) const noexcept
            = 0;

        /// Provide the maximal binary length in bytes given the base64 input.
        /// As long as the input does not contain ignorable characters (e.g., ASCII
        /// spaces or linefeed characters), the UnicodeResult is exact. In particular, the
        /// function checks for padding characters.
        ///
        /// The function is fast (constant time). It checks up to two characters at
        /// the end of the string. The input is not otherwise validated or read..
        ///
        /// @param input         the base64 input to process
        /// @param length        the length of the base64 input in bytes
        /// @return maximal number of binary bytes
         [[nodiscard]] size_t maximal_binary_length_from_base64(
            const char* input, size_t length) const noexcept;

        /// Provide the maximal binary length in bytes given the base64 input.
        /// As long as the input does not contain ignorable characters (e.g., ASCII
        /// spaces or linefeed characters), the UnicodeResult is exact. In particular, the
        /// function checks for padding characters.
        ///
        /// The function is fast (constant time). It checks up to two characters at
        /// the end of the string. The input is not otherwise validated or read.
        ///
        /// @param input         the base64 input to process, in ASCII stored as 16-bit
        /// units
        /// @param length        the length of the base64 input in 16-bit units
        /// @return maximal number of binary bytes
         [[nodiscard]] size_t maximal_binary_length_from_base64(
            const char16_t* input, size_t length) const noexcept;

        /// Compute the binary length from a base64 input with ASCII spaces.
        /// This function is useful for well-formed base64 inputs that may contain
        /// ASCII spaces (such as line breaks). For such inputs, the UnicodeResult is exact.
        ///
        /// The function counts non-whitespace characters (ASCII value > 0x20) and
        /// subtracts padding characters ('=') found at the end.
        ///
        /// @param input         the base64 input to process
        /// @param length        the length of the base64 input in bytes
        /// @return number of binary bytes
         [[nodiscard]] virtual size_t
        binary_length_from_base64(const char* input, size_t length) const noexcept;

        /// Compute the binary length from a base64 input with ASCII spaces.
        /// This function is useful for well-formed base64 inputs that may contain
        /// ASCII spaces (such as line breaks). For such inputs, the UnicodeResult is exact.
        ///
        /// The function counts non-whitespace characters (ASCII value > 0x20) and
        /// subtracts padding characters ('=') found at the end.
        ///
        /// @param input         the base64 input to process, in ASCII stored as 16-bit
        /// units
        /// @param length        the length of the base64 input in 16-bit units
        /// @return number of binary bytes
         [[nodiscard]] virtual size_t
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
        /// UnicodeResult (should be at least maximal_binary_length_from_base64(input, length)
        /// bytes long).
        /// @param options       the base64 options to use, can be base64_default or
        /// base64_url, is base64_default by default.
        /// @param last_chunk_options the handling of the last chunk (default: loose)
        /// @return a UnicodeResult pair struct (of type turbo::UnicodeResult containing the two
        /// fields error and count) with an error code and either position of the error
        /// (in the input in bytes) if any, or the number of bytes written if
        /// successful.
         [[nodiscard]] virtual UnicodeResult
        base64_to_binary(const char* input, size_t length, char* output,
            Base64Options options = base64_default,
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
        /// UnicodeResult (should be at least maximal_binary_length_from_base64(input, length)
        /// bytes long).
        /// @param options       the base64 options to use, can be base64_default or
        /// base64_url, is base64_default by default.
        /// @param last_chunk_options the handling of the last chunk (default: loose)
        /// @return a full_result pair struct (of type turbo::UnicodeResult containing the
        /// three fields error, input_count and output_count).
         [[nodiscard]] virtual full_result base64_to_binary_details(
            const char* input, size_t length, char* output,
            Base64Options options = base64_default,
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
        /// UnicodeResult (should be at least maximal_binary_length_from_base64(input, length)
        /// bytes long).
        /// @param options       the base64 options to use, can be base64_default or
        /// base64_url, is base64_default by default.
        /// @param last_chunk_options the handling of the last chunk (default: loose)
        /// @return a UnicodeResult pair struct (of type turbo::UnicodeResult containing the two
        /// fields error and count) with an error code and position of the
        /// INVALID_BASE64_CHARACTER error (in the input in units) if any, or the
        /// number of bytes written if successful.
         [[nodiscard]] virtual UnicodeResult
        base64_to_binary(const char16_t* input, size_t length, char* output,
            Base64Options options = base64_default,
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
        /// UnicodeResult (should be at least maximal_binary_length_from_base64(input, length)
        /// bytes long).
        /// @param options       the base64 options to use, can be base64_default or
        /// base64_url, is base64_default by default.
        /// @param last_chunk_options the handling of the last chunk (default: loose)
        /// @return a full_result pair struct (of type turbo::UnicodeResult containing the
        /// three fields error, input_count and output_count).
         [[nodiscard]] virtual full_result base64_to_binary_details(
            const char16_t* input, size_t length, char* output,
            Base64Options options = base64_default,
            last_chunk_handling_options last_chunk_options = last_chunk_handling_options::loose) const noexcept
            = 0;

        /// Provide the base64 length in bytes given the length of a binary input.
        ///
        /// @param length        the length of the input in bytes
        /// @param options       the base64 options to use, can be base64_default or
        /// base64_url, is base64_default by default.
        /// @return number of base64 bytes
         [[nodiscard]] size_t base64_length_from_binary(
            size_t length, Base64Options options = base64_default) const noexcept;

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
        /// UnicodeResult (should be at least base64_length_from_binary(length) bytes long)
        /// @param options       the base64 options to use, can be base64_default or
        /// base64_url, is base64_default by default.
        /// @return number of written bytes, will be equal to
        /// base64_length_from_binary(length, options)
        virtual size_t
        binary_to_base64(const char* input, size_t length, char* output,
            Base64Options options = base64_default) const noexcept
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
        /// UnicodeResult (should be at least base64_length_from_binary_with_lines(length,
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
            Base64Options options = base64_default) const noexcept
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

#ifdef UNICODE_INTERNAL_TESTS
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
        KUMO_FORCE_INLINE implementation(const char* name,
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

} // namespace turbo
