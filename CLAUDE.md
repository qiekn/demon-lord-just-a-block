# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

A from-scratch remake of *卡牌魔王，只剩个头* (Steam app 3720420), originally built in Unity. Currently a scaffold: raylib window with NotoSansSC default font, spdlog logger, ImGui demo overlay, window position persistence. No gameplay yet.

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
- Everything else (imgui, entt, spdlog, project-internal headers) — plain `#include`. Don't try to wrap them in modules.

If a raylib symbol is missing from `import raylib;`, it almost certainly needs to be added to `deps/Raylib-Hpp/src/raylib.cppm` upstream — the module re-exports a curated subset by design.

### `import std;` ✗ `#include <std-header>` — the load-bearing rule

Clang 21 + libc++ rejects mixing `import std;` with traditional `#include <format>` (and a handful of other heavily-templated std headers) in the same TU. The std types end up declared twice — once through the module path, once through the header path — and template alias redefinition errors blow up the build.

**Project rule:** project-internal headers (e.g. `src/log.hpp`) that callers include from a TU using `import std;` must be std-header-free. Use one of these escape hatches when the API would naturally want std types:

- **Macros that expand `std::format` at the call site.** `src/log.hpp` is the canonical example: it declares only `void log::Info(const char*)` etc., and ships `BLOCK_LOG_*` macros that expand to `::block::log::Info(::std::format(...).c_str())`. The caller's TU is responsible for having `std::format` in scope (via `import std;`).
- **Opaque interfaces / PIMPL.** Public headers take POD types (`const char*`, ints, raw pointers) and hide std containers in the .cpp via `struct Impl;`.
- **Local non-`import-std` TUs.** Files like `src/imgui_layer.cpp` and `src/application.cpp` use plain `#include <vector>` etc. and never `import std;`. They can pull in any headers they like; main.cpp just stays away from those headers itself.

If you find yourself adding `#include <format>`, `#include <ranges>`, etc. to a header that main.cpp transitively pulls in, stop — go pick one of the patterns above instead.

## Logging

```cpp
#include "log.hpp"

block::Log::Init();              // call once at startup
block::log::Info("Block ready"); // const char* (no format args)
BLOCK_LOG_INFO("frame {:.2f} ms", dt * 1000.0f);  // formatted
```

Levels: `BLOCK_LOG_{TRACE,DEBUG,INFO,WARN,ERROR,FATAL}` and the matching `block::log::*` plain-string functions. Both route to a single shared spdlog stdout-color sink. Pattern: `[HH:MM:SS] [level] msg`. Default level is `trace`.

## Window State

`window.state` (gitignored) holds the previous run's window geometry as `x y w h` on a single line. `Application` loads it before `InitWindow` and saves before `CloseWindow`. The file lives in CWD; running from outside the repo root creates a stray copy.

## Architecture

**Current** (top-down, all under `src/`):

- `main.cpp` — entry point. `import std;` + `import raylib;`. Initializes `block::Log`, creates `block::Application`, loads the default font, pushes layers, runs.
- `application.hpp/cpp` — `block::Application` owns the raylib window and the `LayerStack`. Window geometry is loaded/saved through `window_state.hpp/cpp`. ApplicationSpec carries name/size/fps/dpi.
- `layer.hpp` — `block::Layer` base with `OnAttach / OnDetach / OnUpdate(dt) / OnRender / OnImGuiRender / OnImGuiBegin / OnImGuiEnd`. Std-free header so consumers can `import std;` freely.
- `imgui_layer.hpp/cpp` — overlay derived from `Layer`. Owns the ImGui context + the `imgui_impl_glfw` + `imgui_impl_opengl3` backends. Overrides `OnImGuiBegin/End` to bracket the frame around every other layer's `OnImGuiRender`. raylib uses GLFW + GL 3.3 internally on PLATFORM_DESKTOP, so the backend grabs the active context via `glfwGetCurrentContext()` after `InitWindow`.
- `log.hpp/cpp` — spdlog-backed logger. See "Logging" above.
- `window_state.hpp/cpp` — `x y w h` persistence. See "Window State" above.
- `assets.hpp` — `BLOCK_ASSET("rel/path")` compile-time macro that prefixes with `assets/`, plus a runtime `block::AssetPath()` for paths built from dynamic strings.

**Planned** (not yet implemented):

- **World** — what other engines call "Scene". Wraps `entt::registry`. Owns actors and runs systems.
- **Actor** — what other engines call "Entity" or "GameObject". Thin handle over `(entt::entity, World*)`; templated `AddComponent / GetComponent / HasComponent / RemoveComponent`.

Naming chosen deliberately: **World** instead of Scene, **Actor** instead of Entity/GameObject. Stick to that vocabulary in new code so the public API stays self-consistent.

## Dependencies (`deps/`)

Submodules:
- `deps/Raylib-Hpp` — qiekn/Raylib-Hpp; provides `import raylib;` (target `raylib_hpp_modules`) and bundles raylib at `refs/raylib` (gitignored by the wrapper; must be cloned locally).
- `deps/imgui` — ocornut/imgui on the **docking** branch. Built as a local static `imgui` target with the glfw + opengl3 backends.
- `deps/entt` — header-only; exposed as INTERFACE target `entt`. Linked but not yet used.
- `deps/spdlog` — header-only; exposed as INTERFACE target `spdlog_headers`. Linked via `log.cpp` only.

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
- `src/` — game source. Engine modules (layer/application/log/window_state/assets/imgui_layer) live alongside `main.cpp`. Add new layers (e.g. game logic) as additional `*_layer.{hpp,cpp}` files.
- `deps/` — submodules + `CMakeLists.txt` wiring.
- `assets/` — runtime assets (committed). Reach into via `BLOCK_ASSET("...")` from `src/assets.hpp`.
- `export/` — Unity asset dump (local only, gitignored).
- `docs/` — mdBook scaffold for design notes (separate from this guide).

## Git

Auto-commit is on (no `.claude/.no-autocommit`). After completing a logical change, commit it without waiting to be asked. Commit format follows the `git-commit-message` skill — `type(scope): summary`, imperative mood, one idea per commit.

**Commit messages are a single line.** No extended body, no bullet lists. If the summary can't carry the change, split the commit instead. Prefer losing detail over multi-line messages — the diff is the source of truth.
