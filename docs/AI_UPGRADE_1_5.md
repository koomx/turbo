# kmcmake 1.5 Upgrade (for AI Agents)

Operational steps only. Replace `xlog` / paths with the real project.

**Target version:** `1.5.0+`  
**Audience:** AI agents upgrading an existing kmcmake-based repo.

## Preconditions

- Project already has `kmcmake/` + `cmake/` split (1.4-style).
- If still pre-v1 flat layout → stop; follow `docs/AI_UPGRADE.md` first.

## DO / DO NOT

| DO | DO NOT |
|----|--------|
| Generate a fresh skeleton under `/tmp/<proj>_upgrade` | Edit `kmcmake/` by hand patches |
| Replace the real repo's `kmcmake/` from that skeleton | Overwrite `cmake/`, `<project>/`, `tests/`, `examples/` |
| Copy `CMakePresets.json` from the skeleton | Generate/install the template into the real project tree |
| Backup before replace | Delete `*.bak-*` until the user confirms |
| Wait for user approval before `git commit` / `git push` | Touch kmcmake **source** repo files as the upgrade target |

## Procedure

Example project name: `xlog`.

### 1) Locate kmcmake template source

Set `KMCMAKE_SRC` to a checkout / release of `kumose/kmcmake` at **1.5.0+**.

Template root: `$KMCMAKE_SRC/template`

### 2) Generate skeleton into `/tmp` (never into the real project)

```bash
PROJ=xlog
TMP=/tmp/${PROJ}_upgrade
KMCMAKE_SRC=<path-to-kmcmake-repo>   # e.g. /home/jeff/github/kumose/kmcmake
REAL=<path-to-real-project>          # e.g. /home/jeff/github/kumose/xlog

rm -rf "$TMP"
cmake -S "$KMCMAKE_SRC/template" -B "$TMP/build" -DCHANGEME="$PROJ"
cmake --install "$TMP/build" --prefix "$TMP/skel"
```

Expect at least:

- `$TMP/skel/kmcmake/`
- `$TMP/skel/CMakePresets.json`
- `$TMP/skel/docs/AI_UPGRADE_1_5.md`

### 3) Backup real project files

```bash
cp -a "$REAL/kmcmake" "$REAL/kmcmake.bak-pre-1.5.0"
if [ -f "$REAL/CMakePresets.json" ]; then
  cp -a "$REAL/CMakePresets.json" "$REAL/CMakePresets.json.bak-pre-1.5.0"
fi
```

### 4) Replace `kmcmake/` only

```bash
rm -rf "$REAL/kmcmake"
cp -a "$TMP/skel/kmcmake" "$REAL/kmcmake"
```

Whole-tree replace only. Do not copy individual `.cmake` files.

### 5) Copy `CMakePresets.json` (required)

1.5 template presets:

- `default` — Unix Makefiles → `build/`
- `ninja` — Ninja → `build-ninja/`
- both inherit `base` with `CMAKE_TOOLCHAIN_FILE=$env{KMPKG_CMAKE}`

```bash
cp -a "$TMP/skel/CMakePresets.json" "$REAL/CMakePresets.json"
```

If the project had custom presets: keep skeleton `base` / `default` / `ninja`, then re-add custom entries. Do not keep an old presets file that lacks `ninja` when Windows / CI expects it.

### 6) Optional docs sync

```bash
mkdir -p "$REAL/docs"
cp -a "$TMP/skel/docs/AI.md" "$REAL/docs/AI.md"
cp -a "$TMP/skel/docs/AI_UPGRADE_1_5.md" "$REAL/docs/AI_UPGRADE_1_5.md"
```

Do **not** copy `$TMP/skel/cmake/`, `$TMP/skel/$PROJ/`, or `$TMP/skel/tests/` into the real project.

### 7) Verify (only if the user asked to build/test)

```bash
cd "$REAL"
rm -rf build build-ninja
cmake --preset=default -DKMCMAKE_BUILD_TEST=ON -DCMAKE_BUILD_TYPE=Release
# or: cmake --preset=ninja -DKMCMAKE_BUILD_TEST=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure -j1
```

Spot-check: for a library with `PLINKS`/`LINKS` to packaged deps (e.g. fmt / gtest), `*_OBJECT` compile includes should contain `kmpkg_installed/.../include`.

### 8) Cleanup

```bash
rm -rf "$TMP"
# Keep *.bak-* until the user confirms the upgrade.
```

## From 1.4.x → 1.5.0 checklist

| Item | Action |
|------|--------|
| Framework | Steps 2–4 (replace `kmcmake/`) |
| Presets | Step 5 (**always** copy `CMakePresets.json`) |
| Shared libs | Need `.so`/`.dll`? Ensure `SHARE` on that `kmcmake_cc_library` (no global share flag) |
| Hand-edited `kmcmake/` | Discard local edits; whole-tree replace |
| CI x-ci `@v1` | Optional: migrate to x-ci `@v2` platform workflows — not required by this guide |

## Related docs

- Legacy (pre-v1 → layered): `docs/AI_UPGRADE.md`
- AI context / API: `docs/AI.md`
- Release notes: repo root `CHANGELOG.md` / `CHANGELOG_CN.md`
