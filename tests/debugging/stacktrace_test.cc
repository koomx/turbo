// Copyright 2023 The Abseil Authors.
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

#include <turbo/debugging/stacktrace.h>

#include <stddef.h>
#include <stdint.h>

#include <cerrno>
#include <csignal>
#include <cstring>
#include <memory>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <turbo/macros/config.h>
#include <turbo/platform/strerror.h>
#include <turbo/types/span.h>

static int g_should_fixup_calls = 0;
static int g_fixup_calls = 0;
static bool g_enable_fixup = false;
static uintptr_t g_last_fixup_frame_address = 0;

#if TURBO_HAVE_ATTRIBUTE_WEAK
bool turbo::internal_stacktrace::ShouldFixUpStack() {
  ++g_should_fixup_calls;
  return g_enable_fixup;
}

void turbo::internal_stacktrace::FixUpStack(void**, uintptr_t*, int*, size_t,
                                           size_t&) {
  const void* frame_address = nullptr;
#if KUMO_HAVE_BUILTIN(__builtin_frame_address)
  frame_address = __builtin_frame_address(0);
#endif
  g_last_fixup_frame_address = reinterpret_cast<uintptr_t>(frame_address);
  ++g_fixup_calls;
}
#endif

namespace {

using ::testing::ContainerEq;
using ::testing::Contains;
using ::testing::internal::Cleanup;

struct StackTrace {
  static constexpr int kStackCount = 64;
  int depth;
  void* result[kStackCount];
  int sizes[kStackCount];
};

// This test is currently only known to pass on Linux x86_64/aarch64.
#if defined(__linux__) && (defined(__x86_64__) || defined(__aarch64__))
KUMO_ATTRIBUTE_NOINLINE void Unwind(void* p) {
  KUMO_ATTRIBUTE_UNUSED static void* volatile sink = p;
  constexpr int kSize = 16;
  void* stack[kSize];
  int frames[kSize];
  turbo::GetStackTrace(stack, kSize, 0);
  turbo::GetStackFrames(stack, frames, kSize, 0);
}

KUMO_ATTRIBUTE_NOINLINE void HugeFrame() {
  char buffer[1 << 20];
  Unwind(buffer);
  KUMO_BLOCK_TAIL_CALL_OPTIMIZATION();
}

TEST(StackTrace, HugeFrame) {
  // Ensure that the unwinder is not confused by very large stack frames.
  HugeFrame();
  KUMO_BLOCK_TAIL_CALL_OPTIMIZATION();
}
#endif

// This is a separate function to avoid inlining.
KUMO_ATTRIBUTE_NOINLINE static void FixupNoFixupEquivalenceNoInline() {
#if !TURBO_HAVE_ATTRIBUTE_WEAK
  const char* kSkipReason = "Need weak symbol support";
#elif defined(__riscv)
  const char* kSkipReason =
      "Skipping test on RISC-V due to pre-existing failure";
#elif defined(_WIN32)
  // TODO(b/434184677): Add support for fixups on Windows if needed
  const char* kSkipReason =
      "Skipping test on Windows due to lack of support for fixups";
#else
  const char* kSkipReason = nullptr;
#endif

  // This conditional is to avoid an unreachable code warning.
  if (kSkipReason != nullptr) {
    GTEST_SKIP() << kSkipReason;
  }

  bool can_rely_on_frame_pointers = false;
  if (!can_rely_on_frame_pointers) {
    GTEST_SKIP() << "Frame pointers are required, but not guaranteed in OSS";
  }

  // This test is known not to pass on MSVC (due to weak symbols)

  const Cleanup restore_state([enable_fixup = g_enable_fixup,
                               fixup_calls = g_fixup_calls,
                               should_fixup_calls = g_should_fixup_calls]() {
    g_enable_fixup = enable_fixup;
    g_fixup_calls = fixup_calls;
    g_should_fixup_calls = should_fixup_calls;
  });

  constexpr int kSkip = 1;  // Skip our own frame, whose return PCs won't match
  constexpr auto kStackCount = 1;

  StackTrace a;
  StackTrace b;

  // ==========================================================================

  g_fixup_calls = 0;
  g_should_fixup_calls = 0;
  a.depth = turbo::GetStackTrace(a.result, kStackCount, kSkip);
  g_enable_fixup = !g_enable_fixup;
  b.depth = turbo::GetStackTrace(b.result, kStackCount, kSkip);
  EXPECT_THAT(
      turbo::make_span(a.result, static_cast<size_t>(a.depth)),
      ContainerEq(turbo::make_span(b.result, static_cast<size_t>(b.depth))));
  EXPECT_GT(g_should_fixup_calls, 0);
  EXPECT_GE(g_should_fixup_calls, g_fixup_calls);

  // ==========================================================================

  g_fixup_calls = 0;
  g_should_fixup_calls = 0;
  a.depth = turbo::GetStackFrames(a.result, a.sizes, kStackCount, kSkip);
  g_enable_fixup = !g_enable_fixup;
  b.depth = turbo::GetStackFrames(b.result, b.sizes, kStackCount, kSkip);
  EXPECT_THAT(
      turbo::make_span(a.result, static_cast<size_t>(a.depth)),
      ContainerEq(turbo::make_span(b.result, static_cast<size_t>(b.depth))));
  EXPECT_THAT(
      turbo::make_span(a.sizes, static_cast<size_t>(a.depth)),
      ContainerEq(turbo::make_span(b.sizes, static_cast<size_t>(b.depth))));
  EXPECT_GT(g_should_fixup_calls, 0);
  EXPECT_GE(g_should_fixup_calls, g_fixup_calls);

  // ==========================================================================

  g_fixup_calls = 0;
  g_should_fixup_calls = 0;
  a.depth = turbo::GetStackTraceWithContext(a.result, kStackCount, kSkip,
                                           nullptr, nullptr);
  g_enable_fixup = !g_enable_fixup;
  b.depth = turbo::GetStackTraceWithContext(b.result, kStackCount, kSkip,
                                           nullptr, nullptr);
  EXPECT_THAT(
      turbo::make_span(a.result, static_cast<size_t>(a.depth)),
      ContainerEq(turbo::make_span(b.result, static_cast<size_t>(b.depth))));
  EXPECT_GT(g_should_fixup_calls, 0);
  EXPECT_GE(g_should_fixup_calls, g_fixup_calls);

  // ==========================================================================

  g_fixup_calls = 0;
  g_should_fixup_calls = 0;
  a.depth = turbo::GetStackFramesWithContext(a.result, a.sizes, kStackCount,
                                            kSkip, nullptr, nullptr);
  g_enable_fixup = !g_enable_fixup;
  b.depth = turbo::GetStackFramesWithContext(b.result, b.sizes, kStackCount,
                                            kSkip, nullptr, nullptr);
  EXPECT_THAT(
      turbo::make_span(a.result, static_cast<size_t>(a.depth)),
      ContainerEq(turbo::make_span(b.result, static_cast<size_t>(b.depth))));
  EXPECT_THAT(
      turbo::make_span(a.sizes, static_cast<size_t>(a.depth)),
      ContainerEq(turbo::make_span(b.sizes, static_cast<size_t>(b.depth))));
  EXPECT_GT(g_should_fixup_calls, 0);
  EXPECT_GE(g_should_fixup_calls, g_fixup_calls);
}

TEST(StackTrace, FixupNoFixupEquivalence) { FixupNoFixupEquivalenceNoInline(); }

TEST(StackTrace, FixupLowStackUsage) {
#if !TURBO_HAVE_ATTRIBUTE_WEAK
  const char* kSkipReason = "Skipping test on MSVC due to weak symbols";
#elif defined(_WIN32)
  // TODO(b/434184677): Add support for fixups on Windows if needed
  const char* kSkipReason =
      "Skipping test on Windows due to lack of support for fixups";
#else
  const char* kSkipReason = nullptr;
#endif

  // This conditional is to avoid an unreachable code warning.
  if (kSkipReason != nullptr) {
    GTEST_SKIP() << kSkipReason;
  }

  const Cleanup restore_state([enable_fixup = g_enable_fixup,
                               fixup_calls = g_fixup_calls,
                               should_fixup_calls = g_should_fixup_calls]() {
    g_enable_fixup = enable_fixup;
    g_fixup_calls = fixup_calls;
    g_should_fixup_calls = should_fixup_calls;
  });

  g_enable_fixup = true;

  // Request a ton of stack frames, regardless of how many are actually used.
  // It's fine to request more frames than we have, since functions preallocate
  // memory before discovering how high the stack really is, and we're really
  // just trying to make sure the preallocations don't overflow the stack.
  //
  // Note that we loop in order to cover all sides of any branches in the
  // implementation that switch allocation behavior (e.g., from stack to heap)
  // and to ensure that no sides allocate too much stack space.
  constexpr size_t kPageSize = 4096;
  for (size_t depth = 2; depth < (1 << 20); depth += depth / 2) {
    const auto stack = std::make_unique<void*[]>(depth);
    const auto frames = std::make_unique<int[]>(depth);

    turbo::GetStackFrames(stack.get(), frames.get(), static_cast<int>(depth), 0);
    const void* frame_address = nullptr;
#if KUMO_HAVE_BUILTIN(__builtin_frame_address)
    frame_address = __builtin_frame_address(0);
#endif
    size_t stack_usage =
        reinterpret_cast<uintptr_t>(frame_address) - g_last_fixup_frame_address;
    EXPECT_LT(stack_usage, kPageSize);
  }
}

TEST(StackTrace, CustomUnwinderPerformsFixup) {
#if !TURBO_HAVE_ATTRIBUTE_WEAK
  const char* kSkipReason = "Need weak symbol support";
#elif defined(_WIN32)
  // TODO(b/434184677): Add support for fixups on Windows if needed
  const char* kSkipReason =
      "Skipping test on Windows due to lack of support for fixups";
#else
  const char* kSkipReason = nullptr;
#endif

  // This conditional is to avoid an unreachable code warning.
  if (kSkipReason != nullptr) {
    GTEST_SKIP() << kSkipReason;
  }

  constexpr int kSkip = 1;  // Skip our own frame, whose return PCs won't match
  constexpr auto kStackCount = 1;

  turbo::SetStackUnwinder(turbo::DefaultStackUnwinder);
  const Cleanup restore_state([enable_fixup = g_enable_fixup,
                               fixup_calls = g_fixup_calls,
                               should_fixup_calls = g_should_fixup_calls]() {
    turbo::SetStackUnwinder(nullptr);
    g_enable_fixup = enable_fixup;
    g_fixup_calls = fixup_calls;
    g_should_fixup_calls = should_fixup_calls;
  });

  StackTrace trace;

  g_enable_fixup = true;
  g_should_fixup_calls = 0;
  g_fixup_calls = 0;
  turbo::GetStackTrace(trace.result, kSkip, kStackCount);
  EXPECT_GT(g_should_fixup_calls, 0);
  EXPECT_GT(g_fixup_calls, 0);

  g_enable_fixup = true;
  g_should_fixup_calls = 0;
  g_fixup_calls = 0;
  turbo::GetStackFrames(trace.result, trace.sizes, kSkip, kStackCount);
  EXPECT_GT(g_should_fixup_calls, 0);
  EXPECT_GT(g_fixup_calls, 0);

  g_enable_fixup = true;
  g_should_fixup_calls = 0;
  g_fixup_calls = 0;
  turbo::GetStackTraceWithContext(trace.result, kSkip, kStackCount, nullptr,
                                 nullptr);
  EXPECT_GT(g_should_fixup_calls, 0);
  EXPECT_GT(g_fixup_calls, 0);

  g_enable_fixup = true;
  g_should_fixup_calls = 0;
  g_fixup_calls = 0;
  turbo::GetStackFramesWithContext(trace.result, trace.sizes, kSkip, kStackCount,
                                  nullptr, nullptr);
  EXPECT_GT(g_should_fixup_calls, 0);
  EXPECT_GT(g_fixup_calls, 0);
}

// This test is Linux specific.
#if defined(__linux__)
const void* g_return_address = nullptr;
bool g_sigusr2_raised = false;

void SigUsr2Handler(int, siginfo_t*, void* uc) {
  turbo::ErrnoSaver errno_saver;
  // Many platforms don't support this by default.
  bool support_is_expected = false;
  constexpr int kMaxStackDepth = 64;
  void* result[kMaxStackDepth];
  int depth =
      turbo::GetStackTraceWithContext(result, kMaxStackDepth, 0, uc, nullptr);
  // Verify we can unwind past the nested signal handlers.
  if (support_is_expected) {
    EXPECT_THAT(turbo::make_span(result, static_cast<size_t>(depth)),
                Contains(g_return_address).Times(1));
  }
  depth = turbo::GetStackTrace(result, kMaxStackDepth, 0);
  if (support_is_expected) {
    EXPECT_THAT(turbo::make_span(result, static_cast<size_t>(depth)),
                Contains(g_return_address).Times(1));
  }
  g_sigusr2_raised = true;
}

void SigUsr1Handler(int, siginfo_t*, void*) {
  raise(SIGUSR2);
  KUMO_BLOCK_TAIL_CALL_OPTIMIZATION();
}

KUMO_ATTRIBUTE_NOINLINE void RaiseSignal() {
  g_return_address = __builtin_return_address(0);
  raise(SIGUSR1);
  KUMO_BLOCK_TAIL_CALL_OPTIMIZATION();
}

KUMO_ATTRIBUTE_NOINLINE void TestNestedSignal() {
  constexpr size_t kAltstackSize = 1 << 14;
  // Allocate altstack on regular stack to make sure it'll have a higher
  // address than some of the regular stack frames.
  char space[kAltstackSize];
  stack_t altstack;
  stack_t old_stack;
  altstack.ss_sp = space;
  altstack.ss_size = kAltstackSize;
  altstack.ss_flags = 0;
  ASSERT_EQ(sigaltstack(&altstack, &old_stack), 0) << strerror(errno);
  struct sigaction act;
  struct sigaction oldusr1act;
  struct sigaction oldusr2act;
  act.sa_sigaction = SigUsr1Handler;
  act.sa_flags = SA_SIGINFO | SA_ONSTACK;
  sigemptyset(&act.sa_mask);
  ASSERT_EQ(sigaction(SIGUSR1, &act, &oldusr1act), 0) << strerror(errno);
  act.sa_sigaction = SigUsr2Handler;
  ASSERT_EQ(sigaction(SIGUSR2, &act, &oldusr2act), 0) << strerror(errno);
  RaiseSignal();
  ASSERT_EQ(sigaltstack(&old_stack, nullptr), 0) << strerror(errno);
  ASSERT_EQ(sigaction(SIGUSR1, &oldusr1act, nullptr), 0) << strerror(errno);
  ASSERT_EQ(sigaction(SIGUSR2, &oldusr2act, nullptr), 0) << strerror(errno);
  KUMO_BLOCK_TAIL_CALL_OPTIMIZATION();
}

TEST(StackTrace, NestedSignal) {
  // Verify we can unwind past the nested signal handlers.
  TestNestedSignal();
  EXPECT_TRUE(g_sigusr2_raised);
}
#endif

TEST(StackTrace, NoNullptrInPopulatedRange) {
  constexpr int kMaxDepth = 1024;
  void* results[kMaxDepth];
  int depth = turbo::GetStackTrace(results, kMaxDepth, 0);
  for (int i = 0; i < depth; ++i) {
    EXPECT_NE(results[i], nullptr) << "Unexpected nullptr found at index " << i;
  }
}


#if defined(__aarch64__) && defined(__linux__)
static void CorruptedSigStackHandler(int, siginfo_t*, void*) {
  void** fp = reinterpret_cast<void**>(__builtin_frame_address(0));
  void* saved_fp = fp[0];
  fp[0] = reinterpret_cast<void*>(0x7deadbeef000ULL);  // Unmapped address

  void* stack[16];
  turbo::GetStackTrace(stack, 16, 0);

  fp[0] = saved_fp;
}
#endif

TEST(StackTrace, CorruptedSignalStackFrameSafety) {
#if defined(__aarch64__) && defined(__linux__)
  stack_t sigstk{};
  constexpr size_t kAltstackSize = 1 << 14;
  char altstack[kAltstackSize];
  sigstk.ss_sp = altstack;
  sigstk.ss_size = kAltstackSize;
  sigstk.ss_flags = 0;
  ASSERT_EQ(sigaltstack(&sigstk, nullptr), 0);

  struct sigaction act{}, oldact{};
  act.sa_sigaction = CorruptedSigStackHandler;
  act.sa_flags = SA_SIGINFO | SA_ONSTACK;
  ASSERT_EQ(sigaction(SIGUSR1, &act, &oldact), 0);

  raise(SIGUSR1);

  sigaction(SIGUSR1, &oldact, nullptr);
  stack_t disable_stk{};
  disable_stk.ss_flags = SS_DISABLE;
  sigaltstack(&disable_stk, nullptr);
#endif
}

}  // namespace
