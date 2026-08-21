#include <turbo/unicode/utf.h>
#include <turbo/unicode/api/find.h>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#ifdef __linux__
#include <sys/mman.h>
#include <unistd.h>
#endif

#include <tests/unicode/helpers/fixed_string.h>
#include <tests/unicode/helpers/test.h>

const uint64_t seed = 0x123456789ABCDEF0;

template <typename char_type, typename impl>
void random_char_search(impl& implementation) {
    // Random number generator
    std::random_device rd;
    std::mt19937 gen(rd());

    // Generate random size between 0 and 1024
    std::uniform_int_distribution<size_t> size_dist(0, 1024);
    size_t size = size_dist(gen);

    // Create vector of random characters
    std::vector<char_type> arr(size);
    std::uniform_int_distribution<int> char_dist(32, 126);

    for (size_t i = 0; i < size; ++i) {
        arr[i] = static_cast<char_type>(char_dist(gen));
    }

    // Pick a random character to search for
    char_type search_char = static_cast<char_type>(char_dist(gen));

    // Use std::find to search for the character
    auto result = std::find(arr.data(), arr.data() + size, search_char);

    // Nest use turbo::find to search for the character
    auto simd_result = implementation.find(arr.data(), arr.data() + size, search_char);
    // Check if the results are the same
    ASSERT_TRUE(simd_result == result);
    simd_result = turbo::find_token(arr.data(), arr.data() + size, search_char);
    // Check if the results are the same
    ASSERT_TRUE(simd_result == result);
}

TEST(random_char_search_char) {
    for (size_t i = 0; i < 1000; ++i) {
        random_char_search<char>(implementation);
    }
}
TEST(random_char_search_char16_t) {
    for (size_t i = 0; i < 1000; ++i) {
        random_char_search<char16_t>(implementation);
    }
}

// Helper: place a buffer at a specific alignment modulo 64.
static std::pair<std::vector<char>, char*> make_aligned_buf(size_t size,
    size_t align_mod) {
    std::vector<char> backing(size + 128, '\0');
    uintptr_t base = reinterpret_cast<uintptr_t>(backing.data());
    size_t current_mod = base % 64;
    size_t offset = (align_mod >= current_mod) ? (align_mod - current_mod)
                                               : (64 - current_mod + align_mod);
    char* ptr = backing.data() + offset;
    return { std::move(backing), ptr };
}

TEST(find_char_null_needle_all_alignments) {
    const char needle = '\0';
    const uint8_t payload[] = { 0x26, 0x26, 0x26, 0x26, 0x26, 0x26, 0x26, 0x26,
        0x26, 0x26, 0x26, 0x26, 0x26, 0x26, 0x01, 0x00 };
    const size_t len = sizeof(payload);

    const char* expected = std::find(reinterpret_cast<const char*>(payload),
        reinterpret_cast<const char*>(payload) + len, needle);
    size_t expected_offset = static_cast<size_t>(expected - reinterpret_cast<const char*>(payload));

    for (size_t align = 0; align < 64; ++align) {
        auto aligned_buf = make_aligned_buf(len, align);
        std::vector<char> backing = std::move(aligned_buf.first);
        char* buf = aligned_buf.second;
        std::memcpy(buf, payload, len);

        const char* result = implementation.find(buf, buf + len, needle);
        size_t got = static_cast<size_t>(result - buf);
        ASSERT_EQUAL(got, expected_offset);
    }
}

TEST(find_char_null_needle_various_sizes) {
    const char needle = '\0';
    for (size_t len = 1; len <= 256; ++len) {
        for (size_t align = 0; align < 64; align += 7) {
            auto aligned_buf = make_aligned_buf(len, align);
            std::vector<char> backing = std::move(aligned_buf.first);
            char* buf = aligned_buf.second;
            std::memset(buf, 'A', len);
            buf[len - 1] = '\0';

            const char* result = implementation.find(buf, buf + len, needle);
            size_t got = static_cast<size_t>(result - buf);
            ASSERT_EQUAL(got, len - 1);
        }
    }
}

