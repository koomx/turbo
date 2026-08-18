# Copyright (C) Kumo inc. and its affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

add_subdirectory(src)

kmcmake_cc_benchmark(
        MODULE unicode
        NAME stream
        SOURCES
        stream.cpp
        CXXOPTS
        ${KMCMAKE_CXX_OPTIONS}
        LINKS
        turbo::turbo_static
        ${KMCMAKE_DEPS_LINK}
)

kmcmake_cc_benchmark(
        MODULE unicode
        NAME benchmark
        SOURCES
        benchmark.cpp
        CXXOPTS
        ${KMCMAKE_CXX_OPTIONS}
        LINKS
        turbo::turbo_static
        turbo::unicode_benchmark_static
        ${KMCMAKE_DEPS_LINK}
)

kmcmake_cc_benchmark(
        MODULE unicode
        NAME alignment
        SOURCES
        alignment.cpp
        CXXOPTS
        ${KMCMAKE_CXX_OPTIONS}
        LINKS
        turbo::turbo_static
        ${KMCMAKE_DEPS_LINK}
)

find_package(Threads)
if (Threads_FOUND)
    kmcmake_cc_benchmark(
            MODULE unicode
            NAME threaded
            SOURCES
            threaded.cpp
            CXXOPTS
            ${KMCMAKE_CXX_OPTIONS}
            LINKS
            turbo::turbo_static
            Threads::Threads
            ${KMCMAKE_DEPS_LINK}
    )
endif ()

kmcmake_cc_benchmark(
        MODULE unicode
        NAME benchmark_to_well_formed_utf16
        SOURCES
        benchmark_to_well_formed_utf16.cpp
        CXXOPTS
        ${KMCMAKE_CXX_OPTIONS}
        LINKS
        turbo::turbo_static
        turbo::unicode_benchmark_static
        ${KMCMAKE_DEPS_LINK}
)

kmcmake_cc_benchmark(
        MODULE unicode
        NAME shortbench
        SOURCES
        shortbench.cpp
        CXXOPTS
        ${KMCMAKE_CXX_OPTIONS}
        LINKS
        turbo::turbo_static
        turbo::unicode_benchmark_static
        ${KMCMAKE_DEPS_LINK}
)

add_subdirectory(find)
