// Copyright 2020 The Abseil Authors.
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

#include <turbo/status/statusor.h>

#include <any>
#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <map>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <turbo/bits/casts.h>
#include <turbo/memory/memory.h>
#include <turbo/status/status.h>
#include <turbo/status/status_matchers.h>
#include <turbo/strings/str_cat.h>
#include <string_view>
#include <source_location>

namespace {

using ::turbo_testing::IsOk;
using ::turbo_testing::IsOkAndHolds;
using ::testing::AllOf;
using ::testing::AnyOf;
using ::testing::AnyWith;
using ::testing::ElementsAre;
using ::testing::EndsWith;
using ::testing::Field;
using ::testing::HasSubstr;
using ::testing::Ne;
using ::testing::Not;
using ::testing::Pointee;
using ::testing::StartsWith;
using ::testing::VariantWith;

struct CopyDetector {
  CopyDetector() = default;
  explicit CopyDetector(int xx) : x(xx) {}
  CopyDetector(CopyDetector&& d) noexcept
      : x(d.x), copied(false), moved(true) {}
  CopyDetector(const CopyDetector& d) : x(d.x), copied(true), moved(false) {}
  CopyDetector& operator=(const CopyDetector& c) {
    x = c.x;
    copied = true;
    moved = false;
    return *this;
  }
  CopyDetector& operator=(CopyDetector&& c) noexcept {
    x = c.x;
    copied = false;
    moved = true;
    return *this;
  }
  int x = 0;
  bool copied = false;
  bool moved = false;
};

testing::Matcher<const CopyDetector&> CopyDetectorHas(int a, bool b, bool c) {
  return AllOf(Field(&CopyDetector::x, a), Field(&CopyDetector::moved, b),
               Field(&CopyDetector::copied, c));
}

class Base1 {
 public:
  virtual ~Base1() = default;
  int pad;
};

class Base2 {
 public:
  virtual ~Base2() = default;
  int yetotherpad;
};

class Derived : public Base1, public Base2 {
 public:
  ~Derived() override = default;
  int evenmorepad;
};

class CopyNoAssign {
 public:
  explicit CopyNoAssign(int value) : foo(value) {}
  CopyNoAssign(const CopyNoAssign& other) = default;
  int foo;

 private:
  const CopyNoAssign& operator=(const CopyNoAssign&);
};

turbo::StatusOr<std::unique_ptr<int>> ReturnUniquePtr() {
  // Uses implicit constructor from T&&
  return std::make_unique<int>(0);
}

TEST(StatusOr, ElementType) {
  static_assert(std::is_same<turbo::StatusOr<int>::value_type, int>(), "");
  static_assert(std::is_same<turbo::StatusOr<char>::value_type, char>(), "");
}

TEST(StatusOr, TestMoveOnlyInitialization) {
  turbo::StatusOr<std::unique_ptr<int>> thing(ReturnUniquePtr());
  ASSERT_TRUE(thing.ok());
  EXPECT_EQ(0, **thing);
  int* previous = thing->get();

  thing = ReturnUniquePtr();
  EXPECT_TRUE(thing.ok());
  EXPECT_EQ(0, **thing);
  EXPECT_NE(previous, thing->get());
}

TEST(StatusOr, TestMoveOnlyValueExtraction) {
  turbo::StatusOr<std::unique_ptr<int>> thing(ReturnUniquePtr());
  ASSERT_TRUE(thing.ok());
  std::unique_ptr<int> ptr = *std::move(thing);
  EXPECT_EQ(0, *ptr);

  thing = std::move(ptr);
  ptr = std::move(*thing);
  EXPECT_EQ(0, *ptr);
}

TEST(StatusOr, TestMoveOnlyInitializationFromTemporaryByValueOrDie) {
  std::unique_ptr<int> ptr(*ReturnUniquePtr());
  EXPECT_EQ(0, *ptr);
}

TEST(StatusOr, TestValueOrDieOverloadForConstTemporary) {
  static_assert(
      std::is_same<
          const int&&,
          decltype(std::declval<const turbo::StatusOr<int>&&>().value())>(),
      "value() for const temporaries should return const T&&");
}

TEST(StatusOr, TestMoveOnlyConversion) {
  turbo::StatusOr<std::unique_ptr<const int>> const_thing(ReturnUniquePtr());
  EXPECT_TRUE(const_thing.ok());
  EXPECT_EQ(0, **const_thing);

  // Test rvalue converting assignment
  const int* const_previous = const_thing->get();
  const_thing = ReturnUniquePtr();
  EXPECT_TRUE(const_thing.ok());
  EXPECT_EQ(0, **const_thing);
  EXPECT_NE(const_previous, const_thing->get());
}

TEST(StatusOr, TestMoveOnlyVector) {
  // Sanity check that turbo::StatusOr<MoveOnly> works in vector.
  std::vector<turbo::StatusOr<std::unique_ptr<int>>> vec;
  vec.push_back(ReturnUniquePtr());
  vec.resize(2);
  auto another_vec = std::move(vec);
  EXPECT_EQ(0, **another_vec[0]);
  EXPECT_EQ(turbo::UnknownError(""), another_vec[1].status());
}

TEST(StatusOr, TestDefaultCtor) {
  turbo::StatusOr<int> thing;
  EXPECT_FALSE(thing.ok());
  EXPECT_EQ(thing.status().code(), turbo::StatusCode::kUnknown);
}

TEST(StatusOr, StatusCtorForwards) {
  turbo::Status status(turbo::StatusCode::kInternal, "Some error");

  EXPECT_EQ(turbo::StatusOr<int>(status).status().message(), "Some error");
  EXPECT_EQ(status.message(), "Some error");

  EXPECT_EQ(turbo::StatusOr<int>(std::move(status)).status().message(),
            "Some error");
  EXPECT_NE(status.message(), "Some error");
}

TEST(BadStatusOrAccessTest, CopyConstructionWhatOk) {
  turbo::Status error =
      turbo::InternalError("some arbitrary message too big for the sso buffer");
  turbo::BadStatusOrAccess e1{error};
  turbo::BadStatusOrAccess e2{e1};
  EXPECT_THAT(e1.what(), HasSubstr(error.ToString()));
  EXPECT_THAT(e2.what(), HasSubstr(error.ToString()));
}

TEST(BadStatusOrAccessTest, CopyAssignmentWhatOk) {
  turbo::Status error =
      turbo::InternalError("some arbitrary message too big for the sso buffer");
  turbo::BadStatusOrAccess e1{error};
  turbo::BadStatusOrAccess e2{turbo::InternalError("other")};
  e2 = e1;
  EXPECT_THAT(e1.what(), HasSubstr(error.ToString()));
  EXPECT_THAT(e2.what(), HasSubstr(error.ToString()));
}

TEST(BadStatusOrAccessTest, MoveConstructionWhatOk) {
  turbo::Status error =
      turbo::InternalError("some arbitrary message too big for the sso buffer");
  turbo::BadStatusOrAccess e1{error};
  turbo::BadStatusOrAccess e2{std::move(e1)};
  EXPECT_THAT(e2.what(), HasSubstr(error.ToString()));
}

TEST(BadStatusOrAccessTest, MoveAssignmentWhatOk) {
  turbo::Status error =
      turbo::InternalError("some arbitrary message too big for the sso buffer");
  turbo::BadStatusOrAccess e1{error};
  turbo::BadStatusOrAccess e2{turbo::InternalError("other")};
  e2 = std::move(e1);
  EXPECT_THAT(e2.what(), HasSubstr(error.ToString()));
}

// Define `EXPECT_DEATH_OR_THROW` to test the behavior of `StatusOr::value`,
// which either throws `BadStatusOrAccess` or `KLOG(FATAL)` based on whether
// exceptions are enabled.
#if KUMO_HAVE_EXCEPTIONS
#define EXPECT_DEATH_OR_THROW(statement, status_)                  \
  EXPECT_THROW(                                                    \
      {                                                            \
        try {                                                      \
          statement;                                               \
        } catch (const turbo::BadStatusOrAccess& e) {               \
          EXPECT_EQ(e.status(), status_);                          \
          EXPECT_THAT(e.what(), HasSubstr(e.status().ToString())); \
          throw;                                                   \
        }                                                          \
      },                                                           \
      turbo::BadStatusOrAccess);
#else  // KUMO_HAVE_EXCEPTIONS
#define EXPECT_DEATH_OR_THROW(statement, status) \
  EXPECT_DEATH_IF_SUPPORTED(statement, status.ToString());
#endif  // KUMO_HAVE_EXCEPTIONS

TEST(StatusOrDeathTest, TestDefaultCtorValue) {
  turbo::StatusOr<int> thing;
  EXPECT_DEATH_OR_THROW(thing.value(), turbo::UnknownError(""));
  const turbo::StatusOr<int> thing2;
  EXPECT_DEATH_OR_THROW(thing2.value(), turbo::UnknownError(""));
}

TEST(StatusOrDeathTest, TestValueNotOk) {
  turbo::StatusOr<int> thing(turbo::CancelledError());
  EXPECT_DEATH_OR_THROW(thing.value(), turbo::CancelledError());
}

TEST(StatusOrDeathTest, TestValueNotOkConst) {
  const turbo::StatusOr<int> thing(turbo::UnknownError(""));
  EXPECT_DEATH_OR_THROW(thing.value(), turbo::UnknownError(""));
}

TEST(StatusOrDeathTest, TestPointerDefaultCtorValue) {
  turbo::StatusOr<int*> thing;
  EXPECT_DEATH_OR_THROW(thing.value(), turbo::UnknownError(""));
}

TEST(StatusOrDeathTest, TestPointerValueNotOk) {
  turbo::StatusOr<int*> thing(turbo::CancelledError());
  EXPECT_DEATH_OR_THROW(thing.value(), turbo::CancelledError());
}

TEST(StatusOrDeathTest, TestPointerValueNotOkConst) {
  const turbo::StatusOr<int*> thing(turbo::CancelledError());
  EXPECT_DEATH_OR_THROW(thing.value(), turbo::CancelledError());
}

#if GTEST_HAS_DEATH_TEST
TEST(StatusOrDeathTest, TestStatusCtorStatusOk) {
  EXPECT_DEBUG_DEATH(
      {
        // This will DKCHECK
        turbo::StatusOr<int> thing(turbo::OkStatus());
        // In optimized mode, we are actually going to get error::INTERNAL for
        // status here, rather than crashing, so check that.
        EXPECT_FALSE(thing.ok());
        EXPECT_EQ(thing.status().code(), turbo::StatusCode::kInternal);
      },
      "An OK status is not a valid constructor argument");
}

TEST(StatusOrDeathTest, TestPointerStatusCtorStatusOk) {
  EXPECT_DEBUG_DEATH(
      {
        turbo::StatusOr<int*> thing(turbo::OkStatus());
        // In optimized mode, we are actually going to get error::INTERNAL for
        // status here, rather than crashing, so check that.
        EXPECT_FALSE(thing.ok());
        EXPECT_EQ(thing.status().code(), turbo::StatusCode::kInternal);
      },
      "An OK status is not a valid constructor argument");
}
#endif

TEST(StatusOr, ValueAccessor) {
  const int kIntValue = 110;
  {
    turbo::StatusOr<int> status_or(kIntValue);
    EXPECT_EQ(kIntValue, status_or.value());
    EXPECT_EQ(kIntValue, std::move(status_or).value());
  }
  {
    turbo::StatusOr<CopyDetector> status_or(kIntValue);
    EXPECT_THAT(status_or,
                IsOkAndHolds(CopyDetectorHas(kIntValue, false, false)));
    CopyDetector copy_detector = status_or.value();
    EXPECT_THAT(copy_detector, CopyDetectorHas(kIntValue, false, true));
    copy_detector = std::move(status_or).value();
    EXPECT_THAT(copy_detector, CopyDetectorHas(kIntValue, true, false));
  }
}

TEST(StatusOr, BadValueAccess) {
  const turbo::Status kError = turbo::CancelledError("message");
  turbo::StatusOr<int> status_or(kError);
  EXPECT_DEATH_OR_THROW(status_or.value(), kError);
}

TEST(StatusOr, TestStatusCtor) {
  turbo::StatusOr<int> thing(turbo::CancelledError());
  EXPECT_FALSE(thing.ok());
  EXPECT_EQ(thing.status().code(), turbo::StatusCode::kCancelled);
}

TEST(StatusOr, TestValueCtor) {
  const int kI = 4;
  const turbo::StatusOr<int> thing(kI);
  EXPECT_TRUE(thing.ok());
  EXPECT_EQ(kI, *thing);
}

struct Foo {
  const int x;
  explicit Foo(int y) : x(y) {}
};

TEST(StatusOr, InPlaceConstruction) {
  EXPECT_THAT(turbo::StatusOr<Foo>(std::in_place, 10),
              IsOkAndHolds(Field(&Foo::x, 10)));
}

struct InPlaceHelper {
  InPlaceHelper(std::initializer_list<int> xs, std::unique_ptr<int> yy)
      : x(xs), y(std::move(yy)) {}
  const std::vector<int> x;
  std::unique_ptr<int> y;
};

TEST(StatusOr, InPlaceInitListConstruction) {
  turbo::StatusOr<InPlaceHelper> status_or(std::in_place, {10, 11, 12},
                                          std::make_unique<int>(13));
  EXPECT_THAT(status_or, IsOkAndHolds(AllOf(
                             Field(&InPlaceHelper::x, ElementsAre(10, 11, 12)),
                             Field(&InPlaceHelper::y, Pointee(13)))));
}

TEST(StatusOr, Emplace) {
  turbo::StatusOr<Foo> status_or_foo(10);
  status_or_foo.emplace(20);
  EXPECT_THAT(status_or_foo, IsOkAndHolds(Field(&Foo::x, 20)));
  status_or_foo = turbo::InvalidArgumentError("msg");
  EXPECT_FALSE(status_or_foo.ok());
  EXPECT_EQ(status_or_foo.status().code(), turbo::StatusCode::kInvalidArgument);
  EXPECT_EQ(status_or_foo.status().message(), "msg");
  status_or_foo.emplace(20);
  EXPECT_THAT(status_or_foo, IsOkAndHolds(Field(&Foo::x, 20)));
}

TEST(StatusOr, EmplaceInitializerList) {
  turbo::StatusOr<InPlaceHelper> status_or(std::in_place, {10, 11, 12},
                                          std::make_unique<int>(13));
  status_or.emplace({1, 2, 3}, std::make_unique<int>(4));
  EXPECT_THAT(status_or,
              IsOkAndHolds(AllOf(Field(&InPlaceHelper::x, ElementsAre(1, 2, 3)),
                                 Field(&InPlaceHelper::y, Pointee(4)))));
  status_or = turbo::InvalidArgumentError("msg");
  EXPECT_FALSE(status_or.ok());
  EXPECT_EQ(status_or.status().code(), turbo::StatusCode::kInvalidArgument);
  EXPECT_EQ(status_or.status().message(), "msg");
  status_or.emplace({1, 2, 3}, std::make_unique<int>(4));
  EXPECT_THAT(status_or,
              IsOkAndHolds(AllOf(Field(&InPlaceHelper::x, ElementsAre(1, 2, 3)),
                                 Field(&InPlaceHelper::y, Pointee(4)))));
}

#if KUMO_HAVE_EXCEPTIONS
class ThrowOnEmplace {
 public:
  explicit ThrowOnEmplace(int* counter, int val) : destructor_calls_(counter) {
    if (val < 0) {
      throw std::runtime_error("expected");
    }
    // While destructor_calls tracks the logic, ptr_ ensures that a double
    // destruction actually results in a reliable crash. Performing a real heap
    // allocation and deallocation (new/delete) guarantees that AddressSanitizer
    // (ASAN) or the heap allocator will instantly catch the double-free if the
    // bug regresses, rather than relying solely on the integer check.
    ptr_ = new int(val);
  }

