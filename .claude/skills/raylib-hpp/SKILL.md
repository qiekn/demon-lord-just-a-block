---
name: raylib-hpp
description: How to use the Raylib-Hpp C++23 wrapper that this project bundles at deps/raylib-hpp. Covers import paths, namespaces (ck::, ck::raii::, rl::), RAII patterns, the colors-and-macros trick, and how to add symbols to the import raylib; module.
---

# Raylib-Hpp Usage Guide

This project does **not** call raylib's C API directly in most files. It uses
the bundled wrapper at `deps/raylib-hpp/` (a header-only library + a C++23
named module). When writing or reviewing code that touches raylib, use this
guide.

The wrapper's own docs live at `deps/raylib-hpp/CLAUDE.md` — read that for
implementation details when extending the wrapper itself. This file is the
**consumer** guide.

## How to bring raylib in

`import raylib;` is the **only** allowed entry point in this project. `#include <raylib.h>` is forbidden — it leaks macros (color constants `RED`/`WHITE`/..., shadows `ck::` constants) and sidesteps the whole reason the wrapper exists. The header-form aggregate `#include "raylib.hpp"` exists upstream but isn't used here either.

| Path | Project policy |
|---|---|
| `import raylib;` | **Use this.** Curated subset; see "Adding to the module" below if a symbol is missing. |
| `#include "raylib.hpp"` | Not used in this repo. Same surface as the module, header form — only relevant when porting code outside this project. |
| `#include <raylib.h>` | **Forbidden.** If you find existing files doing this, migrate them when next touched. |

If `import raylib;` doesn't expose what you need, the two escape hatches in order of preference:

1. `rl::Symbol(...)` — every raylib `RLAPI` is mirrored in the `rl::` namespace. Cheapest fix when the call site is the only consumer (e.g. `rl::GetScreenWidth()`).
2. Add a `using ck::Symbol;` (or `using ::DrawXxx;`) export to `deps/raylib-hpp/src/raylib.cppm` upstream. Preferred when the symbol is broadly useful and should join the curated surface.

## Namespaces — what lives where

| Namespace | Contains | Example |
|---|---|---|
| `::` (global) | Math/POD types from raylib C headers | `Vector2`, `Color`, `Rectangle`, `Camera2D`, `Image`, `Ray` |
| `ck::raii::` | RAII resource handles | `Window`, `Texture`, `Font`, `Shader`, `Image`, `RenderTexture`, `Mesh`, `Model`, `Sound`, `Music`, `Wave`, `AudioStream`, `AudioDevice`, `Material`, `ModelAnimations` |
| `ck::` | Scope guards, free functions, color constants, enum aliases | `ck::Drawing`, `ck::RED`, `ck::IsKeyDown`, `ck::DrawText`, `ck::KEY_W` (via `using enum`) |
| `rl::` | Every raylib `RLAPI` C function, unmodified | `rl::DrawRectangle`, `rl::LoadTexture` — escape hatch when `ck::` doesn't expose what you need |

### Inside `namespace ck` — drop the prefix

The project lives in `namespace ck`. Inside a TU that's wrapped in `namespace ck { ... }`, **don't repeat the `ck::` prefix** — write the nested namespace directly:

```cpp
namespace ck {

void MainMenuScene::OnRender() {
  // YES — inside namespace ck:
  const int saved = gui::GetStyle(DEFAULT, TEXT_SIZE);
  log::Info("hello");
  const Vector2 m = GetMousePosition();           // ck::GetMousePosition via using

  // NO — redundant prefix:
  // const int saved = ck::gui::GetStyle(...);
  // ck::log::Info("hello");
}

}  // namespace ck
```

The `ck::` qualification is appropriate only in TUs that aren't themselves inside `namespace ck` (e.g. `main.cpp`, or external code). Inside, the prefix is noise.

For files that aren't wrapped in `namespace ck` and don't want to repeat `ck::` everywhere, `using namespace ck;` + `using namespace ck::raii;` at the top is the alternative (see `src/main.cpp`).

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

**Consequence:** use `ck::RED`, not `RED`. After `using namespace ck;` both read as `RED`. Since `#include <raylib.h>` is forbidden in this project, the macros never re-leak — but be aware of the trap if you ever copy code from outside.

The same fix is applied to `KEY_*` and `MOUSE_BUTTON_*` (via `using enum ::KeyboardKey` / `using enum ::MouseButton` in the module).

## Text — use `ck::DrawText`, not `::DrawText`

`ck::DrawText` / `ck::MeasureText` (defined in `font_default.hpp`) route through a registered default font when one is set via `ck::SetDefaultFont`. `main.cpp` registers NotoSansSC at startup, so anything calling `ck::DrawText` gets Chinese glyphs for free. Calling `::DrawText` bypasses this and renders the raylib builtin (ASCII only).

If you find any file still using bare `::DrawText` (legacy `#include <raylib.h>` survivors), migrate to `ck::DrawText` (or just `DrawText` from inside `namespace ck`) when next touched.

## Adding a symbol that's missing from `import raylib;`

The module re-exports a **curated subset**. If `import raylib;` doesn't see
the symbol you need:

1. Check `deps/raylib-hpp/src/raylib.cppm` — is there a `using` declaration
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

1. New header in `deps/raylib-hpp/include/raylib-hpp/foo.hpp`. Include
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
// Inside a Scene::OnRender (which lives in namespace ck) — Application has
// already opened BeginDrawing via Application::Run, so DO NOT add another
// Drawing here.
namespace ck {
void GameplayScene::OnRender() {
  rl::DrawRectangle(x, y, w, h, RED);   // rl:: for symbols not in ck::; RED via ck::
  DrawText("hello", 10, 10, 24, BLACK); // ck::DrawText resolves to default-font aware
}
}
```

```cpp
// Camera2D
Camera2D cam{.offset = {w/2.f, h/2.f}, .target = player_pos, .zoom = 1.0f};
{
  Mode2D mode{cam};  // ck::Mode2D, prefix dropped because we're in namespace ck
  // world-space draws here
}
```

## Don't

- **Don't `#include <raylib.h>`.** Forbidden project-wide. Use `import raylib;` plus `rl::` for un-exported symbols.
- **Don't write `ck::xxx` inside `namespace ck { }`.** Drop the prefix — `gui::SetStyle`, `log::Info`, `raii::Texture`. See "Inside `namespace ck`" above.
- Don't wrap `::DrawRectangle` / `::DrawCircle` / etc. just to add a `ck::` prefix. They're already re-exported under `ck::` via the module. Wrap only when adding value.
- Don't move a `ck::raii::Window`. It's deleted, and the deletion is load-bearing.
- Don't call `Reset(handle.Get())` expecting it to be a no-op via reference equality — `Resource::Reset` byte-compares the POD handle to guard against this.
- Don't store a raw `::Texture2D` (or other handle) and let it outlive its `ck::raii::` owner. Always pass `tex.Get()` at the use site.
