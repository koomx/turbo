// Copyright 2023 The Abseil Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <turbo/log/internal/fnmatch.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace {
using ::testing::IsFalse;
using ::testing::IsTrue;

TEST(FNMatchTest, Works) {
  using turbo::log_internal::fnmatch;
  EXPECT_THAT(fnmatch("foo", "foo"), IsTrue());
  EXPECT_THAT(fnmatch("foo", "bar"), IsFalse());
  EXPECT_THAT(fnmatch("foo", "fo"), IsFalse());
  EXPECT_THAT(fnmatch("foo", "foo2"), IsFalse());
  EXPECT_THAT(fnmatch("foo/*", "foo/1/2/3/4"), IsTrue());
  EXPECT_THAT(fnmatch("bar/foo.ext", "bar/foo.ext"), IsTrue());
  EXPECT_THAT(fnmatch("*ba*r/fo*o.ext*", "bar/foo.ext"), IsTrue());
  EXPECT_THAT(fnmatch("bar/foo.ext", "bar/baz.ext"), IsFalse());
  EXPECT_THAT(fnmatch("bar/foo.ext", "bar/foo"), IsFalse());
  EXPECT_THAT(fnmatch("bar/foo.ext", "bar/foo.ext.zip"), IsFalse());
  EXPECT_THAT(fnmatch("ba?/*.ext", "bar/foo.ext"), IsTrue());
  EXPECT_THAT(fnmatch("ba?/*.ext", "baZ/FOO.ext"), IsTrue());
  EXPECT_THAT(fnmatch("ba?/*.ext", "barr/foo.ext"), IsFalse());
  EXPECT_THAT(fnmatch("ba?/*.ext", "bar/foo.ext2"), IsFalse());
  EXPECT_THAT(fnmatch("ba?/*", "bar/foo.ext2"), IsTrue());
  EXPECT_THAT(fnmatch("ba?/*", "bar/"), IsTrue());
  EXPECT_THAT(fnmatch("ba?/?", "bar/"), IsFalse());
  EXPECT_THAT(fnmatch("ba?/*", "bar"), IsFalse());
  EXPECT_THAT(fnmatch("?x", "zx"), IsTrue());
  EXPECT_THAT(fnmatch("*b", "aab"), IsTrue());
  EXPECT_THAT(fnmatch("a*b", "aXb"), IsTrue());
  EXPECT_THAT(fnmatch("", ""), IsTrue());
  EXPECT_THAT(fnmatch("", "a"), IsFalse());
  EXPECT_THAT(fnmatch("ab*", "ab"), IsTrue());
  EXPECT_THAT(fnmatch("ab**", "ab"), IsTrue());
  EXPECT_THAT(fnmatch("ab*?", "ab"), IsFalse());
  EXPECT_THAT(fnmatch("*", "bbb"), IsTrue());
  EXPECT_THAT(fnmatch("*", ""), IsTrue());
  EXPECT_THAT(fnmatch("?", ""), IsFalse());
  EXPECT_THAT(fnmatch("***", "**p"), IsTrue());
  EXPECT_THAT(fnmatch("**", "*"), IsTrue());
  EXPECT_THAT(fnmatch("*?", "*"), IsTrue());
}

}  // namespace
