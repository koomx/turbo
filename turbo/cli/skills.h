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
/// @brief xcli — header-only C++ CLI library (CLI11 fork), binds local T& or turbo::Flag

/// @brief What is xcli
/// AI: xcli is a header-only C++ command-line parsing library forked from CLI11 (v2.6.2).
/// AI: It inherits CLI11's full API (App, Option, subcommands, validators, formatters, config)
/// AI: and binds turbo::Flag<T> for process startup parameters (TURBO_FLAG / FLAGS_*).
/// AI:
/// AI: Key design principles:
/// AI:   - Single `#include <turbo/cli/cli.h>` to use everything
/// AI:   - Local T& for app-local options; TURBO_FLAG for shared startup params
/// AI:   - Flag binding writes via ParseFrom (on_validate applies)
/// AI:   - No exceptions disabled — errors are reported via ParseError (catchable)
/// AI:   - Header-only, no .cc to compile

/// @defgroup public_api Public API Overview
/// AI: All public types are in namespace `xcli`. Include `<turbo/cli/cli.h>` for everything.
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
/// AI: Also binds turbo::Flag<T> (see turbo_flag_binding below).

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

/// @defgroup turbo_flag_binding turbo::Flag binding
/// @brief Bind TURBO_FLAG / FLAGS_* as startup parameters.
/// @{

/// @brief Binding turbo::Flag to App options
/// AI:   TURBO_FLAG(int, port, 8080, "listening port");
/// AI:   TURBO_FLAG(bool, verbose, false, "enable verbose");
/// AI:   app.add_option("--port,-p", FLAGS_port);       // names like T&; desc/default from flag
/// AI:   app.add_flag("--verbose,-V", FLAGS_verbose);
/// AI: Empty description uses flag.Help(); default_str uses flag.DefaultValue().
/// AI: Parse writes via Flag ParseFrom (on_validate applies). Local T& binding remains.
/// AI: Read values with turbo::GetFlag(FLAGS_port).
/// AI: App::parse(argc,argv) also sets FLAGS_argv (full argv including argv[0]).
/// AI: ShortProgramInvocationName() is Basename(FLAGS_argv[0]).
/// @}

/// @defgroup api_reference API Reference (all public headers)
/// @brief Single-include: `<turbo/cli/cli.h>`
/// AI: The library is organized into these headers:
/// AI:
/// AI:   turbo/cli/cli.h          — aggregator, includes everything below
/// AI:   turbo/cli/app.h          — App class (main entry point; Flag binding)
/// AI:   turbo/cli/option.h       — Option class and OptionDefaults
/// AI:   turbo/cli/validators.h   — Validator class + built-in validators
/// AI:   turbo/cli/extra_validators.h — additional validators
/// AI:   turbo/cli/formatter.h    — Formatter for help output
/// AI:   turbo/cli/config.h       — TOML/INI config file parser
/// AI:   turbo/cli/error.h        — Error types (ParseError, Success, etc.)
/// AI:   turbo/cli/split.h        — String splitting utilities
/// AI:   turbo/cli/string_tools.h — String utilities
/// AI:   turbo/cli/type_tools.h   — Type traits
/// AI:   turbo/cli/macros.h       — Utility macros (inline, etc.)
/// AI:   turbo/cli/encoding.h     — to_path (UTF-8 → native path)
/// AI:   turbo/cli/argv.h         — Windows UTF-8 argv helpers (ensure_utf8)
/// AI:   turbo/cli/version_cli.h  — CLI version (2.6.2, tracks CLI11)
/// AI:   turbo/cli/version.h      — Generated build info + SIMD macros
/// AI:   turbo/cli/timer.h        — Simple timer utility

/// @defgroup examples Examples
/// @brief See examples/ directory for runnable demos
/// AI:
/// AI:   examples/cli/simple.cc              — basic add_flag + add_option
/// AI:   examples/cli/subcommands.cc         — nested subcommands
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
/// AI:   examples/cli/shapes.cc              — shape example
/// AI:   examples/cli/testEXE.cc             — test example
/// AI:   examples/cli/minimal.cc             — minimal example
/// AI:   examples/cli/nested.cc              — nested subcommands
///
/// AI: Build all examples:
/// AI:   cmake -B build -DKMCMAKE_BUILD_EXAMPLES=ON
/// AI:   cmake --build build

/// @defgroup key_conventions Key Conventions
/// AI:
/// AI: - ALWAYS use `XCLI_PARSE(app, argc, argv)` instead of raw `app.parse()`
/// AI:   to get proper error handling and exit codes.
/// AI: - Shared startup params: `TURBO_FLAG` + `app.add_option("--x", FLAGS_x)`.
/// AI: - Local-only params: `app.add_option("--x", local_var)`.
/// AI: - The library is namespace-clean: CLI types are in `xcli::`; flags are `FLAGS_*`.
/// AI: - Single include: `#include <turbo/cli/cli.h>` provides everything.
///
/// @}

// AI: End of skills.h — AI assistants should read this before other source files.
// AI: See docs/AI.md for AI behavioral constraints and instructions.
