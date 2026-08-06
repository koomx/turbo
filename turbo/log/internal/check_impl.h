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

// SKIP_TURBO_INLINE_NAMESPACE_CHECK

#ifndef TURBO_LOG_INTERNAL_CHECK_IMPL_H_
#define TURBO_LOG_INTERNAL_CHECK_IMPL_H_

#include <turbo/macros/config.h>
#include <turbo/log/internal/check_op.h>
#include <turbo/log/internal/conditions.h>
#include <turbo/log/internal/strip.h>

// KCHECK
#define TURBO_LOG_INTERNAL_CHECK_IMPL(condition, condition_text)       \
  TURBO_LOG_INTERNAL_CONDITION_FATAL(STATELESS,                        \
                                    KUMO_UNLIKELY(!(condition))) \
  TURBO_LOG_INTERNAL_CHECK(condition_text).internal_stream()

#define TURBO_LOG_INTERNAL_QCHECK_IMPL(condition, condition_text)       \
  TURBO_LOG_INTERNAL_CONDITION_QFATAL(STATELESS,                        \
                                     KUMO_UNLIKELY(!(condition))) \
  TURBO_LOG_INTERNAL_QCHECK(condition_text).internal_stream()

#define TURBO_LOG_INTERNAL_PCHECK_IMPL(condition, condition_text) \
  TURBO_LOG_INTERNAL_CHECK_IMPL(condition, condition_text).with_perror()

#ifndef NDEBUG
#define TURBO_LOG_INTERNAL_DCHECK_IMPL(condition, condition_text) \
  TURBO_LOG_INTERNAL_CHECK_IMPL(condition, condition_text)
#else
#define TURBO_LOG_INTERNAL_DCHECK_IMPL(condition, condition_text) \
  TURBO_LOG_INTERNAL_CHECK_IMPL(true || (condition), "true")
#endif

// KCHECK_EQ
#define TURBO_LOG_INTERNAL_CHECK_EQ_IMPL(val1, val1_text, val2, val2_text) \
  TURBO_LOG_INTERNAL_CHECK_OP(Check_EQ, ==, val1, val1_text, val2, val2_text)
#define TURBO_LOG_INTERNAL_CHECK_NE_IMPL(val1, val1_text, val2, val2_text) \
  TURBO_LOG_INTERNAL_CHECK_OP(Check_NE, !=, val1, val1_text, val2, val2_text)
#define TURBO_LOG_INTERNAL_CHECK_LE_IMPL(val1, val1_text, val2, val2_text) \
  TURBO_LOG_INTERNAL_CHECK_OP(Check_LE, <=, val1, val1_text, val2, val2_text)
#define TURBO_LOG_INTERNAL_CHECK_LT_IMPL(val1, val1_text, val2, val2_text) \
  TURBO_LOG_INTERNAL_CHECK_OP(Check_LT, <, val1, val1_text, val2, val2_text)
#define TURBO_LOG_INTERNAL_CHECK_GE_IMPL(val1, val1_text, val2, val2_text) \
  TURBO_LOG_INTERNAL_CHECK_OP(Check_GE, >=, val1, val1_text, val2, val2_text)
#define TURBO_LOG_INTERNAL_CHECK_GT_IMPL(val1, val1_text, val2, val2_text) \
  TURBO_LOG_INTERNAL_CHECK_OP(Check_GT, >, val1, val1_text, val2, val2_text)
#define TURBO_LOG_INTERNAL_QCHECK_EQ_IMPL(val1, val1_text, val2, val2_text) \
  TURBO_LOG_INTERNAL_QCHECK_OP(Check_EQ, ==, val1, val1_text, val2, val2_text)
#define TURBO_LOG_INTERNAL_QCHECK_NE_IMPL(val1, val1_text, val2, val2_text) \
  TURBO_LOG_INTERNAL_QCHECK_OP(Check_NE, !=, val1, val1_text, val2, val2_text)
#define TURBO_LOG_INTERNAL_QCHECK_LE_IMPL(val1, val1_text, val2, val2_text) \
  TURBO_LOG_INTERNAL_QCHECK_OP(Check_LE, <=, val1, val1_text, val2, val2_text)
#define TURBO_LOG_INTERNAL_QCHECK_LT_IMPL(val1, val1_text, val2, val2_text) \
  TURBO_LOG_INTERNAL_QCHECK_OP(Check_LT, <, val1, val1_text, val2, val2_text)
