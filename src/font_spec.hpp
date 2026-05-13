#pragma once

// Project-wide text-size convention.
//
// All sizes are expressed in **logical** pixels (raylib's coordinate space
// with FLAG_WINDOW_HIGHDPI). raylib's HIGHDPI projection scales them to the
// framebuffer automatically — 18 logical px renders at `18 * dpi` framebuffer
// px, so the same numbers stay visually consistent across DPI scales.
//
// kFontAtlasBake is the per-glyph atlas size in pixels. Pick it so the
// largest text size we draw, multiplied by the highest DPI we support, fits
// without upscaling. 64 covers kFontTitle (32) at 2x DPI; smaller sizes
// downscale and are kept crisp by enabling mipmaps + trilinear filtering on
// the atlas texture (see main.cpp).

namespace ck::ui {

constexpr int kFontBody = 18;
constexpr int kFontLabel = 22;
constexpr int kFontTitle = 32;

constexpr int kFontAtlasBake = 64;

}  // namespace ck::ui
