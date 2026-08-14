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

#include <gtest/gtest.h>
#include <turbo/types/expected.h>

#include <memory>

#define TOKENPASTE(x, y) x##y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)
#undef STATIC_REQUIRE
#define STATIC_REQUIRE(e)                            \
    constexpr bool TOKENPASTE2(rqure, __LINE__) = e; \
    (void)TOKENPASTE2(rqure, __LINE__);              \
    ASSERT_TRUE(e);

TEST(Map, extensions) {
    auto mul2 = [](int a) { return a * 2; };
    auto ret_void = [](int a) { (void)a; };

    {
        turbo::expected<int, int> e = 21;
        auto ret = e.map(mul2);
        ASSERT_TRUE(ret);
        ASSERT_TRUE(*ret == 42);
    }

    {
        const turbo::expected<int, int> e = 21;
        auto ret = e.map(mul2);
        ASSERT_TRUE(ret);
        ASSERT_TRUE(*ret == 42);
    }

    {
        turbo::expected<int, int> e = 21;
        auto ret = std::move(e).map(mul2);
        ASSERT_TRUE(ret);
        ASSERT_TRUE(*ret == 42);
    }

    {
        const turbo::expected<int, int> e = 21;
        auto ret = std::move(e).map(mul2);
        ASSERT_TRUE(ret);
        ASSERT_TRUE(*ret == 42);
    }

    {
        turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = e.map(mul2);
        ASSERT_TRUE(!ret);
        ASSERT_TRUE(ret.error() == 21);
    }

    {
        const turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = e.map(mul2);
        ASSERT_TRUE(!ret);
        ASSERT_TRUE(ret.error() == 21);
    }

    {
        turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = std::move(e).map(mul2);
        ASSERT_TRUE(!ret);
        ASSERT_TRUE(ret.error() == 21);
    }

    {
        const turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = std::move(e).map(mul2);
        ASSERT_TRUE(!ret);
        ASSERT_TRUE(ret.error() == 21);
    }

    {
        turbo::expected<int, int> e = 21;
        auto ret = e.map(ret_void);
        ASSERT_TRUE(ret);
        STATIC_REQUIRE(
            (std::is_same<decltype(ret), turbo::expected<void, int>>::value));
    }

    {
        const turbo::expected<int, int> e = 21;
        auto ret = e.map(ret_void);
        ASSERT_TRUE(ret);
        STATIC_REQUIRE(
            (std::is_same<decltype(ret), turbo::expected<void, int>>::value));
    }

    {
        turbo::expected<int, int> e = 21;
        auto ret = std::move(e).map(ret_void);
        ASSERT_TRUE(ret);
        STATIC_REQUIRE(
            (std::is_same<decltype(ret), turbo::expected<void, int>>::value));
    }

    {
        const turbo::expected<int, int> e = 21;
        auto ret = std::move(e).map(ret_void);
        ASSERT_TRUE(ret);
        STATIC_REQUIRE(
            (std::is_same<decltype(ret), turbo::expected<void, int>>::value));
    }

    {
        turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = e.map(ret_void);
        ASSERT_TRUE(!ret);
        STATIC_REQUIRE(
            (std::is_same<decltype(ret), turbo::expected<void, int>>::value));
    }

    {
        const turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = e.map(ret_void);
        ASSERT_TRUE(!ret);
        STATIC_REQUIRE(
            (std::is_same<decltype(ret), turbo::expected<void, int>>::value));
    }

    {
        turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = std::move(e).map(ret_void);
        ASSERT_TRUE(!ret);
        STATIC_REQUIRE(
            (std::is_same<decltype(ret), turbo::expected<void, int>>::value));
    }

    {
        const turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = std::move(e).map(ret_void);
        ASSERT_TRUE(!ret);
        STATIC_REQUIRE(
            (std::is_same<decltype(ret), turbo::expected<void, int>>::value));
    }

    // mapping functions which return references
    {
        turbo::expected<int, int> e(42);
        auto ret = e.map([](int& i) -> int& { return i; });
        ASSERT_TRUE(ret);
        ASSERT_TRUE(ret == 42);
    }
}

