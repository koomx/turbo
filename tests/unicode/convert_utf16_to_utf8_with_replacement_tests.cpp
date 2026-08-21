#include <turbo/unicode/utf.h>

#include <cstring>
#include <random>
#include <vector>

#include <tests/unicode/helpers/random_utf16.h>
#include <tests/unicode/helpers/test.h>
#include <tests/unicode/helpers/utf16.h>

// U+FFFD in UTF-8 is 0xEF 0xBF 0xBD
constexpr char fffd_utf8[] = { char(0xef), char(0xbf), char(0xbd) };

// Test: valid UTF-16 should produce the same output as convert_utf16_to_utf8
TEST(valid_utf16le_roundtrip) {
    // ASCII + BMP characters
    std::vector<char16_t> input = { u'H', u'e', u'l', u'l', u'o', u' ',
        u'\u00e9', // e-acute (2-byte UTF-8)
        u'\u4e16', // CJK character (3-byte UTF-8)
        u'\u754c' }; // CJK character (3-byte UTF-8)
    size_t expected_len = turbo::utf8_length_from_utf16le(input.data(), input.size());
    std::vector<char> expected(expected_len);
    size_t written_expected = implementation.convert_utf16le_to_utf8(
        input.data(), input.size(), expected.data());

    std::vector<char> actual(expected_len + 10);
    size_t written_actual = implementation.convert_utf16le_to_utf8_with_replacement(
        input.data(), input.size(), actual.data());

    ASSERT_EQUAL(written_expected, written_actual);
    ASSERT_TRUE(std::memcmp(expected.data(), actual.data(), written_actual) == 0);
}

// Test: valid surrogate pairs should produce correct 4-byte UTF-8
TEST(valid_surrogate_pair_le) {
    // U+1F600 (grinning face) = D83D DE00
    std::vector<char16_t> input = { to_utf16le(char16_t(0xD83D)),
        to_utf16le(char16_t(0xDE00)) };
    std::vector<char> output(8);
    size_t written = implementation.convert_utf16le_to_utf8_with_replacement(
        input.data(), input.size(), output.data());
    ASSERT_EQUAL(written, size_t(4));
    // U+1F600 in UTF-8: F0 9F 98 80
    ASSERT_EQUAL(uint8_t(output[0]), uint8_t(0xF0));
    ASSERT_EQUAL(uint8_t(output[1]), uint8_t(0x9F));
    ASSERT_EQUAL(uint8_t(output[2]), uint8_t(0x98));
    ASSERT_EQUAL(uint8_t(output[3]), uint8_t(0x80));
}

// Test: unpaired high surrogate should be replaced with U+FFFD
TEST(unpaired_high_surrogate_le) {
    // D800 followed by a regular character
    std::vector<char16_t> input = { to_utf16le(char16_t(0xD800)),
        to_utf16le(u'A') };
    std::vector<char> output(8);
    size_t written = implementation.convert_utf16le_to_utf8_with_replacement(
        input.data(), input.size(), output.data());
    // U+FFFD (3 bytes) + 'A' (1 byte) = 4
    ASSERT_EQUAL(written, size_t(4));
    ASSERT_TRUE(std::memcmp(output.data(), fffd_utf8, 3) == 0);
    ASSERT_EQUAL(output[3], 'A');
}

// Test: unpaired low surrogate should be replaced with U+FFFD
TEST(unpaired_low_surrogate_le) {
    // DC00 (low surrogate without preceding high surrogate)
    std::vector<char16_t> input = { to_utf16le(u'B'), to_utf16le(char16_t(0xDC00)),
        to_utf16le(u'C') };
    std::vector<char> output(16);
    size_t written = implementation.convert_utf16le_to_utf8_with_replacement(
        input.data(), input.size(), output.data());
    // 'B' (1) + U+FFFD (3) + 'C' (1) = 5
    ASSERT_EQUAL(written, size_t(5));
    ASSERT_EQUAL(output[0], 'B');
    ASSERT_TRUE(std::memcmp(output.data() + 1, fffd_utf8, 3) == 0);
    ASSERT_EQUAL(output[4], 'C');
}

// Test: high surrogate at end of string
TEST(high_surrogate_at_end_le) {
    std::vector<char16_t> input = { to_utf16le(u'X'),
        to_utf16le(char16_t(0xD800)) };
    std::vector<char> output(8);
    size_t written = implementation.convert_utf16le_to_utf8_with_replacement(
        input.data(), input.size(), output.data());
    // 'X' (1) + U+FFFD (3) = 4
    ASSERT_EQUAL(written, size_t(4));
    ASSERT_EQUAL(output[0], 'X');
    ASSERT_TRUE(std::memcmp(output.data() + 1, fffd_utf8, 3) == 0);
}

