#
# Copyright 2017 The Abseil Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
turbo_cc_library(
  NAME
    any
  HDRS
    "any.h"
  COPTS
    ${TURBO_DEFAULT_COPTS}
  DEPS
    turbo::config
    turbo::core_headers
    turbo::utility
  PUBLIC
)

turbo_cc_library(
  NAME
    any_span
  HDRS
    "any_span.h"
  SRCS
    "internal/any_span.h"
  COPTS
    ${TURBO_DEFAULT_COPTS}
  DEPS
    turbo::config
    turbo::core_headers
    turbo::hardening
    turbo::nullability
    turbo::raw_logging_internal
    turbo::throw_delegate
    turbo::type_traits
  PUBLIC
)

turbo_cc_test(
  NAME
    any_span_test
  SRCS
    "any_span_test.cc"
  COPTS
    ${TURBO_TEST_COPTS}
  DEPS
    turbo::config
    turbo::core_headers
    turbo::exception_testing
    turbo::hardening
    turbo::hash_testing
    turbo::raw_logging_internal
    turbo::span
    turbo::strings
    turbo::type_traits
    GTest::gmock_main
)

turbo_cc_library(
  NAME
    span
  HDRS
    "span.h"
  SRCS
    "internal/span.h"
  COPTS
    ${TURBO_DEFAULT_COPTS}
  DEPS
    turbo::algorithm
    turbo::config
    turbo::core_headers
    turbo::hardening
    turbo::nullability
    turbo::throw_delegate
    turbo::type_traits
    turbo::weakly_mixed_integer
  PUBLIC
)

turbo_cc_test(
  NAME
    span_test
  SRCS
    "span_test.cc"
  COPTS
    ${TURBO_TEST_COPTS}
  DEPS
    turbo::span
    turbo::base
    turbo::config
    turbo::core_headers
    turbo::exception_testing
    turbo::fixed_array
    turbo::hardening
    turbo::hash
    turbo::inlined_vector
    turbo::hash_testing
    turbo::strings
    turbo::type_traits
    GTest::gmock_main
)

turbo_cc_test(
  NAME
    span_test_noexceptions
  SRCS
    "span_test.cc"
  COPTS
    ${TURBO_TEST_COPTS}
  DEPS
    turbo::span
    turbo::base
    turbo::config
    turbo::core_headers
    turbo::exception_testing
    turbo::fixed_array
    turbo::inlined_vector
    turbo::hash_testing
    turbo::strings
    GTest::gmock_main
)

turbo_cc_library(
  NAME
    optional
  HDRS
    "optional.h"
  COPTS
    ${TURBO_DEFAULT_COPTS}
  DEPS
    turbo::config
    turbo::core_headers
    turbo::utility
  PUBLIC
)

turbo_cc_library(
  NAME
    variant
  HDRS
    "variant.h"
  COPTS
    ${TURBO_DEFAULT_COPTS}
  DEPS
    turbo::config
    turbo::core_headers
    turbo::utility
  PUBLIC
)

turbo_cc_test(
  NAME
    variant_test
  SRCS
    "variant_test.cc"
  COPTS
    ${TURBO_TEST_COPTS}
  DEPS
    turbo::variant
    GTest::gmock_main
)

turbo_cc_library(
  NAME
    compare
  HDRS
    "compare.h"
  COPTS
    ${TURBO_DEFAULT_COPTS}
  DEPS
    turbo::config
    turbo::core_headers
    turbo::type_traits
  PUBLIC
)

turbo_cc_test(
  NAME
    compare_test
  SRCS
    "compare_test.cc"
  COPTS
    ${TURBO_TEST_COPTS}
  DEPS
    turbo::base
    turbo::compare
    GTest::gmock_main
)

turbo_cc_library(
  NAME
    internal_optional_ref
  HDRS
    "optional_ref.h"
  COPTS
    ${TURBO_DEFAULT_COPTS}
  DEPS
    turbo::config
    turbo::core_headers
    turbo::hardening
  PUBLIC
)

turbo_cc_test(
  NAME
    optional_ref_test
  SRCS
    "optional_ref_test.cc"
  COPTS
    ${TURBO_TEST_COPTS}
  DEPS
    turbo::base
    turbo::config
    turbo::internal_optional_ref
    turbo::strings
    GTest::gmock_main
)

# Deprecated empty library.
# Clients should remove this dependency.
turbo_cc_library(
  NAME
    bad_any_cast
  PUBLIC
)

# Deprecated empty library.
# Clients should remove this dependency.
turbo_cc_library(
  NAME
    bad_optional_access
  PUBLIC
)

# Deprecated empty library.
# Clients should remove this dependency.
turbo_cc_library(
  NAME
    bad_variant_access
  PUBLIC
)
