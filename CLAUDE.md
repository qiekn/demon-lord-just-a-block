# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

A from-scratch remake of *卡牌魔王，只剩个头* (Steam app 3720420), originally built in Unity. Currently a scaffold: a single raylib window with a NotoSansSC default font. No gameplay yet.

Near-term roadmap (in order):
1. Basic player movement.
2. Sprite + map rendering off the exported assets in `./export/`.
3. Design pass on dodge / parry mechanics, then implementation.

## Build & Run

First-time setup pulls all submodules and clones raylib into Raylib-Hpp's `refs/` (gitignored by the wrapper):

```sh
git submodule update --init --recursive
git clone --depth 1 https://github.com/raysan5/raylib.git deps/Raylib-Hpp/refs/raylib
cmake -S . -B build -G Ninja
cmake --build build
./build/block.exe
```

`run.sh` is the rebuild-and-run shortcut.

The binary loads assets relative to the CWD (`assets/fonts/NotoSansSC-Regular.ttf`, `assets/zh-sc-3500.txt`), so **always run from the repo root**, not from `build/`.

No tests yet. Formatting is enforced by Google-based `.clang-format` (2-space indent, 120 col, `PointerAlignment: Left`).

## Toolchain Requirements

- CMake **≥ 3.30** (required by `CMAKE_EXPERIMENTAL_CXX_IMPORT_STD`).
- Clang with `-stdlib=libc++`. GCC/MSVC are not supported because we hardcode libc++ for `import std;`.
- `cmake/EnableCxxImportStd.cmake` downloads the matching `CxxImportStd` GUID from CMake's source for the running CMake version. First configure needs internet; the file is cached under `build/cmExperimental.cxx` afterwards.
- MSYS2 UCRT64 is the dev environment; PowerShell is the default shell.

## Module Policy

- `import std;` — yes, everywhere we can.
- `import raylib;` — yes, Raylib-Hpp ships a C++23 module that exports the curated API (`ck::raii::Window`, `ck::Drawing`, `ck::SetDefaultFont`, color constants, etc.). See `deps/Raylib-Hpp/CLAUDE.md` for the namespace breakdown.
- Everything else (imgui, entt, project-internal headers) — plain `#include`. Don't try to wrap them in modules.

If a raylib symbol is missing from `import raylib;`, it almost certainly needs to be added to `deps/Raylib-Hpp/src/raylib.cppm` upstream — the module re-exports a curated subset by design.

## Architecture

Single executable today (`src/main.cpp` only). Planned layout, modelled on baba (`../baba/`) and ck-engine (`~/projects/ck-engine/`):

- **Layer / LayerStack** — virtual `OnAttach / OnDetach / OnUpdate(dt) / OnRender / OnImGuiRender`, driven by the main loop. Overlays sit on top of layers; events iterate in reverse.
- **World** — what other engines call "Scene". Wraps `entt::registry`. Owns actors and runs systems.
- **Actor** — what other engines call "Entity" or "GameObject". Thin handle over `(entt::entity, World*)`; templated `AddComponent / GetComponent / HasComponent / RemoveComponent`.
- **ImGuiLayer** — overlay that owns the ImGui context and the `imgui_impl_glfw` + `imgui_impl_opengl3` backends. raylib uses GLFW + GL 3.3 internally on PLATFORM_DESKTOP, so we grab the active context via `glfwGetCurrentContext()` after `InitWindow`.

Naming chosen deliberately: **World** instead of Scene, **Actor** instead of Entity/GameObject. Stick to that vocabulary in new code so the public API stays self-consistent.

## Dependencies (`deps/`)

Submodules:
- `deps/Raylib-Hpp` — qiekn/Raylib-Hpp; provides `import raylib;` (target `raylib_hpp_modules`) and bundles raylib at `refs/raylib` (gitignored by the wrapper; must be cloned locally).
- `deps/imgui` — ocornut/imgui on the **docking** branch. Not yet linked into the binary; will be wired up alongside `ImGuiLayer`.
- `deps/entt` — header-only; exposed as INTERFACE target `entt`. Use `#include <entt/entt.hpp>` once linked.

`deps/CMakeLists.txt` wires up the targets. When adding new deps, prefer submodules over vendoring.

## Assets

Two directories with distinct roles:

- `assets/` — runtime assets the binary loads at startup. Committed. Currently only the default font (`fonts/NotoSansSC-Regular.ttf`) and codepoint list (`zh-sc-3500.txt`).
- `export/` — raw Unity asset dump from the original game, sorted by Unity type:

  | dir | count | format | notes |
  |---|---|---|---|
  | `Sprite/` | 2330 | png | UI + character + tile art |
  | `Texture2D/` | 2140 | png | mostly the same set as Sprite (Unity duplicates the export) |
  | `MonoBehaviour/` | 3429 | json | serialized component data — config tables, level data, etc. |
  | `TextAsset/` | 164 | txt | `AchievementConfigs.txt`, `DialogueConfigs.txt`, `DialogueShortConfig.txt`, ... |
  | `AudioClip/` | 155 | wav | SFX + music |
  | `Animator/` | 40 | fbx | animator state machines |
  | `Mesh/` | 13 | obj | mostly Unity primitives |
  | `Font/` | 6 | ttf | original-game fonts |
  | `VideoClip/` | 2 | mp4 | `doggeTeach.mp4`, `parryTeach.mp4` (in-game tutorials for dodge/parry) |

  `export/` is **not** loaded by the binary directly; treat it as the source dump. When a sprite or config is needed, copy/convert it into `assets/` (and any binary tool / sheet packer lives under a future `tools/`).

  `export/` is gitignored — it's a local working copy, not committed.

## Repo Layout

- `build/` — gitignored build output.
- `cmake/` — `EnableCxxImportStd.cmake` only.
- `src/` — game source. Right now just `main.cpp`; engine modules land here as Layer/World/Actor get implemented.
- `deps/` — submodules + `CMakeLists.txt` wiring.
- `assets/` — runtime assets (committed).
- `export/` — Unity asset dump (local only, gitignored).

## Git

Auto-commit is on (no `.claude/.no-autocommit`). After completing a logical change, commit it without waiting to be asked. Commit format follows the `git-commit-message` skill — `type(scope): summary`, imperative mood, one idea per commit.
