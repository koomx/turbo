#!/usr/bin/env python3
"""Plot log benchmark JSON to PNG (matplotlib when available, else stdlib)."""
from __future__ import annotations

import json
import struct
import subprocess
import sys
import zlib
from pathlib import Path

IN = Path("benchmark-results/raw/bench.json")
OUT = Path("benchmark-results/plots/summary.png")
VENDOR = Path(".bench-py")


def ensure_matplotlib() -> bool:
    """Install matplotlib only from wheels; return False if unavailable."""
    try:
        import matplotlib  # noqa: F401

        return True
    except ImportError:
        pass
    VENDOR.mkdir(exist_ok=True)
    try:
        subprocess.check_call(
            [
                sys.executable,
                "-m",
                "pip",
                "install",
                "-q",
                "--only-binary=:all:",
                "--target",
                str(VENDOR),
                "matplotlib",
            ]
        )
    except subprocess.CalledProcessError:
        return False
    sys.path.insert(0, str(VENDOR))
    try:
        import matplotlib  # noqa: F401

        return True
    except ImportError:
        return False


def _png_chunk(tag: bytes, data: bytes) -> bytes:
    return (
        struct.pack(">I", len(data))
        + tag
        + data
        + struct.pack(">I", zlib.crc32(tag + data) & 0xFFFFFFFF)
    )


def write_png_rgba(path: Path, width: int, height: int, rgba: bytes) -> None:
    if len(rgba) != width * height * 4:
        raise ValueError("rgba buffer size mismatch")
    raw = b"".join(
        b"\x00" + rgba[y * width * 4 : (y + 1) * width * 4] for y in range(height)
    )
    ihdr = struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0)
    png = (
        b"\x89PNG\r\n\x1a\n"
        + _png_chunk(b"IHDR", ihdr)
        + _png_chunk(b"IDAT", zlib.compress(raw, 9))
        + _png_chunk(b"IEND", b"")
    )
    path.write_bytes(png)


def plot_stdlib(names: list[str], values: list[float], title: str, unit: str) -> None:
    """Minimal bar chart PNG with no third-party deps."""
    width, height = 960, 520
    margin_l, margin_r, margin_t, margin_b = 64, 24, 48, 100
    bg = (248, 250, 252, 255)
    bar = (59, 130, 246, 255)
    axis = (100, 116, 139, 255)
    pixels = bytearray(width * height * 4)

    def put(x: int, y: int, color: tuple[int, int, int, int]) -> None:
        if 0 <= x < width and 0 <= y < height:
            i = (y * width + x) * 4
            pixels[i : i + 4] = bytes(color)

    for i in range(0, len(pixels), 4):
        pixels[i : i + 4] = bytes(bg)

    plot_w = width - margin_l - margin_r
    plot_h = height - margin_t - margin_b
    vmax = max(values) if values else 1.0
    if vmax <= 0:
        vmax = 1.0

    for x in range(margin_l, width - margin_r):
        put(x, height - margin_b, axis)
    for y in range(margin_t, height - margin_b + 1):
        put(margin_l, y, axis)

    n = max(len(names), 1)
    slot = plot_w / n
    bar_w = max(int(slot * 0.6), 8)
    for idx, value in enumerate(values):
        bh = int((value / vmax) * (plot_h - 4))
        x0 = margin_l + int(idx * slot + (slot - bar_w) / 2)
        y0 = height - margin_b - bh
        for y in range(y0, height - margin_b):
            for x in range(x0, x0 + bar_w):
                put(x, y, bar)

    for x in range(margin_l, width - margin_r):
        put(x, margin_t // 2, axis)

    OUT.parent.mkdir(parents=True, exist_ok=True)
    write_png_rgba(OUT, width, height, bytes(pixels))
    print(f"wrote {OUT} (stdlib fallback; title={title!r} unit={unit!r})")
    print("labels:", ", ".join(f"{n}={v}" for n, v in zip(names, values)))


def plot_matplotlib(names: list[str], values: list[float], title: str, unit: str) -> None:
    import matplotlib

    matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    OUT.parent.mkdir(parents=True, exist_ok=True)
    fig, ax = plt.subplots(figsize=(10, 5))
    ax.bar(names, values, color="#3b82f6")
    ax.set_title(title)
    ax.set_ylabel(unit)
    ax.tick_params(axis="x", labelrotation=35)
    fig.tight_layout()
    fig.savefig(OUT, dpi=120)
    plt.close(fig)
    print(f"wrote {OUT}")


def main() -> None:
    data = json.loads(IN.read_text(encoding="utf-8"))
    names = [s["name"] for s in data["samples"]]
    values = [float(s["value"]) for s in data["samples"]]
    title = data.get("title", "benchmark")
    unit = data.get("unit", "")

    if ensure_matplotlib():
        plot_matplotlib(names, values, title, unit)
    else:
        plot_stdlib(names, values, title, unit)


if __name__ == "__main__":
    main()