// Test: two consecutive unpaired high surrogates
TEST(consecutive_unpaired_high_surrogates_le) {
    std::vector<char16_t> input = { to_utf16le(char16_t(0xD800)),
        to_utf16le(char16_t(0xD801)) };
    std::vector<char> output(16);
    size_t written = implementation.convert_utf16le_to_utf8_with_replacement(
        input.data(), input.size(), output.data());
    // Two U+FFFD = 6 bytes
    ASSERT_EQUAL(written, size_t(6));
    ASSERT_TRUE(std::memcmp(output.data(), fffd_utf8, 3) == 0);
    ASSERT_TRUE(std::memcmp(output.data() + 3, fffd_utf8, 3) == 0);
}

// Test: low surrogate followed by high surrogate (reversed pair)
TEST(reversed_surrogate_pair_le) {
    std::vector<char16_t> input = { to_utf16le(char16_t(0xDC00)),
        to_utf16le(char16_t(0xD800)) };
    std::vector<char> output(16);
    size_t written = implementation.convert_utf16le_to_utf8_with_replacement(
        input.data(), input.size(), output.data());
    // Both unpaired: two U+FFFD = 6 bytes
    ASSERT_EQUAL(written, size_t(6));
    ASSERT_TRUE(std::memcmp(output.data(), fffd_utf8, 3) == 0);
    ASSERT_TRUE(std::memcmp(output.data() + 3, fffd_utf8, 3) == 0);
}

// Test: empty input
TEST(empty_input_le) {
    std::vector<char> output(8);
    size_t written = implementation.convert_utf16le_to_utf8_with_replacement(
        nullptr, 0, output.data());
    ASSERT_EQUAL(written, size_t(0));
}

// Test: pure ASCII
TEST(pure_ascii_le) {
    std::vector<char16_t> input = { to_utf16le(u'H'), to_utf16le(u'e'),
        to_utf16le(u'l'), to_utf16le(u'l'),
        to_utf16le(u'o') };
    std::vector<char> output(8);
    size_t written = implementation.convert_utf16le_to_utf8_with_replacement(
        input.data(), input.size(), output.data());
    ASSERT_EQUAL(written, size_t(5));
    ASSERT_TRUE(std::memcmp(output.data(), "Hello", 5) == 0);
}

TEST(valid_utf16be_roundtrip) {
    std::vector<char16_t> input = {
        to_utf16be(u'H'), to_utf16be(u'e'), to_utf16be(u'l'),
        to_utf16be(u'l'), to_utf16be(u'o'), to_utf16be(u' '),
        to_utf16be(u'\u00e9'), to_utf16be(u'\u4e16'), to_utf16be(u'\u754c')
    };
    size_t expected_len = turbo::utf8_length_from_utf16be(input.data(), input.size());
    std::vector<char> expected(expected_len);
    size_t written_expected = implementation.convert_utf16be_to_utf8(
        input.data(), input.size(), expected.data());

    std::vector<char> actual(expected_len + 10);
    size_t written_actual = implementation.convert_utf16be_to_utf8_with_replacement(
        input.data(), input.size(), actual.data());

    ASSERT_EQUAL(written_expected, written_actual);
    ASSERT_TRUE(std::memcmp(expected.data(), actual.data(), written_actual) == 0);
}

TEST(valid_surrogate_pair_be) {
    std::vector<char16_t> input = { to_utf16be(char16_t(0xD83D)),
        to_utf16be(char16_t(0xDE00)) };
    std::vector<char> output(8);
    size_t written = implementation.convert_utf16be_to_utf8_with_replacement(
        input.data(), input.size(), output.data());
    ASSERT_EQUAL(written, size_t(4));
    ASSERT_EQUAL(uint8_t(output[0]), uint8_t(0xF0));
    ASSERT_EQUAL(uint8_t(output[1]), uint8_t(0x9F));
    ASSERT_EQUAL(uint8_t(output[2]), uint8_t(0x98));
    ASSERT_EQUAL(uint8_t(output[3]), uint8_t(0x80));
}

TEST(unpaired_high_surrogate_be) {
    std::vector<char16_t> input = { to_utf16be(char16_t(0xD800)),
        to_utf16be(u'A') };
    std::vector<char> output(8);
    size_t written = implementation.convert_utf16be_to_utf8_with_replacement(
        input.data(), input.size(), output.data());
    ASSERT_EQUAL(written, size_t(4));
    ASSERT_TRUE(std::memcmp(output.data(), fffd_utf8, 3) == 0);
    ASSERT_EQUAL(output[3], 'A');
}

