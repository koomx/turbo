namespace turbo {
    namespace UNICODE_IMPLEMENTATION {
        namespace {
            namespace utf16 {

                template <Endian big_endian>
                KUMO_FORCE_INLINE size_t utf8_length_from_utf16(const char16_t* in,
                    size_t size) {
                    size_t pos = 0;
                    size_t count = 0;
                    // This algorithm could no doubt be improved!
                    for (; pos < size / 32 * 32; pos += 32) {
                        simd16x32<uint16_t> input(reinterpret_cast<const uint16_t*>(in + pos));
                        if constexpr (!match_system(big_endian)) {
                            input.swap_bytes();
                        }
                        uint64_t ascii_mask = input.lteq(0x7F);
                        uint64_t twobyte_mask = input.lteq(0x7FF);
                        uint64_t not_pair_mask = input.not_in_range(0xD800, 0xDFFF);

                        size_t ascii_count = popcount(ascii_mask) / 2;
                        size_t twobyte_count = popcount(twobyte_mask & ~ascii_mask) / 2;
                        size_t threebyte_count = popcount(not_pair_mask & ~twobyte_mask) / 2;
                        size_t fourbyte_count = 32 - popcount(not_pair_mask) / 2;
                        count += 2 * fourbyte_count + 3 * threebyte_count + 2 * twobyte_count + ascii_count;
                    }
                    return count + scalar::utf16::utf8_length_from_utf16<big_endian>(in + pos, size - pos);
                }

            } // namespace utf16
        } // unnamed namespace
    } // namespace UNICODE_IMPLEMENTATION
} // namespace turbo
