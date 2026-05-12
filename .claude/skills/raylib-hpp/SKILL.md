---
name: raylib-hpp
description: How to use the Raylib-Hpp C++23 wrapper that this project bundles at deps/Raylib-Hpp. Covers import paths, namespaces (ck::, ck::raii::, rl::), RAII patterns, the colors-and-macros trick, and how to add symbols to the import raylib; module.
---

# Raylib-Hpp Usage Guide

This project does **not** call raylib's C API directly in most files. It uses
the bundled wrapper at `deps/Raylib-Hpp/` (a header-only library + a C++23
named module). When writing or reviewing code that touches raylib, use this
guide.

The wrapper's own docs live at `deps/Raylib-Hpp/CLAUDE.md` — read that for
implementation details when extending the wrapper itself. This file is the
**consumer** guide.

## How to bring raylib in

Three entry points, **one per TU** (do not mix):

| Path | When | Notes |
|---|---|---|
| `import raylib;` | Default in `src/main.cpp` and any TU that also uses `import std;` | Curated subset — see "Adding to the module" below if a symbol is missing |
| `#include "raylib.hpp"` | Same surface as the module, header form | Pulls all of `include/raylib-hpp/*.hpp` |
| `#include <raylib.h>` | Only in TUs that do NOT `import std;` AND only when you need raw C symbols | Leaks macros (`RED`, `KEY_*`); shadows `ck::` constants |

`src/application.cpp`, `src/game_layer.cpp`, `src/imgui_layer.cpp` use the
third form because they don't `import std;` and they pre-date the module.
**New code should use `import raylib;`** unless there's a reason not to.

## Namespaces — what lives where

| Namespace | Contains | Example |
|---|---|---|
| `::` (global) | Math/POD types from raylib C headers | `Vector2`, `Color`, `Rectangle`, `Camera2D`, `Image`, `Ray` |
| `ck::raii::` | RAII resource handles | `Window`, `Texture`, `Font`, `Shader`, `Image`, `RenderTexture`, `Mesh`, `Model`, `Sound`, `Music`, `Wave`, `AudioStream`, `AudioDevice`, `Material`, `ModelAnimations` |
| `ck::` | Scope guards, free functions, color constants, enum aliases | `ck::Drawing`, `ck::RED`, `ck::IsKeyDown`, `ck::DrawText`, `ck::KEY_W` (via `using enum`) |
| `rl::` | Every raylib `RLAPI` C function, unmodified | `rl::DrawRectangle`, `rl::LoadTexture` — escape hatch when `ck::` doesn't expose what you need |

`using namespace ck;` + `using namespace ck::raii;` at the top of a TU is the
common pattern (see `src/main.cpp`).

## RAII handles — the rules

All GPU resources are CRTP-derived from `Resource<Derived, Handle>`
(`include/raylib-hpp/resource.hpp`):

- **Non-copyable, movable** via the `RLHPP_DECLARE_MOVE` macro.
- `Get()` returns the raw POD handle. `Release()` hands ownership out.
  `Reset()` (no args) unloads. `Reset(handle)` swaps; guards against
  `Reset(Get())`.
- `explicit operator bool()` delegates to `Valid()`.

```cpp
ck::raii::Texture tex(CK_ASSET("sprites/player.png"));
if (!tex) { /* load failed */ }
tex.SetFilter(TEXTURE_FILTER_BILINEAR).DrawV({100, 100});  // member chaining
::DrawTextureV(tex.Get(), {100, 100}, WHITE);              // raw handle if needed
```

**`ck::raii::Window` is special:** non-copyable AND non-movable, by design.
A single GL context. GPU resources (textures, shaders, render targets) must
not outlive the window — declare the `Window` (or `ck::Application`,
which owns one) first, then any resources, and let normal scope order tear
them down in reverse.

## Scope guards — prefer over Begin/End pairs

Every `Begin*`/`End*` pair has an RAII guard in `ck::`:

```cpp
{
  ck::Drawing draw;             // BeginDrawing / EndDrawing
  ck::ClearBackground(ck::BLACK);
  {
    ck::Mode2D cam{camera};     // BeginMode2D / EndMode2D
    ::DrawRectangle(0, 0, 32, 32, ck::RED);
  }
}
```

Guards available: `Drawing`, `Mode2D`, `Mode3D`, `ShaderMode`, `TextureMode`,
`BlendMode`, `ScissorMode`. All are `[[nodiscard]]`.

The raw `BeginDrawing()`/`EndDrawing()` etc. are also exported under `ck::`
for cases where a guard is awkward (e.g. ImGui layer wrapping the frame).

## Colors — the macro trick

`raylib.h` defines colors as macros: `#define RED CLITERAL(Color){...}`.
The wrapper:

1. `raylib_c.hpp` includes `raylib.h` and `#undef`s every color macro.
2. `colors.hpp` re-declares them as `inline constexpr ::Color` in `ck::`.

**Consequence:** use `ck::RED`, not `RED`. After `using namespace ck;` both
read as `RED`, but if you ever `#include <raylib.h>` in the same TU you'll
re-introduce the macros and break the module build. Pick one path per TU.

The same fix is applied to `KEY_*` and `MOUSE_BUTTON_*` (via `using enum ::KeyboardKey` / `using enum ::MouseButton` in the module).

