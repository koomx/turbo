# turbo/macros 当前计划

> 状态：现行  
> 旧「base → macros 迁移」规划已废弃；`turbo/macros/config.h` / `macros.h` 已基本改为转调 `turbo/macros`。  
> API 摘要见 `skills.h`。

---

## 1. 首要目标：C 兼容

**默认要求：** `#include <turbo/macros/macros.h>`（及各子伞头）在 **C99/C11** 与 **C++11+** 下均可编译。

| 原则 | 说明 |
|------|------|
| 能共用则共用 | 探测宏、属性、分支提示等用纯预处理 / C 语法 |
| C++ 专用必须门闩 | `namespace` / `template` / `std::` / `decltype` / `static_cast` 等包在 `#ifdef __cplusplus` |
| 缺能力可后补 | 不要求每个宏在 C 都有完整等价实现；C 下可降级为 no-op 或不定义（调用方 `#ifdef`） |
| 值宏契约不变 | 检测类 `KUMO_*` 仍为 `0\|1`，用 `#if` 不用 `#ifdef`（C++ 专用探测除外） |

**非目标（本阶段）：**

- 不把 C++ 库特性（ordering / source_location / trivially_*）搬到 C
- 不强制全库 `.cc` 改写成 C
- 不重开 base 宏迁移叙事

---

## 2. 验收标准

1. 最小 `.c` 能 `#include <turbo/macros/macros.h>` 并通过 `-fsyntax-only`（或等价编译）。
2. 同一翻译单元在 `NDEBUG` / `KUMO_OPTION_HARDENED` 常见组合下 C/C++ 均可过。
3. C++ 既有语义不回退（尤其 `KUMO_ASSERT` constexpr 友好路径、attributes、hardening）。
4. `skills.h` 标明哪些宏仅 C++。

---

## 3. 工作清单（按优先级）

### P0 — 伞头可被 C include

保证 include 链上无「裸 C++」：

| 区域 | 关注点 |
|------|--------|
| `optimization/assume.h` | `void()` / `decltype` / `std::unreachable` → 双路径 |
| `optimization/unreachable.h` | `<utility>` / `std::unreachable` 仅 C++ |
| `optimization/optimization.h` | 勿在 C 拉 C++ 头；`BLOCK_TAIL_CALL` 等 C 可用 |
| `utility/basic.h` | `ARRAYSIZE` 模板、`GLOBAL_INIT` 等已有分支则核对；缺门闩补上 |
| `macros/attributes.h` | 抽查未门闩的 C++-only 片段 |
| `macros/using_std.h` / `have/std20.h` / `have/system.h` 中 STL 探测 | 整文件或整段 `#ifdef __cplusplus`，C 不展开 |
| `macros/raw_log.h` | 已有 C 路径则回归；避免 C 编译进 `namespace` 实现 |

样板：`macros/assert.h`、`optimization/like.h`（`0`/`1` 而非 `true`/`false`）。

### P1 — 回归与文档

- 固定一两个 smoke `.c` / `.cc`（或测试目标）锁住伞头 C 可编译
- 更新 `skills.h`：C 兼容为设计原则；标注 C++-only API
- 各子目录 `README.md` 若仍写迁移故事则改为一句现状 + 指向本 plan

### P2 — 按需补齐（以后再说）

- C 版 `ARRAYSIZE` / 更强的静态检查
- C 版 `*_MSG` assert、更完整的 raw_log
- 个别属性在 C 前端的差异（MSVC C 等）

---

## 4. 做法约定

```c
#ifdef __cplusplus
// C++-only implementation or definition
#else
// C: equivalent, no-op, or omit the macro
#endif
```

- 优先改现有文件，不新建「c_compat」平行树。
- 不主动扩依赖；C 路径只用 C 标准头（如 `assert.h` / `stdlib.h` / `stdbool.h` 若必要）。
- `false`/`true` 出现在会进 C 的宏里时改为 `0`/`1`。

---

## 5. 已完成（现状备忘）

- 平台 / arch / SIMD / compiler / libc / gpu / have（含 sanitizer）主体为值宏
- `turbo/macros/config.h`、`macros.h` 已基本转调 macros
- `assert.h`、`like.h`、`raw_log.h` 等已具备 C/C++ 双路径样板
- P0 核心：`assume.h` / `unreachable.h` / `optimization.h` / `exception.h` / `std20` 源位置探测已按 C 可用修正
- `tests/macros/c_header_test.c` 覆盖伞头 + `ASSERT`/`ASSUME`/`UNREACHABLE`/`ARRAYSIZE`/`LIKELY` 等常用宏

---

## 6. 模块依赖检查（框架）

- 规则：`turbo/module_deps.toml`（严格白名单；未列出模块暂 `unchecked`）
- 工具：`scripts/check_module_deps.py`
  - `list <module> [-v]` — 列出该模块依赖的其它 turbo 模块
  - `list-all` / `modules` / `check [--staged|--all]`
- Hook：`scripts/git-hooks/pre-commit`（`git config core.hooksPath scripts/git-hooks`）

## 7. AI / 协作者

- 以本文件 + `skills.h` 为准；勿再执行已删除的 base→macros 迁移步骤。
- 改 macros 前先确认是否破坏「伞头 C 可编译」。
- 实现前仍遵守仓库 `docs/AI.md`（提案 → 用户确认 → 再改）。
