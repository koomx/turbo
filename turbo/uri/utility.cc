// Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <turbo/uri/utility.h>
#include <turbo/uri/character_sets.h>
#include <turbo/uri/idna/idna.h>

namespace turbo {

    void parse_prepared_path(std::string_view input,
                                           turbo::SchemaType type,
                                           std::string& path) {
  uint8_t accumulator = turbo::path_signature(input);
  // Let us first detect a trivial case.
  // If it is special, we check that we have no dot, no %,  no \ and no
  // character needing percent encoding. Otherwise, we check that we have no %,
  // no dot, and no character needing percent encoding.
  constexpr uint8_t need_encoding = 1;
  constexpr uint8_t backslash_char = 2;
  constexpr uint8_t dot_char = 4;
  constexpr uint8_t percent_char = 8;
  bool special = type != turbo::SchemaType::NOT_SPECIAL;
  bool may_need_slow_file_handling = (type == turbo::SchemaType::FILE &&
                                      turbo::is_windows_drive_letter(input));
  bool trivial_path =
      (special ? (accumulator == 0)
               : ((accumulator & (need_encoding | dot_char | percent_char)) ==
                  0)) &&
      (!may_need_slow_file_handling);
  if (accumulator == dot_char && !may_need_slow_file_handling) {
    // '4' means that we have at least one dot, but nothing that requires
    // percent encoding or decoding. The only part that is not trivial is
    // that we may have single dots and double dots path segments.
    // If we have such segments, then we either have a path that begins
    // with '.' (easy to check), or we have the sequence './'.
    // Note: input cannot be empty, it must at least contain one character ('.')
    // Note: we know that '\' is not present.
    if (input[0] != '.') {
      size_t slashdot = input.find("/.");
      if (slashdot == std::string_view::npos) {  // common case
        trivial_path = true;
      } else {  // uncommon
        // only three cases matter: /./, /.. or a final /
        trivial_path =
            !(slashdot + 2 == input.size() || input[slashdot + 2] == '.' ||
              input[slashdot + 2] == '/');
      }
    }
  }
  if (trivial_path) {
    path += '/';
    path += input;
    return;
  }
  // We are going to need to look a bit at the path, but let us see if we can
  // ignore percent encoding *and* backslashes *and* percent characters.
  // Except for the trivial case, this is likely to capture 99% of paths out
  // there.
  bool fast_path =
      (special &&
       (accumulator & (need_encoding | backslash_char | percent_char)) == 0) &&
      (type != turbo::SchemaType::FILE);
  if (fast_path) {
    // Here we don't need to worry about \ or percent encoding.
    // We also do not have a file protocol. We might have dots, however,
    // but dots must as appear as '.', and they cannot be encoded because
    // the symbol '%' is not present.
    size_t previous_location = 0;  // We start at 0.
    do {
      size_t new_location = input.find('/', previous_location);
      // std::string_view path_view = input;
      //  We process the last segment separately:
      if (new_location == std::string_view::npos) {
        std::string_view path_view = input.substr(previous_location);
        if (path_view == "..") {  // The path ends with ..
          // e.g., if you receive ".." with an empty path, you go to "/".
          if (path.empty()) {
            path = '/';
            return;
          }
          // Fast case where we have nothing to do:
          if (path.back() == '/') {
            return;
          }
          // If you have the path "/joe/myfriend",
          // then you delete 'myfriend'.
          path.resize(path.rfind('/') + 1);
          return;
        }
        path += '/';
        if (path_view != ".") {
          path.append(path_view);
        }
        return;
      } else {
        // This is a non-final segment.
        std::string_view path_view =
            input.substr(previous_location, new_location - previous_location);
        previous_location = new_location + 1;
        if (path_view == "..") {
          size_t last_delimiter = path.rfind('/');
          if (last_delimiter != std::string::npos) {
            path.erase(last_delimiter);
          }
        } else if (path_view != ".") {
          path += '/';
          path.append(path_view);
        }
      }
    } while (true);
  } else {
    // we have reached the general case
    bool needs_percent_encoding = (accumulator & 1);
    std::string path_buffer_tmp;
    do {
      size_t location = (special && (accumulator & 2))
                            ? input.find_first_of("/\\")
                            : input.find('/');
      std::string_view path_view = input;
      if (location != std::string_view::npos) {
        path_view.remove_suffix(path_view.size() - location);
        input.remove_prefix(location + 1);
      }
      // path_buffer is either path_view or it might point at a percent encoded
      // temporary file.
      std::string_view path_buffer =
          (needs_percent_encoding &&
           percent_encode<false>(
               path_view, turbo::uri_charsets::PATH_PERCENT_ENCODE, path_buffer_tmp))
              ? path_buffer_tmp
              : path_view;
      if (is_double_dot_path_segment(path_buffer)) {
        if ((turbo::shorten_path(path, type) || special) &&
            location == std::string_view::npos) {
          path += '/';
        }
      } else if (is_single_dot_path_segment(path_buffer) &&
                 (location == std::string_view::npos)) {
        path += '/';
      }
      // Otherwise, if path_buffer is not a single-dot path segment, then:
      else if (!is_single_dot_path_segment(path_buffer)) {
        // If url's scheme is "file", url's path is empty, and path_buffer is a
        // Windows drive letter, then replace the second code point in
        // path_buffer with U+003A (:).
        if (type == turbo::SchemaType::FILE && path.empty() &&
            turbo::is_windows_drive_letter(path_buffer)) {
          path += '/';
          path += path_buffer[0];
          path += ':';
          path_buffer.remove_prefix(2);
          path.append(path_buffer);
        } else {
          // Append path_buffer to url's path.
          path += '/';
          path.append(path_buffer);
        }
      }
      if (location == std::string_view::npos) {
        return;
      }
    } while (true);
  }
}



