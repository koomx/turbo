#!/usr/bin/env python3
"""Run ctest; skip on Linux aarch64/arm64 (QEMU hang). Windows-safe (no bash)."""
from __future__ import annotations

import platform
import subprocess
import sys


def main() -> int:
    machine = platform.machine().lower()
    if platform.system() == "Linux" and machine in ("aarch64", "arm64"):
        print("skip ctest on linux arm")
        return 0
    return subprocess.call(
        ["ctest", "--test-dir", "build-ninja", "--output-on-failure", "-j1"]
    )


if __name__ == "__main__":
    sys.exit(main())
