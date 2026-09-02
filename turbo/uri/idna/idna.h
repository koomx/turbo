#pragma once

#include <string>
#include <string_view>

namespace turbo::idna {

    // Map the characters according to IDNA, returning the empty string on error.
    std::u32string map(std::u32string_view input);

    // Normalize the characters according to IDNA (Unicode Normalization Form C).
    void normalize(std::u32string& input);

    bool punycode_to_utf32(std::string_view input, std::u32string& out);
    bool verify_punycode(std::string_view input);
    bool utf32_to_punycode(std::u32string_view input, std::string& out);

    /**
     * @see https://www.unicode.org/reports/tr46/#Validity_Criteria
     */
    bool is_label_valid(std::u32string_view label);

    // Converts a domain (e.g., www.google.com) possibly containing international
    // characters to an ascii domain (with punycode). It will not do percent
    // decoding: percent decoding should be done prior to calling this function. We
    // do not remove tabs and spaces, they should have been removed prior to calling
    // this function. We also do not trim control characters. We also assume that
    // the input is not empty. We return "" on error.
    //
    //
    // This function may accept or even produce invalid domains.
    std::string to_ascii(std::string_view ut8_string);

    // Returns true if the string contains a forbidden code point according to the
    // WHATGL URL specification:
    // https://url.spec.whatwg.org/#forbidden-domain-code-point
    bool contains_forbidden_domain_code_point(std::string_view ascii_string);

    std::string to_unicode(std::string_view input);

} // namespace turbo::idna
