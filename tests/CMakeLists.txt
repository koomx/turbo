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



configure_file(${PROJECT_SOURCE_DIR}/tests/config.h.in ${PROJECT_SOURCE_DIR}/tests/config.h @ONLY)

if (NOT TARGET GTest::gtest)
    find_package(GTest REQUIRED)
endif ()


add_subdirectory(algorithm)
add_subdirectory(base)
add_subdirectory(bits)
add_subdirectory(cctz)
add_subdirectory(cleanup)
add_subdirectory(container)
add_subdirectory(cord)
add_subdirectory(crc)
add_subdirectory(debugging)
add_subdirectory(flags)
add_subdirectory(functional)
add_subdirectory(format)
add_subdirectory(hash)
add_subdirectory(log)
add_subdirectory(macros)
add_subdirectory(memory)
add_subdirectory(meta)
add_subdirectory(numeric)
add_subdirectory(random)
add_subdirectory(status)
add_subdirectory(strings)
#add_subdirectory(synchronization)
add_subdirectory(time)
add_subdirectory(types)