#ifdef __linux__
static char* alloc_at_page_end(size_t size) {
    const size_t page = static_cast<size_t>(sysconf(_SC_PAGESIZE));
    const size_t pages_needed = (size + page - 1) / page + 1;
    const size_t total = pages_needed * page;
    char* base = static_cast<char*>(mmap(nullptr, total, PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
    if (base == MAP_FAILED) {
        return nullptr;
    }
    mmap(base + total - page, page, PROT_NONE,
        MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    return base + total - page - size;
}

static void free_at_page_end(char* buf, size_t size) {
    const size_t page = static_cast<size_t>(sysconf(_SC_PAGESIZE));
    const size_t pages_needed = (size + page - 1) / page + 1;
    const size_t total = pages_needed * page;
    char* base = buf + size + page - total;
    munmap(base, total);
}

TEST(find_char_guard_page_various_sizes) {
    for (size_t len = 1; len <= 256; ++len) {
        char* buf = alloc_at_page_end(len);
        ASSERT_TRUE(buf != nullptr);
        std::memset(buf, 'A', len);
        buf[len - 1] = 'Z';

        const char* expected = buf + len - 1;
        const char* result = implementation.find(buf, buf + len, 'Z');
        ASSERT_TRUE(result == expected);

        free_at_page_end(buf, len);
    }
}

TEST(find_char_guard_page_needle_absent) {
    for (size_t len = 1; len <= 256; ++len) {
        char* buf = alloc_at_page_end(len);
        ASSERT_TRUE(buf != nullptr);
        std::memset(buf, 'X', len);

        const char* result = implementation.find(buf, buf + len, 'Y');
        ASSERT_TRUE(result == buf + len);

        free_at_page_end(buf, len);
    }
}
#endif

TEST(find_char_all_same_character) {
    for (size_t len = 1; len <= 128; ++len) {
        const std::vector<char> input(len, '&');
        const char* start = input.data();
        const char* end = start + input.size();

        const char* result = implementation.find(start, end, '&');
        ASSERT_TRUE(result == start);
    }
}

TEST(find_char_needle_at_end) {
    for (size_t len = 1; len <= 128; ++len) {
        std::vector<char> input(len, 'A');
        input.back() = 'Z';
        const char* start = input.data();
        const char* end = start + input.size();

        const char* expected = end - 1;
        const char* result = implementation.find(start, end, 'Z');
        ASSERT_TRUE(result == expected);
    }
}

TEST(find_char_needle_absent) {
    for (size_t len = 0; len <= 128; ++len) {
        const std::vector<char> input(len, 'X');
        const char* start = input.data();
        const char* end = start + input.size();

        const char* result = implementation.find(start, end, 'Y');
        ASSERT_TRUE(result == end);
    }
}

static bool byte_in_set(char c, std::string_view symbols) {
    return symbols.find(c) != std::string_view::npos;
}

static const char* naive_first(const char* begin, const char* end,
    std::string_view symbols, bool positive) {
    for (const char* p = begin; p < end; ++p) {
        if (byte_in_set(*p, symbols) == positive) {
            return p;
        }
    }
    return end;
}

static const char* naive_last_or_null(const char* begin, const char* end,
    std::string_view symbols, bool positive) {
    for (const char* p = end; p > begin;) {
        --p;
        if (byte_in_set(*p, symbols) == positive) {
            return p;
        }
    }
    return nullptr;
}

static std::vector<char> make_haystack(size_t len, unsigned seed) {
    std::mt19937 gen(seed);
    std::uniform_int_distribution<int> dist(0, 255);
    std::vector<char> buf(len);
    for (size_t i = 0; i < len; ++i) {
        buf[i] = static_cast<char>(dist(gen));
    }
    return buf;
}

TEST(find_token_char_empty_and_positions) {
    {
        const char* empty = "";
        ASSERT_TRUE(turbo::find_token(empty, empty, 'a') == empty);
    }
    const char data[] = "abcXdefX";
    const char* begin = data;
    const char* end = data + sizeof(data) - 1;
    ASSERT_TRUE(turbo::find_token(begin, end, 'a') == begin);
    ASSERT_TRUE(turbo::find_token(begin, end, 'X') == begin + 3);
    ASSERT_TRUE(turbo::find_token(begin, end, 'f') == begin + 6);
    ASSERT_TRUE(turbo::find_token(begin, end, 'z') == end);
}

TEST(find_token_char16) {
    const char16_t data[] = { u'a', u'b', 0, u'X', u'c' };
    const char16_t* begin = data;
    const char16_t* end = data + 5;
    ASSERT_TRUE(turbo::find_token(begin, end, u'a') == begin);
    ASSERT_TRUE(turbo::find_token(begin, end, char16_t(0)) == begin + 2);
    ASSERT_TRUE(turbo::find_token(begin, end, u'X') == begin + 3);
    ASSERT_TRUE(turbo::find_token(begin, end, u'z') == end);
}

TEST(find_first_symbols_compile_time) {
    const char hay[] = "hello, world\t\n";
    const char* begin = hay;
    const char* end = hay + sizeof(hay) - 1;

    ASSERT_TRUE(turbo::find_first_symbols<','>(begin, end) == begin + 5);
    ASSERT_TRUE((turbo::find_first_symbols<' ', '\t'>(begin, end) == begin + 6));
    ASSERT_TRUE((turbo::find_first_symbols<'x', 'y', 'z'>(begin, end) == end));
    ASSERT_TRUE(turbo::find_first_symbols<'h'>(begin, end) == begin);
    ASSERT_TRUE(turbo::find_first_symbols<'\n'>(begin, end) == end - 1);

    char mutable_hay[] = "abc,def";
    char* mbegin = mutable_hay;
    char* mend = mutable_hay + 7;
    char* found = turbo::find_first_symbols<','>(mbegin, mend);
    ASSERT_TRUE(found == mbegin + 3);
    *found = ';';
    ASSERT_EQUAL(mutable_hay[3], ';');
}

TEST(find_first_not_symbols_compile_time) {
    const char hay[] = "   abc";
    const char* begin = hay;
    const char* end = hay + sizeof(hay) - 1;
    ASSERT_TRUE(turbo::find_first_not_symbols<' '>(begin, end) == begin + 3);
    ASSERT_TRUE((turbo::find_first_not_symbols<' ', 'a', 'b', 'c'>(begin, end) == end));
}

TEST(find_first_symbols_or_null) {
    const char hay[] = "no-match-here";
    const char* begin = hay;
    const char* end = hay + sizeof(hay) - 1;
    ASSERT_TRUE(turbo::find_first_symbols_or_null<'x'>(begin, end) == nullptr);
    ASSERT_TRUE(turbo::find_first_symbols_or_null<'m'>(begin, end) == begin + 3);
    ASSERT_TRUE((turbo::find_first_not_symbols_or_null<'n', 'o', '-', 'm', 'a', 't', 'c', 'h', 'e'>(
                    begin, end)
        == begin + 11));
}

TEST(find_last_symbols_or_null) {
    const char hay[] = "abXcdXef";
    const char* begin = hay;
    const char* end = hay + sizeof(hay) - 1;
    ASSERT_TRUE(turbo::find_last_symbols_or_null<'X'>(begin, end) == begin + 5);
    ASSERT_TRUE(turbo::find_last_symbols_or_null<'z'>(begin, end) == nullptr);
    ASSERT_TRUE(turbo::find_last_not_symbols_or_null<'f'>(begin, end) == begin + 6);
    ASSERT_TRUE((turbo::find_last_not_symbols_or_null<'a', 'b', 'X', 'c', 'd', 'e', 'f'>(
                    begin, end)
        == nullptr));
}

TEST(find_symbols_searchsymbols_runtime) {
    const std::string hay = "tab\tseparated,values\n";
    const turbo::SearchSymbols delim("\t,\n");
    const char* begin = hay.data();
    const char* end = hay.data() + hay.size();

    ASSERT_TRUE(turbo::find_first_symbols(hay, delim) == begin + 3);
    ASSERT_TRUE(turbo::find_first_symbols_or_null(hay, delim) == begin + 3);
    ASSERT_TRUE(turbo::find_last_symbols_or_null(hay, delim) == begin + 20);
    ASSERT_TRUE(turbo::find_first_not_symbols(hay, turbo::SearchSymbols("tab"))
        == begin + 3);

    const turbo::SearchSymbols empty;
    ASSERT_TRUE(turbo::find_first_symbols(hay, empty) == end);
    ASSERT_TRUE(turbo::find_first_symbols_or_null(hay, empty) == nullptr);
    ASSERT_TRUE(turbo::find_first_not_symbols(hay, empty) == begin);
    ASSERT_TRUE(turbo::find_last_not_symbols_or_null(hay, empty) == end - 1);
    ASSERT_TRUE(turbo::find_last_symbols_or_null(hay, empty) == nullptr);

    std::string_view empty_hay;
    ASSERT_TRUE(turbo::find_first_symbols(empty_hay, empty) == empty_hay.data());
    ASSERT_TRUE(turbo::find_first_not_symbols(empty_hay, empty) == empty_hay.data());
}

TEST(search_symbols_rejects_too_many) {
    bool threw = false;
    try {
        turbo::SearchSymbols too_many("0123456789abcdefg");
    } catch (const std::runtime_error&) {
        threw = true;
    }
    ASSERT_TRUE(threw);
}

TEST(count_symbols_and_split) {
    const char hay[] = "a,b,,c,d";
    const char* begin = hay;
    const char* end = hay + sizeof(hay) - 1;
    ASSERT_EQUAL(turbo::count_symbols<','>(begin, end), 4u);
    ASSERT_EQUAL(turbo::count_symbols<'x'>(begin, end), 0u);
    ASSERT_EQUAL((turbo::count_symbols<',', 'a'>(begin, end)), 5u);

    std::vector<std::string> parts;
    turbo::splitInto<','>(parts, hay);
    ASSERT_EQUAL(parts.size(), 5u);
    ASSERT_TRUE(parts[0] == "a");
    ASSERT_TRUE(parts[1] == "b");
    ASSERT_TRUE(parts[2] == "");
    ASSERT_TRUE(parts[3] == "c");
    ASSERT_TRUE(parts[4] == "d");

    parts.clear();
    turbo::splitInto<','>(parts, hay, true);
    ASSERT_EQUAL(parts.size(), 4u);
    ASSERT_TRUE(parts[0] == "a");
    ASSERT_TRUE(parts[1] == "b");
    ASSERT_TRUE(parts[2] == "c");
    ASSERT_TRUE(parts[3] == "d");
}

TEST(find_symbols_matches_naive_short_and_long) {
    const size_t lengths[] = { 0, 1, 7, 8, 15, 16, 17, 31, 32, 33, 64, 65, 127, 128 };
    const std::string_view needles[] = { ",", " \t", ",\t\n; ", "abcde", "0123456789abcdef" };

    for (size_t len : lengths) {
        for (unsigned seed = 0; seed < 8; ++seed) {
            const auto hay = make_haystack(len, 0x9E3779B9u + seed * 17u + static_cast<unsigned>(len));
            const char* begin = hay.data();
            const char* end = begin + hay.size();
            for (std::string_view needle : needles) {
                const turbo::SearchSymbols symbols { std::string(needle) };
                const char* first = turbo::find_first_symbols(std::string_view(begin, hay.size()), symbols);
                ASSERT_TRUE(first == naive_first(begin, end, needle, true));
                const char* first_null = turbo::find_first_symbols_or_null(
                    std::string_view(begin, hay.size()), symbols);
                const char* naive_f = naive_first(begin, end, needle, true);
                ASSERT_TRUE(first_null == (naive_f == end ? nullptr : naive_f));

                const char* first_not = turbo::find_first_not_symbols(
                    std::string_view(begin, hay.size()), symbols);
                ASSERT_TRUE(first_not == naive_first(begin, end, needle, false));

                const char* last = turbo::find_last_symbols_or_null(
                    std::string_view(begin, hay.size()), symbols);
                ASSERT_TRUE(last == naive_last_or_null(begin, end, needle, true));
                const char* last_not = turbo::find_last_not_symbols_or_null(
                    std::string_view(begin, hay.size()), symbols);
                ASSERT_TRUE(last_not == naive_last_or_null(begin, end, needle, false));
            }
        }
    }
}

TEST(find_first_symbols_embedded_nul) {
    char hay[20];
    std::memset(hay, 'A', sizeof(hay));
    hay[3] = '\0';
    hay[11] = ',';
    const char* begin = hay;
    const char* end = hay + sizeof(hay);
    ASSERT_TRUE(turbo::find_first_symbols<'\0'>(begin, end) == begin + 3);
    ASSERT_TRUE(turbo::find_first_symbols<','>(begin, end) == begin + 11);
    ASSERT_TRUE((turbo::find_first_symbols<'\0', ','>(begin, end) == begin + 3));
}

TEST(find_symbols_sse42_many_needles) {
    std::string hay(40, 'z');
    hay[19] = '5';
    hay[35] = 'e';
    const char* begin = hay.data();
    const char* end = begin + hay.size();
    ASSERT_TRUE((turbo::find_first_symbols<'0', '1', '2', '3', '4', '5'>(begin, end)) == begin + 19);
    ASSERT_TRUE((turbo::find_first_symbols<'a', 'b', 'c', 'd', 'e', 'f'>(begin, end)) == begin + 35);
    ASSERT_TRUE((turbo::find_last_symbols_or_null<'0', '1', '2', '3', '4', '5'>(begin, end))
        == begin + 19);
}

#ifdef __linux__
TEST(find_first_symbols_guard_page) {
    for (size_t len = 1; len <= 64; ++len) {
        char* buf = alloc_at_page_end(len);
        ASSERT_TRUE(buf != nullptr);
        std::memset(buf, 'A', len);
        buf[len - 1] = ',';
        ASSERT_TRUE(turbo::find_first_symbols<','>(buf, buf + len) == buf + len - 1);
        ASSERT_TRUE(turbo::find_first_symbols<'X'>(buf, buf + len) == buf + len);
        ASSERT_TRUE(turbo::find_last_symbols_or_null<','>(buf, buf + len) == buf + len - 1);
        free_at_page_end(buf, len);
    }
}
#endif

TEST_MAIN
