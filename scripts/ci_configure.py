#!/usr/bin/env python3
"""CI configure helper: force MSVC on Windows, plain cmake elsewhere.

x-ci's windows job runs Ninja without a VS developer prompt, so CMake may pick
MinGW from PATH. This script loads vcvarsall, persists the env via GITHUB_ENV
for later build/test steps, and configures with cl.exe explicitly.
"""
from __future__ import annotations

import os
import platform
import subprocess
import sys
from pathlib import Path

# Env vars vcvarsall must propagate for cl/link/lib to work in later steps.
_MSVC_ENV_KEYS = (
    "INCLUDE",
    "LIB",
    "LIBPATH",
    "WindowsSdkDir",
    "WindowsSDKVersion",
    "WindowsSDKLibVersion",
    "UniversalCRTSdkDir",
    "UCRTVersion",
    "VCToolsInstallDir",
    "VCToolsRedistDir",
    "VCINSTALLDIR",
    "VSINSTALLDIR",
    "VSCMD_ARG_TGT_ARCH",
    "VSCMD_ARG_HOST_ARCH",
    "Platform",
    "DevEnvDir",
    "ExtensionSdkDir",
    "NETFXSDKDir",
    "FrameworkDir",
    "FrameworkDir64",
    "FrameworkVersion",
    "FrameworkVersion64",
    "IFCPATH",
    "EXTERNAL_INCLUDE",
)


def _vswhere() -> Path:
    pf86 = os.environ.get("ProgramFiles(x86)", r"C:\Program Files (x86)")
    return Path(pf86) / "Microsoft Visual Studio" / "Installer" / "vswhere.exe"


def _vs_install_path() -> Path:
    vswhere = _vswhere()
    if not vswhere.is_file():
        raise RuntimeError(f"vswhere not found: {vswhere}")
    out = subprocess.check_output(
        [
            str(vswhere),
            "-latest",
            "-products",
            "*",
            "-requires",
            "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
            "-property",
            "installationPath",
        ],
        text=True,
    ).strip()
    if not out:
        raise RuntimeError("vswhere returned no Visual Studio installation")
    return Path(out)


def _host_arch() -> str:
    machine = (os.environ.get("PROCESSOR_ARCHITECTURE") or platform.machine() or "").lower()
    if machine in ("arm64", "aarch64"):
        return "arm64"
    return "x64"


def _parse_set_output(text: str) -> dict[str, str]:
    env: dict[str, str] = {}
    for line in text.splitlines():
        if "=" not in line:
            continue
        key, _, value = line.partition("=")
        if key:
            env[key] = value
    return env


def _env_get_ci(env: dict[str, str], key: str) -> str | None:
    if key in env:
        return env[key]
    key_l = key.lower()
    for k, v in env.items():
        if k.lower() == key_l:
            return v
    return None


def _load_msvc_env() -> dict[str, str]:
    vs = _vs_install_path()
    vcvars = vs / "VC" / "Auxiliary" / "Build" / "vcvarsall.bat"
    if not vcvars.is_file():
        raise RuntimeError(f"vcvarsall.bat not found: {vcvars}")
    arch = _host_arch()
    # Capture env after vcvarsall; use cmd.exe so bat works reliably.
    cmd = f'"{vcvars}" {arch} >NUL && set'
    completed = subprocess.run(
        cmd,
        shell=True,
        check=True,
        capture_output=True,
        text=True,
        executable=os.environ.get("COMSPEC"),
    )
    env = _parse_set_output(completed.stdout)
    if not env:
        raise RuntimeError("failed to parse environment from vcvarsall.bat")
    return env


def _persist_github_env(msvc_env: dict[str, str]) -> None:
    # Apply to this process (configure runs in the same step).
    os.environ.update(msvc_env)

    github_env = os.environ.get("GITHUB_ENV")
    if not github_env:
        return

    path_value = _env_get_ci(msvc_env, "PATH") or ""
    with open(github_env, "a", encoding="utf-8") as fh:
        # Full PATH replacement preserves vcvars order (MSVC before MinGW).
        if path_value:
            fh.write(f"PATH<<__MSVC_ENV_EOF__\n{path_value}\n__MSVC_ENV_EOF__\n")
        for key in _MSVC_ENV_KEYS:
            value = _env_get_ci(msvc_env, key)
            if value is None:
                continue
            fh.write(f"{key}<<__MSVC_ENV_EOF__\n{value}\n__MSVC_ENV_EOF__\n")


def _cmake_cmd() -> list[str]:
    cmd = [
        "cmake",
        "--preset=ci",
        "-DCMAKE_BUILD_TYPE=Release",
    ]
    if platform.system() == "Windows":
        cmd.extend(
            [
                "-DCMAKE_C_COMPILER=cl",
                "-DCMAKE_CXX_COMPILER=cl",
            ]
        )
    return cmd


def main() -> int:
    if platform.system() == "Windows":
        print("=== Activating MSVC (vcvarsall) for Windows CI ===", flush=True)
        msvc_env = _load_msvc_env()
        _persist_github_env(msvc_env)
        which = subprocess.run(
            ["where", "cl"],
            capture_output=True,
            text=True,
            check=False,
        )
        print(which.stdout or which.stderr, flush=True)
        if which.returncode != 0:
            raise RuntimeError("cl.exe not on PATH after vcvarsall; refusing MinGW fallback")

    cmd = _cmake_cmd()
    print("+", " ".join(cmd), flush=True)
    subprocess.check_call(cmd)
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as exc:  # noqa: BLE001 — CI entrypoint
        print(f"ci_configure failed: {exc}", file=sys.stderr)
        raise SystemExit(1)
