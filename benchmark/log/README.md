# Log benchmarks

CI runs `log_log_benchmark`, plots results, and publishes PNGs to the
[`benchmark-results`](https://github.com/koomx/turbo/tree/benchmark-results)
branch. Images below refresh on push / `workflow_dispatch` (not on PR).

Dependencies (gtest / google-benchmark) are fetched via **CPM**, not vcpkg.
The CI workflow still uses `koomx/x-ci` `vcpkg-template` only for the
runner/matrix/publish plumbing.

## Gallery

#### `std20-ubuntu24-amd64`

![std20-ubuntu24-amd64](https://raw.githubusercontent.com/koomx/turbo/benchmark-results/latest/std20-ubuntu24-amd64/plots/summary.png)

#### `std20-ubuntu24-arm64`

![std20-ubuntu24-arm64](https://raw.githubusercontent.com/koomx/turbo/benchmark-results/latest/std20-ubuntu24-arm64/plots/summary.png)

## Local

```bash
cmake --preset=ci
cmake --build build-ninja -j
python3 scripts/run_log_bench.py
python3 scripts/plot_log_bench.py
# → benchmark-results/plots/summary.png
```