TEST(unpaired_low_surrogate_be) {
    std::vector<char16_t> input = { to_utf16be(u'B'), to_utf16be(char16_t(0xDC00)),
        to_utf16be(u'C') };
    std::vector<char> output(16);
    size_t written = implementation.convert_utf16be_to_utf8_with_replacement(
        input.data(), input.size(), output.data());
    ASSERT_EQUAL(written, size_t(5));
    ASSERT_EQUAL(output[0], 'B');
    ASSERT_TRUE(std::memcmp(output.data() + 1, fffd_utf8, 3) == 0);
    ASSERT_EQUAL(output[4], 'C');
}

TEST(reversed_surrogate_pair_be) {
    std::vector<char16_t> input = { to_utf16be(char16_t(0xDC00)),
        to_utf16be(char16_t(0xD800)) };
    std::vector<char> output(16);
    size_t written = implementation.convert_utf16be_to_utf8_with_replacement(
        input.data(), input.size(), output.data());
    ASSERT_EQUAL(written, size_t(6));
    ASSERT_TRUE(std::memcmp(output.data(), fffd_utf8, 3) == 0);
    ASSERT_TRUE(std::memcmp(output.data() + 3, fffd_utf8, 3) == 0);
}

TEST(length_consistency_be) {
    std::vector<char16_t> input = { to_utf16be(u'A'),
        to_utf16be(char16_t(0xD83D)),
        to_utf16be(char16_t(0xDE00)),
        to_utf16be(char16_t(0xD800)),
        to_utf16be(u'B'),
        to_utf16be(char16_t(0xDC00)),
        to_utf16be(char16_t(0xDC00)),
        to_utf16be(char16_t(0xD800)),
        to_utf16be(u'\u4e16') };

    turbo::UnicodeResult len_result = turbo::utf8_length_from_utf16be_with_replacement(input.data(),
        input.size());

    std::vector<char> output(input.size() * 3 + 4);
    size_t written = implementation.convert_utf16be_to_utf8_with_replacement(
        input.data(), input.size(), output.data());

    ASSERT_EQUAL(len_result.count, written);
}

TEST(matches_well_formed_approach_be) {
    std::vector<char16_t> input = { to_utf16be(u'A'),
        to_utf16be(char16_t(0xD800)),
        to_utf16be(u'B'),
        to_utf16be(char16_t(0xDC00)),
        to_utf16be(char16_t(0xDC00)),
        to_utf16be(char16_t(0xD800)),
        to_utf16be(char16_t(0xD83D)),
        to_utf16be(char16_t(0xDE00)) };

    std::vector<char16_t> well_formed(input.size());
    turbo::to_well_formed_utf16be(input.data(), input.size(),
        well_formed.data());
    size_t utf8_len = turbo::utf8_length_from_utf16be(well_formed.data(), well_formed.size());
    std::vector<char> expected(utf8_len + 1);
    size_t written_expected = turbo::convert_utf16be_to_utf8(
        well_formed.data(), well_formed.size(), expected.data());

    std::vector<char> actual(utf8_len + 1);
    size_t written_actual = implementation.convert_utf16be_to_utf8_with_replacement(
        input.data(), input.size(), actual.data());

    ASSERT_EQUAL(written_expected, written_actual);
    ASSERT_TRUE(std::memcmp(expected.data(), actual.data(), written_actual) == 0);
}

TEST(output_is_valid_utf8_be) {
    std::vector<char16_t> input = { to_utf16be(u'A'),
        to_utf16be(char16_t(0xD83D)),
        to_utf16be(char16_t(0xDE00)),
        to_utf16be(char16_t(0xD800)),
        to_utf16be(u'B'),
        to_utf16be(char16_t(0xDC00)),
        to_utf16be(u'\u4e16') };

    std::vector<char> output(input.size() * 3 + 4);
    size_t written = implementation.convert_utf16be_to_utf8_with_replacement(
        input.data(), input.size(), output.data());

    ASSERT_TRUE(turbo::validate_utf8(output.data(), written));
}

// Test: consistency with utf8_length_from_utf16_with_replacement
TEST_LOOP(length_consistency_le) {
    const size_t length = 64 + (seed % 128);
    std::mt19937 gen(seed);
    std::uniform_int_distribution<uint16_t> dist(0, 0xFFFF);

    std::vector<char16_t> input(length);
    for (size_t i = 0; i < length; i++) {
        input[i] = static_cast<char16_t>(dist(gen));
    }

    turbo::UnicodeResult len_result = turbo::utf8_length_from_utf16le_with_replacement(input.data(),
        input.size());

    std::vector<char> output(length * 3 + 1);
    size_t written = implementation.convert_utf16le_to_utf8_with_replacement(
        input.data(), input.size(), output.data());

    ASSERT_EQUAL(len_result.count, written);
}

