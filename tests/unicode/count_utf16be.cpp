#include <turbo/unicode/utf.h>

#include <array>
#include <vector>

#include <tests/unicode/helpers/fixed_string.h>
#include <tests/unicode/helpers/random_int.h>
#include <tests/unicode/helpers/random_utf16.h>
#include <tests/unicode/helpers/test.h>

namespace {
    std::array<size_t, 7> input_size { 7, 16, 12, 64, 67, 128, 256 };

} // namespace

TEST_LOOP(count_just_one_word) {
    turbo::tests::helpers::random_utf16 random(seed, 1, 0);

    for (size_t size : input_size) {
        const auto g = random.generate_counted_be(size);
        const auto& utf16 = g.first;
        const auto utf16_count = g.second;

        const size_t count = implementation.count_utf16be(utf16.data(), size);
        ASSERT_EQUAL(count, utf16_count);
    }
}

TEST_LOOP(count_1_or_2_UTF16_words) {
    turbo::tests::helpers::random_utf16 random(seed, 1, 1);

    for (size_t size : input_size) {
        const auto g = random.generate_counted_be(size);
        const auto& utf16 = g.first;
        const auto utf16_count = g.second;

        const size_t count = implementation.count_utf16be(utf16.data(), size);
        ASSERT_EQUAL(count, utf16_count);
    }
}

TEST_LOOP(count_2_UTF16_words) {
    turbo::tests::helpers::random_utf16 random(seed, 0, 1);

    for (size_t size : input_size) {
        const auto g = random.generate_counted_be(size);
        const auto& utf16 = g.first;
        const auto utf16_count = g.second;

        const size_t count = implementation.count_utf16be(utf16.data(), size);
        ASSERT_EQUAL(count, utf16_count);
    }
}

TEST_MAIN
