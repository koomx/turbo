#!/usr/bin/env python3
# Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.
#
# Scan turbo/ module #include edges and enforce / report dependency rules.
#
# Usage:
#   scripts/check_module_deps.py list <module> [--verbose]
#   scripts/check_module_deps.py list-all [--verbose]
#   scripts/check_module_deps.py check [--staged|--all]
#   scripts/check_module_deps.py modules

"""Check and list cross-module #include dependencies under turbo/."""

from __future__ import annotations

import argparse
import os
import re
import subprocess
import sys
from collections import defaultdict
from pathlib import Path
from typing import Dict, Iterable, List, Optional, Set, Tuple

REPO_ROOT = Path(__file__).resolve().parents[1]
TURBO_ROOT = REPO_ROOT / "turbo"
DEFAULT_RULES = TURBO_ROOT / "module_deps.toml"

SOURCE_SUFFIXES = {".h", ".hh", ".hpp", ".inc", ".c", ".cc", ".cpp", ".cxx"}

# #include <turbo/foo/...> or #include "turbo/foo/..."
INCLUDE_RE = re.compile(
    r'^\s*#\s*include\s*[<"]turbo/([A-Za-z0-9_]+)/'
)

Violation = Tuple[str, int, str, str]  # path, line, from_mod, to_mod


def die(msg: str, code: int = 2) -> None:
    print(f"module-deps: {msg}", file=sys.stderr)
    raise SystemExit(code)


def discover_modules() -> List[str]:
    mods = []
    for p in sorted(TURBO_ROOT.iterdir()):
        if p.is_dir() and not p.name.startswith(".") and p.name != "CMakeFiles":
            # skip non-module dirs if any
            if p.name in {"deb", "rpm"}:
                continue
            mods.append(p.name)
    return mods


def parse_rules(path: Path) -> Tuple[str, Dict[str, List[str]], Set[str]]:
    """Return (default_unlisted, allow_map, unchecked_set).

    Minimal TOML subset parser for this file's shape only.
    """
    if not path.is_file():
        die(f"rules file not found: {path}")

    default_unlisted = "unchecked"
    allow: Dict[str, List[str]] = {}
    unchecked: Set[str] = set()
    section: Optional[str] = None

    for raw in path.read_text(encoding="utf-8").splitlines():
        line = raw.split("#", 1)[0].strip()
        if not line:
            continue
        if line.startswith("[") and line.endswith("]"):
            section = line[1:-1].strip()
            continue
        if "=" not in line:
            continue
        key, _, val = line.partition("=")
        key = key.strip()
        val = val.strip().strip('"').strip("'")

        if section == "policy" and key == "default_unlisted":
            if val not in ("unchecked", "deny"):
                die(f"invalid default_unlisted={val!r} (use unchecked|deny)")
            default_unlisted = val
        elif section == "allow":
            deps = [d.strip() for d in val.split(",") if d.strip()]
            allow[key] = deps
        elif section == "unchecked":
            # allow bare names as keys with empty values, or key = true
            name = key
            unchecked.add(name)

    return default_unlisted, allow, unchecked


def module_of(path: Path) -> Optional[str]:
    try:
        rel = path.resolve().relative_to(TURBO_ROOT.resolve())
    except ValueError:
        return None
    parts = rel.parts
    if not parts:
        return None
    return parts[0]


def iter_source_files(root: Path) -> Iterable[Path]:
    for dirpath, dirnames, filenames in os.walk(root):
        # prune build / vcs noise
        dirnames[:] = [
            d
            for d in dirnames
            if d not in {".git", "CMakeFiles", "build"} and not d.startswith(".")
        ]
        for name in filenames:
            p = Path(dirpath) / name
            if p.suffix in SOURCE_SUFFIXES:
                yield p


def staged_turbo_sources() -> List[Path]:
    try:
        out = subprocess.check_output(
            ["git", "diff", "--cached", "--name-only", "--diff-filter=ACMR"],
            cwd=REPO_ROOT,
            text=True,
        )
    except (subprocess.CalledProcessError, FileNotFoundError) as e:
        die(f"git diff --cached failed: {e}")
    files: List[Path] = []
    for line in out.splitlines():
        line = line.strip()
        if not line.startswith("turbo/"):
            continue
        p = REPO_ROOT / line
        if p.suffix in SOURCE_SUFFIXES and p.is_file():
            files.append(p)
    return files


def scan_file(path: Path) -> List[Tuple[int, str]]:
    """Return list of (line_no, target_module) for turbo includes."""
    edges: List[Tuple[int, str]] = []
    try:
        text = path.read_text(encoding="utf-8", errors="replace")
    except OSError as e:
        die(f"cannot read {path}: {e}")
    for i, line in enumerate(text.splitlines(), 1):
        # strip // comments (naive, good enough for #include lines)
        code = line.split("//", 1)[0]
        m = INCLUDE_RE.match(code)
        if m:
            edges.append((i, m.group(1)))
    return edges