// Test: output matches to_well_formed_utf16 + convert_utf16_to_utf8 approach
TEST_LOOP(matches_well_formed_approach_le) {
    const size_t length = 32 + (seed % 64);
    std::mt19937 gen(seed);
    std::uniform_int_distribution<uint16_t> dist(0, 0xFFFF);

    std::vector<char16_t> input(length);
    for (size_t i = 0; i < length; i++) {
        input[i] = static_cast<char16_t>(dist(gen));
    }

    // Approach 1: to_well_formed_utf16 + convert_utf16_to_utf8
    std::vector<char16_t> well_formed(length);
    turbo::to_well_formed_utf16le(input.data(), length, well_formed.data());
    size_t utf8_len = turbo::utf8_length_from_utf16le(well_formed.data(), length);
    std::vector<char> expected(utf8_len + 1);
    size_t written_expected = turbo::convert_utf16le_to_utf8(
        well_formed.data(), length, expected.data());

    // Approach 2: convert_utf16_to_utf8_with_replacement (new function)
    std::vector<char> actual(utf8_len + 1);
    size_t written_actual = implementation.convert_utf16le_to_utf8_with_replacement(
        input.data(), length, actual.data());

    ASSERT_EQUAL(written_expected, written_actual);
    ASSERT_TRUE(std::memcmp(expected.data(), actual.data(), written_actual) == 0);
}

// Test: output is always valid UTF-8
TEST_LOOP(output_is_valid_utf8_le) {
    const size_t length = 64 + (seed % 256);
    std::mt19937 gen(seed);
    std::uniform_int_distribution<uint16_t> dist(0, 0xFFFF);

    std::vector<char16_t> input(length);
    for (size_t i = 0; i < length; i++) {
        input[i] = static_cast<char16_t>(dist(gen));
    }

    std::vector<char> output(length * 3 + 1);
    size_t written = implementation.convert_utf16le_to_utf8_with_replacement(
        input.data(), input.size(), output.data());

    ASSERT_TRUE(turbo::validate_utf8(output.data(), written));
}

// Stress the SIMD block boundaries: slide an unpaired high surrogate, an
// unpaired low surrogate, and a valid surrogate pair through every offset of a
// multi-block buffer, comparing the with_replacement output byte-for-byte
// against the independent to_well_formed + convert oracle. This is the case a
// naive SIMD implementation is most likely to get wrong (a valid pair or a lone
// surrogate landing on a 16-unit lane boundary).
TEST(surrogate_sweep_across_boundaries_le) {
    const size_t length = 200; // spans many 16-unit SIMD lanes
    for (size_t p = 0; p + 1 < length; p++) {
        for (int mode = 0; mode < 3; mode++) {
            std::vector<char16_t> input(length);
            for (size_t i = 0; i < length; i++) {
                // valid, non-surrogate BMP content (mix of 1-, 2- and 3-byte forms)
                input[i] = to_utf16le(char16_t(0x0041 + (i % 0x0500)));
            }
            if (mode == 0) {
                input[p] = to_utf16le(char16_t(0xD800)); // unpaired high
            } else if (mode == 1) {
                input[p] = to_utf16le(char16_t(0xDC00)); // unpaired low
            } else {
                input[p] = to_utf16le(char16_t(0xD800)); // valid pair straddling p/p+1
                input[p + 1] = to_utf16le(char16_t(0xDC00));
            }

            // Oracle: replace unpaired surrogates, then convert as valid UTF-16.
            std::vector<char16_t> well_formed(length);
            turbo::to_well_formed_utf16le(input.data(), length, well_formed.data());
            const size_t utf8_len = turbo::utf8_length_from_utf16le(well_formed.data(), length);
            std::vector<char> expected(utf8_len + 1);
            const size_t nexp = turbo::convert_utf16le_to_utf8(
                well_formed.data(), length, expected.data());

            std::vector<char> actual(utf8_len + 1);
            const size_t nact = implementation.convert_utf16le_to_utf8_with_replacement(
                input.data(), length, actual.data());

            ASSERT_EQUAL(nexp, nact);
            ASSERT_TRUE(std::memcmp(expected.data(), actual.data(), nact) == 0);
        }
    }
}

TEST_MAIN
