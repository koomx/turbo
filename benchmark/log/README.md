# Log benchmarks

CI runs `log_log_benchmark` and `log_zlog_benchmark`, plots results, and
publishes PNGs to the
[`benchmark-results`](https://github.com/koomx/turbo/tree/benchmark-results)
branch. Images below refresh on push / `workflow_dispatch` (not on PR).

Dependencies (gtest / google-benchmark) are fetched via **CPM**.
The CI workflow uses `koomx/x-ci` `vcpkg-template` only for runner / matrix /
publish plumbing.

Threaded log benches use `ThreadRange(1, 2 * hardware_concurrency())`
locally. On GitHub Actions (`GITHUB_ACTIONS` set), `MaxBenchThreads()` is `1`
and `scripts/run_log_bench.py` also filters to single-thread cases, so ARM CI
does not oversubscribe / hang.

## Gallery

#### `std20-ubuntu24-amd64`

![std20-ubuntu24-amd64](https://raw.githubusercontent.com/koomx/turbo/benchmark-results/latest/std20-ubuntu24-amd64/plots/summary.png)

#### `std20-ubuntu24-arm64`

![std20-ubuntu24-arm64](https://raw.githubusercontent.com/koomx/turbo/benchmark-results/latest/std20-ubuntu24-arm64/plots/summary.png)

#### `macos-arm64`

![macos-arm64](https://raw.githubusercontent.com/koomx/turbo/benchmark-results/latest/macos-arm64/plots/summary.png)

#### `windows`

![windows](https://raw.githubusercontent.com/koomx/turbo/benchmark-results/latest/windows/plots/summary.png)

## Local

```bash
cmake --preset=ci
cmake --build build-ninja --parallel
python3 scripts/run_log_bench.py
python3 scripts/plot_log_bench.py
# → benchmark-results/plots/summary.png
```