def collect_module_deps(
    module: str, files: Optional[Iterable[Path]] = None
) -> Tuple[Set[str], Dict[str, List[Tuple[str, int]]]]:
    """Return (dep_modules, dep -> [(relpath, line), ...])."""
    mod_root = TURBO_ROOT / module
    if not mod_root.is_dir():
        die(f"unknown module: {module} (expected directory {mod_root})")

    deps: Set[str] = set()
    detail: Dict[str, List[Tuple[str, int]]] = defaultdict(list)

    sources = list(files) if files is not None else list(iter_source_files(mod_root))
    for path in sources:
        if module_of(path) != module:
            continue
        rel = str(path.resolve().relative_to(REPO_ROOT))
        for line_no, target in scan_file(path):
            if target == module:
                continue
            deps.add(target)
            detail[target].append((rel, line_no))
    return deps, detail


def cmd_modules(_: argparse.Namespace) -> int:
    for m in discover_modules():
        print(m)
    return 0


def cmd_list(args: argparse.Namespace) -> int:
    deps, detail = collect_module_deps(args.module)
    if not deps:
        print(f"{args.module}: (no external turbo module includes)")
        return 0
    print(f"{args.module}:")
    for dep in sorted(deps):
        print(f"  - {dep}")
        if args.verbose:
            for rel, line_no in sorted(detail[dep]):
                print(f"      {rel}:{line_no}")
    return 0


def cmd_list_all(args: argparse.Namespace) -> int:
    rc = 0
    for m in discover_modules():
        ns = argparse.Namespace(module=m, verbose=args.verbose)
        if cmd_list(ns) != 0:
            rc = 1
        print()
    return rc


def allowed_for(
    module: str,
    default_unlisted: str,
    allow: Dict[str, List[str]],
    unchecked: Set[str],
) -> Optional[Set[str]]:
    """Return allowed set, or None if module is unchecked (skip enforcement)."""
    if module in unchecked:
        return None
    if module in allow:
        return set(allow[module])
    if default_unlisted == "unchecked":
        return None
    return set()  # deny: no external deps allowed


def cmd_check(args: argparse.Namespace) -> int:
    default_unlisted, allow, unchecked = parse_rules(Path(args.rules))
    known = set(discover_modules())

    if args.staged:
        files = staged_turbo_sources()
        if not files:
            return 0
    else:
        files = list(iter_source_files(TURBO_ROOT))

    violations: List[Violation] = []
    for path in files:
        mod = module_of(path)
        if mod is None or mod not in known:
            continue
        allowed = allowed_for(mod, default_unlisted, allow, unchecked)
        if allowed is None:
            continue
        rel = str(path.resolve().relative_to(REPO_ROOT))
        for line_no, target in scan_file(path):
            if target == mod:
                continue
            if target not in known:
                # include of unknown turbo/<name>/ — treat as violation if enforcing
                violations.append((rel, line_no, mod, target))
                continue
            if target not in allowed:
                violations.append((rel, line_no, mod, target))

    if not violations:
        return 0

    print("module-deps: illegal cross-module includes:", file=sys.stderr)
    for rel, line_no, mod, target in violations:
        allowed = allowed_for(mod, default_unlisted, allow, unchecked) or set()
        allow_txt = ", ".join(sorted(allowed)) if allowed else "(none)"
        print(
            f"  {rel}:{line_no}: {mod} includes turbo/{target}/ "
            f"(allowed: {allow_txt})",
            file=sys.stderr,
        )
    print(
        f"module-deps: {len(violations)} violation(s). "
        f"See turbo/module_deps.toml",
        file=sys.stderr,
    )
    return 1


def build_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(
        description="List / enforce turbo/ module #include dependencies."
    )
    p.add_argument(
        "--rules",
        default=str(DEFAULT_RULES),
        help=f"rules file (default: {DEFAULT_RULES})",
    )
    sub = p.add_subparsers(dest="cmd", required=True)

    p_mods = sub.add_parser("modules", help="list discovered turbo modules")
    p_mods.set_defaults(func=cmd_modules)

    p_list = sub.add_parser(
        "list", help="list external turbo modules depended on by MODULE"
    )
    p_list.add_argument("module", help="module name under turbo/")
    p_list.add_argument(
        "-v",
        "--verbose",
        action="store_true",
        help="show file:line for each edge",
    )
    p_list.set_defaults(func=cmd_list)

    p_all = sub.add_parser("list-all", help="list deps for every module")
    p_all.add_argument("-v", "--verbose", action="store_true")
    p_all.set_defaults(func=cmd_list_all)

    p_check = sub.add_parser(
        "check",
        help="enforce allow-list in rules file (strict for listed modules)",
    )
    g = p_check.add_mutually_exclusive_group()
    g.add_argument(
        "--staged",
        action="store_true",
        help="only staged turbo sources (for git pre-commit)",
    )
    g.add_argument(
        "--all",
        dest="scan_all",
        action="store_true",
        help="scan all turbo sources (default when --staged is omitted)",
    )
    p_check.set_defaults(func=cmd_check)
    return p


def main(argv: Optional[List[str]] = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)
    if not TURBO_ROOT.is_dir():
        die(f"turbo root missing: {TURBO_ROOT}")
    return args.func(args)


if __name__ == "__main__":
    sys.exit(main())