TEST(Map, error_extensions) {
    auto mul2 = [](int a) { return a * 2; };
    auto ret_void = [](int a) { (void)a; };

    {
        turbo::expected<int, int> e = 21;
        auto ret = e.map_error(mul2);
        ASSERT_TRUE(ret);
        ASSERT_TRUE(*ret == 21);
    }

    {
        const turbo::expected<int, int> e = 21;
        auto ret = e.map_error(mul2);
        ASSERT_TRUE(ret);
        ASSERT_TRUE(*ret == 21);
    }

    {
        turbo::expected<int, int> e = 21;
        auto ret = std::move(e).map_error(mul2);
        ASSERT_TRUE(ret);
        ASSERT_TRUE(*ret == 21);
    }

    {
        const turbo::expected<int, int> e = 21;
        auto ret = std::move(e).map_error(mul2);
        ASSERT_TRUE(ret);
        ASSERT_TRUE(*ret == 21);
    }

    {
        turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = e.map_error(mul2);
        ASSERT_TRUE(!ret);
        ASSERT_TRUE(ret.error() == 42);
    }

    {
        const turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = e.map_error(mul2);
        ASSERT_TRUE(!ret);
        ASSERT_TRUE(ret.error() == 42);
    }

    {
        turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = std::move(e).map_error(mul2);
        ASSERT_TRUE(!ret);
        ASSERT_TRUE(ret.error() == 42);
    }

    {
        const turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = std::move(e).map_error(mul2);
        ASSERT_TRUE(!ret);
        ASSERT_TRUE(ret.error() == 42);
    }

    {
        turbo::expected<int, int> e = 21;
        auto ret = e.map_error(ret_void);
        ASSERT_TRUE(ret);
    }

    {
        const turbo::expected<int, int> e = 21;
        auto ret = e.map_error(ret_void);
        ASSERT_TRUE(ret);
    }

    {
        turbo::expected<int, int> e = 21;
        auto ret = std::move(e).map_error(ret_void);
        ASSERT_TRUE(ret);
    }

    {
        const turbo::expected<int, int> e = 21;
        auto ret = std::move(e).map_error(ret_void);
        ASSERT_TRUE(ret);
    }

    {
        turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = e.map_error(ret_void);
        ASSERT_TRUE(!ret);
    }

    {
        const turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = e.map_error(ret_void);
        ASSERT_TRUE(!ret);
    }

    {
        turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = std::move(e).map_error(ret_void);
        ASSERT_TRUE(!ret);
    }

    {
        const turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = std::move(e).map_error(ret_void);
        ASSERT_TRUE(!ret);
    }
}

TEST(And, thenextensions) {
    auto succeed = [](int a) { (void)a; return turbo::expected<int, int>(21 * 2); };
    auto fail = [](int a) { (void)a; return turbo::expected<int, int>(turbo::unexpect, 17); };

    {
        turbo::expected<int, int> e = 21;
        auto ret = e.and_then(succeed);
        ASSERT_TRUE(ret);
        ASSERT_TRUE(*ret == 42);
    }

    {
        const turbo::expected<int, int> e = 21;
        auto ret = e.and_then(succeed);
        ASSERT_TRUE(ret);
        ASSERT_TRUE(*ret == 42);
    }

    {
        turbo::expected<int, int> e = 21;
        auto ret = std::move(e).and_then(succeed);
        ASSERT_TRUE(ret);
        ASSERT_TRUE(*ret == 42);
    }

    {
        const turbo::expected<int, int> e = 21;
        auto ret = std::move(e).and_then(succeed);
        ASSERT_TRUE(ret);
        ASSERT_TRUE(*ret == 42);
    }

    {
        turbo::expected<int, int> e = 21;
        auto ret = e.and_then(fail);
        ASSERT_TRUE(!ret);
        ASSERT_TRUE(ret.error() == 17);
    }

    {
        const turbo::expected<int, int> e = 21;
        auto ret = e.and_then(fail);
        ASSERT_TRUE(!ret);
        ASSERT_TRUE(ret.error() == 17);
    }

    {
        turbo::expected<int, int> e = 21;
        auto ret = std::move(e).and_then(fail);
        ASSERT_TRUE(!ret);
        ASSERT_TRUE(ret.error() == 17);
    }

    {
        const turbo::expected<int, int> e = 21;
        auto ret = std::move(e).and_then(fail);
        ASSERT_TRUE(!ret);
        ASSERT_TRUE(ret.error() == 17);
    }

    {
        turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = e.and_then(succeed);
        ASSERT_TRUE(!ret);
        ASSERT_TRUE(ret.error() == 21);
    }

    {
        const turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = e.and_then(succeed);
        ASSERT_TRUE(!ret);
        ASSERT_TRUE(ret.error() == 21);
    }

    {
        turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = std::move(e).and_then(succeed);
        ASSERT_TRUE(!ret);
        ASSERT_TRUE(ret.error() == 21);
    }

    {
        const turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = std::move(e).and_then(succeed);
        ASSERT_TRUE(!ret);
        ASSERT_TRUE(ret.error() == 21);
    }

    {
        turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = e.and_then(fail);
        ASSERT_TRUE(!ret);
        ASSERT_TRUE(ret.error() == 21);
    }

    {
        const turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = e.and_then(fail);
        ASSERT_TRUE(!ret);
        ASSERT_TRUE(ret.error() == 21);
    }

    {
        turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = std::move(e).and_then(fail);
        ASSERT_TRUE(!ret);
        ASSERT_TRUE(ret.error() == 21);
    }

    {
        const turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = std::move(e).and_then(fail);
        ASSERT_TRUE(!ret);
        ASSERT_TRUE(ret.error() == 21);
    }
}

