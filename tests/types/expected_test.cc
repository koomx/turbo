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


struct takes_init_and_variadic {
    std::vector<int> v;
    std::tuple<int, int> t;
    template <class... Args>
    takes_init_and_variadic(std::initializer_list<int> l, Args &&... args)
        : v(l), t(std::forward<Args>(args)...) {}
};


TEST(expected, Emplace) {
    {
        turbo::expected<std::unique_ptr<int>,int> e;
        e.emplace(new int{42});
        ASSERT_TRUE(e);
        ASSERT_TRUE(**e == 42);
    }

    {
        turbo::expected<std::vector<int>,int> e;
        e.emplace({0,1});
        ASSERT_TRUE(e);
        ASSERT_TRUE((*e)[0] == 0);
        ASSERT_TRUE((*e)[1] == 1);
    }

    {
        turbo::expected<std::tuple<int,int>,int> e;
        e.emplace(2,3);
        ASSERT_TRUE(e);
        ASSERT_TRUE(std::get<0>(*e) == 2);
        ASSERT_TRUE(std::get<1>(*e) == 3);
    }

    {
        turbo::expected<takes_init_and_variadic,int> e = turbo::make_unexpected(0);
        e.emplace({0,1}, 2, 3);
        ASSERT_TRUE(e);
        ASSERT_TRUE(e->v[0] == 0);
        ASSERT_TRUE(e->v[1] == 1);
        ASSERT_TRUE(std::get<0>(e->t) == 2);
        ASSERT_TRUE(std::get<1>(e->t) == 3);
    }
}


