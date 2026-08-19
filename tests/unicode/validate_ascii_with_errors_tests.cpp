#include <turbo/unicode/utf.h>

#include <tests/unicode/helpers/fixed_string.h>
#include <tests/unicode/helpers/random_utf8.h>
#include <tests/unicode/helpers/test.h>

TEST_LOOP(no_error_ASCII) {
    turbo::tests::helpers::random_utf8 generator { seed, 1, 0, 0, 0 };
    const auto ascii { generator.generate(512) };

    turbo::UnicodeResult res = implementation.validate_ascii_with_errors(
        reinterpret_cast<const char*>(ascii.data()), ascii.size());

    ASSERT_EQUAL(res.error, turbo::UnicodeError::SUCCESS);
    ASSERT_EQUAL(res.count, ascii.size());
}

TEST_LOOP(error_ASCII) {
    turbo::tests::helpers::random_utf8 generator { seed, 1, 0, 0, 0 };

    auto ascii { generator.generate(512) };

    for (unsigned int i = 0; i < ascii.size(); i++) {
        ascii[i] += 0b10000000;

        turbo::UnicodeResult res = implementation.validate_ascii_with_errors(
            reinterpret_cast<const char*>(ascii.data()), ascii.size());

        ASSERT_EQUAL(res.error, turbo::UnicodeError::TOO_LARGE);
        ASSERT_EQUAL(res.count, i);

        ascii[i] -= 0b10000000;
    }
}

TEST_MAIN