#define TURBO_LOG_INTERNAL_QCHECK_GE_IMPL(val1, val1_text, val2, val2_text) \
  TURBO_LOG_INTERNAL_QCHECK_OP(Check_GE, >=, val1, val1_text, val2, val2_text)
#define TURBO_LOG_INTERNAL_QCHECK_GT_IMPL(val1, val1_text, val2, val2_text) \
  TURBO_LOG_INTERNAL_QCHECK_OP(Check_GT, >, val1, val1_text, val2, val2_text)
#ifndef NDEBUG
#define TURBO_LOG_INTERNAL_DCHECK_EQ_IMPL(val1, val1_text, val2, val2_text) \
  TURBO_LOG_INTERNAL_CHECK_EQ_IMPL(val1, val1_text, val2, val2_text)
#define TURBO_LOG_INTERNAL_DCHECK_NE_IMPL(val1, val1_text, val2, val2_text) \
  TURBO_LOG_INTERNAL_CHECK_NE_IMPL(val1, val1_text, val2, val2_text)
#define TURBO_LOG_INTERNAL_DCHECK_LE_IMPL(val1, val1_text, val2, val2_text) \
  TURBO_LOG_INTERNAL_CHECK_LE_IMPL(val1, val1_text, val2, val2_text)
#define TURBO_LOG_INTERNAL_DCHECK_LT_IMPL(val1, val1_text, val2, val2_text) \
  TURBO_LOG_INTERNAL_CHECK_LT_IMPL(val1, val1_text, val2, val2_text)
#define TURBO_LOG_INTERNAL_DCHECK_GE_IMPL(val1, val1_text, val2, val2_text) \
  TURBO_LOG_INTERNAL_CHECK_GE_IMPL(val1, val1_text, val2, val2_text)
#define TURBO_LOG_INTERNAL_DCHECK_GT_IMPL(val1, val1_text, val2, val2_text) \
  TURBO_LOG_INTERNAL_CHECK_GT_IMPL(val1, val1_text, val2, val2_text)
#else  // ndef NDEBUG
#define TURBO_LOG_INTERNAL_DCHECK_EQ_IMPL(val1, val1_text, val2, val2_text) \
  TURBO_LOG_INTERNAL_DCHECK_NOP(val1, val2)
#define TURBO_LOG_INTERNAL_DCHECK_NE_IMPL(val1, val1_text, val2, val2_text) \
  TURBO_LOG_INTERNAL_DCHECK_NOP(val1, val2)
#define TURBO_LOG_INTERNAL_DCHECK_LE_IMPL(val1, val1_text, val2, val2_text) \
  TURBO_LOG_INTERNAL_DCHECK_NOP(val1, val2)
#define TURBO_LOG_INTERNAL_DCHECK_LT_IMPL(val1, val1_text, val2, val2_text) \
  TURBO_LOG_INTERNAL_DCHECK_NOP(val1, val2)
#define TURBO_LOG_INTERNAL_DCHECK_GE_IMPL(val1, val1_text, val2, val2_text) \
  TURBO_LOG_INTERNAL_DCHECK_NOP(val1, val2)
#define TURBO_LOG_INTERNAL_DCHECK_GT_IMPL(val1, val1_text, val2, val2_text) \
  TURBO_LOG_INTERNAL_DCHECK_NOP(val1, val2)
#endif  // def NDEBUG

// KCHECK_OK
#define TURBO_LOG_INTERNAL_CHECK_OK_IMPL(status, status_text) \
  TURBO_LOG_INTERNAL_CHECK_OK(status, status_text)
#define TURBO_LOG_INTERNAL_QCHECK_OK_IMPL(status, status_text) \
  TURBO_LOG_INTERNAL_QCHECK_OK(status, status_text)
#ifndef NDEBUG
#define TURBO_LOG_INTERNAL_DCHECK_OK_IMPL(status, status_text) \
  TURBO_LOG_INTERNAL_CHECK_OK(status, status_text)
#else
#define TURBO_LOG_INTERNAL_DCHECK_OK_IMPL(status, status_text) \
  TURBO_LOG_INTERNAL_DCHECK_NOP(status, nullptr)
#endif