TEST(expacted, Constructors) {
    {
        turbo::expected<int,int> e;
        ASSERT_TRUE(e);
        ASSERT_TRUE(e == 0);
    }

    {
        turbo::expected<int,int> e = turbo::make_unexpected(0);
        ASSERT_TRUE(!e);
        ASSERT_TRUE(e.error() == 0);
    }

    {
        turbo::expected<int,int> e (turbo::unexpect, 0);
        ASSERT_TRUE(!e);
        ASSERT_TRUE(e.error() == 0);
    }

    {
        turbo::expected<int,int> e (turbo::in_place, 42);
        ASSERT_TRUE(e);
        ASSERT_TRUE(e == 42);
    }

    {
        turbo::expected<std::vector<int>,int> e (turbo::in_place, {0,1});
        ASSERT_TRUE(e);
        ASSERT_TRUE((*e)[0] == 0);
        ASSERT_TRUE((*e)[1] == 1);
    }

    {
        turbo::expected<std::tuple<int,int>,int> e (turbo::in_place, 0, 1);
        ASSERT_TRUE(e);
        ASSERT_TRUE(std::get<0>(*e) == 0);
        ASSERT_TRUE(std::get<1>(*e) == 1);
    }

    {
        turbo::expected<takes_init_and_variadic,int> e (turbo::in_place, {0,1}, 2, 3);
        ASSERT_TRUE(e);
        ASSERT_TRUE(e->v[0] == 0);
        ASSERT_TRUE(e->v[1] == 1);
        ASSERT_TRUE(std::get<0>(e->t) == 2);
        ASSERT_TRUE(std::get<1>(e->t) == 3);
    }

	{
		turbo::expected<int, int> e;
		ASSERT_TRUE(std::is_default_constructible<decltype(e)>::value);
		ASSERT_TRUE(std::is_copy_constructible<decltype(e)>::value);
		ASSERT_TRUE(std::is_move_constructible<decltype(e)>::value);
		ASSERT_TRUE(std::is_copy_assignable<decltype(e)>::value);
		ASSERT_TRUE(std::is_move_assignable<decltype(e)>::value);
		ASSERT_TRUE(std::is_trivially_copy_constructible<decltype(e)>::value);
		ASSERT_TRUE(std::is_trivially_copy_assignable<decltype(e)>::value);
		ASSERT_TRUE(std::is_trivially_move_constructible<decltype(e)>::value);
		ASSERT_TRUE(std::is_trivially_move_assignable<decltype(e)>::value);
	}

	{
		turbo::expected<int, std::string> e;
		ASSERT_TRUE(std::is_default_constructible<decltype(e)>::value);
		ASSERT_TRUE(std::is_copy_constructible<decltype(e)>::value);
		ASSERT_TRUE(std::is_move_constructible<decltype(e)>::value);
		ASSERT_TRUE(std::is_copy_assignable<decltype(e)>::value);
		ASSERT_TRUE(std::is_move_assignable<decltype(e)>::value);
		ASSERT_TRUE(!std::is_trivially_copy_constructible<decltype(e)>::value);
		ASSERT_TRUE(!std::is_trivially_copy_assignable<decltype(e)>::value);
		ASSERT_TRUE(!std::is_trivially_move_constructible<decltype(e)>::value);
		ASSERT_TRUE(!std::is_trivially_move_assignable<decltype(e)>::value);
	}

	{
		turbo::expected<std::string, int> e;
		ASSERT_TRUE(std::is_default_constructible<decltype(e)>::value);
		ASSERT_TRUE(std::is_copy_constructible<decltype(e)>::value);
		ASSERT_TRUE(std::is_move_constructible<decltype(e)>::value);
		ASSERT_TRUE(std::is_copy_assignable<decltype(e)>::value);
		ASSERT_TRUE(std::is_move_assignable<decltype(e)>::value);
		ASSERT_TRUE(!std::is_trivially_copy_constructible<decltype(e)>::value);
		ASSERT_TRUE(!std::is_trivially_copy_assignable<decltype(e)>::value);
		ASSERT_TRUE(!std::is_trivially_move_constructible<decltype(e)>::value);
		ASSERT_TRUE(!std::is_trivially_move_assignable<decltype(e)>::value);
	}

	{
		turbo::expected<std::string, std::string> e;
		ASSERT_TRUE(std::is_default_constructible<decltype(e)>::value);
		ASSERT_TRUE(std::is_copy_constructible<decltype(e)>::value);
		ASSERT_TRUE(std::is_move_constructible<decltype(e)>::value);
		ASSERT_TRUE(std::is_copy_assignable<decltype(e)>::value);
		ASSERT_TRUE(std::is_move_assignable<decltype(e)>::value);
		ASSERT_TRUE(!std::is_trivially_copyable<decltype(e)>::value);
		ASSERT_TRUE(!std::is_trivially_copy_assignable<decltype(e)>::value);
		ASSERT_TRUE(!std::is_trivially_move_constructible<decltype(e)>::value);
		ASSERT_TRUE(!std::is_trivially_move_assignable<decltype(e)>::value);
	}

    {
        turbo::expected<void,int> e;
        ASSERT_TRUE(e);
    }

    {
        turbo::expected<void,int> e (turbo::unexpect, 42);
        ASSERT_TRUE(!e);
        ASSERT_TRUE(e.error() == 42);
    }
}


TEST(Relational, operators) {
    turbo::expected<int, int> o1 = 42;
    turbo::expected<int, int> o2{turbo::unexpect, 0};
    const turbo::expected<int, int> o3 = 42;

    ASSERT_TRUE(o1 == o1);
    ASSERT_TRUE(o1 != o2);
    ASSERT_TRUE(o1 == o3);
    ASSERT_TRUE(o3 == o3);

    turbo::expected<void, int> o6;

    ASSERT_TRUE(o6 == o6);
}