## Text — use `ck::DrawText`, not `::DrawText`

`ck::DrawText` / `ck::MeasureText` (defined in `font_default.hpp`) route
through a registered default font when one is set via `ck::SetDefaultFont`.
`main.cpp` registers NotoSansSC at startup, so anything calling `ck::DrawText`
gets Chinese glyphs for free. Calling `::DrawText` bypasses this and renders
the raylib builtin (ASCII only).

`game_layer.cpp` currently calls bare `DrawText` because it `#include`s
`<raylib.h>` — that should migrate to `ck::DrawText` when it next gets touched.

## Adding a symbol that's missing from `import raylib;`

The module re-exports a **curated subset**. If `import raylib;` doesn't see
the symbol you need:

1. Check `deps/Raylib-Hpp/src/raylib.cppm` — is there a `using` declaration
   for it? If not, this is the gap.
2. The symbol is probably defined as a free function in one of the
   `include/raylib-hpp/*.hpp` topic files (e.g. `shapes.hpp` for `DrawCircle`).
3. Add `using ck::TheSymbol;` in the matching section of `raylib.cppm`.
   Drawing functions are C functions in the global namespace — re-export
   those as `export using ::DrawXxx;` outside `namespace ck`.
4. Rebuild. The module re-export is **not automatic**.

If the symbol doesn't have a `ck::` wrapper at all, you have two choices:
- Call it as `rl::TheSymbol(...)` — every `RLAPI` is mirrored there.
- Add a thin `ck::` wrapper in the right `*.hpp` (e.g. `shapes.hpp`), then
  add the `using` to `raylib.cppm`. Match the existing pattern: only wrap
  if you're adding value (`std::string` overload, RAII, etc.).

## Adding a new RAII resource type

Mirror `texture.hpp`:

1. New header in `deps/Raylib-Hpp/include/raylib-hpp/foo.hpp`. Include
   `raylib_c.hpp` (never `raylib.h` directly) and `resource.hpp`.
2. `class Foo : public Resource<Foo, ::FooHandle>` in `namespace ck::raii`.
3. Implement `Valid()` (calls raylib's `IsFooValid` or equivalent) and
   `Reset()` (no-arg; unloads the resource and zeroes the handle).
4. Destructor calls `Reset()`.
5. `RLHPP_DECLARE_MOVE(Foo)` to get defaulted move ops.
6. Add `#include "foo.hpp"` to `raylib.hpp`.
7. Add `using ck::raii::Foo;` to the RAII section of `raylib.cppm`.

## `import std;` + raylib — the load-bearing rule

This is the wider project rule (see root `CLAUDE.md`), restated for context:
Clang 21 + libc++ rejects mixing `import std;` with traditional
`#include <format>` (and other heavy std headers) in the same TU.

This is why `src/log.hpp` is std-free and ships `CK_LOG_*` macros that
expand `std::format(...)` at the call site. Raylib-Hpp itself is fine to
`#include` from an `import std;` TU because its headers stick to `<string>`,
`<span>`, `<format>` only inside `.hpp` files that are not transitively
included by main.cpp's import-std path — `import raylib;` is the safe route
from those TUs.

If you find yourself debugging a "template alias redefinition" error in a TU
that uses both `import std;` and a project header, the project header is
probably pulling a heavy std header. Fix the header, not the TU.

## Quick reference — common patterns

```cpp
// main.cpp pattern
import std;
import raylib;
using namespace ck;
using namespace ck::raii;

ck::Application app{{.name = "Block"}};   // owns the Window
auto codepoints = LoadCodepoints(ReadFile(CK_ASSET("zh-sc-3500.txt")));
Font noto(CK_ASSET("fonts/NotoSansSC-Regular.ttf"), 64, codepoints);
SetTextureFilter(noto.Get().texture, TEXTURE_FILTER_BILINEAR);
SetDefaultFont(noto);
```

```cpp
// Inside a Layer::OnRender — Application has already opened BeginDrawing
// via Application::Run, so DO NOT add another ck::Drawing here.
::DrawRectangle(x, y, w, h, ck::RED);
ck::DrawText("hello", 10, 10, 24, ck::BLACK);   // default-font aware
```

```cpp
// Camera2D
::Camera2D cam{.offset = {w/2.f, h/2.f}, .target = player_pos, .zoom = 1.0f};
{
  ck::Mode2D mode{cam};
  // world-space draws here
}
```

## Don't

- Don't `#include <raylib.h>` in a TU that also uses `import raylib;` — the
  macros leak and shadow `ck::` constants.
- Don't wrap `::DrawRectangle` / `::DrawCircle` / etc. just to add a `ck::`
  prefix. They're already re-exported under `ck::` via the module; in
  header-include builds they live at `::`. Wrap only when adding value.
- Don't move a `ck::raii::Window`. It's deleted, and the deletion is load-bearing.
- Don't call `Reset(handle.Get())` expecting it to be a no-op via reference
  equality — `Resource::Reset` byte-compares the POD handle to guard against this.
- Don't store a raw `::Texture2D` (or other handle) and let it outlive its
  `ck::raii::` owner. Always pass `tex.Get()` at the use site.