// KCHECK_STREQ
#define TURBO_LOG_INTERNAL_CHECK_STREQ_IMPL(s1, s1_text, s2, s2_text) \
  TURBO_LOG_INTERNAL_CHECK_STROP(strcmp, ==, true, s1, s1_text, s2, s2_text)
#define TURBO_LOG_INTERNAL_CHECK_STRNE_IMPL(s1, s1_text, s2, s2_text) \
  TURBO_LOG_INTERNAL_CHECK_STROP(strcmp, !=, false, s1, s1_text, s2, s2_text)
#define TURBO_LOG_INTERNAL_CHECK_STRCASEEQ_IMPL(s1, s1_text, s2, s2_text) \
  TURBO_LOG_INTERNAL_CHECK_STROP(strcasecmp, ==, true, s1, s1_text, s2, s2_text)
#define TURBO_LOG_INTERNAL_CHECK_STRCASENE_IMPL(s1, s1_text, s2, s2_text) \
  TURBO_LOG_INTERNAL_CHECK_STROP(strcasecmp, !=, false, s1, s1_text, s2, s2_text)
#define TURBO_LOG_INTERNAL_QCHECK_STREQ_IMPL(s1, s1_text, s2, s2_text) \
  TURBO_LOG_INTERNAL_QCHECK_STROP(strcmp, ==, true, s1, s1_text, s2, s2_text)
#define TURBO_LOG_INTERNAL_QCHECK_STRNE_IMPL(s1, s1_text, s2, s2_text) \
  TURBO_LOG_INTERNAL_QCHECK_STROP(strcmp, !=, false, s1, s1_text, s2, s2_text)
#define TURBO_LOG_INTERNAL_QCHECK_STRCASEEQ_IMPL(s1, s1_text, s2, s2_text) \
  TURBO_LOG_INTERNAL_QCHECK_STROP(strcasecmp, ==, true, s1, s1_text, s2, s2_text)
#define TURBO_LOG_INTERNAL_QCHECK_STRCASENE_IMPL(s1, s1_text, s2, s2_text) \
  TURBO_LOG_INTERNAL_QCHECK_STROP(strcasecmp, !=, false, s1, s1_text, s2,  \
                                 s2_text)
#ifndef NDEBUG
#define TURBO_LOG_INTERNAL_DCHECK_STREQ_IMPL(s1, s1_text, s2, s2_text) \
  TURBO_LOG_INTERNAL_CHECK_STREQ_IMPL(s1, s1_text, s2, s2_text)
#define TURBO_LOG_INTERNAL_DCHECK_STRCASEEQ_IMPL(s1, s1_text, s2, s2_text) \
  TURBO_LOG_INTERNAL_CHECK_STRCASEEQ_IMPL(s1, s1_text, s2, s2_text)
#define TURBO_LOG_INTERNAL_DCHECK_STRNE_IMPL(s1, s1_text, s2, s2_text) \
  TURBO_LOG_INTERNAL_CHECK_STRNE_IMPL(s1, s1_text, s2, s2_text)
#define TURBO_LOG_INTERNAL_DCHECK_STRCASENE_IMPL(s1, s1_text, s2, s2_text) \
  TURBO_LOG_INTERNAL_CHECK_STRCASENE_IMPL(s1, s1_text, s2, s2_text)
#else  // ndef NDEBUG
#define TURBO_LOG_INTERNAL_DCHECK_STREQ_IMPL(s1, s1_text, s2, s2_text) \
  TURBO_LOG_INTERNAL_DCHECK_NOP(s1, s2)
#define TURBO_LOG_INTERNAL_DCHECK_STRCASEEQ_IMPL(s1, s1_text, s2, s2_text) \
  TURBO_LOG_INTERNAL_DCHECK_NOP(s1, s2)
#define TURBO_LOG_INTERNAL_DCHECK_STRNE_IMPL(s1, s1_text, s2, s2_text) \
  TURBO_LOG_INTERNAL_DCHECK_NOP(s1, s2)
#define TURBO_LOG_INTERNAL_DCHECK_STRCASENE_IMPL(s1, s1_text, s2, s2_text) \
  TURBO_LOG_INTERNAL_DCHECK_NOP(s1, s2)
#endif  // def NDEBUG

#endif  // TURBO_LOG_INTERNAL_CHECK_IMPL_H_
