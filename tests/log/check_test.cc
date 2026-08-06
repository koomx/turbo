//
// Copyright 2022 The Abseil Authors.
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

#include <turbo/log/kcheck.h>

#define TURBO_TEST_CHECK KCHECK
#define TURBO_TEST_CHECK_OK KCHECK_OK
#define TURBO_TEST_CHECK_EQ KCHECK_EQ
#define TURBO_TEST_CHECK_NE KCHECK_NE
#define TURBO_TEST_CHECK_GE KCHECK_GE
#define TURBO_TEST_CHECK_LE KCHECK_LE
#define TURBO_TEST_CHECK_GT KCHECK_GT
#define TURBO_TEST_CHECK_LT KCHECK_LT
#define TURBO_TEST_CHECK_STREQ KCHECK_STREQ
#define TURBO_TEST_CHECK_STRNE KCHECK_STRNE
#define TURBO_TEST_CHECK_STRCASEEQ KCHECK_STRCASEEQ
#define TURBO_TEST_CHECK_STRCASENE KCHECK_STRCASENE

#define TURBO_TEST_DCHECK DKCHECK
#define TURBO_TEST_DCHECK_OK DKCHECK
#define TURBO_TEST_DCHECK_EQ DKCHECK_EQ
#define TURBO_TEST_DCHECK_NE DKCHECK_NE
#define TURBO_TEST_DCHECK_GE DKCHECK_GE
#define TURBO_TEST_DCHECK_LE DKCHECK_LE
#define TURBO_TEST_DCHECK_GT DKCHECK_GT
#define TURBO_TEST_DCHECK_LT DKCHECK_LT
#define TURBO_TEST_DCHECK_STREQ DKCHECK_STREQ
#define TURBO_TEST_DCHECK_STRNE DKCHECK_STRNE
#define TURBO_TEST_DCHECK_STRCASEEQ DKCHECK_STRCASEEQ
#define TURBO_TEST_DCHECK_STRCASENE DKCHECK_STRCASENE

#define TURBO_TEST_QCHECK QKCHECK
#define TURBO_TEST_QCHECK_OK QKCHECK_OK
#define TURBO_TEST_QCHECK_EQ QKCHECK_EQ
#define TURBO_TEST_QCHECK_NE QKCHECK_NE
#define TURBO_TEST_QCHECK_GE QKCHECK_GE
#define TURBO_TEST_QCHECK_LE QKCHECK_LE
#define TURBO_TEST_QCHECK_GT QKCHECK_GT
#define TURBO_TEST_QCHECK_LT QKCHECK_LT
#define TURBO_TEST_QCHECK_STREQ QKCHECK_STREQ
#define TURBO_TEST_QCHECK_STRNE QKCHECK_STRNE
#define TURBO_TEST_QCHECK_STRCASEEQ QKCHECK_STRCASEEQ
#define TURBO_TEST_QCHECK_STRCASENE QKCHECK_STRCASENE

#include <gtest/gtest.h>
#include <tests/log/check_test_impl.inc>