TEST(Simple, assignment) {
    turbo::expected<int, int> e1 = 42;
    turbo::expected<int, int> e2 = 17;
    turbo::expected<int, int> e3 = 21;
    turbo::expected<int, int> e4 = turbo::make_unexpected(42);
    turbo::expected<int, int> e5 = turbo::make_unexpected(17);
    turbo::expected<int, int> e6 = turbo::make_unexpected(21);

    e1 = e2;
    ASSERT_TRUE(e1);
    ASSERT_TRUE(*e1 == 17);
    ASSERT_TRUE(e2);
    ASSERT_TRUE(*e2 == 17);

    e1 = std::move(e2);
    ASSERT_TRUE(e1);
    ASSERT_TRUE(*e1 == 17);
    ASSERT_TRUE(e2);
    ASSERT_TRUE(*e2 == 17);

    e1 = 42;
    ASSERT_TRUE(e1);
    ASSERT_TRUE(*e1 == 42);

    auto unex = turbo::make_unexpected(12);
    e1 = unex;
    ASSERT_TRUE(!e1);
    ASSERT_TRUE(e1.error() == 12);

    e1 = turbo::make_unexpected(42);
    ASSERT_TRUE(!e1);
    ASSERT_TRUE(e1.error() == 42);

    e1 = e3;
    ASSERT_TRUE(e1);
    ASSERT_TRUE(*e1 == 21);

    e4 = e5;
    ASSERT_TRUE(!e4);
    ASSERT_TRUE(e4.error() == 17);

    e4 = std::move(e6);
    ASSERT_TRUE(!e4);
    ASSERT_TRUE(e4.error() == 21);

    e4 = e1;
    ASSERT_TRUE(e4);
    ASSERT_TRUE(*e4 == 21);
}

TEST(Assignment, deletion) {
    struct has_all {
        has_all() = default;
        has_all(const has_all &) = default;
        has_all(has_all &&) noexcept = default;
        has_all &operator=(const has_all &) = default;
    };

    turbo::expected<has_all, has_all> e1 = {};
    turbo::expected<has_all, has_all> e2 = {};
    e1 = e2;

    struct except_move {
        except_move() = default;
        except_move(const except_move &) = default;
        except_move(except_move &&) noexcept(false){};
        except_move &operator=(const except_move &) = default;
    };
    turbo::expected<except_move, except_move> e3 = {};
    turbo::expected<except_move, except_move> e4 = {};
    // e3 = e4; should not compile
}


struct move_detector {
    move_detector() = default;
    move_detector(move_detector &&rhs) { rhs.been_moved = true; }
    bool been_moved = false;
};

TEST(expected, Observers) {
    turbo::expected<int,int> o1 = 42;
    turbo::expected<int,int> o2 {turbo::unexpect, 0};
    const turbo::expected<int,int> o3 = 42;

    ASSERT_TRUE(*o1 == 42);
    ASSERT_TRUE(*o1 == o1.value());
    ASSERT_TRUE(o2.value_or(42) == 42);
    ASSERT_TRUE(o2.error() == 0);
    ASSERT_TRUE(o3.value() == 42);
    auto success = std::is_same<decltype(o1.value()), int &>::value;
    ASSERT_TRUE(success);
    success = std::is_same<decltype(o3.value()), const int &>::value;
    ASSERT_TRUE(success);
    success = std::is_same<decltype(std::move(o1).value()), int &&>::value;
    ASSERT_TRUE(success);

    success = std::is_same<decltype(std::move(o3).value()), const int &&>::value;
    ASSERT_TRUE(success);

    turbo::expected<move_detector,int> o4{turbo::in_place};
    move_detector o5 = std::move(o4).value();
    ASSERT_TRUE(o4->been_moved);
    ASSERT_TRUE(!o5.been_moved);
}


struct no_throw {
  no_throw(std::string i) : i(i) {}
  std::string i;
};
struct canthrow_move {
  canthrow_move(std::string i) : i(i) {}
  canthrow_move(canthrow_move const &) = default;
  canthrow_move(canthrow_move &&other) noexcept(false) : i(other.i) {}
  canthrow_move &operator=(canthrow_move &&) = default;
  std::string i;
};

bool should_throw = false;

#ifdef STATUS_EXPECTED_EXCEPTIONS_ENABLED
struct willthrow_move {
  willthrow_move(std::string i) : i(i) {}
  willthrow_move(willthrow_move const &) = default;
  willthrow_move(willthrow_move &&other) : i(other.i) {
    if (should_throw)
      throw 0;
  }
  willthrow_move &operator=(willthrow_move &&) = default;
  std::string i;
};
#endif // STATUS_EXPECTED_EXCEPTIONS_ENABLED

