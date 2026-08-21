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

kmcmake_cc_library(
        NAME unicode_benchmark
        NAMESPACE ${PROJECT_NAME}
        SOURCES
        benchmark_base.cpp
        cmdline.cpp
        benchmark.cpp
        CXXOPTS
        ${KMCMAKE_CXX_OPTIONS}
        PLINKS
        turbo::turbo_static
        turbo::unicode_test_helpers_static
        ${KMCMAKE_DEPS_LINK}
)
