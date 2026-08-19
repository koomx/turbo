#!/usr/bin/env python3
"""Strip #if/#ifdef SIMDUTF_FEATURE_* and matching #endif; keep inner lines."""

from __future__ import annotations

import re
import sys
from pathlib import Path

ROOTS = [
    Path("turbo/unicode"),
    Path("tests/unicode"),
]

EXTS = {".h", ".hh", ".hpp", ".c", ".cc", ".cpp", ".cxx", ".inl"}

DIR_RE = re.compile(r"^\s*#\s*(if|ifdef|ifndef|elif|else|endif)\b(.*)$")


def is_feature_open(kind: str, rest: str) -> bool:
    rest = rest.strip()
    if "SIMDUTF_FEATURE_" not in rest:
        return False
    if kind == "ifndef":
        return False
    if kind == "ifdef":
        return True
    if kind != "if":
        return False
    # Do not unwrap negated feature tests.
    if re.match(r"!", rest) or re.match(r"!\s*SIMDUTF_FEATURE_", rest):
        return False
    if re.match(r"!\s*defined\s*\(\s*SIMDUTF_FEATURE_", rest):
        return False
    return True


def process_text(text: str) -> str:
    lines = text.splitlines(keepends=True)
    out: list[str] = []
    # stack of True = this #if was unwrapped (skip matching endif)
    stack: list[bool] = []

    for line in lines:
        m = DIR_RE.match(line.split("//")[0] if True else line)
        # Match on code part without trailing comment for classification,
        # but use full line for output.
        code = line
        if "//" in line:
            # keep strings simple: preprocessor lines rarely have // in the expr first
            pass
        mm = DIR_RE.match(line)
        if not mm:
            out.append(line)
            continue
        kind, rest = mm.group(1), mm.group(2)
        if kind in ("if", "ifdef", "ifndef"):
            unwrap = is_feature_open(kind, rest)
            stack.append(unwrap)
            if not unwrap:
                out.append(line)
            continue
        if kind in ("elif", "else"):
            if stack and stack[-1]:
                # keep unexpected else/elif so the file still shows the problem
                out.append(line)
            else:
                out.append(line)
            continue
        # endif
        if not stack:
            out.append(line)
            continue
        unwrap = stack.pop()
        if not unwrap:
            out.append(line)
        continue

    return "".join(out)


def iter_files(root: Path):
    if not root.exists():
        return
    for p in root.rglob("*"):
        if not p.is_file():
            continue
        if p.suffix.lower() in EXTS or p.name.endswith(".inl.cpp"):
            yield p


def main() -> int:
    repo = Path(__file__).resolve().parents[1]
    n_files = 0
    n_changed = 0
    for rel in ROOTS:
        for path in iter_files(repo / rel):
            n_files += 1
            raw = path.read_text(encoding="utf-8", errors="surrogateescape")
            new = process_text(raw)
            if new != raw:
                path.write_text(new, encoding="utf-8", errors="surrogateescape")
                n_changed += 1
    print(f"scanned_via_walk={n_files} changed={n_changed}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
