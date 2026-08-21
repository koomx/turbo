namespace turbo {
    namespace UNICODE_IMPLEMENTATION {
        namespace {
            namespace utf16 {

                template <Endian big_endian>
                KUMO_FORCE_INLINE size_t utf32_length_from_utf16(const char16_t* in,
                    size_t size) {
                    return count_code_points<big_endian>(in, size);
                }

            } // namespace utf16
        } // unnamed namespace
    } // namespace UNICODE_IMPLEMENTATION
} // namespace turbo
