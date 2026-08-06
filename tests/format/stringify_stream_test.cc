// Copyright 2026 The Abseil Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <turbo/format/internal/stringify_stream.h>

#include <cstddef>
#include <iomanip>
#include <ostream>
#include <sstream>

#include <gtest/gtest.h>
#include <turbo/macros/config.h>
#include <turbo/format/str_format.h>
#include <string_view>

namespace turbo {

namespace strings_internal {
namespace {

// Exercises the Append(size_t, char) overload
struct AppendNCharsTest {
  size_t count;
  char ch;

  template <typename Sink>
  friend void turbo_stringify(Sink& sink, const AppendNCharsTest& t) {
    sink.Append(t.count, t.ch);
  }
};
TEST(StringifyStreamTest, AppendNChars) {
  std::ostringstream os;
  StringifyStream(os) << AppendNCharsTest{5, 'a'};
  EXPECT_EQ(os.str(), "aaaaa");
}

// Exercises the Append(std::string_view) overload
struct AppendStringViewTest {
  std::string_view v;

  template <typename Sink>
  friend void turbo_stringify(Sink& sink, const AppendStringViewTest& t) {
    sink.Append(t.v);
  }
};
TEST(StringifyStreamTest, AppendStringView) {
  std::ostringstream os;
  StringifyStream(os) << AppendStringViewTest{"abc"};
  EXPECT_EQ(os.str(), "abc");
}

// Exercises TurboFormatFlush(OStringStreamSink*, std::string_view v)
struct TurboFormatFlushTest {
  std::string_view a, b, c;

  template <typename Sink>
  friend void turbo_stringify(Sink& sink, const TurboFormatFlushTest& t) {
    turbo::str_printf_to(&sink, "%s, %s, %s", t.a, t.b, t.c);
  }
};
TEST(StringifyStreamTest, TurboFormatFlush) {
  std::ostringstream os;
  StringifyStream(os) << TurboFormatFlushTest{"a", "b", "c"};
  EXPECT_EQ(os.str(), "a, b, c");
}

// If overloads of both turbo_stringify and operator<< are defined for the type,
// the operator<< overload should take precedence.
struct PreferStreamInsertionOverTurboStringifyTest {
  friend std::ostream& operator<<(  // NOLINT(clang-diagnostic-unused-function)
      std::ostream& os, const PreferStreamInsertionOverTurboStringifyTest&) {
    return os << "good";
  }

  template <typename Sink>
  friend void turbo_stringify  // NOLINT(clang-diagnostic-unused-function)
      (Sink& sink, const PreferStreamInsertionOverTurboStringifyTest&) {
    sink.Append("bad");
  }
};
TEST(StringifyStreamTest, PreferStreamInsertionOverTurboStringify) {
  std::ostringstream os;
  StringifyStream(os) << PreferStreamInsertionOverTurboStringifyTest{};
  EXPECT_EQ(os.str(), "good");
}
TEST(StringifyStreamTest, SupportEndl) {
  std::ostringstream os;
  StringifyStream(os) << std::endl;
  EXPECT_EQ(os.str(), "\n");
}
TEST(StringifyStreamTest, SupportSetbase) {
  std::ostringstream os;
  StringifyStream(os) << std::setbase(16) << 255;
  EXPECT_EQ(os.str(), "ff");
}

}  // namespace
}  // namespace strings_internal

}  // namespace turbo
