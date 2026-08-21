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
###########################################################################
# below are examples, after initialization, modify it to your own.
###########################################################################
###kmcmake_example

set(ARCH_SRC
        arch/isa.cc
)
set(BASE_SRC
        base/log_severity.cc
        base/throw_delegate.cc
        base/internal/cpu_detect.cc
        base/internal/cycleclock.cc
        base/internal/hardening.cc
        base/internal/low_level_alloc.cc
        base/internal/poison.cc
        base/internal/raw_logging.cc
        base/internal/scoped_set_env.cc
        base/internal/spinlock_akaros.inc
        base/internal/spinlock.cc
        base/internal/spinlock_linux.inc
        base/internal/spinlock_posix.inc
        base/internal/spinlock_wait.cc
        base/internal/spinlock_win32.inc
        base/internal/strerror.cc
        base/internal/sysinfo.cc
        base/internal/thread_identity.cc
        base/internal/tracing.cc
        base/internal/unscaledcycleclock.cc
)


set(BITS_SRC
        bits/casts.cc
)


set(CRC_SRC
        crc/crc32c.cc
        crc/internal/crc.cc
        crc/internal/crc_cord_state.cc
        crc/internal/crc_memcpy_fallback.cc
        crc/internal/crc_memcpy_x86_arm_combined.cc
        crc/internal/crc_non_temporal_memcpy.cc
        crc/internal/crc_x86_arm_combined.cc
)

set(DBG_SRC
        debugging/failure_signal_handler.cc
        debugging/internal/address_is_readable.cc
        debugging/internal/decode_rust_punycode.cc
        debugging/internal/demangle.cc
        debugging/internal/demangle_rust.cc
        debugging/internal/elf_mem_image.cc
        debugging/internal/examine_stack.cc
        debugging/internal/stack_consumption.cc
        debugging/internal/stacktrace_aarch64-inl.inc
        debugging/internal/stacktrace_arm-inl.inc
        debugging/internal/stacktrace_emscripten-inl.inc
        debugging/internal/stacktrace_generic-inl.inc
        debugging/internal/stacktrace_powerpc-inl.inc
        debugging/internal/stacktrace_riscv-inl.inc
        debugging/internal/stacktrace_unimplemented-inl.inc
        debugging/internal/stacktrace_win32-inl.inc
        debugging/internal/stacktrace_x86-inl.inc
        debugging/internal/utf8_for_code_point.cc
        debugging/internal/vdso_support.cc
        debugging/leak_check.cc
        debugging/stacktrace.cc
        debugging/symbolize.cc
        debugging/symbolize_darwin.inc
        debugging/symbolize_elf.inc
        debugging/symbolize_emscripten.inc
        debugging/symbolize_unimplemented.inc
        debugging/symbolize_win32.inc
)


set(ENCODING_SRC
        encoding/base58.cc
        encoding/base64.cc
        encoding/url.cc
)

set(FLAGS_SRC
        flags/commandlineflag.cc
        flags/internal/commandlineflag.cc
        flags/internal/flag.cc
        flags/internal/private_handle_accessor.cc
        flags/argv.cc
        flags/marshalling.cc
        flags/reflection.cc
        flags/usage_config.cc
)

set(HASH_SRC
        hash/internal/city.cc
        hash/internal/hash.cc
)

set(LOG_SRC
        log/die_if_null.cc
        log/globals.cc
        log/initialize.cc
        log/internal/check_op.cc
        log/internal/conditions.cc
        log/internal/globals.cc
        log/internal/log_format.cc
        log/internal/log_message.cc
        log/internal/log_sink_set.cc
        log/internal/nullguard.cc
        log/internal/proto.cc
        log/internal/structured_proto.cc
        log/internal/vlog_config.cc
        log/log_entry.cc
        log/log_sink.cc
        log/sinks/android_log_sink.cc
        log/sinks/ansi_color_sink.cc
        log/sinks/async_sink.cc
        log/sinks/daily_file_sink.cc
        log/sinks/hourly_file_sink.cc
        log/sinks/internal/append_file.cc
        log/sinks/internal/fs_helper.cc
        log/sinks/log_filename.cc
        log/sinks/rotating_file_sink.cc
        log/sinks/sink_factory.cc
        log/sinks/stderr_log_sink.cc
        log/sinks/windows_debugger_sink.cc
)

