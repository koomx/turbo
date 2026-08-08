// Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// AI: This file is a human/AI-readable summary of the project's public API and
// AI: design principles. Read this instead of scanning all source files.
// AI: Style: triple-slash Doxygen comments for all entries.

#pragma once

/// @defgroup project_summary Project Summary
/// @brief xcli — header-only C++ CLI library, derived from CLI11 with gflags-compatible global flags

/// @brief What is xcli
/// AI: xcli is a header-only C++ command-line parsing library forked from CLI11 (v2.6.2).
/// AI: It inherits CLI11's full API (App, Option, subcommands, validators, formatters, config)
/// AI: and adds a gflags-compatible global flag system (XDEFINE / XDECLARE / FlagRegistry).
/// AI:
/// AI: Key design principles:
/// AI:   - Single `#include <turbo/cli/cli.h>` to use everything
/// AI:   - Two API styles: local variables (CLI11-style) and global flags (gflags-style)
/// AI:   - Zero type erasure — parsing uses templates and SFINAE
/// AI:   - No exceptions disabled — errors are reported via ParseError (catchable)
/// AI:   - Header-only, no .cc to compile

/// @defgroup public_api Public API Overview
/// AI: All public types are in namespace `xcli`. Include `<xcli/cli.h>` for everything.
/// @{

/// @brief xcli::App — the main program / subcommand
/// AI: Create an App, add options/flags, then call parse(argc, argv).
/// AI:   xcli::App app("my tool");
/// AI:   app.add_option("--name", name_var, "your name");
/// AI:   app.add_flag("--verbose", verbose_var, "enable verbose");
/// AI:   XCLI_PARSE(app, argc, argv);
/// AI:
/// AI: Subcommands: app.add_subcommand("serve", "start server");
/// AI: Option groups: app.add_option_group("advanced");

/// @brief xcli::Option — one option/flag on an App
/// AI: Returned by add_flag / add_option. Chain setters:
/// AI:   app.add_flag("--flag")->required()->group("advanced");
/// AI:
/// AI: Key methods:
/// AI:   - required(), group(), check(validator), transform(validator)
/// AI:   - multi_option_policy(TakeAll|Sum|Join|...)
/// AI:   - default_str(), default_val(), capture_default_str()
/// AI:   - needs(), excludes() — dependency management
/// AI:   - ignore_case(), ignore_underscore()

/// @brief xcli::add_option — typed option (takes a value)
/// AI:   std::string name;
/// AI:   app.add_option("--name,-n", name, "your name");
/// AI:   int port = 8080;
/// AI:   app.add_option("--port,-p", port, "listening port");
/// AI: Works with any type that supports streaming >> / <<.

/// @brief xcli::add_flag — boolean / counter flag
/// AI:   bool verbose = false;
/// AI:   app.add_flag("--verbose,-V", verbose);
/// AI:   int count = 0;
/// AI:   app.add_flag("--verbosity{1},-V{1},--quiet{-1}", count);
/// AI:
/// AI: For integral types > 1 byte, flag defaults to Sum mode (counter).
/// AI: For bool and small types, defaults to always_capture_default.

/// @brief xcli::add_subcommand — nested commands
/// AI:   auto *serve = app.add_subcommand("serve", "start server");
/// AI:   serve->add_option("--port", port);
/// AI:   serve->callback([] { run_server(); });
/// AI:   XCLI_PARSE(app, argc, argv);
/// AI:   if (*serve) { /* was invoked */ }

/// @brief Validators — input checking
/// AI: Built-in: xcli::ExistingFile, xcli::ExistingDirectory,
/// AI:   xcli::NonexistentPath, xcli::Range(min,max),
/// AI:   xcli::PositiveNumber, xcli::Number.
/// AI: Custom: app.add_option("--age", age)->check(xcli::Range(0, 150));
/// AI: or:     app.add_option("--email", email)->check([](const std::string &s) {
/// AI:           return s.find('@') == std::string::npos ? "no @" : std::string{};
/// AI:         });

