#pragma once

// Sample-the-neighbors text outliner. Renders text onto an internal
// RenderTexture, then draws it through assets/shaders/outline.fs to emit a
// solid outline of the requested color and radius. Replaces the "draw text 8
// times at offsets" trick used for cheap pixel outlines.
//
// Lifetime: the shader + RT are GL resources, so a TextOutliner must be
// constructed AFTER raylib's InitWindow and destroyed BEFORE CloseWindow.
// Owning it inside a Layer's State (constructed in OnAttach, destructed in
// OnDetach) is the canonical placement.

#include "render_texture.hpp"
#include "shader.hpp"

namespace ck {

class TextOutliner {
 public:
  TextOutliner() = default;
  ~TextOutliner() = default;

  TextOutliner(const TextOutliner&) = delete;
  TextOutliner& operator=(const TextOutliner&) = delete;

  // Draw `text` at (x, y) — the top-left of the glyph box, matching the
  // ck::DrawText convention. Uses the registered ck::SetDefaultFont. The
  // outline extends `outline_px` pixels outward in every direction.
  void DrawText(const char* text, int x, int y, int font_size,
                ::Color text_color, ::Color outline_color, float outline_px);

 private:
  void EnsureLoaded();
  void EnsureCapacity(int min_w, int min_h);

  bool initialized_ = false;
  ::ck::raii::Shader shader_;
  ::ck::raii::RenderTexture rt_;

  int loc_texel_size_ = -1;
  int loc_outline_radius_ = -1;
  int loc_outline_color_ = -1;
};

}  // namespace ck
