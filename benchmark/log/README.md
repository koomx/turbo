# Log benchmarks

CI runs `log_log_benchmark`, plots results, and publishes PNGs to the
[`benchmark-results`](https://github.com/koomx/turbo/tree/benchmark-results)
branch. Images below refresh on push / `workflow_dispatch` (not on PR).

## Gallery

#### `std20-ubuntu24-amd64`

![std20-ubuntu24-amd64](https://raw.githubusercontent.com/koomx/turbo/benchmark-results/latest/std20-ubuntu24-amd64/plots/summary.png)

#### `std20-ubuntu24-arm64`

![std20-ubuntu24-arm64](https://raw.githubusercontent.com/koomx/turbo/benchmark-results/latest/std20-ubuntu24-arm64/plots/summary.png)

## Local

```bash
cmake --preset=ci -DCMAKE_TOOLCHAIN_FILE=$VCPKG_CMAKE
cmake --build build-ninja -j
python3 scripts/run_log_bench.py
python3 scripts/plot_log_bench.py
# → benchmark-results/plots/summary.png
```

With CPM instead of vcpkg:

```bash
cmake --preset=ninja -DKMCMAKE_BUILD_BENCHMARK=ON
cmake --build build-ninja -j
python3 scripts/run_log_bench.py
python3 scripts/plot_log_bench.py
```
