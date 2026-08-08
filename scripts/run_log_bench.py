#!/usr/bin/env python3
"""Run log_log_benchmark and write plottable JSON under benchmark-results/."""
from __future__ import annotations

import json
import os
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RAW_GBENCH = Path("benchmark-results/raw/log_bench.json")
OUT = Path("benchmark-results/raw/bench.json")

CANDIDATES = [
    Path("build-ninja/benchmark/log/log_log_benchmark"),
    Path("build/benchmark/log/log_log_benchmark"),
    Path("build-ninja/benchmark/log/log_log_benchmark.exe"),
    Path("build/benchmark/log/log_log_benchmark.exe"),
]


def find_binary() -> Path:
    env = os.environ.get("TURBO_LOG_BENCHMARK")
    if env:
        p = Path(env)
        if p.is_file():
            return p
        raise SystemExit(f"TURBO_LOG_BENCHMARK not found: {p}")
    for rel in CANDIDATES:
        p = ROOT / rel
        if p.is_file():
            return p
    raise SystemExit(
        "log_log_benchmark not found; build with KMCMAKE_BUILD_BENCHMARK=ON "
        f"(looked under {', '.join(str(c) for c in CANDIDATES)})"
    )


def short_name(name: str) -> str:
    # Prefer single-thread / default fixtures: drop "/threads:N" style suffixes
    # and keep the base BM_* label for a readable bar chart.
    base = name.split("/")[0]
    return base


def gbench_to_samples(data: dict) -> dict:
    samples: list[dict] = []
    seen: set[str] = set()
    unit = "ns"
    for b in data.get("benchmarks", []):
        if b.get("run_type") == "aggregate":
            continue
        name = short_name(str(b.get("name", "")))
        if not name or name in seen:
            # Keep first (usually threads:1 / default) per family.
            continue
        seen.add(name)
        value = float(b.get("cpu_time", b.get("real_time", 0.0)))
        unit = str(b.get("time_unit", unit))
        samples.append({"name": name, "value": round(value, 3)})
    return {
        "title": "turbo log benchmark",
        "unit": unit,
        "samples": samples,
    }


def main() -> None:
    os.chdir(ROOT)
    binary = find_binary()
    RAW_GBENCH.parent.mkdir(parents=True, exist_ok=True)
    cmd = [
        str(binary),
        f"--benchmark_out={RAW_GBENCH}",
        "--benchmark_out_format=json",
    ]
    print("running:", " ".join(cmd), flush=True)
    subprocess.check_call(cmd)
    gbench = json.loads(RAW_GBENCH.read_text(encoding="utf-8"))
    plottable = gbench_to_samples(gbench)
    OUT.write_text(json.dumps(plottable, indent=2) + "\n", encoding="utf-8")
    print(f"wrote {OUT} ({len(plottable['samples'])} samples)")


if __name__ == "__main__":
    main()