    std::string percent_encode(const std::string_view input,
                               const uint8_t character_set[]) {
        auto pointer =
            std::find_if(input.begin(), input.end(), [character_set](const char c) {
              return turbo::uri_charsets::bit_at(character_set, c);
            });
        // Optimization: Don't iterate if percent encode is not required
        if (pointer == input.end()) {
            return std::string(input);
        }

        std::string result;
        result.reserve(input.length());  // in the worst case, percent encoding might
        // produce 3 characters.
        result.append(input.substr(0, std::distance(input.begin(), pointer)));

        for (; pointer != input.end(); pointer++) {
            if (turbo::uri_charsets::bit_at(character_set, *pointer)) {
                result.append(turbo::uri_charsets::hex + uint8_t(*pointer) * 4, 3);
            } else {
                result += *pointer;
            }
        }

        return result;
    }

    std::string percent_encode(const std::string_view input,
                               const uint8_t character_set[], size_t index) {
        std::string out;
        out.append(input.data(), index);
        auto pointer = input.begin() + index;
        for (; pointer != input.end(); pointer++) {
            if (turbo::uri_charsets::bit_at(character_set, *pointer)) {
                out.append(turbo::uri_charsets::hex + uint8_t(*pointer) * 4, 3);
            } else {
                out += *pointer;
            }
        }
        return out;
    }


    std::string percent_decode(const std::string_view input, size_t first_percent) {
        // next line is for safety only, we expect users to avoid calling
        // percent_decode when first_percent is outside the range.
        if (first_percent == std::string_view::npos) {
            return std::string(input);
        }
        std::string dest;
        dest.reserve(input.length());
        dest.append(input.substr(0, first_percent));
        const char* pointer = input.data() + first_percent;
        const char* end = input.data() + input.size();
        // Optimization opportunity: if the following code gets
        // called often, it can be optimized quite a bit.
        while (pointer < end) {
            const char ch = pointer[0];
            size_t remaining = end - pointer - 1;
            if (ch != '%' || remaining < 2 ||
                (  // ch == '%' && // It is unnecessary to check that ch == '%'.
                    (!turbo::ascii_isxdigit(pointer[1]) ||
                     !turbo::ascii_isxdigit(pointer[2])))) {
                dest += ch;
                pointer++;
                continue;
                     } else {
                         unsigned a = convert_hex_to_binary(pointer[1]);
                         unsigned b = convert_hex_to_binary(pointer[2]);
                         char c = static_cast<char>(a * 16 + b);
                         dest += c;
                         pointer += 3;
                     }
        }
        return dest;
    }


    bool to_ascii(std::optional<std::string>& out, const std::string_view plain,
                  size_t first_percent) {
        std::string percent_decoded_buffer;
        std::string_view input = plain;
        if (first_percent != std::string_view::npos) {
            percent_decoded_buffer = turbo::percent_decode(plain, first_percent);
            input = percent_decoded_buffer;
        }
        // input is a non-empty UTF-8 string, must be percent decoded
        std::string idna_ascii = idna::to_ascii(input);
        if (idna_ascii.empty() || contains_forbidden_domain_code_point(
                                      idna_ascii.data(), idna_ascii.size())) {
            return false;
                                      }
        out = std::move(idna_ascii);
        return true;
    }


    constexpr uint64_t broadcast(uint8_t v) noexcept {
        return 0x101010101010101ull * v;
    }

     bool to_lower_ascii(char* input, size_t length) noexcept {
        uint64_t broadcast_80 = broadcast(0x80);
        uint64_t broadcast_Ap = broadcast(128 - 'A');
        uint64_t broadcast_Zp = broadcast(128 - 'Z' - 1);
        uint64_t non_ascii = 0;
        size_t i = 0;

        for (; i + 7 < length; i += 8) {
            uint64_t word{};
            memcpy(&word, input + i, sizeof(word));
            non_ascii |= (word & broadcast_80);
            word ^=
                (((word + broadcast_Ap) ^ (word + broadcast_Zp)) & broadcast_80) >> 2;
            memcpy(input + i, &word, sizeof(word));
        }
        if (i < length) {
            uint64_t word{};
            memcpy(&word, input + i, length - i);
            non_ascii |= (word & broadcast_80);
            word ^=
                (((word + broadcast_Ap) ^ (word + broadcast_Zp)) & broadcast_80) >> 2;
            memcpy(input + i, &word, length - i);
        }
        return non_ascii == 0;
    }
} // namespace turbo
