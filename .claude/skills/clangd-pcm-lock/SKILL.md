---
name: clangd-pcm-lock
description: Recover from C++23 modules build failures on Windows where `cmake --build` fails to write a `.pcm` file because clangd has it memory-mapped. Symptoms include "unable to open output file ... .pcm: The requested operation cannot be performed on a file with a user-mapped section open" or "Permission denied" on `.pcm`. Fix is to stop clangd before building - the editor's LSP client respawns it automatically. Apply preemptively before any `cmake --build` of a modules project on Windows.
---

# Clangd .pcm File Lock (Windows-only)

## Symptom

A `cmake --build` of a C++ modules project on Windows fails with one of these:

```
error: unable to open output file 'CMakeFiles/.../foo.pcm':
       'The requested operation cannot be performed on a file with
        a user-mapped section open'

error: unable to open output file '.../foo.pcm': 'Permission denied'
```

This happens even on a fresh `Remove-Item -Recurse -Force build` rebuild, and is non-deterministic: sometimes the build succeeds, sometimes the same edit fails right after.

## Root cause

Clangd (the C++ LSP server used by VS Code, Neovim, Helix, etc.) memory-maps the `.pcm` (Clang pre-compiled module) files in your build tree to resolve module symbols during code navigation and completion. On Windows, an open memory mapping locks the underlying file — the OS refuses writes from any other process until every mapping is released.

The race:

1. You edit `src/foo.cppm`.
2. cmake/ninja decides `foo.pcm` (and dependents) must be rebuilt.
3. clang opens `foo.pcm` for writing → Windows: "denied, clangd has it mapped".
4. Build fails.

This is **Windows-only** — Linux/macOS `mmap` doesn't lock the underlying file, so the same workflow has no problem there.

## Fix

Stop clangd before invoking the build. The LSP client in the editor will respawn clangd automatically on the next request (usually within ~1 second of returning focus to a code file). No editor restart needed.

```powershell
Get-Process -Name clangd -ErrorAction SilentlyContinue | Stop-Process -Force
cmake --build build
```

Or as one line:

```powershell
Get-Process -Name clangd -ErrorAction SilentlyContinue | Stop-Process -Force; cmake --build build
```

The `-ErrorAction SilentlyContinue` makes the call a safe no-op if no clangd is running.

## When the agent should apply this

Apply preemptively, **before every `cmake --build`** invoked on Windows in a C++ modules project. It is cheap (≤ 50ms when no clangd is running) and the cost of skipping is a confusing build failure mid-iteration.

Detection signals from a previous failed build that should trigger immediate retry with the fix:

- The string `user-mapped section open` in stderr
- `Permission denied` on a path ending in `.pcm`
- `unable to open output file` referencing the build tree

Recovery flow:

```powershell
# 1. The fix
Get-Process -Name clangd -ErrorAction SilentlyContinue | Stop-Process -Force

# 2. Retry the same build command
cmake --build build
```

The first attempt after killing clangd should succeed. If it still fails with the same error, check for:

- Another PowerShell session running a parallel build
- A second clangd respawned by an editor focus event during the kill+build window (run the kill+build combo as a single PowerShell command, joined by `;`, so nothing slips in between)
- A non-clangd process holding the file (rare — `handle.exe` from Sysinternals can identify it)

## What NOT to do

- **Don't `Remove-Item -Recurse -Force build`** as a workaround — Windows will refuse to delete the mapped `.pcm` files too, surfacing the same lock as a different "in use" error.
- **Don't disable clangd's module support** in `~/.config/clangd/config.yaml` — you lose completion and goto-definition on imported names, which is the whole point of using modules through an LSP.
- **Don't insert `Start-Sleep` thinking the lock will time out** — `mmap` locks don't time out; they release only when the mapping is closed (i.e. when clangd exits).
- **Don't restart the editor each time** — killing the clangd process is sufficient; LSP clients auto-restart it. Restarting the editor is the same fix with extra steps.

## Scope

- Windows + Clang + C++ modules (`FILE_SET CXX_MODULES` or any `.pcm`-producing build).
- Linux and macOS users can ignore this skill entirely.
- Does not apply to non-module C++ builds (no `.pcm` files are generated).
- The same lock affects the `.obj` files that pair with each `.pcm` (e.g. `foo.cppm.obj`); killing clangd resolves both since the mapping is on the `.pcm` and the `.obj` is rewritten when its sibling is.