/// @brief Formatter — customize help output
/// AI: Replace the default formatter:
/// AI:   app.formatter(std::make_shared<xcli::Formatter>());
/// AI: Or use a lambda:
/// AI:   app.formatter_fn([](const xcli::App *, std::string, xcli::AppFormatMode) {
/// AI:     return "custom help";
/// AI:   });

/// @brief Config — TOML/INI config file support
/// AI:   app.set_config("--config", "config.toml");
/// AI: Reads options from a config file before CLI args (CLI args win).
/// @}

/// @defgroup gflags_api GFlags Layer (XDEFINE / XDECLARE)
/// @brief Global flag system, inspired by Google Flags (gflags).
/// AI: Use XDEFINE/XFLAGS for flags shared across translation units.
/// AI: Use XDECLARE to access a flag defined in another .cc file.
///
/// AI: The gflags layer is designed for large projects where multiple
/// AI: translation units need to read/write the same flag. For simple
/// AI: single-file programs, prefer `app.add_flag(local_var)` instead.
/// @{

/// @brief XDEFINE_bool / XDEFINE_int32 / XDEFINE_uint32 / XDEFINE_int64 / XDEFINE_uint64 / XDEFINE_double / XDEFINE_string
/// AI: Define a global flag. Each macro registers the flag in FlagRegistry at static init time.
/// AI:   XDEFINE_bool(verbose, false, "enable verbose output");
/// AI:   XDEFINE_string(output, "", "output file path");
/// AI:   XDEFINE_int32(port, 8080, "listening port");
/// AI:
/// AI: Access the value via XFLAGS_<name>:
/// AI:   if (XFLAGS_verbose) { ... }
/// AI:   std::cout << XFLAGS_port;

/// @brief XDECLARE_bool / XDECLARE_int32 / XDECLARE_uint32 / XDECLARE_int64 / XDECLARE_uint64 / XDECLARE_double / XDECLARE_string
/// AI: Declare a flag defined in another .cc file:
/// AI:   // file1.cc:
/// AI:   XDEFINE_string(output, "", "output path");
/// AI:
/// AI:   // file2.cc:
/// AI:   XDECLARE_string(output);
/// AI:   void log() { std::ofstream out(XFLAGS_output); ... }
/// AI:
/// AI: Type mismatches are caught at link time (per-type namespaces).

/// @brief XFLAGS_<name> — access a gflags global variable
/// AI: Each XDEFINE creates a global variable XFLAGS_<name>.
/// AI: Read or write it like any variable:
/// AI:   XFLAGS_verbose = true;
/// AI:   std::string out = XFLAGS_output;

/// @brief xcli::detail::FlagRegistry — runtime flag introspection
/// AI: Singleton map of name -> CommandFlag. Query or set flags programmatically:
/// AI:   auto *reg = &xcli::detail::FlagRegistry::global_registry();
/// AI:   auto *flag = reg->find("verbose");
/// AI:   flag->set_value("true");
/// AI:   auto all = reg->get_all_flags();
/// AI:
/// AI: Free function: xcli::detail::set_flag("verbose", "true");

/// @brief Binding gflags to App options
/// AI: XFLAGS_ globals can be bound to App options for CLI parsing:
/// AI:   app.add_flag("--verbose,-V", XFLAGS_verbose, "enable verbose output");
/// AI:   app.add_option("--port,-p", XFLAGS_port, "listening port");
/// AI:
/// AI: The same XFLAGS can be bound on multiple subcommands.
/// AI: When the help text is the same, it's redundant with XDEFINE's desc;
/// AI: you can pass "" to add_flag and the flag still works.
/// @}