static_assert(turbo::detail::is_swappable<no_throw>::value, "");

template <class T1, class T2> void swap_test() {
  std::string s1 = "abcdefghijklmnopqrstuvwxyz";
  std::string s2 = "zyxwvutsrqponmlkjihgfedcba";

  turbo::expected<T1, T2> a{s1};
  turbo::expected<T1, T2> b{s2};
  swap(a, b);
  ASSERT_TRUE(a->i == s2);
  ASSERT_TRUE(b->i == s1);

  a = s1;
  b = turbo::unexpected<T2>(s2);
  swap(a, b);
  ASSERT_TRUE(a.error().i == s2);
  ASSERT_TRUE(b->i == s1);

  a = turbo::unexpected<T2>(s1);
  b = s2;
  swap(a, b);
  ASSERT_TRUE(a->i == s2);
  ASSERT_TRUE(b.error().i == s1);

  a = turbo::unexpected<T2>(s1);
  b = turbo::unexpected<T2>(s2);
  swap(a, b);
  ASSERT_TRUE(a.error().i == s2);
  ASSERT_TRUE(b.error().i == s1);

  a = s1;
  b = s2;
  a.swap(b);
  ASSERT_TRUE(a->i == s2);
  ASSERT_TRUE(b->i == s1);

  a = s1;
  b = turbo::unexpected<T2>(s2);
  a.swap(b);
  ASSERT_TRUE(a.error().i == s2);
  ASSERT_TRUE(b->i == s1);

  a = turbo::unexpected<T2>(s1);
  b = s2;
  a.swap(b);
  ASSERT_TRUE(a->i == s2);
  ASSERT_TRUE(b.error().i == s1);

  a = turbo::unexpected<T2>(s1);
  b = turbo::unexpected<T2>(s2);
  a.swap(b);
  ASSERT_TRUE(a.error().i == s2);
  ASSERT_TRUE(b.error().i == s1);
}

#ifdef STATUS_EXPECTED_EXCEPTIONS_ENABLED
TEST(expecedtd, swap) {

  swap_test<no_throw, no_throw>();
  swap_test<no_throw, canthrow_move>();
  swap_test<canthrow_move, no_throw>();

  std::string s1 = "abcdefghijklmnopqrstuvwxyz";
  std::string s2 = "zyxwvutsrqponmlkjihgfedcbaxxx";
  turbo::expected<no_throw, willthrow_move> a{s1};
  turbo::expected<no_throw, willthrow_move> b{turbo::unexpect, s2};
  should_throw = 1;

  #ifdef _MSC_VER
  // this seems to break catch on GCC and Clang
  // Extra parens: gtest macros split on commas in swap(a, b).
  // willthrow_move throws int 0 when should_throw is set.
  ASSERT_THROW((swap(a, b)), int);
  #endif

  ASSERT_TRUE(a->i == s1);
  ASSERT_TRUE(b.error().i == s2);
}
#endif // STATUS_EXPECTED_EXCEPTIONS_ENABLED


