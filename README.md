turbo
=============================

[中文版](./README_CN.md)

C++17 foundation library: vocabulary types, strings, time, status, log, flags/CLI, unicode, and related helpers. Built with [kmcmake](https://github.com/koomx/kmcmake). User config lives in `cmake/`; do not edit `kmcmake/`.

Public API summary for humans and agents: [`turbo/skills.h`](turbo/skills.h). Style and edit rules: [`docs/c_cpp_project_rules.md`](docs/c_cpp_project_rules.md).

## Build

Presets in [`CMakePresets.json`](CMakePresets.json) inherit `base`, which sets `KMCMAKE_USE_CPM=ON`. Test/benchmark packages (GoogleTest, google/benchmark) are listed in [`cmake/turbo_cpm.cmake`](cmake/turbo_cpm.cmake) and fetched only when those options are on.

### Environment

- Linux / macOS / Windows (MSVC + Ninja recommended)
- CMake >= 3.20
- GCC >= 9.4 / Clang >= 12 / MSVC (VS 2022+)
- Optional: `ninja` on PATH for `--preset=ninja` and `--preset=ci`

### Configure and compile

From the repository root:

```bash
# Unix Makefiles → build/
cmake --preset=default
cmake --build build -j$(nproc)

# Ninja → build-ninja/
cmake --preset=ninja
cmake --build build-ninja -j$(nproc)

# Tests + benchmarks (Ninja) → build-ninja/
cmake --preset=ci
cmake --build build-ninja --parallel
```

`CMAKE_CXX_STANDARD` is 17 ([`cmake/turbo_user_option.cmake`](cmake/turbo_user_option.cmake)).

### Tests

```bash
ctest --test-dir build
# or, after ninja / ci presets:
ctest --test-dir build-ninja --output-on-failure
```

## Modules

Sources live under `turbo/<module>/`. Cross-module `#include <turbo/<other>/...>` is an allow-list in [`turbo/module_deps.toml`](turbo/module_deps.toml). The graph below is that allow-list by layer (L0–L10). Same-module includes are always allowed. There are no cycles.

```mermaid
flowchart TB
  subgraph L0 [L0 macros]
    macros
  end
  subgraph L1 [L1 foundation]
    base
    meta
    utility
    cctz
    arch
  end
  subgraph L2 [L2 helpers]
    bits
    algorithm
    cleanup
    memory
    functional
  end
  subgraph L3 [L3 numeric debug hash unicode]
    numeric
    debugging
    hash
    unicode
  end
  subgraph L4 [L4 types]
    types
  end
  subgraph L5 [L5 format]
    format
  end
  subgraph L6 [L6 crc]
    crc
  end
  subgraph L7 [L7 strings]
    strings
  end
  subgraph L8 [L8 time status cord]
    time
    status
    cord
  end
  subgraph L9 [L9 flags log]
    flags
    log
  end
  subgraph L10 [L10 cli]
    cli
  end

  base --> macros
  meta --> macros
  utility --> macros
  cctz --> macros
  arch --> macros

  bits --> base
  bits --> macros
  bits --> meta
  algorithm --> base
  algorithm --> macros
  algorithm --> meta
  cleanup --> base
  cleanup --> macros
  memory --> hash
  memory --> macros
  memory --> meta
  memory --> utility
  functional --> base
  functional --> macros
  functional --> meta
  functional --> utility

  numeric --> algorithm
  numeric --> bits
  numeric --> macros
  debugging --> base
  debugging --> bits
  debugging --> macros
  hash --> bits
  hash --> functional
  hash --> macros
  hash --> meta
  hash --> numeric
  hash --> utility
  unicode --> bits
  unicode --> arch
  unicode --> macros

  types --> algorithm
  types --> base
  types --> hash
  types --> macros
  types --> memory
  types --> meta
  types --> utility

  format --> base
  format --> bits
  format --> functional
  format --> macros
  format --> meta
  format --> numeric
  format --> types
  format --> utility

  crc --> base
  crc --> bits
  crc --> format
  crc --> macros
  crc --> memory

  strings --> base
  strings --> bits
  strings --> format
  strings --> macros
  strings --> meta
  strings --> numeric
  strings --> types

  time --> base
  time --> bits
  time --> cctz
  time --> macros
  time --> numeric
  time --> strings
  status --> base
  status --> debugging
  status --> format
  status --> functional
  status --> hash
  status --> macros
  status --> meta
  status --> strings
  status --> time
  status --> types
  status --> utility
  cord --> algorithm
  cord --> base
  cord --> bits
  cord --> crc
  cord --> functional
  cord --> hash
  cord --> macros
  cord --> memory
  cord --> meta
  cord --> strings
  cord --> types

  flags --> algorithm
  flags --> base
  flags --> bits
  flags --> format
  flags --> macros
  flags --> memory
  flags --> meta
  flags --> numeric
  flags --> strings
  flags --> utility
  log --> base
  log --> bits
  log --> cleanup
  log --> debugging
  log --> format
  log --> functional
  log --> hash
  log --> macros
  log --> meta
  log --> strings
  log --> time
  log --> types
  log --> utility

  cli --> base
  cli --> flags
  cli --> format
  cli --> macros
  cli --> strings
  cli --> utility
```

Refresh measured edges or enforce the allow-list:

```bash
python3 scripts/check_module_deps.py list-all
python3 scripts/check_module_deps.py list <module> -v
python3 scripts/check_module_deps.py check
```

Install the pre-commit hook (staged turbo sources only):

```bash
git config core.hooksPath scripts/git-hooks
```

Skip once: `SKIP_MODULE_DEPS=1 git commit ...`

## CI

Workflow: [`.github/workflows/ci.yml`](.github/workflows/ci.yml) (`koomx/x-ci` reusable workflow). Configure via `python3 scripts/ci_configure.py`.

Currently enabled jobs:

- `std20-ubuntu24-amd64`
- `macos-arm64`
- `windows`

**Linux ARM is skipped.** GitHub-hosted CI can hang; do not treat a stuck GitHub run as a signal that the other jobs failed.