/// @defgroup api_reference API Reference (all public headers)
/// @brief Single-include: `<xcli/cli.h>`
/// AI: The library is organized into these headers:
/// AI:
/// AI:   xcli/cli.h              — aggregator, includes everything below
/// AI:   xcli/app.h              — App class (main entry point)
/// AI:   xcli/option.h           — Option class and OptionDefaults
/// AI:   xcli/flags.h            — XDEFINE/XDECLARE/FlagRegistry (gflags layer)
/// AI:   xcli/validators.h       — Validator class + built-in validators
/// AI:   xcli/extra_validators.h — additional validators
/// AI:   xcli/formatter.h        — Formatter for help output
/// AI:   xcli/config.h           — TOML/INI config file parser
/// AI:   xcli/error.h            — Error types (ParseError, Success, etc.)
/// AI:   xcli/split.h            — String splitting utilities
/// AI:   xcli/string_tools.h     — String utilities
/// AI:   xcli/type_tools.h       — Type traits
/// AI:   xcli/macros.h           — Utility macros (XCLI_INLINE, etc.)
/// AI:   xcli/encoding.h         — Encoding utilities
/// AI:   xcli/argv.h             — Argument vector handling
/// AI:   xcli/version_cli.h      — CLI version (2.6.2, tracks CLI11)
/// AI:   xcli/version.h          — Generated build info + SIMD macros
/// AI:   xcli/timer.h            — Simple timer utility

/// @defgroup examples Examples
/// @brief See examples/ directory for runnable demos
/// AI:
/// AI:   examples/cli/simple.cc              — basic add_flag + add_option
/// AI:   examples/cli/subcommands.cc         — nested subcommands
/// AI:   examples/cli/xflags.cc              — gflags XDEFINE + App binding
/// AI:   examples/cli/validators.cc          — input validation
/// AI:   examples/cli/custom_validator.cc    — custom validator function
/// AI:   examples/cli/formatter.cc           — custom help formatter
/// AI:   examples/cli/config_app.cc          — TOML config file
/// AI:   examples/cli/enum.cc                — enum options
/// AI:   examples/cli/groups.cc              — option groups
/// AI:   examples/cli/ranges.cc              — Range validator
/// AI:   examples/cli/json.cc                — JSON output
/// AI:   examples/cli/prefix_command.cc      — prefix command mode
/// AI:   examples/cli/subcom_partitioned.cc  — partitioned subcommands
/// AI:   examples/cli/digit_args.cc          — digit flags with defaults
/// AI:   examples/cli/positional_arity.cc    — positional argument arity
/// AI:   examples/cli/positional_validation.cc — positional validation
/// AI:   examples/cli/option_groups.cc       — OptionGroup
/// AI:   examples/cli/close_match.cc         — close match suggestions
/// AI:   examples/cli/help_usage.cc          — custom usage + help
/// AI:   examples/cli/inter_argument_order.cc — ordering of args
/// AI:   examples/cli/custom_parse.cc        — custom parse function
/// AI:   examples/cli/callback_passthrough.cc — callback passthrough
/// AI:   examples/cli/date_validator.cc      — date validator
/// AI:   examples/cli/arg_capture.cc         — argument capture
/// AI:   examples/cli/retired.cc             — retired flags
/// AI:   examples/cli/modhelp.cc             — modular help
/// AI:   examples/cli/array_option.cc        — array options
/// AI:   examples/cli/subcom_help.cc         — subcommand help
/// AI:   examples/cli/string_tools.cc        — (not present)
/// AI:   examples/cli/shapes.cc              — shape example
/// AI:   examples/cli/testEXE.cc             — test example
/// AI:   examples/cli/minimal.cc             — minimal example
/// AI:   examples/cli/nested.cc              — nested subcommands
///
/// AI: Build all examples:
/// AI:   cmake -B build -DKMCMAKE_BUILD_EXAMPLES=ON
/// AI:   cmake --build build
/// AI:   ./build/examples/cli/xflags --verbose --port=9090

/// @defgroup key_conventions Key Conventions
/// AI:
/// AI: - ALWAYS use `XCLI_PARSE(app, argc, argv)` instead of raw `app.parse()`
/// AI:   to get proper error handling and exit codes.
/// AI: - Use `XDEFINE_*` + `XFLAGS_*` when the same flag is needed in
/// AI:   multiple .cc files. Otherwise, use local variables with add_flag().
/// AI: - The library is namespace-clean: everything is in `xcli::`.
/// AI: - Only `XFLAGS_*` globals leak into the global namespace (by design,
/// AI:   matching gflags behavior).
/// AI: - Single include: `#include <turbo/cli/cli.h>` provides everything.
///
/// @}

// AI: End of skills.h — AI assistants should read this before other source files.
// AI: See docs/AI.md for AI behavioral constraints and instructions.