TEST(expected,Triviality) {
    ASSERT_TRUE((std::is_trivially_copy_constructible<turbo::expected<int,int>>::value));
    ASSERT_TRUE((std::is_trivially_copy_assignable<turbo::expected<int,int>>::value));
    ASSERT_TRUE((std::is_trivially_move_constructible<turbo::expected<int,int>>::value));
    ASSERT_TRUE((std::is_trivially_move_assignable<turbo::expected<int,int>>::value));
    ASSERT_TRUE((std::is_trivially_destructible<turbo::expected<int,int>>::value));

    ASSERT_TRUE((std::is_trivially_copy_constructible<turbo::expected<void,int>>::value));
    ASSERT_TRUE((std::is_trivially_move_constructible<turbo::expected<void,int>>::value));
    ASSERT_TRUE((std::is_trivially_destructible<turbo::expected<void,int>>::value));


    {
        struct T {
            T(const T&) = default;
            T(T&&) = default;
            T& operator=(const T&) = default;
            T& operator=(T&&) = default;
            ~T() = default;
        };
        ASSERT_TRUE((std::is_trivially_copy_constructible<turbo::expected<T,int>>::value));
        ASSERT_TRUE((std::is_trivially_copy_assignable<turbo::expected<T,int>>::value));
        ASSERT_TRUE((std::is_trivially_move_constructible<turbo::expected<T,int>>::value));
        ASSERT_TRUE((std::is_trivially_move_assignable<turbo::expected<T,int>>::value));
        ASSERT_TRUE((std::is_trivially_destructible<turbo::expected<T,int>>::value));
    }

    {
        struct T {
            T(const T&){}
            T(T&&) {}
            T& operator=(const T&) { return *this; }
            T& operator=(T&&) { return *this; }
            ~T(){}
        };
        ASSERT_TRUE(!(std::is_trivially_copy_constructible<turbo::expected<T,int>>::value));
        ASSERT_TRUE(!(std::is_trivially_copy_assignable<turbo::expected<T,int>>::value));
        ASSERT_TRUE(!(std::is_trivially_move_constructible<turbo::expected<T,int>>::value));
        ASSERT_TRUE(!(std::is_trivially_move_assignable<turbo::expected<T,int>>::value));
        ASSERT_TRUE(!(std::is_trivially_destructible<turbo::expected<T,int>>::value));
    }

}

