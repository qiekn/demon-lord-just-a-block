---
name: raygui-hpp
description: How to use the Raygui-Hpp C++23 wrapper that this project bundles at deps/raygui-hpp. Covers import paths, the ck::gui:: namespace surface, control return values, what raygui internals are NOT reachable through the module, the bitmap-font HiDPI workaround, and which raygui macros/enums leak through which path.
---

# Raygui-Hpp Usage Guide

This project does not call raygui's C API directly when a wrapper exists. It uses the bundled wrapper at `deps/raygui-hpp/` — a thin set of inline headers in `include/raygui-hpp/*.hpp` plus the C++23 module `import raygui;`. The actual `raygui.h` implementation is compiled once into a static lib (`raygui_impl`) so your TU does **not** define `RAYGUI_IMPLEMENTATION`.

The wrapper's own architecture lives at `deps/raygui-hpp/CLAUDE.md`. This file is the **consumer** guide. The sibling skill `raylib-hpp` covers the raylib side and is a prerequisite — most raygui examples touch both.

## How to bring raygui in

Three entry points, **one per TU** (don't mix):

| Path | When | Notes |
|---|---|---|
| `import raygui;` | Default. Use alongside `import raylib;`. | Exports the `ck::gui::*` thin wrappers + raygui POD types/enums (`GuiStyleProp`, `GuiState`, `GuiControl`, `GuiIconName`, …). Does **not** export raw `::GuiXxx` C functions or the `GuiPropertyElement` enum (BORDER/BASE/TEXT/OTHER). |
| `#include "raygui-hpp/raygui.hpp"` | Header form of the same surface | Pulls every `raygui-hpp/*.hpp`. Use in TUs that aren't using `import std;` for some reason. |
| `#include "raygui-hpp/raygui_c.hpp"` | Need a raw C raygui function (`::GuiSetStyle`, `::GuiGetStyle`, `::GuiLoadStyle`, `::GuiLoadStyleDefault`) | Coexists with `import raygui;` — raygui.h has include guards. Brings the C declarations into the current TU. |

The examples under `deps/raygui-hpp/examples/` show the common shape:

```cpp
import std;
import raylib;
import raygui;

#include "raygui-hpp/raygui_c.hpp"  // ::GuiSetStyle etc., when needed
#include "raylib_c.hpp"             // raylib C decls not in import raylib; (see raylib-hpp skill)
```

## What's in `ck::gui::` — the consumer surface

Thin wrappers, one per raygui control. State-bearing controls return `int` (raygui's event code); click-style controls (`Button`, `LabelButton`) are converted to `bool` so `if (ck::gui::Button(…))` reads naturally.

| Topic header | Controls / functions |
|---|---|
| `containers.hpp` | `WindowBox`, `GroupBox`, `Line`, `Panel`, `ScrollPanel`, `TabBar` |
| `controls.hpp` | `Label`, `Button`, `LabelButton`, `Toggle`, `ToggleGroup`, `ToggleSlider`, `CheckBox`, `ComboBox`, `DropdownBox`, `Spinner`, `ValueBox`, `ValueBoxFloat`, `TextBox`, `Slider`, `SliderBar`, `ProgressBar`, `StatusBar`, `DummyRec`, `Grid` |
| `advanced.hpp` | `ListView`, `ListViewEx`, `MessageBox`, `TextInputBox`, `ColorPicker`, `ColorPanel`, `ColorBarAlpha`, `ColorBarHue`, `ColorPickerHSV`, `ColorPanelHSV` |
| `state.hpp` | `Enable`, `Disable`, `Lock`, `Unlock`, `IsLocked`, `SetAlpha`, `SetState`, `GetState` |
| `style.hpp` | `LoadStyle`, `LoadStyleDefault`, `GetStyle`, `SetStyle`, `kScrollbarLeftSide`, `kScrollbarRightSide` |
| `font.hpp` | `SetFont`, `GetFont` |
| `icons.hpp` | `IconText`, `DrawIcon`, `SetIconScale`, `GetIcons`, `LoadIcons` |
| `tooltip.hpp` | `SetTooltip`, `EnableTooltip`, `DisableTooltip` |
| `text.hpp` | `GetTextWidth` |

All take `::Rectangle` (raylib's POD) for bounds, `const char*` for text, and pointers for in/out state. The wrapper does **not** take `std::string` overloads — callers pass `.c_str()`. The `text` buffers passed to `TextBox` / `ValueBoxFloat` are mutated in place by raygui, so use `std::array<char, N>` and pass `.data()`.

## Common per-call patterns

```cpp
::Rectangle r{10, 10, 100, 25};

// Click button
if (ck::gui::Button(r, "Save")) { save(); }

// Text box — raygui mutates the buffer
std::array<char, 64> buf{};
std::ranges::copy(std::string_view{"initial"}, buf.begin());
bool edit_mode = false;
if (ck::gui::TextBox(r, buf.data(), 64, edit_mode)) edit_mode = !edit_mode;

// Slider — value passed by pointer
float v = 0.5f;
ck::gui::Slider(r, "lo", "hi", &v, 0.0f, 1.0f);

// ListViewEx wants char** even though it only reads. const_cast through
// an array of string literals is the standard escape:
const char* items[3] = {"one", "two", "three"};
int scroll = 0, active = -1, focus = -1;
ck::gui::ListViewEx(r, const_cast<char**>(items), 3, &scroll, &active, &focus);
```

## Controls return values

Most raygui controls return `int`, where 0 = "no event this frame" and any non-zero value carries control-specific info:

- `Button`, `LabelButton` — wrapper converts to `bool` (`pressed-this-frame`)
- `WindowBox` — returns `1` when the close button is pressed
- `Spinner`, `ValueBox`, `TextBox`, `DropdownBox` — return `1` when the user finishes editing (commit on Enter / click-outside); standard pattern is `if (...) edit_mode = !edit_mode;`
- `MessageBox`, `TextInputBox` — return `1` / `2` for the buttons (Yes/No, Ok/Cancel), `0` for "still open"

When in doubt, search the matching `::GuiXxx` in `deps/raygui-hpp/raygui.h` — the return-value semantics are documented at each declaration.

## Calling raw raygui C functions

Several raygui APIs are intentionally not wrapped — they're configuration sinks that don't benefit from C++ ergonomics:

```cpp
#include "raygui-hpp/raygui_c.hpp"

::GuiSetStyle(LISTVIEW, LIST_ITEMS_HEIGHT, 24);
::GuiSetStyle(LISTVIEW, SCROLLBAR_WIDTH, 12);
const int prev = ::GuiGetStyle(SCROLLBAR, BORDER_WIDTH);
::GuiLoadStyleDefault();
::GuiLoadStyle("path/to.rgs");
```

The style header functions exported by rGuiStyler (`GuiLoadStyleDark()`, `GuiLoadStyleJungle()`, …) are also raw C symbols brought into scope by `#include "../styles/style_<name>.h"`. See **Style headers** below.

## What you canNOT reach through `import raygui;`

raygui keeps a lot of state inside its implementation TU (`raygui_impl.c`). None of these are visible from a consuming TU — neither through the module nor through `#include "raygui-hpp/raygui_c.hpp"`:

- `guiState`, `guiAlpha`, `guiLocked`, `guiControlExclusiveMode`, `guiFont` (the variable; use `::GuiGetFont()` to read it)
- `GuiDrawRectangle`, `GuiDrawText`, `GetTextBounds` (internal draw helpers)
- The `RAYGUI_MESSAGEBOX_BUTTON_*`, `RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT` and similar macros defined inside `#if defined(RAYGUI_IMPLEMENTATION)`
- The `RAYGUI_MALLOC` / `RAYGUI_CALLOC` / `RAYGUI_FREE` macros (same reason)

**Symptom:** the C source you're trying to port reaches into one of these. **Resolution:**

| Inaccessible | Reach for instead |
|---|---|
| `guiAlpha` | Track a local `float alpha = 1.0f;` and pass `::Fade(color, alpha)` |
| `guiLocked` | `ck::gui::IsLocked()` |
| `guiState` | `ck::gui::GetState()` |
| `GuiDrawRectangle(rect, border, border_color, base_color)` | `::DrawRectangleRec(rect, base_color)` + `::DrawRectangleLinesEx(rect, border, border_color)` |
| `GuiDrawText` | `::DrawText` / `::DrawTextEx` with `ck::gui::GetFont()` |
| `GetTextBounds` | Manual rect calc using `ck::gui::GetTextWidth` + `::GuiGetStyle(DEFAULT, TEXT_SIZE)` |
| `RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT` | Hard-code `24` (its value) with a comment |
| `RAYGUI_MALLOC` (needed by style headers, see below) | `#define RAYGUI_MALLOC(sz) malloc(sz)` etc. locally before including the style header |

The "custom_sliders" and "property_list" example ports under `deps/raygui-hpp/examples/` are the canonical references for these workarounds.

## What you canNOT reach because the second `#include` is a no-op

raygui.h has include guards. When `import raygui;` runs, the module's internal TU `#include`s raygui.h once. A subsequent `#include "raygui-hpp/raygui_c.hpp"` in your TU goes through raygui.h's include guard and pulls in **nothing**. Therefore:

- Enumerators of `GuiPropertyElement` (`BORDER = 0`, `BASE = 1`, `TEXT = 2`, `OTHER = 3`) are NOT visible in your TU.
- Same for any other type/macro that `raygui.cppm` does not `using enum` re-export.

**Resolution:** declare local `constexpr int kPropBorder = 0;` etc. or `using enum ::GuiPropertyElement;` after the include (if the enum type is exported by the module — verify in `raygui.cppm`).

## Style headers (`examples/styles/style_*.h`)

These are rGuiStyler v2.0 generator output — do not hand-edit. Each declares a `static void GuiLoadStyleXxx(void)` that calls `GuiSetStyle` repeatedly and (for the few styles with a custom font) does `RAYGUI_MALLOC` + `memcpy` to set up `guiFont.glyphs` / `guiFont.recs`.

To consume them from a `.cpp` example:

```cpp
#include "raygui-hpp/raygui_c.hpp"   // brings raygui.h C decls into scope

// Style headers expect malloc/memcpy/RAYGUI_MALLOC in scope but raygui.h only
// emits those inside its RAYGUI_IMPLEMENTATION block (which lives in
// raygui_impl.c, not here). Re-emit them locally.
#include <stdlib.h>
#include <string.h>
#ifndef RAYGUI_MALLOC
#define RAYGUI_MALLOC(sz)    malloc(sz)
#define RAYGUI_CALLOC(n, sz) calloc(n, sz)
#define RAYGUI_FREE(p)       free(p)
#endif

#include "../styles/style_dark.h"
#include "../styles/style_cyber.h"
// ...

// Then later:
GuiLoadStyleDark();
```

The `<stdlib.h>` / `<string.h>` are C headers, not the libc++ `<cstdlib>` / `<cstring>` wrappers — they coexist with `import std;` without the "template alias redefinition" problem that the heavier std headers cause.

## HiDPI — bitmap font scaling

raygui ships an embedded bitmap font baked at 10 px. With `FLAG_WINDOW_HIGHDPI` on (required on 4K displays so the UI isn't tiny — see the user-scope memory `window_hidpi_required.md`), raylib renders into a framebuffer at physical resolution and projects from logical coords; the projection's default bilinear filter turns the 10-px atlas into a blurry mess.

`deps/raygui-hpp/examples/common/example_setup.hpp` provides the standard fix as `ck::gui_demo::SetupHiDpi()`. It:

1. Calls `::SetMouseScale(1/dpi.x, 1/dpi.y)` so mouse coords (returned physical) match the logical drawing space (raygui's hit-testing needs this).
2. Generates mipmaps + sets `TEXTURE_FILTER_TRILINEAR` on `::GuiGetFont().texture` so the upsample is no longer pure bilinear.
3. Bumps `::GuiSetStyle(DEFAULT, TEXT_SIZE, …)` to a readable logical size (18 px, matching the main `block` app).

Use it from every example:

```cpp
ck::raii::Window window{w, h, "title", ck::FLAG_WINDOW_HIGHDPI};
ck::gui_demo::SetupHiDpi();  // first line after Window construction
```

For the main `block` app and any new app that needs a sharper font than the trilinear-filtered bitmap, follow `src/main.cpp` instead: load a TTF (e.g. NotoSansSC) at a large bake size + mipmaps + trilinear, then `ck::gui::SetFont(...)`.

## Adding a symbol that's missing from `import raygui;`

The module re-exports a curated subset. If `import raygui;` doesn't see the symbol you need:

1. Check `deps/raygui-hpp/src/raygui.cppm` — is the symbol listed in a `using` declaration?
2. If not, find it in the matching `include/raygui-hpp/*.hpp` (or add a thin wrapper there following the pattern of the existing ones — `inline X Foo(...)` calling `::GuiFoo(...)`).
3. Add `using ck::gui::Foo;` in the `ck::gui::` block at the bottom of `raygui.cppm`.
4. Rebuild.

Don't wrap a raygui C function just to add a `ck::gui::` prefix — only wrap when adding value (bool conversion, default-arg ergonomics, etc.). The escape hatch for any unwrapped raygui C function is `::GuiXxx(...)` via `#include "raygui-hpp/raygui_c.hpp"`.

## Quick reference — minimal app

```cpp
import std;
import raylib;
import raygui;

#include "example_setup.hpp"   // examples-only; bundle equivalent into your own app

int main() {
  ck::raii::Window window{800, 450, "demo", ck::FLAG_WINDOW_HIGHDPI};
  ck::gui_demo::SetupHiDpi();

  std::array<char, 64> name{};
  std::ranges::copy(std::string_view{"world"}, name.begin());
  bool editing = false;
  int clicks = 0;

  ck::SetTargetFPS(60);
  while (!window.ShouldClose()) {
    ck::Drawing draw;
    ck::ClearBackground(ck::RAYWHITE);

    if (ck::gui::TextBox({10, 10, 200, 28}, name.data(), 64, editing)) {
      editing = !editing;
    }
    if (ck::gui::Button({10, 50, 100, 28}, "Hello")) ++clicks;

    const auto label = std::format("hello {} ({} clicks)", name.data(), clicks);
    ck::DrawText(label.c_str(), 10, 90, 18, ck::DARKGRAY);
  }
  return 0;
}
```

## Don't

- Don't `#define RAYGUI_IMPLEMENTATION` in your TU. The implementation lives in `raygui_impl` (a static lib). Defining it again will produce duplicate symbols at link time.
- Don't `#include "raygui.h"` directly — go through `"raygui-hpp/raygui_c.hpp"` so the color-macro `#undef`s run (otherwise `RED` / `BLACK` etc. shadow the `ck::` constants).
- Don't reach for `guiAlpha`, `guiState`, `guiLocked`, `GuiDrawRectangle`, `GuiDrawText`, `GetTextBounds` — they are not visible. Use the public API substitutes table above.
- Don't expect `GuiPropertyElement` enumerators (`BORDER`, `TEXT`) to be in scope after `import raygui;`. They're not re-exported. Hard-code or `using enum`.
- Don't wrap a raygui call (`::GuiXxx`) in `extern "C"` — raygui.h already does that inside `raygui_c.hpp`. Stray `extern "C"` blocks at the call site are harmless but noise.
- Don't bake a TTF font once and assume raygui will pick it up automatically. raygui owns its own `guiFont` separately from raylib's default font — call `ck::gui::SetFont(...)` explicitly.
