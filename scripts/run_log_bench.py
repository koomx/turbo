#!/usr/bin/env python3
"""Run log / zlog google-benchmarks and write plottable JSON under benchmark-results/."""
from __future__ import annotations

import json
import os
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT = Path("benchmark-results/raw/bench.json")

# (label, relative binary paths without / with .exe)
BINARIES = [
    (
        "log",
        [
            Path("build-ninja/benchmark/log/log_log_benchmark"),
            Path("build/benchmark/log/log_log_benchmark"),
            Path("build-ninja/benchmark/log/log_log_benchmark.exe"),
            Path("build/benchmark/log/log_log_benchmark.exe"),
        ],
    ),
    (
        "zlog",
        [
            Path("build-ninja/benchmark/log/log_zlog_benchmark"),
            Path("build/benchmark/log/log_zlog_benchmark"),
            Path("build-ninja/benchmark/log/log_zlog_benchmark.exe"),
            Path("build/benchmark/log/log_zlog_benchmark.exe"),
        ],
    ),
]


def find_binary(candidates: list[Path]) -> Path | None:
    for rel in candidates:
        p = ROOT / rel
        if p.is_file():
            return p
    return None


def short_name(name: str) -> str:
    return name.split("/")[0]


def gbench_to_samples(data: dict, prefix: str) -> tuple[list[dict], str]:
    samples: list[dict] = []
    seen: set[str] = set()
    unit = "ns"
    for b in data.get("benchmarks", []):
        if b.get("run_type") == "aggregate":
            continue
        base = short_name(str(b.get("name", "")))
        if not base:
            continue
        name = f"{prefix}:{base}"
        if name in seen:
            continue
        seen.add(name)
        value = float(b.get("cpu_time", b.get("real_time", 0.0)))
        unit = str(b.get("time_unit", unit))
        samples.append({"name": name, "value": round(value, 3)})
    return samples, unit


def run_one(label: str, binary: Path, raw_path: Path) -> tuple[list[dict], str]:
    raw_path.parent.mkdir(parents=True, exist_ok=True)
    cmd = [
        str(binary),
        f"--benchmark_out={raw_path}",
        "--benchmark_out_format=json",
    ]
    # CI: binary already caps MaxBenchThreads()=1 when GITHUB_ACTIONS is set;
    # filter keeps only single-thread / non-threaded cases as a second guard.
    if os.environ.get("GITHUB_ACTIONS"):
        # Single-thread cases only; short min_time so overloaded ARM runners
        # do not spend minutes on pathological / noisy cases.
        cmd.append(r"--benchmark_filter=(^BM_[^/]+$|/threads:1$)")
        cmd.append("--benchmark_min_time=0.05s")
    print("running:", " ".join(cmd), flush=True)
    subprocess.check_call(cmd)
    gbench = json.loads(raw_path.read_text(encoding="utf-8"))
    return gbench_to_samples(gbench, label)


def main() -> None:
    os.chdir(ROOT)
    all_samples: list[dict] = []
    unit = "ns"
    ran = 0
    for label, candidates in BINARIES:
        binary = find_binary(candidates)
        if binary is None:
            print(f"skip {label}: binary not found", flush=True)
            continue
        samples, unit = run_one(
            label, binary, Path(f"benchmark-results/raw/{label}_bench.json")
        )
        all_samples.extend(samples)
        ran += 1

    if ran == 0:
        raise SystemExit(
            "no log benchmarks found; build with KMCMAKE_BUILD_BENCHMARK=ON "
            "(expected log_log_benchmark and/or log_zlog_benchmark)"
        )

    plottable = {
        "title": "turbo log + zlog benchmark",
        "unit": unit,
        "samples": all_samples,
    }
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(plottable, indent=2) + "\n", encoding="utf-8")
    print(
        f"wrote {OUT} ({len(all_samples)} samples from {ran} binaries)",
        flush=True,
    )


if __name__ == "__main__":
    main()
