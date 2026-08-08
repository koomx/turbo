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


add_executable(ensure_utf8 ensure_utf8.cc)
add_executable(ensure_utf8_twice ensure_utf8_twice.cc)
target_link_libraries(ensure_utf8 PRIVATE ${PROJECT_NAME}::turbo_static)
target_link_libraries(ensure_utf8_twice PRIVATE ${PROJECT_NAME}::turbo_static)

set(CLI_TESTS
        simple_test
        app_test
        helpers_test
        creation_test
        subcommand_test
        option_type_test
        numeric_type_test
        set_test
        transform_test
        help_test
        formatter_test
        new_parse_test
        config_file_test
        optional_test
        string_parse_test
        complex_type_test
        true_false_test
        locale_test
        option_group_test
        extra_validators_test
        encoding_test
        flag_bind_test
)

foreach (T ${CLI_TESTS})
    kmcmake_cc_test(
            NAME ${T}
            MODULE cli
            SOURCES
            ${T}.cc
            main.cc
            CXXOPTS
            ${KMCMAKE_CXX_OPTIONS}
            LINKS
            ${PROJECT_NAME}::turbo_static
    )
    if (${T} STREQUAL "app_test")
        target_compile_definitions(cli_${T} PRIVATE
                XCLI_ENSURE_UTF8_EXE="$<TARGET_FILE:ensure_utf8>"
                XCLI_ENSURE_UTF8_TWICE_EXE="$<TARGET_FILE:ensure_utf8_twice>"
        )
        target_link_libraries(cli_${T} PRIVATE Threads::Threads)
        add_dependencies(cli_${T} ensure_utf8 ensure_utf8_twice)
    endif ()
endforeach ()
