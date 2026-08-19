#include <turbo/unicode/utf.h>


#include <iomanip>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

#include <tests/unicode/helpers/test.h>
using random_generator = std::mt19937;
static random_generator::result_type seed = 42;
// check if we are running with thread sanitizer
#if defined(__clang__)
#if __has_feature(thread_sanitizer)
#define RUNNING_UNDER_THREAD_SANITIZER 1
#else
#define RUNNING_UNDER_THREAD_SANITIZER 0
#endif
#elif defined(__GNUC__)
#if defined(__SANITIZE_THREAD__)
#define RUNNING_UNDER_THREAD_SANITIZER 1
#else
#define RUNNING_UNDER_THREAD_SANITIZER 0
#endif
#else
#define RUNNING_UNDER_THREAD_SANITIZER 0
#endif

TEST_MAIN