TEST(expated, or_else) {
    using eptr = std::unique_ptr<int>;
    auto succeed = [](int a) { (void)a; return turbo::expected<int, int>(21 * 2); };
    auto succeedptr = [](eptr e) { (void)e; return turbo::expected<int,eptr>(21*2); };
    auto fail = [](int a) { (void)a; return turbo::expected<int,int>(turbo::unexpect, 17); };
    auto failptr = [](eptr e) { *e = 17;return turbo::expected<int,eptr>(turbo::unexpect, std::move(e)); };
    auto failvoid = [](int) { };
    auto failvoidptr = [](const eptr&) { /* don't consume */ };
    auto consumeptr = [](eptr) { };
    auto make_u_int = [](int n) { return std::unique_ptr<int>(new int(n)); };

    {
        turbo::expected<int, int> e = 21;
        auto ret = e.or_else(succeed);
        ASSERT_TRUE(ret);
        ASSERT_TRUE(*ret == 21);
    }

    {
        const turbo::expected<int, int> e = 21;
        auto ret = e.or_else(succeed);
        ASSERT_TRUE(ret);
        ASSERT_TRUE(*ret == 21);
    }

    {
        turbo::expected<int, int> e = 21;
        auto ret = std::move(e).or_else(succeed);
        ASSERT_TRUE(ret);
        ASSERT_TRUE(*ret == 21);
    }

    {
        turbo::expected<int, eptr> e = 21;
        auto ret = std::move(e).or_else(succeedptr);
        ASSERT_TRUE(ret);
        ASSERT_TRUE(*ret == 21);
    }

    {
        const turbo::expected<int, int> e = 21;
        auto ret = std::move(e).or_else(succeed);
        ASSERT_TRUE(ret);
        ASSERT_TRUE(*ret == 21);
    }

    {
        turbo::expected<int, int> e = 21;
        auto ret = e.or_else(fail);
        ASSERT_TRUE(ret);
        ASSERT_TRUE(*ret == 21);
    }

    {
        const turbo::expected<int, int> e = 21;
        auto ret = e.or_else(fail);
        ASSERT_TRUE(ret);
        ASSERT_TRUE(*ret == 21);
    }

    {
        turbo::expected<int, int> e = 21;
        auto ret = std::move(e).or_else(fail);
        ASSERT_TRUE(ret);
        ASSERT_TRUE(ret == 21);
    }

    {
        turbo::expected<int, eptr> e = 21;
        auto ret = std::move(e).or_else(failptr);
        ASSERT_TRUE(ret);
        ASSERT_TRUE(ret == 21);
    }

    {
        const turbo::expected<int, int> e = 21;
        auto ret = std::move(e).or_else(fail);
        ASSERT_TRUE(ret);
        ASSERT_TRUE(*ret == 21);
    }

    {
        turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = e.or_else(succeed);
        ASSERT_TRUE(ret);
        ASSERT_TRUE(*ret == 42);
    }

    {
        const turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = e.or_else(succeed);
        ASSERT_TRUE(ret);
        ASSERT_TRUE(*ret == 42);
    }

    {
        turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = std::move(e).or_else(succeed);
        ASSERT_TRUE(ret);
        ASSERT_TRUE(*ret == 42);
    }

    {
        turbo::expected<int, eptr> e(turbo::unexpect, make_u_int(21));
        auto ret = std::move(e).or_else(succeedptr);
        ASSERT_TRUE(ret);
        ASSERT_TRUE(*ret == 42);
    }

    {
        const turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = std::move(e).or_else(succeed);
        ASSERT_TRUE(ret);
        ASSERT_TRUE(*ret == 42);
    }

    {
        turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = e.or_else(fail);
        ASSERT_TRUE(!ret);
        ASSERT_TRUE(ret.error() == 17);
    }

    {
        turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = e.or_else(failvoid);
        ASSERT_TRUE(!ret);
        ASSERT_TRUE(ret.error() == 21);
    }

    {
        const turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = e.or_else(fail);
        ASSERT_TRUE(!ret);
        ASSERT_TRUE(ret.error() == 17);
    }

    {
        const turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = e.or_else(failvoid);
        ASSERT_TRUE(!ret);
        ASSERT_TRUE(ret.error() == 21);
    }

    {
        turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = std::move(e).or_else(fail);
        ASSERT_TRUE(!ret);
        ASSERT_TRUE(ret.error() == 17);
    }

    {
        turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = std::move(e).or_else(failvoid);
        ASSERT_TRUE(!ret);
        ASSERT_TRUE(ret.error() == 21);
    }

    {
        turbo::expected<int, eptr> e(turbo::unexpect, make_u_int(21));
        auto ret = std::move(e).or_else(failvoidptr);
        ASSERT_TRUE(!ret);
        ASSERT_TRUE(*ret.error() == 21);
    }

    {
        turbo::expected<int, eptr> e(turbo::unexpect, make_u_int(21));
        auto ret = std::move(e).or_else(consumeptr);
        ASSERT_TRUE(!ret);
        ASSERT_TRUE(ret.error() == nullptr);
    }

    {
        const turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = std::move(e).or_else(fail);
        ASSERT_TRUE(!ret);
        ASSERT_TRUE(ret.error() == 17);
    }

    {
        const turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = std::move(e).or_else(failvoid);
        ASSERT_TRUE(!ret);
        ASSERT_TRUE(ret.error() == 21);
    }
}