TEST(base,Deletion) {
    ASSERT_TRUE((std::is_copy_constructible<turbo::expected<int,int>>::value));
    ASSERT_TRUE((std::is_copy_assignable<turbo::expected<int,int>>::value));
    ASSERT_TRUE((std::is_move_constructible<turbo::expected<int,int>>::value));
    ASSERT_TRUE((std::is_move_assignable<turbo::expected<int,int>>::value));
    ASSERT_TRUE((std::is_destructible<turbo::expected<int,int>>::value));

    {
        struct T {
            T()=default;
        };
        ASSERT_TRUE((std::is_default_constructible<turbo::expected<T,int>>::value));
    }

    {
        struct T {
            T(int){}
        };
        ASSERT_TRUE((!std::is_default_constructible<turbo::expected<T,int>>::value));
    }

    {
        struct T {
            T(const T&) = default;
            T(T&&) = default;
            T& operator=(const T&) = default;
            T& operator=(T&&) = default;
            ~T() = default;
        };
        ASSERT_TRUE((std::is_copy_constructible<turbo::expected<T,int>>::value));
        ASSERT_TRUE((std::is_copy_assignable<turbo::expected<T,int>>::value));
        ASSERT_TRUE((std::is_move_constructible<turbo::expected<T,int>>::value));
        ASSERT_TRUE((std::is_move_assignable<turbo::expected<T,int>>::value));
        ASSERT_TRUE((std::is_destructible<turbo::expected<T,int>>::value));
    }

    {
        struct T {
            T(const T&)=delete;
            T(T&&)=delete;
            T& operator=(const T&)=delete;
            T& operator=(T&&)=delete;
        };
        ASSERT_TRUE((!std::is_copy_constructible<turbo::expected<T,int>>::value));
        ASSERT_TRUE((!std::is_copy_assignable<turbo::expected<T,int>>::value));
        ASSERT_TRUE((!std::is_move_constructible<turbo::expected<T,int>>::value));
        ASSERT_TRUE((!std::is_move_assignable<turbo::expected<T,int>>::value));
    }

    {
        struct T {
            T(const T&)=delete;
            T(T&&)=default;
            T& operator=(const T&)=delete;
            T& operator=(T&&)=default;
        };
        ASSERT_TRUE((!std::is_copy_constructible<turbo::expected<T,int>>::value));
        ASSERT_TRUE((!std::is_copy_assignable<turbo::expected<T,int>>::value));
        ASSERT_TRUE((std::is_move_constructible<turbo::expected<T,int>>::value));
        ASSERT_TRUE((std::is_move_assignable<turbo::expected<T,int>>::value));
    }

    {
        struct T {
            T(const T&)=default;
            T(T&&)=delete;
            T& operator=(const T&)=default;
            T& operator=(T&&)=delete;
        };
        ASSERT_TRUE((std::is_copy_constructible<turbo::expected<T,int>>::value));
        ASSERT_TRUE((std::is_copy_assignable<turbo::expected<T,int>>::value));
    }

	{
		turbo::expected<int, int> e;
		ASSERT_TRUE(std::is_default_constructible<decltype(e)>::value);
		ASSERT_TRUE(std::is_copy_constructible<decltype(e)>::value);
		ASSERT_TRUE(std::is_move_constructible<decltype(e)>::value);
		ASSERT_TRUE(std::is_copy_assignable<decltype(e)>::value);
		ASSERT_TRUE(std::is_move_assignable<decltype(e)>::value);
		ASSERT_TRUE(std::is_trivially_copy_constructible<decltype(e)>::value);
		ASSERT_TRUE(std::is_trivially_copy_assignable<decltype(e)>::value);
#	if !defined(STATUS_EXPECTED_GCC49)
		ASSERT_TRUE(std::is_trivially_move_constructible<decltype(e)>::value);
		ASSERT_TRUE(std::is_trivially_move_assignable<decltype(e)>::value);
#	endif
	}

	{
		turbo::expected<int, std::string> e;
		ASSERT_TRUE(std::is_default_constructible<decltype(e)>::value);
		ASSERT_TRUE(std::is_copy_constructible<decltype(e)>::value);
		ASSERT_TRUE(std::is_move_constructible<decltype(e)>::value);
		ASSERT_TRUE(std::is_copy_assignable<decltype(e)>::value);
		ASSERT_TRUE(std::is_move_assignable<decltype(e)>::value);
		ASSERT_TRUE(!std::is_trivially_copy_constructible<decltype(e)>::value);
		ASSERT_TRUE(!std::is_trivially_copy_assignable<decltype(e)>::value);
#	if !defined(STATUS_EXPECTED_GCC49)
		ASSERT_TRUE(!std::is_trivially_move_constructible<decltype(e)>::value);
		ASSERT_TRUE(!std::is_trivially_move_assignable<decltype(e)>::value);
#	endif
	}

	{
		turbo::expected<std::string, int> e;
		ASSERT_TRUE(std::is_default_constructible<decltype(e)>::value);
		ASSERT_TRUE(std::is_copy_constructible<decltype(e)>::value);
		ASSERT_TRUE(std::is_move_constructible<decltype(e)>::value);
		ASSERT_TRUE(std::is_copy_assignable<decltype(e)>::value);
		ASSERT_TRUE(std::is_move_assignable<decltype(e)>::value);
		ASSERT_TRUE(!std::is_trivially_copy_constructible<decltype(e)>::value);
		ASSERT_TRUE(!std::is_trivially_copy_assignable<decltype(e)>::value);
		ASSERT_TRUE(!std::is_trivially_move_constructible<decltype(e)>::value);
		ASSERT_TRUE(!std::is_trivially_move_assignable<decltype(e)>::value);
	}

	{
		turbo::expected<std::string, std::string> e;
		ASSERT_TRUE(std::is_default_constructible<decltype(e)>::value);
		ASSERT_TRUE(std::is_copy_constructible<decltype(e)>::value);
		ASSERT_TRUE(std::is_move_constructible<decltype(e)>::value);
		ASSERT_TRUE(std::is_copy_assignable<decltype(e)>::value);
		ASSERT_TRUE(std::is_move_assignable<decltype(e)>::value);
		ASSERT_TRUE(!std::is_trivially_copy_constructible<decltype(e)>::value);
		ASSERT_TRUE(!std::is_trivially_copy_assignable<decltype(e)>::value);
		ASSERT_TRUE(!std::is_trivially_move_constructible<decltype(e)>::value);
		ASSERT_TRUE(!std::is_trivially_move_assignable<decltype(e)>::value);
	}

}

