// Copyright (c) 2017-2026, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <turbo/cli/cli.h>

#include <iostream>

int main() {
    std::cout << "\nCLI11 information:\n";

    std::cout << "  C++ standard: ";
#if defined(XCLI_CPP20)
    std::cout << 20;
#elif defined(XCLI_CPP17)
    std::cout << 17;
#elif defined(XCLI_CPP14)
    std::cout << 14;
#else
    std::cout << 11;
#endif
    std::cout << "\n";

    std::cout << "  __has_include: ";
#ifdef __has_include
    std::cout << "yes\n";
#else
    std::cout << "no\n";
#endif

#if XCLI_OPTIONAL
    std::cout << "  [Available as xcli::optional]";
#else
    std::cout << "  No optional library found\n";
#endif

#if XCLI_STD_OPTIONAL
    std::cout << "  std::optional support active\n";
#endif

#if XCLI_EXPERIMENTAL_OPTIONAL
    std::cout << "  std::experimental::optional support active\n";
#endif

#if XCLI_BOOST_OPTIONAL
    std::cout << "  boost::optional support active\n";
#endif

    std::cout << '\n';
}