TEST(Transform, extensions) {
    auto mul2 = [](int a) { return a * 2; };
    auto ret_void = [](int a) { (void)a; };

    {
        turbo::expected<int, int> e = 21;
        auto ret = e.transform(mul2);
        ASSERT_TRUE(ret);
        ASSERT_TRUE(*ret == 42);
    }

    {
        const turbo::expected<int, int> e = 21;
        auto ret = e.transform(mul2);
        ASSERT_TRUE(ret);
        ASSERT_TRUE(*ret == 42);
    }

    {
        turbo::expected<int, int> e = 21;
        auto ret = std::move(e).transform(mul2);
        ASSERT_TRUE(ret);
        ASSERT_TRUE(*ret == 42);
    }

    {
        const turbo::expected<int, int> e = 21;
        auto ret = std::move(e).transform(mul2);
        ASSERT_TRUE(ret);
        ASSERT_TRUE(*ret == 42);
    }

    {
        turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = e.transform(mul2);
        ASSERT_TRUE(!ret);
        ASSERT_TRUE(ret.error() == 21);
    }

    {
        const turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = e.transform(mul2);
        ASSERT_TRUE(!ret);
        ASSERT_TRUE(ret.error() == 21);
    }

    {
        turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = std::move(e).transform(mul2);
        ASSERT_TRUE(!ret);
        ASSERT_TRUE(ret.error() == 21);
    }

    {
        const turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = std::move(e).transform(mul2);
        ASSERT_TRUE(!ret);
        ASSERT_TRUE(ret.error() == 21);
    }

    {
        turbo::expected<int, int> e = 21;
        auto ret = e.transform(ret_void);
        ASSERT_TRUE(ret);
        STATIC_REQUIRE(
            (std::is_same<decltype(ret), turbo::expected<void, int>>::value));
    }

    {
        const turbo::expected<int, int> e = 21;
        auto ret = e.transform(ret_void);
        ASSERT_TRUE(ret);
        STATIC_REQUIRE(
            (std::is_same<decltype(ret), turbo::expected<void, int>>::value));
    }

    {
        turbo::expected<int, int> e = 21;
        auto ret = std::move(e).transform(ret_void);
        ASSERT_TRUE(ret);
        STATIC_REQUIRE(
            (std::is_same<decltype(ret), turbo::expected<void, int>>::value));
    }

    {
        const turbo::expected<int, int> e = 21;
        auto ret = std::move(e).transform(ret_void);
        ASSERT_TRUE(ret);
        STATIC_REQUIRE(
            (std::is_same<decltype(ret), turbo::expected<void, int>>::value));
    }

    {
        turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = e.transform(ret_void);
        ASSERT_TRUE(!ret);
        STATIC_REQUIRE(
            (std::is_same<decltype(ret), turbo::expected<void, int>>::value));
    }

    {
        const turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = e.transform(ret_void);
        ASSERT_TRUE(!ret);
        STATIC_REQUIRE(
            (std::is_same<decltype(ret), turbo::expected<void, int>>::value));
    }

    {
        turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = std::move(e).transform(ret_void);
        ASSERT_TRUE(!ret);
        STATIC_REQUIRE(
            (std::is_same<decltype(ret), turbo::expected<void, int>>::value));
    }

    {
        const turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = std::move(e).transform(ret_void);
        ASSERT_TRUE(!ret);
        STATIC_REQUIRE(
            (std::is_same<decltype(ret), turbo::expected<void, int>>::value));
    }

    // mapping functions which return references
    {
        turbo::expected<int, int> e(42);
        auto ret = e.transform([](int& i) -> int& { return i; });
        ASSERT_TRUE(ret);
        ASSERT_TRUE(ret == 42);
    }
}