  ThrowOnEmplace(const ThrowOnEmplace&) = delete;
  ThrowOnEmplace& operator=(const ThrowOnEmplace&) = delete;

  ~ThrowOnEmplace() {
    if (destructor_calls_) {
      ++(*destructor_calls_);
    }
    delete ptr_;
  }

 private:
  int* destructor_calls_ = nullptr;
  int* ptr_ = nullptr;
};

TEST(StatusOr, EmplaceThrowsExceptionSafety) {
  int destructor_calls = 0;
  {
    turbo::StatusOr<ThrowOnEmplace> status_or(std::in_place, &destructor_calls,
                                             1);
    EXPECT_TRUE(status_or.ok());
    EXPECT_THROW(status_or.emplace(&destructor_calls, -1), std::runtime_error);
    EXPECT_FALSE(status_or.ok());
    EXPECT_EQ(status_or.status().code(), turbo::StatusCode::kInternal);
  }
  // Verifies that the initial object is properly destroyed by Clear() (count is
  // 1), and that the exception thrown during replacement does not cause a
  // second destruction (double-free) during stack unwinding.
  EXPECT_EQ(destructor_calls, 1);
}
#endif  // KUMO_HAVE_EXCEPTIONS

TEST(StatusOr, TestCopyCtorStatusOk) {
  const int kI = 4;
  const turbo::StatusOr<int> original(kI);
  const turbo::StatusOr<int> copy(original);
  EXPECT_THAT(copy.status(), IsOk());
  EXPECT_EQ(*original, *copy);
}

TEST(StatusOr, TestCopyCtorStatusNotOk) {
  turbo::StatusOr<int> original(turbo::CancelledError());
  turbo::StatusOr<int> copy(original);
  EXPECT_EQ(copy.status().code(), turbo::StatusCode::kCancelled);
}

TEST(StatusOr, TestCopyCtorNonAssignable) {
  const int kI = 4;
  CopyNoAssign value(kI);
  turbo::StatusOr<CopyNoAssign> original(value);
  turbo::StatusOr<CopyNoAssign> copy(original);
  EXPECT_THAT(copy.status(), IsOk());
  EXPECT_EQ(original->foo, copy->foo);
}

TEST(StatusOr, TestCopyCtorStatusOKConverting) {
  const int kI = 4;
  turbo::StatusOr<int> original(kI);
  turbo::StatusOr<double> copy(original);
  EXPECT_THAT(copy.status(), IsOk());
  EXPECT_DOUBLE_EQ(*original, *copy);
}

TEST(StatusOr, TestCopyCtorStatusNotOkConverting) {
  turbo::StatusOr<int> original(turbo::CancelledError());
  turbo::StatusOr<double> copy(original);
  EXPECT_EQ(copy.status(), original.status());
}

TEST(StatusOr, TestAssignmentStatusOk) {
  // Copy assignmment
  {
    const auto p = std::make_shared<int>(17);
    turbo::StatusOr<std::shared_ptr<int>> source(p);

    turbo::StatusOr<std::shared_ptr<int>> target;
    target = source;

    ASSERT_TRUE(target.ok());
    EXPECT_THAT(target.status(), IsOk());
    EXPECT_EQ(p, *target);

    ASSERT_TRUE(source.ok());
    EXPECT_THAT(source.status(), IsOk());
    EXPECT_EQ(p, *source);
  }

  // Move assignment
  {
    const auto p = std::make_shared<int>(17);
    turbo::StatusOr<std::shared_ptr<int>> source(p);

    turbo::StatusOr<std::shared_ptr<int>> target;
    target = std::move(source);

    ASSERT_TRUE(target.ok());
    EXPECT_THAT(target.status(), IsOk());
    EXPECT_EQ(p, *target);

    ASSERT_TRUE(source.ok());
    EXPECT_THAT(source.status(), IsOk());
    EXPECT_EQ(nullptr, *source);
  }
}

TEST(StatusOr, TestAssignmentStatusNotOk) {
  // Copy assignment
  {
    const turbo::Status expected = turbo::CancelledError();
    turbo::StatusOr<int> source(expected);

    turbo::StatusOr<int> target;
    target = source;

    EXPECT_FALSE(target.ok());
    EXPECT_EQ(expected, target.status());

    EXPECT_FALSE(source.ok());
    EXPECT_EQ(expected, source.status());
  }

  // Move assignment
  {
    const turbo::Status expected = turbo::CancelledError();
    turbo::StatusOr<int> source(expected);

    turbo::StatusOr<int> target;
    target = std::move(source);

    EXPECT_FALSE(target.ok());
    EXPECT_EQ(expected, target.status());

    EXPECT_FALSE(source.ok());
    EXPECT_EQ(source.status().code(), turbo::StatusCode::kInternal);
  }
}

TEST(StatusOr, TestAssignmentStatusOKConverting) {
  // Copy assignment
  {
    const int kI = 4;
    turbo::StatusOr<int> source(kI);

    turbo::StatusOr<double> target;
    target = source;

    ASSERT_TRUE(target.ok());
    EXPECT_THAT(target.status(), IsOk());
    EXPECT_DOUBLE_EQ(kI, *target);

    ASSERT_TRUE(source.ok());
    EXPECT_THAT(source.status(), IsOk());
    EXPECT_DOUBLE_EQ(kI, *source);
  }

  // Move assignment
  {
    const auto p = new int(17);
    turbo::StatusOr<std::unique_ptr<int>> source(turbo::WrapUnique(p));

    turbo::StatusOr<std::shared_ptr<int>> target;
    target = std::move(source);

    ASSERT_TRUE(target.ok());
    EXPECT_THAT(target.status(), IsOk());
    EXPECT_EQ(p, target->get());

    ASSERT_TRUE(source.ok());
    EXPECT_THAT(source.status(), IsOk());
    EXPECT_EQ(nullptr, source->get());
  }
}

struct A {
  int x;
};

struct ImplicitConstructibleFromA {
  int x;
  bool moved;
  ImplicitConstructibleFromA(const A& a)  // NOLINT
      : x(a.x), moved(false) {}
  ImplicitConstructibleFromA(A&& a)  // NOLINT
      : x(a.x), moved(true) {}
};

TEST(StatusOr, ImplicitConvertingConstructor) {
  EXPECT_THAT(
      turbo::implicit_cast<turbo::StatusOr<ImplicitConstructibleFromA>>(
          turbo::StatusOr<A>(A{11})),
      IsOkAndHolds(AllOf(Field(&ImplicitConstructibleFromA::x, 11),
                         Field(&ImplicitConstructibleFromA::moved, true))));
  turbo::StatusOr<A> a(A{12});
  EXPECT_THAT(
      turbo::implicit_cast<turbo::StatusOr<ImplicitConstructibleFromA>>(a),
      IsOkAndHolds(AllOf(Field(&ImplicitConstructibleFromA::x, 12),
                         Field(&ImplicitConstructibleFromA::moved, false))));
}

struct ExplicitConstructibleFromA {
  int x;
  bool moved;
  explicit ExplicitConstructibleFromA(const A& a) : x(a.x), moved(false) {}
  explicit ExplicitConstructibleFromA(A&& a) : x(a.x), moved(true) {}
};

TEST(StatusOr, ExplicitConvertingConstructor) {
  EXPECT_FALSE(
      (std::is_convertible_v<const turbo::StatusOr<A>&,
                             turbo::StatusOr<ExplicitConstructibleFromA>>));
  EXPECT_FALSE(
      (std::is_convertible_v<turbo::StatusOr<A>&&,
                             turbo::StatusOr<ExplicitConstructibleFromA>>));
  EXPECT_THAT(
      turbo::StatusOr<ExplicitConstructibleFromA>(turbo::StatusOr<A>(A{11})),
      IsOkAndHolds(AllOf(Field(&ExplicitConstructibleFromA::x, 11),
                         Field(&ExplicitConstructibleFromA::moved, true))));
  turbo::StatusOr<A> a(A{12});
  EXPECT_THAT(
      turbo::StatusOr<ExplicitConstructibleFromA>(a),
      IsOkAndHolds(AllOf(Field(&ExplicitConstructibleFromA::x, 12),
                         Field(&ExplicitConstructibleFromA::moved, false))));
}

struct ImplicitConstructibleFromBool {
  ImplicitConstructibleFromBool(bool y) : x(y) {}  // NOLINT
  bool x = false;
};

struct ConvertibleToBool {
  explicit ConvertibleToBool(bool y) : x(y) {}
  operator bool() const { return x; }  // NOLINT
  bool x = false;
};

TEST(StatusOr, ImplicitBooleanConstructionWithImplicitCasts) {
  EXPECT_THAT(turbo::StatusOr<bool>(turbo::StatusOr<ConvertibleToBool>(true)),
              IsOkAndHolds(true));
  EXPECT_THAT(turbo::StatusOr<bool>(turbo::StatusOr<ConvertibleToBool>(false)),
              IsOkAndHolds(false));
  EXPECT_THAT(
      turbo::implicit_cast<turbo::StatusOr<ImplicitConstructibleFromBool>>(
          turbo::StatusOr<bool>(false)),
      IsOkAndHolds(Field(&ImplicitConstructibleFromBool::x, false)));
  EXPECT_FALSE(
      (std::is_convertible_v<turbo::StatusOr<ConvertibleToBool>,
                             turbo::StatusOr<ImplicitConstructibleFromBool>>));
}

TEST(StatusOr, BooleanConstructionWithImplicitCasts) {
  EXPECT_THAT(turbo::StatusOr<bool>(turbo::StatusOr<ConvertibleToBool>(true)),
              IsOkAndHolds(true));
  EXPECT_THAT(turbo::StatusOr<bool>(turbo::StatusOr<ConvertibleToBool>(false)),
              IsOkAndHolds(false));
  EXPECT_THAT(
      turbo::StatusOr<ImplicitConstructibleFromBool>{
          turbo::StatusOr<bool>(false)},
      IsOkAndHolds(Field(&ImplicitConstructibleFromBool::x, false)));
  EXPECT_THAT(
      turbo::StatusOr<ImplicitConstructibleFromBool>{
          turbo::StatusOr<bool>(turbo::InvalidArgumentError(""))},
      Not(IsOk()));

  EXPECT_THAT(
      turbo::StatusOr<ImplicitConstructibleFromBool>{
          turbo::StatusOr<ConvertibleToBool>(ConvertibleToBool{false})},
      IsOkAndHolds(Field(&ImplicitConstructibleFromBool::x, false)));
  EXPECT_THAT(
      turbo::StatusOr<ImplicitConstructibleFromBool>{
          turbo::StatusOr<ConvertibleToBool>(turbo::InvalidArgumentError(""))},
      Not(IsOk()));
}

TEST(StatusOr, ConstImplicitCast) {
  EXPECT_THAT(turbo::implicit_cast<turbo::StatusOr<bool>>(
                  turbo::StatusOr<const bool>(true)),
              IsOkAndHolds(true));
  EXPECT_THAT(turbo::implicit_cast<turbo::StatusOr<bool>>(
                  turbo::StatusOr<const bool>(false)),
              IsOkAndHolds(false));
  EXPECT_THAT(turbo::implicit_cast<turbo::StatusOr<const bool>>(
                  turbo::StatusOr<bool>(true)),
              IsOkAndHolds(true));
  EXPECT_THAT(turbo::implicit_cast<turbo::StatusOr<const bool>>(
                  turbo::StatusOr<bool>(false)),
              IsOkAndHolds(false));
  EXPECT_THAT(turbo::implicit_cast<turbo::StatusOr<const std::string>>(
                  turbo::StatusOr<std::string>("foo")),
              IsOkAndHolds("foo"));
  EXPECT_THAT(turbo::implicit_cast<turbo::StatusOr<std::string>>(
                  turbo::StatusOr<const std::string>("foo")),
              IsOkAndHolds("foo"));
  EXPECT_THAT(
      turbo::implicit_cast<turbo::StatusOr<std::shared_ptr<const std::string>>>(
          turbo::StatusOr<std::shared_ptr<std::string>>(
              std::make_shared<std::string>("foo"))),
      IsOkAndHolds(Pointee(std::string("foo"))));
}

TEST(StatusOr, ConstExplicitConstruction) {
  EXPECT_THAT(turbo::StatusOr<bool>(turbo::StatusOr<const bool>(true)),
              IsOkAndHolds(true));
  EXPECT_THAT(turbo::StatusOr<bool>(turbo::StatusOr<const bool>(false)),
              IsOkAndHolds(false));
  EXPECT_THAT(turbo::StatusOr<const bool>(turbo::StatusOr<bool>(true)),
              IsOkAndHolds(true));
  EXPECT_THAT(turbo::StatusOr<const bool>(turbo::StatusOr<bool>(false)),
              IsOkAndHolds(false));
}

struct ExplicitConstructibleFromInt {
  int x;
  explicit ExplicitConstructibleFromInt(int y) : x(y) {}
};

TEST(StatusOr, ExplicitConstruction) {
  EXPECT_THAT(turbo::StatusOr<ExplicitConstructibleFromInt>(10),
              IsOkAndHolds(Field(&ExplicitConstructibleFromInt::x, 10)));
}

TEST(StatusOr, ImplicitConstruction) {
  // Check implicit casting works.
  auto status_or =
      turbo::implicit_cast<turbo::StatusOr<std::variant<int, std::string>>>(10);
  EXPECT_THAT(status_or, IsOkAndHolds(VariantWith<int>(10)));
}

TEST(StatusOr, ImplicitConstructionFromInitliazerList) {
  // Note: dropping the explicit std::initializer_list<int> is not supported
  // by turbo::StatusOr or std::optional.
  auto status_or =
      turbo::implicit_cast<turbo::StatusOr<std::vector<int>>>({{10, 20, 30}});
  EXPECT_THAT(status_or, IsOkAndHolds(ElementsAre(10, 20, 30)));
}

TEST(StatusOr, UniquePtrImplicitConstruction) {
  auto status_or = turbo::implicit_cast<turbo::StatusOr<std::unique_ptr<Base1>>>(
      std::make_unique<Derived>());
  EXPECT_THAT(status_or, IsOkAndHolds(Ne(nullptr)));
}

TEST(StatusOr, NestedStatusOrCopyAndMoveConstructorTests) {
  turbo::StatusOr<turbo::StatusOr<CopyDetector>> status_or = CopyDetector(10);
  turbo::StatusOr<turbo::StatusOr<CopyDetector>> status_error =
      turbo::InvalidArgumentError("foo");
  EXPECT_THAT(status_or,
              IsOkAndHolds(IsOkAndHolds(CopyDetectorHas(10, true, false))));
  turbo::StatusOr<turbo::StatusOr<CopyDetector>> a = status_or;
  EXPECT_THAT(a, IsOkAndHolds(IsOkAndHolds(CopyDetectorHas(10, false, true))));
  turbo::StatusOr<turbo::StatusOr<CopyDetector>> a_err = status_error;
  EXPECT_THAT(a_err, Not(IsOk()));

  const turbo::StatusOr<turbo::StatusOr<CopyDetector>>& cref = status_or;
  turbo::StatusOr<turbo::StatusOr<CopyDetector>> b = cref;  // NOLINT
  EXPECT_THAT(b, IsOkAndHolds(IsOkAndHolds(CopyDetectorHas(10, false, true))));
  const turbo::StatusOr<turbo::StatusOr<CopyDetector>>& cref_err = status_error;
  turbo::StatusOr<turbo::StatusOr<CopyDetector>> b_err = cref_err;  // NOLINT
  EXPECT_THAT(b_err, Not(IsOk()));

  turbo::StatusOr<turbo::StatusOr<CopyDetector>> c = std::move(status_or);
  EXPECT_THAT(c, IsOkAndHolds(IsOkAndHolds(CopyDetectorHas(10, true, false))));
  turbo::StatusOr<turbo::StatusOr<CopyDetector>> c_err = std::move(status_error);
  EXPECT_THAT(c_err, Not(IsOk()));
}

TEST(StatusOr, NestedStatusOrCopyAndMoveAssignment) {
  turbo::StatusOr<turbo::StatusOr<CopyDetector>> status_or = CopyDetector(10);
  turbo::StatusOr<turbo::StatusOr<CopyDetector>> status_error =
      turbo::InvalidArgumentError("foo");
  turbo::StatusOr<turbo::StatusOr<CopyDetector>> a;
  a = status_or;
  EXPECT_THAT(a, IsOkAndHolds(IsOkAndHolds(CopyDetectorHas(10, false, true))));
  a = status_error;
  EXPECT_THAT(a, Not(IsOk()));

  const turbo::StatusOr<turbo::StatusOr<CopyDetector>>& cref = status_or;
  a = cref;
  EXPECT_THAT(a, IsOkAndHolds(IsOkAndHolds(CopyDetectorHas(10, false, true))));
  const turbo::StatusOr<turbo::StatusOr<CopyDetector>>& cref_err = status_error;
  a = cref_err;
  EXPECT_THAT(a, Not(IsOk()));
  a = std::move(status_or);
  EXPECT_THAT(a, IsOkAndHolds(IsOkAndHolds(CopyDetectorHas(10, true, false))));
  a = std::move(status_error);
  EXPECT_THAT(a, Not(IsOk()));
}

struct Copyable {
  Copyable() = default;
  Copyable(const Copyable&) = default;
  Copyable& operator=(const Copyable&) = default;
};

struct MoveOnly {
  MoveOnly() = default;
  MoveOnly(MoveOnly&&) {}
  MoveOnly& operator=(MoveOnly&&) { return *this; }
};

struct NonMovable {
  NonMovable() = default;
  NonMovable(const NonMovable&) = delete;
  NonMovable(NonMovable&&) = delete;
  NonMovable& operator=(const NonMovable&) = delete;
  NonMovable& operator=(NonMovable&&) = delete;
};

TEST(StatusOr, CopyAndMoveAbility) {
  EXPECT_TRUE(std::is_copy_constructible_v<Copyable>);
  EXPECT_TRUE(std::is_copy_assignable_v<Copyable>);
  EXPECT_TRUE(std::is_move_constructible_v<Copyable>);
  EXPECT_TRUE(std::is_move_assignable_v<Copyable>);
  EXPECT_FALSE(std::is_copy_constructible_v<MoveOnly>);
  EXPECT_FALSE(std::is_copy_assignable_v<MoveOnly>);
  EXPECT_TRUE(std::is_move_constructible_v<MoveOnly>);
  EXPECT_TRUE(std::is_move_assignable_v<MoveOnly>);
  EXPECT_FALSE(std::is_copy_constructible_v<NonMovable>);
  EXPECT_FALSE(std::is_copy_assignable_v<NonMovable>);
  EXPECT_FALSE(std::is_move_constructible_v<NonMovable>);
  EXPECT_FALSE(std::is_move_assignable_v<NonMovable>);
}

TEST(StatusOr, StatusOrAnyCopyAndMoveConstructorTests) {
  turbo::StatusOr<std::any> status_or = CopyDetector(10);
  turbo::StatusOr<std::any> status_error = turbo::InvalidArgumentError("foo");
  EXPECT_THAT(
      status_or,
      IsOkAndHolds(AnyWith<CopyDetector>(CopyDetectorHas(10, true, false))));
  turbo::StatusOr<std::any> a = status_or;
  EXPECT_THAT(
      a, IsOkAndHolds(AnyWith<CopyDetector>(CopyDetectorHas(10, false, true))));
  turbo::StatusOr<std::any> a_err = status_error;
  EXPECT_THAT(a_err, Not(IsOk()));

  const turbo::StatusOr<std::any>& cref = status_or;
  // No lint for no-change copy.
  turbo::StatusOr<std::any> b = cref;  // NOLINT
  EXPECT_THAT(
      b, IsOkAndHolds(AnyWith<CopyDetector>(CopyDetectorHas(10, false, true))));
  const turbo::StatusOr<std::any>& cref_err = status_error;
  // No lint for no-change copy.
  turbo::StatusOr<std::any> b_err = cref_err;  // NOLINT
  EXPECT_THAT(b_err, Not(IsOk()));

  turbo::StatusOr<std::any> c = std::move(status_or);
  EXPECT_THAT(
      c, IsOkAndHolds(AnyWith<CopyDetector>(CopyDetectorHas(10, true, false))));
  turbo::StatusOr<std::any> c_err = std::move(status_error);
  EXPECT_THAT(c_err, Not(IsOk()));
}

TEST(StatusOr, StatusOrAnyCopyAndMoveAssignment) {
  turbo::StatusOr<std::any> status_or = CopyDetector(10);
  turbo::StatusOr<std::any> status_error = turbo::InvalidArgumentError("foo");
  turbo::StatusOr<std::any> a;
  a = status_or;
  EXPECT_THAT(
      a, IsOkAndHolds(AnyWith<CopyDetector>(CopyDetectorHas(10, false, true))));
  a = status_error;
  EXPECT_THAT(a, Not(IsOk()));

  const turbo::StatusOr<std::any>& cref = status_or;
  a = cref;
  EXPECT_THAT(
      a, IsOkAndHolds(AnyWith<CopyDetector>(CopyDetectorHas(10, false, true))));
  const turbo::StatusOr<std::any>& cref_err = status_error;
  a = cref_err;
  EXPECT_THAT(a, Not(IsOk()));
  a = std::move(status_or);
  EXPECT_THAT(
      a, IsOkAndHolds(AnyWith<CopyDetector>(CopyDetectorHas(10, true, false))));
  a = std::move(status_error);
  EXPECT_THAT(a, Not(IsOk()));
}

TEST(StatusOr, StatusOrCopyAndMoveTestsConstructor) {
  turbo::StatusOr<CopyDetector> status_or(10);
  ASSERT_THAT(status_or, IsOkAndHolds(CopyDetectorHas(10, false, false)));
  turbo::StatusOr<CopyDetector> a(status_or);
  EXPECT_THAT(a, IsOkAndHolds(CopyDetectorHas(10, false, true)));
  const turbo::StatusOr<CopyDetector>& cref = status_or;
  turbo::StatusOr<CopyDetector> b(cref);  // NOLINT
  EXPECT_THAT(b, IsOkAndHolds(CopyDetectorHas(10, false, true)));
  turbo::StatusOr<CopyDetector> c(std::move(status_or));
  EXPECT_THAT(c, IsOkAndHolds(CopyDetectorHas(10, true, false)));
}

TEST(StatusOr, StatusOrCopyAndMoveTestsAssignment) {
  turbo::StatusOr<CopyDetector> status_or(10);
  ASSERT_THAT(status_or, IsOkAndHolds(CopyDetectorHas(10, false, false)));
  turbo::StatusOr<CopyDetector> a;
  a = status_or;
  EXPECT_THAT(a, IsOkAndHolds(CopyDetectorHas(10, false, true)));
  const turbo::StatusOr<CopyDetector>& cref = status_or;
  turbo::StatusOr<CopyDetector> b;
  b = cref;
  EXPECT_THAT(b, IsOkAndHolds(CopyDetectorHas(10, false, true)));
  turbo::StatusOr<CopyDetector> c;
  c = std::move(status_or);
  EXPECT_THAT(c, IsOkAndHolds(CopyDetectorHas(10, true, false)));
}

TEST(StatusOr, TurboAnyAssignment) {
  EXPECT_FALSE(
      (std::is_assignable_v<turbo::StatusOr<std::any>, turbo::StatusOr<int>>));
  turbo::StatusOr<std::any> status_or;
  status_or = turbo::InvalidArgumentError("foo");
  EXPECT_THAT(status_or, Not(IsOk()));
}

TEST(StatusOr, ImplicitAssignment) {
  turbo::StatusOr<std::variant<int, std::string>> status_or;
  status_or = 10;
  EXPECT_THAT(status_or, IsOkAndHolds(VariantWith<int>(10)));
}

TEST(StatusOr, SelfDirectInitAssignment) {
  turbo::StatusOr<std::vector<int>> status_or = {{10, 20, 30}};
  status_or = *status_or;
  EXPECT_THAT(status_or, IsOkAndHolds(ElementsAre(10, 20, 30)));
}

TEST(StatusOr, ImplicitCastFromInitializerList) {
  turbo::StatusOr<std::vector<int>> status_or = {{10, 20, 30}};
  EXPECT_THAT(status_or, IsOkAndHolds(ElementsAre(10, 20, 30)));
}

TEST(StatusOr, UniquePtrImplicitAssignment) {
  turbo::StatusOr<std::unique_ptr<Base1>> status_or;
  status_or = std::make_unique<Derived>();
  EXPECT_THAT(status_or, IsOkAndHolds(Ne(nullptr)));
}

TEST(StatusOr, Pointer) {
  struct A {};
  struct B : public A {};
  struct C : private A {};

  EXPECT_TRUE((std::is_constructible_v<turbo::StatusOr<A*>, B*>));
  EXPECT_TRUE((std::is_convertible_v<B*, turbo::StatusOr<A*>>));
  EXPECT_FALSE((std::is_constructible_v<turbo::StatusOr<A*>, C*>));
  EXPECT_FALSE((std::is_convertible_v<C*, turbo::StatusOr<A*>>));
}

TEST(StatusOr, TestAssignmentStatusNotOkConverting) {
  // Copy assignment
  {
    const turbo::Status expected = turbo::CancelledError();
    turbo::StatusOr<int> source(expected);

    turbo::StatusOr<double> target;
    target = source;

    EXPECT_FALSE(target.ok());
    EXPECT_EQ(expected, target.status());

    EXPECT_FALSE(source.ok());
    EXPECT_EQ(expected, source.status());
  }

  // Move assignment
  {
    const turbo::Status expected = turbo::CancelledError();
    turbo::StatusOr<int> source(expected);

    turbo::StatusOr<double> target;
    target = std::move(source);

    EXPECT_FALSE(target.ok());
    EXPECT_EQ(expected, target.status());

    EXPECT_FALSE(source.ok());
    EXPECT_EQ(source.status().code(), turbo::StatusCode::kInternal);
  }
}

TEST(StatusOr, SelfAssignment) {
  // Copy-assignment, status OK
  {
    // A string long enough that it's likely to defeat any inline representation
    // optimization.
    const std::string long_str(128, 'a');

    turbo::StatusOr<std::string> so = long_str;
    so = *&so;

    ASSERT_TRUE(so.ok());
    EXPECT_THAT(so.status(), IsOk());
    EXPECT_EQ(long_str, *so);
  }

  // Copy-assignment, error status
  {
    turbo::StatusOr<int> so = turbo::NotFoundError("taco");
    so = *&so;

    EXPECT_FALSE(so.ok());
    EXPECT_EQ(so.status().code(), turbo::StatusCode::kNotFound);
    EXPECT_EQ(so.status().message(), "taco");
  }

  // Move-assignment with copyable type, status OK
  {
    turbo::StatusOr<int> so = 17;

    // Fool the compiler, which otherwise complains.
    auto& same = so;
    so = std::move(same);

    ASSERT_TRUE(so.ok());
    EXPECT_THAT(so.status(), IsOk());
    EXPECT_EQ(17, *so);
  }

  // Move-assignment with copyable type, error status
  {
    turbo::StatusOr<int> so = turbo::NotFoundError("taco");

    // Fool the compiler, which otherwise complains.
    auto& same = so;
    so = std::move(same);

    EXPECT_FALSE(so.ok());
    EXPECT_EQ(so.status().code(), turbo::StatusCode::kNotFound);
    EXPECT_EQ(so.status().message(), "taco");
  }

  // Move-assignment with non-copyable type, status OK
  {
    const auto raw = new int(17);
    turbo::StatusOr<std::unique_ptr<int>> so = turbo::WrapUnique(raw);

    // Fool the compiler, which otherwise complains.
    auto& same = so;
    so = std::move(same);

    ASSERT_TRUE(so.ok());
    EXPECT_THAT(so.status(), IsOk());
    EXPECT_EQ(raw, so->get());
  }

  // Move-assignment with non-copyable type, error status
  {
    turbo::StatusOr<std::unique_ptr<int>> so = turbo::NotFoundError("taco");

    // Fool the compiler, which otherwise complains.
    auto& same = so;
    so = std::move(same);

    EXPECT_FALSE(so.ok());
    EXPECT_EQ(so.status().code(), turbo::StatusCode::kNotFound);
    EXPECT_EQ(so.status().message(), "taco");
  }
}

// These types form the overload sets of the constructors and the assignment
// operators of `MockValue`. They distinguish construction from assignment,
// lvalue from rvalue.
struct FromConstructibleAssignableLvalue {};
struct FromConstructibleAssignableRvalue {};
struct FromImplicitConstructibleOnly {};
struct FromAssignableOnly {};

// This class is for testing the forwarding value assignments of `StatusOr`.
// `from_rvalue` indicates whether the constructor or the assignment taking
// rvalue reference is called. `from_assignment` indicates whether any
// assignment is called.
struct MockValue {
  // Constructs `MockValue` from `FromConstructibleAssignableLvalue`.
  MockValue(const FromConstructibleAssignableLvalue&)  // NOLINT
      : from_rvalue(false), assigned(false) {}
  // Constructs `MockValue` from `FromConstructibleAssignableRvalue`.
  MockValue(FromConstructibleAssignableRvalue&&)  // NOLINT
      : from_rvalue(true), assigned(false) {}
  // Constructs `MockValue` from `FromImplicitConstructibleOnly`.
  // `MockValue` is not assignable from `FromImplicitConstructibleOnly`.
  MockValue(const FromImplicitConstructibleOnly&)  // NOLINT
      : from_rvalue(false), assigned(false) {}
  // Assigns `FromConstructibleAssignableLvalue`.
  MockValue& operator=(const FromConstructibleAssignableLvalue&) {
    from_rvalue = false;
    assigned = true;
    return *this;
  }
  // Assigns `FromConstructibleAssignableRvalue` (rvalue only).
  MockValue& operator=(FromConstructibleAssignableRvalue&&) {
    from_rvalue = true;
    assigned = true;
    return *this;
  }
  // Assigns `FromAssignableOnly`, but not constructible from
  // `FromAssignableOnly`.
  MockValue& operator=(const FromAssignableOnly&) {
    from_rvalue = false;
    assigned = true;
    return *this;
  }
  bool from_rvalue;
  bool assigned;
};

// operator=(U&&)
TEST(StatusOr, PerfectForwardingAssignment) {
  // U == T
  constexpr int kValue1 = 10, kValue2 = 20;
  turbo::StatusOr<CopyDetector> status_or;
  CopyDetector lvalue(kValue1);
  status_or = lvalue;
  EXPECT_THAT(status_or, IsOkAndHolds(CopyDetectorHas(kValue1, false, true)));
  status_or = CopyDetector(kValue2);
  EXPECT_THAT(status_or, IsOkAndHolds(CopyDetectorHas(kValue2, true, false)));

  // U != T
  EXPECT_TRUE((std::is_assignable_v<turbo::StatusOr<MockValue>&,
                                    const FromConstructibleAssignableLvalue&>));
  EXPECT_TRUE((std::is_assignable_v<turbo::StatusOr<MockValue>&,
                                    FromConstructibleAssignableLvalue&&>));
  EXPECT_FALSE(
      (std::is_assignable_v<turbo::StatusOr<MockValue>&,
                            const FromConstructibleAssignableRvalue&>));
  EXPECT_TRUE((std::is_assignable_v<turbo::StatusOr<MockValue>&,
                                    FromConstructibleAssignableRvalue&&>));
  EXPECT_TRUE((std::is_assignable_v<turbo::StatusOr<MockValue>&,
                                    const FromImplicitConstructibleOnly&>));
  EXPECT_FALSE((std::is_assignable_v<turbo::StatusOr<MockValue>&,
                                     const FromAssignableOnly&>));

  turbo::StatusOr<MockValue> from_lvalue(FromConstructibleAssignableLvalue{});
  EXPECT_FALSE(from_lvalue->from_rvalue);
  EXPECT_FALSE(from_lvalue->assigned);
  from_lvalue = FromConstructibleAssignableLvalue{};
  EXPECT_FALSE(from_lvalue->from_rvalue);
  EXPECT_TRUE(from_lvalue->assigned);

  turbo::StatusOr<MockValue> from_rvalue(FromConstructibleAssignableRvalue{});
  EXPECT_TRUE(from_rvalue->from_rvalue);
  EXPECT_FALSE(from_rvalue->assigned);
  from_rvalue = FromConstructibleAssignableRvalue{};
  EXPECT_TRUE(from_rvalue->from_rvalue);
  EXPECT_TRUE(from_rvalue->assigned);

  turbo::StatusOr<MockValue> from_implicit_constructible(
      FromImplicitConstructibleOnly{});
  EXPECT_FALSE(from_implicit_constructible->from_rvalue);
  EXPECT_FALSE(from_implicit_constructible->assigned);
  // construct a temporary `StatusOr` object and invoke the `StatusOr` move
  // assignment operator.
  from_implicit_constructible = FromImplicitConstructibleOnly{};
  EXPECT_FALSE(from_implicit_constructible->from_rvalue);
  EXPECT_FALSE(from_implicit_constructible->assigned);
}

TEST(StatusOr, TestStatus) {
  turbo::StatusOr<int> good(4);
  EXPECT_TRUE(good.ok());
  turbo::StatusOr<int> bad(turbo::CancelledError());
  EXPECT_FALSE(bad.ok());
  EXPECT_EQ(bad.status().code(), turbo::StatusCode::kCancelled);
}

TEST(StatusOr, OperatorStarRefQualifiers) {
  static_assert(
      std::is_same<const int&,
                   decltype(*std::declval<const turbo::StatusOr<int>&>())>(),
      "Unexpected ref-qualifiers");
  static_assert(
      std::is_same<int&, decltype(*std::declval<turbo::StatusOr<int>&>())>(),
      "Unexpected ref-qualifiers");
  static_assert(
      std::is_same<const int&&,
                   decltype(*std::declval<const turbo::StatusOr<int>&&>())>(),
      "Unexpected ref-qualifiers");
  static_assert(
      std::is_same<int&&, decltype(*std::declval<turbo::StatusOr<int>&&>())>(),
      "Unexpected ref-qualifiers");
}

TEST(StatusOr, OperatorStar) {
  const turbo::StatusOr<std::string> const_lvalue("hello");
  EXPECT_EQ("hello", *const_lvalue);

  turbo::StatusOr<std::string> lvalue("hello");
  EXPECT_EQ("hello", *lvalue);

  // Note: Recall that std::move() is equivalent to a static_cast to an rvalue
  // reference type.
  const turbo::StatusOr<std::string> const_rvalue("hello");
  EXPECT_EQ("hello", *std::move(const_rvalue));  // NOLINT

  turbo::StatusOr<std::string> rvalue("hello");
  EXPECT_EQ("hello", *std::move(rvalue));
}

TEST(StatusOr, OperatorArrowQualifiers) {
  static_assert(
      std::is_same<
          const int*,
          decltype(std::declval<const turbo::StatusOr<int>&>().operator->())>(),
      "Unexpected qualifiers");
  static_assert(
      std::is_same<
          int*, decltype(std::declval<turbo::StatusOr<int>&>().operator->())>(),
      "Unexpected qualifiers");
  static_assert(
      std::is_same<
          const int*,
          decltype(std::declval<const turbo::StatusOr<int>&&>().operator->())>(),
      "Unexpected qualifiers");
  static_assert(
      std::is_same<
          int*, decltype(std::declval<turbo::StatusOr<int>&&>().operator->())>(),
      "Unexpected qualifiers");
}

TEST(StatusOr, OperatorArrow) {
  const turbo::StatusOr<std::string> const_lvalue("hello");
  EXPECT_EQ(std::string("hello"), const_lvalue->c_str());

  turbo::StatusOr<std::string> lvalue("hello");
  EXPECT_EQ(std::string("hello"), lvalue->c_str());
}

TEST(StatusOr, RValueStatus) {
  turbo::StatusOr<int> so(turbo::NotFoundError("taco"));
  const turbo::Status s = std::move(so).status();

  EXPECT_EQ(s.code(), turbo::StatusCode::kNotFound);
  EXPECT_EQ(s.message(), "taco");

  // Check that !ok() still implies !status().ok(), even after moving out of the
  // object. See the note on the rvalue ref-qualified status method.
  EXPECT_FALSE(so.ok());  // NOLINT
  EXPECT_FALSE(so.status().ok());
  EXPECT_EQ(so.status().code(), turbo::StatusCode::kInternal);
  EXPECT_EQ(so.status().message(), "Status accessed after move.");
}

TEST(StatusOr, TestValue) {
  const int kI = 4;
  turbo::StatusOr<int> thing(kI);
  EXPECT_EQ(kI, *thing);
}

TEST(StatusOr, TestValueConst) {
  const int kI = 4;
  const turbo::StatusOr<int> thing(kI);
  EXPECT_EQ(kI, *thing);
}

TEST(StatusOr, TestPointerDefaultCtor) {
  turbo::StatusOr<int*> thing;
  EXPECT_FALSE(thing.ok());
  EXPECT_EQ(thing.status().code(), turbo::StatusCode::kUnknown);
}

TEST(StatusOr, TestPointerStatusCtor) {
  turbo::StatusOr<int*> thing(turbo::CancelledError());
  EXPECT_FALSE(thing.ok());
  EXPECT_EQ(thing.status().code(), turbo::StatusCode::kCancelled);
}

TEST(StatusOr, TestPointerValueCtor) {
  const int kI = 4;

  // Construction from a non-null pointer
  {
    turbo::StatusOr<const int*> so(&kI);
    EXPECT_TRUE(so.ok());
    EXPECT_THAT(so.status(), IsOk());
    EXPECT_EQ(&kI, *so);
  }

  // Construction from a null pointer constant
  {
    turbo::StatusOr<const int*> so(nullptr);
    EXPECT_TRUE(so.ok());
    EXPECT_THAT(so.status(), IsOk());
    EXPECT_EQ(nullptr, *so);
  }

  // Construction from a non-literal null pointer
  {
    const int* const p = nullptr;

    turbo::StatusOr<const int*> so(p);
    EXPECT_TRUE(so.ok());
    EXPECT_THAT(so.status(), IsOk());
    EXPECT_EQ(nullptr, *so);
  }
}

TEST(StatusOr, TestPointerCopyCtorStatusOk) {
  const int kI = 0;
  turbo::StatusOr<const int*> original(&kI);
  turbo::StatusOr<const int*> copy(original);
  EXPECT_THAT(copy.status(), IsOk());
  EXPECT_EQ(*original, *copy);
}

TEST(StatusOr, TestPointerCopyCtorStatusNotOk) {
  turbo::StatusOr<int*> original(turbo::CancelledError());
  turbo::StatusOr<int*> copy(original);
  EXPECT_EQ(copy.status().code(), turbo::StatusCode::kCancelled);
}

TEST(StatusOr, TestPointerCopyCtorStatusOKConverting) {
  Derived derived;
  turbo::StatusOr<Derived*> original(&derived);
  turbo::StatusOr<Base2*> copy(original);
  EXPECT_THAT(copy.status(), IsOk());
  EXPECT_EQ(static_cast<const Base2*>(*original), *copy);
}

TEST(StatusOr, TestPointerCopyCtorStatusNotOkConverting) {
  turbo::StatusOr<Derived*> original(turbo::CancelledError());
  turbo::StatusOr<Base2*> copy(original);
  EXPECT_EQ(copy.status().code(), turbo::StatusCode::kCancelled);
}

TEST(StatusOr, TestPointerAssignmentStatusOk) {
  const int kI = 0;
  turbo::StatusOr<const int*> source(&kI);
  turbo::StatusOr<const int*> target;
  target = source;
  EXPECT_THAT(target.status(), IsOk());
  EXPECT_EQ(*source, *target);
}

TEST(StatusOr, TestPointerAssignmentStatusNotOk) {
  turbo::StatusOr<int*> source(turbo::CancelledError());
  turbo::StatusOr<int*> target;
  target = source;
  EXPECT_EQ(target.status().code(), turbo::StatusCode::kCancelled);
}

TEST(StatusOr, TestPointerAssignmentStatusOKConverting) {
  Derived derived;
  turbo::StatusOr<Derived*> source(&derived);
  turbo::StatusOr<Base2*> target;
  target = source;
  EXPECT_THAT(target.status(), IsOk());
  EXPECT_EQ(static_cast<const Base2*>(*source), *target);
}

TEST(StatusOr, TestPointerAssignmentStatusNotOkConverting) {
  turbo::StatusOr<Derived*> source(turbo::CancelledError());
  turbo::StatusOr<Base2*> target;
  target = source;
  EXPECT_EQ(target.status(), source.status());
}

TEST(StatusOr, TestPointerStatus) {
  const int kI = 0;
  turbo::StatusOr<const int*> good(&kI);
  EXPECT_TRUE(good.ok());
  turbo::StatusOr<const int*> bad(turbo::CancelledError());
  EXPECT_EQ(bad.status().code(), turbo::StatusCode::kCancelled);
}

TEST(StatusOr, TestPointerValue) {
  const int kI = 0;
  turbo::StatusOr<const int*> thing(&kI);
  EXPECT_EQ(&kI, *thing);
}

TEST(StatusOr, TestPointerValueConst) {
  const int kI = 0;
  const turbo::StatusOr<const int*> thing(&kI);
  EXPECT_EQ(&kI, *thing);
}

TEST(StatusOr, StatusOrVectorOfUniquePointerCanReserveAndResize) {
  using EvilType = std::vector<std::unique_ptr<int>>;
  static_assert(std::is_copy_constructible_v<EvilType>, "");
  std::vector<::turbo::StatusOr<EvilType>> v(5);
  v.reserve(v.capacity() + 10);
  v.resize(v.capacity() + 10);
}

TEST(StatusOr, ConstPayload) {
  // A reduced version of a problematic type found in the wild. All of the
  // operations below should compile.
  turbo::StatusOr<const int> a;

  // Copy-construction
  turbo::StatusOr<const int> b(a);

  // Copy-assignment
  EXPECT_FALSE(std::is_copy_assignable_v<turbo::StatusOr<const int>>);

  // Move-construction
  turbo::StatusOr<const int> c(std::move(a));

  // Move-assignment
  EXPECT_FALSE(std::is_move_assignable_v<turbo::StatusOr<const int>>);
}

TEST(StatusOr, MapToStatusOrUniquePtr) {
  // A reduced version of a problematic type found in the wild. All of the
  // operations below should compile.
  using MapType = std::map<std::string, turbo::StatusOr<std::unique_ptr<int>>>;

  MapType a;

  // Move-construction
  MapType b(std::move(a));

  // Move-assignment
  a = std::move(b);
}

TEST(StatusOr, ValueOrOk) {
  const turbo::StatusOr<int> status_or = 0;
  EXPECT_EQ(status_or.value_or(-1), 0);
}

TEST(StatusOr, ValueOrDefault) {
  const turbo::StatusOr<int> status_or = turbo::CancelledError();
  EXPECT_EQ(status_or.value_or(-1), -1);
}

TEST(StatusOr, MoveOnlyValueOrOk) {
  EXPECT_THAT(turbo::StatusOr<std::unique_ptr<int>>(std::make_unique<int>(0))
                  .value_or(std::make_unique<int>(-1)),
              Pointee(0));
}

TEST(StatusOr, MoveOnlyValueOrDefault) {
  EXPECT_THAT(turbo::StatusOr<std::unique_ptr<int>>(turbo::CancelledError())
                  .value_or(std::make_unique<int>(-1)),
              Pointee(-1));
}

static turbo::StatusOr<int> MakeStatus() { return 100; }

TEST(StatusOr, TestIgnoreError) { MakeStatus().IgnoreError(); }

TEST(StatusOr, EqualityOperator) {
  constexpr size_t kNumCases = 4;
  std::array<turbo::StatusOr<int>, kNumCases> group1 = {
      turbo::StatusOr<int>(1), turbo::StatusOr<int>(2),
      turbo::StatusOr<int>(turbo::InvalidArgumentError("msg")),
      turbo::StatusOr<int>(turbo::InternalError("msg"))};
  std::array<turbo::StatusOr<int>, kNumCases> group2 = {
      turbo::StatusOr<int>(1), turbo::StatusOr<int>(2),
      turbo::StatusOr<int>(turbo::InvalidArgumentError("msg")),
      turbo::StatusOr<int>(turbo::InternalError("msg"))};
  for (size_t i = 0; i < kNumCases; ++i) {
    for (size_t j = 0; j < kNumCases; ++j) {
      if (i == j) {
        EXPECT_TRUE(group1[i] == group2[j]);
        EXPECT_FALSE(group1[i] != group2[j]);
      } else {
        EXPECT_FALSE(group1[i] == group2[j]);
        EXPECT_TRUE(group1[i] != group2[j]);
      }
    }
  }
}

struct MyType {
  bool operator==(const MyType&) const { return true; }
};

enum class ConvTraits { kNone = 0, kImplicit = 1, kExplicit = 2 };

// This class has conversion operator to `StatusOr<T>` based on value of
// `conv_traits`.
template <typename T, ConvTraits conv_traits = ConvTraits::kNone>
struct StatusOrConversionBase {};

template <typename T>
struct StatusOrConversionBase<T, ConvTraits::kImplicit> {
  operator turbo::StatusOr<T>() const& {  // NOLINT
    return turbo::InvalidArgumentError("conversion to turbo::StatusOr");
  }
  operator turbo::StatusOr<T>() && {  // NOLINT
    return turbo::InvalidArgumentError("conversion to turbo::StatusOr");
  }
};

template <typename T>
struct StatusOrConversionBase<T, ConvTraits::kExplicit> {
  explicit operator turbo::StatusOr<T>() const& {
    return turbo::InvalidArgumentError("conversion to turbo::StatusOr");
  }
  explicit operator turbo::StatusOr<T>() && {
    return turbo::InvalidArgumentError("conversion to turbo::StatusOr");
  }
};

// This class has conversion operator to `T` based on the value of
// `conv_traits`.
template <typename T, ConvTraits conv_traits = ConvTraits::kNone>
struct ConversionBase {};

template <typename T>
struct ConversionBase<T, ConvTraits::kImplicit> {
  operator T() const& { return t; }         // NOLINT
  operator T() && { return std::move(t); }  // NOLINT
  T t;
};

template <typename T>
struct ConversionBase<T, ConvTraits::kExplicit> {
  explicit operator T() const& { return t; }
  explicit operator T() && { return std::move(t); }
  T t;
};

// This class has conversion operator to `turbo::Status` based on the value of
// `conv_traits`.
template <ConvTraits conv_traits = ConvTraits::kNone>
struct StatusConversionBase {};

template <>
struct StatusConversionBase<ConvTraits::kImplicit> {
  operator turbo::Status() const& {  // NOLINT
    return turbo::InternalError("conversion to Status");
  }
  operator turbo::Status() && {  // NOLINT
    return turbo::InternalError("conversion to Status");
  }
};

template <>
struct StatusConversionBase<ConvTraits::kExplicit> {
  explicit operator turbo::Status() const& {  // NOLINT
    return turbo::InternalError("conversion to Status");
  }
  explicit operator turbo::Status() && {  // NOLINT
    return turbo::InternalError("conversion to Status");
  }
};

static constexpr int kConvToStatus = 1;
static constexpr int kConvToStatusOr = 2;
static constexpr int kConvToT = 4;
static constexpr int kConvExplicit = 8;

constexpr ConvTraits GetConvTraits(int bit, int config) {
  return (config & bit) == 0
             ? ConvTraits::kNone
             : ((config & kConvExplicit) == 0 ? ConvTraits::kImplicit
                                              : ConvTraits::kExplicit);
}

// This class conditionally has conversion operator to `turbo::Status`, `T`,
// `StatusOr<T>`, based on values of the template parameters.
template <typename T, int config>
struct CustomType
    : StatusOrConversionBase<T, GetConvTraits(kConvToStatusOr, config)>,
      ConversionBase<T, GetConvTraits(kConvToT, config)>,
      StatusConversionBase<GetConvTraits(kConvToStatus, config)> {};

struct ConvertibleToAnyStatusOr {
  template <typename T>
  operator turbo::StatusOr<T>() const {  // NOLINT
    return turbo::InvalidArgumentError("Conversion to turbo::StatusOr");
  }
};

// Test the rank of overload resolution for `StatusOr<T>` constructor and
// assignment, from highest to lowest:
// 1. T/Status
// 2. U that has conversion operator to turbo::StatusOr<T>
// 3. U that is convertible to Status
// 4. U that is convertible to T
TEST(StatusOr, ConstructionFromT) {
  // Construct turbo::StatusOr<T> from T when T is convertible to
  // turbo::StatusOr<T>
  {
    ConvertibleToAnyStatusOr v;
    turbo::StatusOr<ConvertibleToAnyStatusOr> statusor(v);
    EXPECT_TRUE(statusor.ok());
  }
  {
    ConvertibleToAnyStatusOr v;
    turbo::StatusOr<ConvertibleToAnyStatusOr> statusor = v;
    EXPECT_TRUE(statusor.ok());
  }
  // Construct turbo::StatusOr<T> from T when T is explicitly convertible to
  // Status
  {
    CustomType<MyType, kConvToStatus | kConvExplicit> v;
    turbo::StatusOr<CustomType<MyType, kConvToStatus | kConvExplicit>> statusor(
        v);
    EXPECT_TRUE(statusor.ok());
  }
  {
    CustomType<MyType, kConvToStatus | kConvExplicit> v;
    turbo::StatusOr<CustomType<MyType, kConvToStatus | kConvExplicit>> statusor =
        v;
    EXPECT_TRUE(statusor.ok());
  }
}

// Construct turbo::StatusOr<T> from U when U is explicitly convertible to T
TEST(StatusOr, ConstructionFromTypeConvertibleToT) {
  {
    CustomType<MyType, kConvToT | kConvExplicit> v;
    turbo::StatusOr<MyType> statusor(v);
    EXPECT_TRUE(statusor.ok());
  }
  {
    CustomType<MyType, kConvToT> v;
    turbo::StatusOr<MyType> statusor = v;
    EXPECT_TRUE(statusor.ok());
  }
}

// Construct turbo::StatusOr<T> from U when U has explicit conversion operator to
// turbo::StatusOr<T>
TEST(StatusOr, ConstructionFromTypeWithConversionOperatorToStatusOrT) {
  {
    CustomType<MyType, kConvToStatusOr | kConvExplicit> v;
    turbo::StatusOr<MyType> statusor(v);
    EXPECT_EQ(statusor, v.operator turbo::StatusOr<MyType>());
  }
  {
    CustomType<MyType, kConvToT | kConvToStatusOr | kConvExplicit> v;
    turbo::StatusOr<MyType> statusor(v);
    EXPECT_EQ(statusor, v.operator turbo::StatusOr<MyType>());
  }
  {
    CustomType<MyType, kConvToStatusOr | kConvToStatus | kConvExplicit> v;
    turbo::StatusOr<MyType> statusor(v);
    EXPECT_EQ(statusor, v.operator turbo::StatusOr<MyType>());
  }
  {
    CustomType<MyType,
               kConvToT | kConvToStatusOr | kConvToStatus | kConvExplicit>
        v;
    turbo::StatusOr<MyType> statusor(v);
    EXPECT_EQ(statusor, v.operator turbo::StatusOr<MyType>());
  }
  {
    CustomType<MyType, kConvToStatusOr> v;
    turbo::StatusOr<MyType> statusor = v;
    EXPECT_EQ(statusor, v.operator turbo::StatusOr<MyType>());
  }
  {
    CustomType<MyType, kConvToT | kConvToStatusOr> v;
    turbo::StatusOr<MyType> statusor = v;
    EXPECT_EQ(statusor, v.operator turbo::StatusOr<MyType>());
  }
  {
    CustomType<MyType, kConvToStatusOr | kConvToStatus> v;
    turbo::StatusOr<MyType> statusor = v;
    EXPECT_EQ(statusor, v.operator turbo::StatusOr<MyType>());
  }
  {
    CustomType<MyType, kConvToT | kConvToStatusOr | kConvToStatus> v;
    turbo::StatusOr<MyType> statusor = v;
    EXPECT_EQ(statusor, v.operator turbo::StatusOr<MyType>());
  }
}

TEST(StatusOr, ConstructionFromTypeConvertibleToStatus) {
  // Construction fails because conversion to `Status` is explicit.
  {
    CustomType<MyType, kConvToStatus | kConvExplicit> v;
    turbo::StatusOr<MyType> statusor(v);
    EXPECT_FALSE(statusor.ok());
    EXPECT_EQ(statusor.status(), static_cast<turbo::Status>(v));
  }
  {
    CustomType<MyType, kConvToT | kConvToStatus | kConvExplicit> v;
    turbo::StatusOr<MyType> statusor(v);
    EXPECT_FALSE(statusor.ok());
    EXPECT_EQ(statusor.status(), static_cast<turbo::Status>(v));
  }
  {
    CustomType<MyType, kConvToStatus> v;
    turbo::StatusOr<MyType> statusor = v;
    EXPECT_FALSE(statusor.ok());
    EXPECT_EQ(statusor.status(), static_cast<turbo::Status>(v));
  }
  {
    CustomType<MyType, kConvToT | kConvToStatus> v;
    turbo::StatusOr<MyType> statusor = v;
    EXPECT_FALSE(statusor.ok());
    EXPECT_EQ(statusor.status(), static_cast<turbo::Status>(v));
  }
}

TEST(StatusOr, AssignmentFromT) {
  // Assign to turbo::StatusOr<T> from T when T is convertible to
  // turbo::StatusOr<T>
  {
    ConvertibleToAnyStatusOr v;
    turbo::StatusOr<ConvertibleToAnyStatusOr> statusor;
    statusor = v;
    EXPECT_TRUE(statusor.ok());
  }
  // Assign to turbo::StatusOr<T> from T when T is convertible to Status
  {
    CustomType<MyType, kConvToStatus> v;
    turbo::StatusOr<CustomType<MyType, kConvToStatus>> statusor;
    statusor = v;
    EXPECT_TRUE(statusor.ok());
  }
}

TEST(StatusOr, AssignmentFromTypeConvertibleToT) {
  // Assign to turbo::StatusOr<T> from U when U is convertible to T
  {
    CustomType<MyType, kConvToT> v;
    turbo::StatusOr<MyType> statusor;
    statusor = v;
    EXPECT_TRUE(statusor.ok());
  }
}

TEST(StatusOr, AssignmentFromTypeWithConversionOperatortoStatusOrT) {
  // Assign to turbo::StatusOr<T> from U when U has conversion operator to
  // turbo::StatusOr<T>
  {
    CustomType<MyType, kConvToStatusOr> v;
    turbo::StatusOr<MyType> statusor;
    statusor = v;
    EXPECT_EQ(statusor, v.operator turbo::StatusOr<MyType>());
  }
  {
    CustomType<MyType, kConvToT | kConvToStatusOr> v;
    turbo::StatusOr<MyType> statusor;
    statusor = v;
    EXPECT_EQ(statusor, v.operator turbo::StatusOr<MyType>());
  }
  {
    CustomType<MyType, kConvToStatusOr | kConvToStatus> v;
    turbo::StatusOr<MyType> statusor;
    statusor = v;
    EXPECT_EQ(statusor, v.operator turbo::StatusOr<MyType>());
  }
  {
    CustomType<MyType, kConvToT | kConvToStatusOr | kConvToStatus> v;
    turbo::StatusOr<MyType> statusor;
    statusor = v;
    EXPECT_EQ(statusor, v.operator turbo::StatusOr<MyType>());
  }
}

TEST(StatusOr, AssignmentFromTypeConvertibleToStatus) {
  // Assign to turbo::StatusOr<T> from U when U is convertible to Status
  {
    CustomType<MyType, kConvToStatus> v;
    turbo::StatusOr<MyType> statusor;
    statusor = v;
    EXPECT_FALSE(statusor.ok());
    EXPECT_EQ(statusor.status(), static_cast<turbo::Status>(v));
  }
  {
    CustomType<MyType, kConvToT | kConvToStatus> v;
    turbo::StatusOr<MyType> statusor;
    statusor = v;
    EXPECT_FALSE(statusor.ok());
    EXPECT_EQ(statusor.status(), static_cast<turbo::Status>(v));
  }
}

TEST(StatusOr, StatusAssignmentFromStatusError) {
  turbo::StatusOr<turbo::Status> statusor;
  statusor.AssignStatus(turbo::CancelledError());

  EXPECT_FALSE(statusor.ok());
  EXPECT_EQ(statusor.status(), turbo::CancelledError());
}

#if GTEST_HAS_DEATH_TEST
TEST(StatusOr, StatusAssignmentFromStatusOk) {
  EXPECT_DEBUG_DEATH(
      {
        turbo::StatusOr<turbo::Status> statusor;
        // This will DKCHECK.
        statusor.AssignStatus(turbo::OkStatus());
        // In optimized mode, we are actually going to get error::INTERNAL for
        // status here, rather than crashing, so check that.
        EXPECT_FALSE(statusor.ok());
        EXPECT_EQ(statusor.status().code(), turbo::StatusCode::kInternal);
      },
      "An OK status is not a valid constructor argument to StatusOr<T>");
}
#endif

TEST(StatusOr, StatusAssignmentFromTypeConvertibleToStatus) {
  CustomType<MyType, kConvToStatus> v;
  turbo::StatusOr<MyType> statusor;
  statusor.AssignStatus(v);

  EXPECT_FALSE(statusor.ok());
  EXPECT_EQ(statusor.status(), static_cast<turbo::Status>(v));
}

struct PrintTestStruct {
  friend std::ostream& operator<<(std::ostream& os, const PrintTestStruct&) {
    return os << "ostream";
  }

