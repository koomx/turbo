#ifndef SIMDUTF_COMMON_DEFS_H
#define SIMDUTF_COMMON_DEFS_H

#include <turbo/unicode/engine/portability.h>
#include <turbo/unicode/engine/avx512.h>

// Sometimes logging is useful, but we want it disabled by default
// and free of any logging code in release builds.
#ifdef SIMDUTF_LOGGING
#include <cstdlib>
#include <iostream>
#define simdutf_log(msg)                                          \
    std::cout << "[" << __FUNCTION__ << "]: " << msg << std::endl \
              << "\t" << __FILE__ << ":" << __LINE__ << std::endl;
#define simdutf_log_assert(cond, msg)                                      \
    do {                                                                   \
        if (!(cond)) {                                                     \
            std::cerr << "[" << __FUNCTION__ << "]: " << msg << std::endl  \
                      << "\t" << __FILE__ << ":" << __LINE__ << std::endl; \
            std::abort();                                                  \
        }                                                                  \
    } while (0)
#else
#define simdutf_log(msg)
#define simdutf_log_assert(cond, msg)
#endif

#if defined(SIMDUTF_REGULAR_VISUAL_STUDIO)

#define SIMDUTF_PUSH_DISABLE_WARNINGS __pragma(warning(push))
#define SIMDUTF_PUSH_DISABLE_ALL_WARNINGS __pragma(warning(push, 0))
#define SIMDUTF_DISABLE_VS_WARNING(WARNING_NUMBER) \
    __pragma(warning(disable : WARNING_NUMBER))
// Get rid of Intellisense-only warnings (Code Analysis)
// Though __has_include is C++17, it is supported in Visual Studio 2017 or
// better (_MSC_VER>=1910).
#ifdef __has_include
#if __has_include(<CppCoreCheck\Warnings.h>)
#include <CppCoreCheck\Warnings.h>
#define SIMDUTF_DISABLE_UNDESIRED_WARNINGS \
    SIMDUTF_DISABLE_VS_WARNING(ALL_CPPCORECHECK_WARNINGS)
#endif
#endif

#ifndef SIMDUTF_DISABLE_UNDESIRED_WARNINGS
#define SIMDUTF_DISABLE_UNDESIRED_WARNINGS
#endif

#define SIMDUTF_DISABLE_DEPRECATED_WARNING SIMDUTF_DISABLE_VS_WARNING(4996)
#define SIMDUTF_DISABLE_STRICT_OVERFLOW_WARNING
#define SIMDUTF_POP_DISABLE_WARNINGS __pragma(warning(pop))
#define SIMDUTF_DISABLE_UNUSED_WARNING
#else // SIMDUTF_REGULAR_VISUAL_STUDIO

// clang-format off
  #define SIMDUTF_PUSH_DISABLE_WARNINGS _Pragma("GCC diagnostic push")
  // gcc doesn't seem to disable all warnings with all and extra, add warnings
  // here as necessary
  #define SIMDUTF_PUSH_DISABLE_ALL_WARNINGS                                    \
    SIMDUTF_PUSH_DISABLE_WARNINGS                                              \
    SIMDUTF_DISABLE_GCC_WARNING(-Weffc++)                                      \
    SIMDUTF_DISABLE_GCC_WARNING(-Wall)                                         \
    SIMDUTF_DISABLE_GCC_WARNING(-Wconversion)                                  \
    SIMDUTF_DISABLE_GCC_WARNING(-Wextra)                                       \
    SIMDUTF_DISABLE_GCC_WARNING(-Wattributes)                                  \
    SIMDUTF_DISABLE_GCC_WARNING(-Wimplicit-fallthrough)                        \
    SIMDUTF_DISABLE_GCC_WARNING(-Wnon-virtual-dtor)                            \
    SIMDUTF_DISABLE_GCC_WARNING(-Wreturn-type)                                 \
    SIMDUTF_DISABLE_GCC_WARNING(-Wshadow)                                      \
    SIMDUTF_DISABLE_GCC_WARNING(-Wunused-parameter)                            \
    SIMDUTF_DISABLE_GCC_WARNING(-Wunused-variable)
  #define SIMDUTF_PRAGMA(P) _Pragma(#P)
  #define SIMDUTF_DISABLE_GCC_WARNING(WARNING)                                 \
    SIMDUTF_PRAGMA(GCC diagnostic ignored #WARNING)
  #if defined(SIMDUTF_CLANG_VISUAL_STUDIO)
    #define SIMDUTF_DISABLE_UNDESIRED_WARNINGS                                 \
      SIMDUTF_DISABLE_GCC_WARNING(-Wmicrosoft-include)
  #else
    #define SIMDUTF_DISABLE_UNDESIRED_WARNINGS
  #endif
  #define SIMDUTF_DISABLE_DEPRECATED_WARNING                                   \
    SIMDUTF_DISABLE_GCC_WARNING(-Wdeprecated-declarations)
  #define SIMDUTF_DISABLE_STRICT_OVERFLOW_WARNING                              \
    SIMDUTF_DISABLE_GCC_WARNING(-Wstrict-overflow)
  #define SIMDUTF_POP_DISABLE_WARNINGS _Pragma("GCC diagnostic pop")
  #define SIMDUTF_DISABLE_UNUSED_WARNING                                       \
    SIMDUTF_PUSH_DISABLE_WARNINGS                                              \
    SIMDUTF_DISABLE_GCC_WARNING(-Wunused-function)                             \
    SIMDUTF_DISABLE_GCC_WARNING(-Wunused-const-variable)
// clang-format on

#endif // MSC_VER

#endif // SIMDUTF_COMMON_DEFS_H