TEST(Transform, error_extensions) {
    auto mul2 = [](int a) { return a * 2; };
    auto ret_void = [](int a) { (void)a; };

    {
        turbo::expected<int, int> e = 21;
        auto ret = e.transform_error(mul2);
        ASSERT_TRUE(ret);
        ASSERT_TRUE(*ret == 21);
    }

    {
        const turbo::expected<int, int> e = 21;
        auto ret = e.transform_error(mul2);
        ASSERT_TRUE(ret);
        ASSERT_TRUE(*ret == 21);
    }

    {
        turbo::expected<int, int> e = 21;
        auto ret = std::move(e).transform_error(mul2);
        ASSERT_TRUE(ret);
        ASSERT_TRUE(*ret == 21);
    }

    {
        const turbo::expected<int, int> e = 21;
        auto ret = std::move(e).transform_error(mul2);
        ASSERT_TRUE(ret);
        ASSERT_TRUE(*ret == 21);
    }

    {
        turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = e.transform_error(mul2);
        ASSERT_TRUE(!ret);
        ASSERT_TRUE(ret.error() == 42);
    }

    {
        const turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = e.transform_error(mul2);
        ASSERT_TRUE(!ret);
        ASSERT_TRUE(ret.error() == 42);
    }

    {
        turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = std::move(e).transform_error(mul2);
        ASSERT_TRUE(!ret);
        ASSERT_TRUE(ret.error() == 42);
    }

    {
        const turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = std::move(e).transform_error(mul2);
        ASSERT_TRUE(!ret);
        ASSERT_TRUE(ret.error() == 42);
    }

    {
        turbo::expected<int, int> e = 21;
        auto ret = e.transform_error(ret_void);
        ASSERT_TRUE(ret);
    }

    {
        const turbo::expected<int, int> e = 21;
        auto ret = e.transform_error(ret_void);
        ASSERT_TRUE(ret);
    }

    {
        turbo::expected<int, int> e = 21;
        auto ret = std::move(e).transform_error(ret_void);
        ASSERT_TRUE(ret);
    }

    {
        const turbo::expected<int, int> e = 21;
        auto ret = std::move(e).transform_error(ret_void);
        ASSERT_TRUE(ret);
    }

    {
        turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = e.transform_error(ret_void);
        ASSERT_TRUE(!ret);
    }

    {
        const turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = e.transform_error(ret_void);
        ASSERT_TRUE(!ret);
    }

    {
        turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = std::move(e).transform_error(ret_void);
        ASSERT_TRUE(!ret);
    }

    {
        const turbo::expected<int, int> e(turbo::unexpect, 21);
        auto ret = std::move(e).transform_error(ret_void);
        ASSERT_TRUE(!ret);
    }
}

struct S {
    int x;
};

struct F {
    int x;
};

TEST(expexted, a14) {
    auto res = turbo::expected<S, F> { turbo::unexpect, F { } };

    (void)res.map_error([](F f) {
        (void)f;
    });
}

TEST(expexted, a32) {
    int i = 0;
    turbo::expected<void, int> a;
    (void)a.map([&i] { i = 42; });
    ASSERT_TRUE(i == 42);

    auto x = a.map([] { return 42; });
    ASSERT_TRUE(*x == 42);
}