  template <typename Sink>
  friend void turbo_stringify(Sink& sink, const PrintTestStruct&) {
    sink.Append("stringify");
  }
};

TEST(StatusOr, OkPrinting) {
  turbo::StatusOr<PrintTestStruct> print_me = PrintTestStruct{};
  std::stringstream stream;
  stream << print_me;
  EXPECT_EQ(stream.str(), "ostream");
  EXPECT_EQ(turbo::StrCat(print_me), "stringify");
}

TEST(StatusOr, ErrorPrinting) {
  turbo::StatusOr<PrintTestStruct> print_me = turbo::UnknownError("error");
  std::stringstream stream;
  stream << print_me;
  const auto error_matcher =
      AllOf(HasSubstr("UNKNOWN"), HasSubstr("error"),
            AnyOf(AllOf(StartsWith("("), EndsWith(")")),
                  AllOf(StartsWith("["), EndsWith("]"))));
  EXPECT_THAT(stream.str(), error_matcher);
  EXPECT_THAT(turbo::StrCat(print_me), error_matcher);
}

#if KUMO_HAVE_BUILTIN_LINE_FILE
#define GET_SOURCE_LOCATION(offset) __builtin_LINE() - offset
#else
#define GET_SOURCE_LOCATION(offset) 1
#endif

template <typename T>
void CheckSourceLocation(
    const turbo::StatusOr<T>& status_or, std::vector<int> lines = {},
    std::source_location loc = std::source_location::current()) {
  ASSERT_EQ(status_or.GetSourceLocations().size(), lines.size())
      << "Size check failed at " << loc.line();
  for (size_t i = 0; i < lines.size(); ++i) {
    EXPECT_EQ(std::string_view(status_or.GetSourceLocations()[i].file_name()),
              std::string_view(loc.file_name()))
        << "File name check failed at " << loc.line();
    EXPECT_EQ(status_or.GetSourceLocations()[i].line(), lines[i])
        << "Line check failed at " << loc.line();
  }
}

TEST(StatusOr, AddSourceLocation) {
  constexpr int kMaxIter = 10;
  {
    // Status that ignores source location.
    turbo::StatusOr<int> status_ignores_source_location[] = {
        123, turbo::Status(turbo::StatusCode::kInternal, "")};
    for (turbo::StatusOr<int>& s : status_ignores_source_location) {
      for (int i = 0; i < kMaxIter; ++i) {
        s.AddSourceLocation(std::source_location::current());
        s.AddSourceLocation(std::source_location());
      }
      CheckSourceLocation(s);
    }
  }
  {
    // Default std::source_location is not added.
    turbo::StatusOr<int> status = turbo::Status(
        turbo::StatusCode::kInternal, "foo", std::source_location::current());
    int line = GET_SOURCE_LOCATION(1);
    for (int i = 0; i < kMaxIter; ++i) {
      status.AddSourceLocation(std::source_location());
    }
    CheckSourceLocation(status, {line});
  }
  {
    // Default std::source_location is not added.
    turbo::StatusOr<int> status = turbo::Status(
        turbo::StatusCode::kInternal, "foo", std::source_location::current());
    int line = GET_SOURCE_LOCATION(1);
    std::vector<int> lines = {line};
    lines.reserve(1 + kMaxIter);
    for (int i = 0; i < kMaxIter; ++i) {
      status.AddSourceLocation(std::source_location::current());
      lines.push_back(GET_SOURCE_LOCATION(1));
    }
    CheckSourceLocation(status, lines);
  }
}

turbo::StatusOr<int>&& IsRvalueStatus(turbo::StatusOr<int>&& s) {
  return std::move(s);
}

TEST(StatusOr, WithSourceLocationMove) {
  turbo::StatusOr<int> original = turbo::Status(
      turbo::StatusCode::kInternal, "message", std::source_location::current());
  int line = GET_SOURCE_LOCATION(1);

  const turbo::StatusOr<int> status_or = IsRvalueStatus(
      std::move(original).WithSourceLocation(std::source_location::current()));
  int line2 = GET_SOURCE_LOCATION(1);

  CheckSourceLocation(status_or, {line, line2});
  EXPECT_FALSE(status_or.ok());
}

TEST(StatusOr, WithSourceLocationReturn) {
  std::source_location loc1 = std::source_location::current();
  int line1 = GET_SOURCE_LOCATION(1);
  std::source_location loc2 = std::source_location::current();
  int line2 = GET_SOURCE_LOCATION(1);

  const auto return_error = [&loc1]() -> turbo::StatusOr<int> {
    return turbo::InvalidArgumentError("I am error", loc1);
  };
  const auto return_error_with_source_location =
      [&return_error, &loc2]() -> turbo::StatusOr<int> {
    return return_error().WithSourceLocation(loc2);
  };

  turbo::StatusOr<int> status_or = return_error_with_source_location();
  CheckSourceLocation(status_or, {line1, line2});
  EXPECT_FALSE(status_or.ok());
}

TEST(StatusOr, SupportsReferenceTypes) {
  int i = 1;
  turbo::StatusOr<int&> s = i;
  EXPECT_EQ(&i, &*s);
  *s = 10;
  EXPECT_EQ(i, 10);
}

TEST(StatusOr, ReferenceFromStatus) {
  int i = 10;
  turbo::StatusOr<int&> s = i;
  s = turbo::InternalError("foo");
  EXPECT_EQ(s.status().message(), "foo");

  turbo::StatusOr<int&> s2 = turbo::InternalError("foo2");
  EXPECT_EQ(s2.status().message(), "foo2");
}

TEST(StatusOr, SupportReferenceValueConstructor) {
  int i = 1;
  turbo::StatusOr<int&> s = i;
  turbo::StatusOr<const int&> cs = i;
  turbo::StatusOr<const int&> cs2 = std::move(i);  // `T&&` to `const T&` is ok.

  EXPECT_EQ(&i, &*s);
  EXPECT_EQ(&i, &*cs);

  Derived d;
  turbo::StatusOr<const Base1&> b = d;
  EXPECT_EQ(&d, &*b);

  // We disallow constructions that cause temporaries.
  EXPECT_FALSE((std::is_constructible_v<turbo::StatusOr<const int&>, double>));
  EXPECT_FALSE(
      (std::is_constructible_v<turbo::StatusOr<const int&>, const double&>));
  EXPECT_FALSE(
      (std::is_constructible_v<turbo::StatusOr<const std::string_view&>,
                               std::string>));

  // We disallow constructions with wrong reference.
  EXPECT_FALSE((std::is_constructible_v<turbo::StatusOr<int&>, int&&>));
  EXPECT_FALSE((std::is_constructible_v<turbo::StatusOr<int&>, const int&>));
}

TEST(StatusOr, SupportReferenceConvertingConstructor) {
  int i = 1;
  turbo::StatusOr<int&> s = i;
  turbo::StatusOr<const int&> cs = s;

  EXPECT_EQ(&i, &*s);
  EXPECT_EQ(&i, &*cs);

  // The other direction is not allowed.
  EXPECT_FALSE((std::is_constructible_v<turbo::StatusOr<int&>,
                                        turbo::StatusOr<const int&>>));

  Derived d;
  turbo::StatusOr<const Base1&> b = turbo::StatusOr<const Derived&>(d);
  EXPECT_EQ(&d, &*b);

  // The other direction is not allowed.
  EXPECT_FALSE((std::is_constructible_v<turbo::StatusOr<const Derived&>,
                                        turbo::StatusOr<const Base1&>>));

  // We disallow conversions that cause temporaries.
  EXPECT_FALSE((std::is_constructible_v<turbo::StatusOr<const int&>,
                                        turbo::StatusOr<int>>));
  EXPECT_FALSE((std::is_constructible_v<turbo::StatusOr<const int&>,
                                        turbo::StatusOr<double>>));
  EXPECT_FALSE((std::is_constructible_v<turbo::StatusOr<const int&>,
                                        turbo::StatusOr<const double&>>));
  EXPECT_FALSE((std::is_constructible_v<turbo::StatusOr<const double&>,
                                        turbo::StatusOr<const int&>>));
  EXPECT_FALSE(
      (std::is_constructible_v<turbo::StatusOr<const std::string_view&>,
                               turbo::StatusOr<std::string>>));

  // We disallow constructions with wrong reference.
  EXPECT_FALSE((std::is_constructible_v<turbo::StatusOr<int&>,
                                        turbo::StatusOr<const int&>>));
}

TEST(StatusOr, SupportReferenceValueAssignment) {
  int i = 1;
  turbo::StatusOr<int&> s = i;
  turbo::StatusOr<const int&> cs;
  cs = i;
  turbo::StatusOr<const int&> cs2;
  cs2 = std::move(i);  // `T&&` to `const T&` is ok.

  EXPECT_EQ(&i, &*s);
  EXPECT_EQ(&i, &*cs);

  Derived d;
  turbo::StatusOr<const Base1&> b;
  b = d;
  EXPECT_EQ(&d, &*b);

  // We disallow constructions that cause temporaries.
  EXPECT_FALSE((std::is_assignable_v<turbo::StatusOr<const int&>, double>));
  EXPECT_FALSE(
      (std::is_assignable_v<turbo::StatusOr<const int&>, const double&>));
  EXPECT_FALSE((std::is_assignable_v<turbo::StatusOr<const std::string_view&>,
                                     std::string>));

  // We disallow constructions with wrong reference.
  EXPECT_FALSE((std::is_assignable_v<turbo::StatusOr<int&>, int&&>));
  EXPECT_FALSE((std::is_assignable_v<turbo::StatusOr<int&>, const int&>));
}

TEST(StatusOr, SupportReferenceConvertingAssignment) {
  int i = 1;
  turbo::StatusOr<int&> s;
  s = i;
  turbo::StatusOr<const int&> cs;
  cs = s;

  EXPECT_EQ(&i, &*s);
  EXPECT_EQ(&i, &*cs);

  // The other direction is not allowed.
  EXPECT_FALSE(
      (std::is_assignable_v<turbo::StatusOr<int&>, turbo::StatusOr<const int&>>));

  Derived d;
  turbo::StatusOr<const Base1&> b;
  b = turbo::StatusOr<const Derived&>(d);
  EXPECT_EQ(&d, &*b);

  // The other direction is not allowed.
  EXPECT_FALSE((std::is_assignable_v<turbo::StatusOr<const Derived&>,
                                     turbo::StatusOr<const Base1&>>));

  // We disallow conversions that cause temporaries.
  EXPECT_FALSE((std::is_assignable_v<turbo::StatusOr<const int&>,
                                     turbo::StatusOr<const double&>>));
  EXPECT_FALSE((std::is_assignable_v<turbo::StatusOr<const int&>,
                                     turbo::StatusOr<double>>));
  EXPECT_FALSE((std::is_assignable_v<turbo::StatusOr<const std::string_view&>,
                                     turbo::StatusOr<std::string>>));

  // We disallow constructions with wrong reference.
  EXPECT_FALSE(
      (std::is_assignable_v<turbo::StatusOr<int&>, turbo::StatusOr<const int&>>));
}

TEST(StatusOr, SupportReferenceToNonReferenceConversions) {
  int i = 17;
  turbo::StatusOr<int&> si = i;
  turbo::StatusOr<float> sf = si;
  EXPECT_THAT(sf, IsOkAndHolds(17.));

  i = 20;
  sf = si;
  EXPECT_THAT(sf, IsOkAndHolds(20.));

  EXPECT_THAT(turbo::StatusOr<int64_t>(turbo::StatusOr<int&>(i)),
              IsOkAndHolds(20));
  EXPECT_THAT(turbo::StatusOr<int64_t>(turbo::StatusOr<const int&>(i)),
              IsOkAndHolds(20));

  std::string str = "str";
  turbo::StatusOr<std::string> sos = turbo::StatusOr<std::string&>(str);
  EXPECT_THAT(sos, IsOkAndHolds("str"));
  str = "str2";
  EXPECT_THAT(sos, IsOkAndHolds("str"));
  sos = turbo::StatusOr<std::string&>(str);
  EXPECT_THAT(sos, IsOkAndHolds("str2"));

  turbo::StatusOr<std::string_view> sosv = turbo::StatusOr<std::string&>(str);
  EXPECT_THAT(sosv, IsOkAndHolds("str2"));
  str = "str3";
  sosv = turbo::StatusOr<std::string&>(str);
  EXPECT_THAT(sosv, IsOkAndHolds("str3"));

  std::string_view view = "view";
  // This way it is constructible, but not convertible because
  // std::string_view->string is explicit
  EXPECT_THAT(
      turbo::StatusOr<std::string>(turbo::StatusOr<std::string_view&>(view)),
      IsOkAndHolds("view"));
  // The assignment doesn't work with normal std::string_view because
  // std::string doesn't know about it.
  sos = turbo::StatusOr<std::string_view&>(view);
  EXPECT_THAT(sos, IsOkAndHolds("view"));

  EXPECT_FALSE((std::is_convertible_v<turbo::StatusOr<std::string_view&>,
                                      turbo::StatusOr<std::string>>));
}

TEST(StatusOr, ReferenceOperatorStarAndArrow) {
  std::string str = "Foo";
  turbo::StatusOr<std::string&> s = str;
  s->assign("Bar");
  EXPECT_EQ(str, "Bar");

  *s = "Baz";
  EXPECT_EQ(str, "Baz");

  const turbo::StatusOr<std::string&> cs = str;
  // Even if the StatusOr is const, the reference it gives is non-const so we
  // can still assign.
  *cs = "Finally";
  EXPECT_EQ(str, "Finally");

  cs->clear();
  EXPECT_EQ(cs.value(), str);
  EXPECT_EQ(str, "");
}

TEST(StatusOr, ReferenceValueOr) {
  int i = 17;
  turbo::StatusOr<int&> si = i;

  int other = 20;
  EXPECT_EQ(&i, &si.value_or(other));

  si = turbo::UnknownError("");
  EXPECT_EQ(&other, &si.value_or(other));

  turbo::StatusOr<const int&> csi = i;
  EXPECT_EQ(&i, &csi.value_or(1));

  const auto value_or_call = [](auto&& sor, auto&& v)
      -> decltype(std::forward<decltype(sor)>(sor).value_or(
          std::forward<decltype(v)>(v))) {};
  using Probe = decltype(value_or_call);
  // Just to verify that Probe works as expected in the good cases.
  EXPECT_TRUE((std::is_invocable_v<Probe, turbo::StatusOr<const int&>, int&&>));
  // Causes temporary conversion.
  EXPECT_FALSE(
      (std::is_invocable_v<Probe, turbo::StatusOr<const int&>, double&&>));
  // Const invalid.
  EXPECT_FALSE((std::is_invocable_v<Probe, turbo::StatusOr<int&>, const int&>));
}

TEST(StatusOr, ReferenceAssignmentFromStatusOr) {
  std::vector<int> v = {1, 2, 3};
  turbo::StatusOr<int&> si = v[0];
  turbo::StatusOr<int&> si2 = v[1];

  EXPECT_THAT(v, ElementsAre(1, 2, 3));
  EXPECT_THAT(si, IsOkAndHolds(1));
  EXPECT_THAT(si2, IsOkAndHolds(2));

  // This rebinds the reference.
  si = si2;
  EXPECT_THAT(v, ElementsAre(1, 2, 3));
  EXPECT_THAT(si, IsOkAndHolds(2));
  EXPECT_THAT(si2, IsOkAndHolds(2));
  EXPECT_EQ(&*si, &*si2);
}

TEST(StatusOr, ReferenceAssignFromReference) {
  std::vector<int> v = {1, 2, 3};
  turbo::StatusOr<int&> si = v[0];

  EXPECT_THAT(v, ElementsAre(1, 2, 3));
  EXPECT_THAT(si, IsOkAndHolds(1));

  // This rebinds the reference.
  si = v[2];
  EXPECT_THAT(v, ElementsAre(1, 2, 3));
  EXPECT_THAT(si, IsOkAndHolds(3));
  EXPECT_EQ(&*si, &v[2]);
}

TEST(StatusOr, ReferenceIsNotLifetimeBoundForStarValue) {
  int i = 0;

  // op*/value should not be LIFETIME_BOUND because the ref is not limited to
  // the lifetime of the StatusOr.
  int& r = *turbo::StatusOr<int&>(i);
  EXPECT_EQ(&r, &i);
  int& r2 = turbo::StatusOr<int&>(i).value();
  EXPECT_EQ(&r2, &i);

  struct S {
    int i;
  };
  S s;
  // op-> should also not be LIFETIME_BOUND for refs.
  int& r3 = turbo::StatusOr<S&>(s)->i;
  EXPECT_EQ(&r3, &s.i);
}

template <typename Expected, typename T>
void TestReferenceDeref() {
  static_assert(std::is_same_v<Expected, decltype(*std::declval<T>())>);
  static_assert(std::is_same_v<Expected, decltype(std::declval<T>().value())>);
}

TEST(StatusOr, ReferenceTypeIsMaintainedOnDeref) {
  TestReferenceDeref<int&, turbo::StatusOr<int&>&>();
  TestReferenceDeref<int&, turbo::StatusOr<int&>&&>();
  TestReferenceDeref<int&, const turbo::StatusOr<int&>&>();
  TestReferenceDeref<int&, const turbo::StatusOr<int&>&&>();

  TestReferenceDeref<const int&, turbo::StatusOr<const int&>&>();
  TestReferenceDeref<const int&, turbo::StatusOr<const int&>&&>();
  TestReferenceDeref<const int&, const turbo::StatusOr<const int&>&>();
  TestReferenceDeref<const int&, const turbo::StatusOr<const int&>&&>();

  struct Struct {
    int value;
  };
  EXPECT_TRUE(
      (std::is_same_v<
          int&, decltype((std::declval<turbo::StatusOr<Struct&>>()->value))>));
  EXPECT_TRUE(
      (std::is_same_v<
          int&,
          decltype((std::declval<const turbo::StatusOr<Struct&>>()->value))>));
  EXPECT_TRUE(
      (std::is_same_v<
          const int&,
          decltype((std::declval<turbo::StatusOr<const Struct&>>()->value))>));
}

}  // namespace