set(NUM_SRC
        numeric/int128.cc
        numeric/int128_have_intrinsic.inc
        numeric/int128_no_intrinsic.inc
)



set(STATUS_SRC
        status/internal/status_internal.cc
        status/status.cc
        status/status_builder.cc
        status/status_payload_printer.cc
        status/result.cc
)

set(STRINGS_SRC
        strings/ascii.cc
        strings/charconv.cc
        strings/escaping.cc
        strings/internal/charconv_bigint.cc
        strings/internal/charconv_parse.cc
        strings/internal/damerau_levenshtein_distance.cc
        log/internal/generic_printer.cc
        strings/internal/memutil.cc
        strings/internal/pow10_helper.cc
        strings/internal/stringify_sink.cc
        strings/match.cc
        strings/numbers.cc
        strings/str_cat.cc
        strings/str_replace.cc
        strings/str_split.cc
        strings/substitute.cc
        strings/strip.cc
)

set(CORD_SRC
        cord/cord.cc
        cord/cord_analysis.cc
        cord/internal/cord_internal.cc
        cord/internal/cord_rep_btree.cc
        cord/internal/cord_rep_btree_navigator.cc
        cord/internal/cord_rep_btree_reader.cc
        cord/internal/cord_rep_consume.cc
        cord/internal/cord_rep_crc.cc
)

set(FORMAT_SRC
        format/str_format/arg.cc
        format/str_format/bind.cc
        format/str_format/extension.cc
        format/str_format/float_conversion.cc
        format/str_format/output.cc
        format/str_format/parser.cc
        format/internal/utf8.cc
        format/ostringstream.cc
        format/fast_to_buffer.cc
)


set(TIME_SRC
        time/civil_time.cc
        time/clock.cc
        time/duration.cc
        time/format.cc
        time/time.cc
)

set(CCTZ_SRC
        cctz/civil_time_detail.cc
        cctz/time_zone_fixed.cc
        cctz/time_zone_format.cc
        cctz/time_zone_if.cc
        cctz/time_zone_impl.cc
        cctz/time_zone_info.cc
        cctz/time_zone_libc.cc
        cctz/time_zone_lookup.cc
        cctz/time_zone_name_win.cc
        cctz/time_zone_posix.cc
        cctz/zone_info_source.cc
)

set(TYPES_SRC
        types/source_location.cc
)

set(UNICODE_SRC
        unicode/text_encoding.cc
        unicode/utf.cc
        unicode/utf_c.cc
        unicode/api/ascii.cc
        unicode/api/base64.cc
        unicode/api/detect.cc
        unicode/api/find.cc
        unicode/api/latin1.cc
        unicode/api/utf8.cc
        unicode/api/utf16.cc
        unicode/api/utf32.cc
        unicode/engine/isa_select.cc
        unicode/engine/interface.cc
        unicode/engine/arm64/implementation.cc
        unicode/engine/fallback/implementation.cc
        unicode/engine/icelake/implementation.cc
        unicode/engine/haswell/implementation.cc
        unicode/engine/ppc64/implementation.cc
        unicode/engine/rvv/implementation.cc
        unicode/engine/westmere/implementation.cc
        unicode/engine/lasx/implementation.cc
        unicode/engine/lsx/implementation.cc
)

set(TURBO_SRC
        ${ARCH_SRC}
        ${BASE_SRC}
        ${BITS_SRC}
        ${CORD_SRC}
        ${CRC_SRC}
        ${DBG_SRC}
        ${ENCODING_SRC}
        ${FLAGS_SRC}
        ${FORMAT_SRC}
        ${HASH_SRC}
        ${LOG_SRC}
        ${NUM_SRC}
        ${STATUS_SRC}
        ${STRINGS_SRC}
        ${TIME_SRC}
        ${CCTZ_SRC}
        ${TYPES_SRC}
        ${UNICODE_SRC}
)

kmcmake_cc_library(
        NAMESPACE ${PROJECT_NAME}
        NAME turbo
        SOURCES
        ${TURBO_SRC}
        CXXOPTS
        ${KMCMAKE_CXX_OPTIONS}
        PLINKS
        ${KMCMAKE_DEPS_LINK}
        PUBLIC
)


kmcmake_cc_interface(
        NAMESPACE ${PROJECT_NAME}
        NAME skills
        HEADERS
        skills.h
        CXXOPTS
        ${KMCMAKE_CXX_OPTIONS}
        PUBLIC
)
