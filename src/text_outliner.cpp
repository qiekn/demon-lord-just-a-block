#include "text_outliner.hpp"

#include <algorithm>
#include <cmath>

#include "colors.hpp"
#include "font_default.hpp"
#include "scope_guard.hpp"

#include "assets.hpp"
#include "log.hpp"

namespace ck {

void TextOutliner::EnsureLoaded() {
  if (initialized_) return;
  shader_ = ::ck::raii::Shader{"", CK_ASSET("shaders/outline.fs")};
  if (!shader_.Valid()) {
    log::Error("TextOutliner: failed to load outline shader");
    return;
  }
  loc_texel_size_ = shader_.GetLocation("uTexelSize");
  loc_outline_radius_ = shader_.GetLocation("uOutlineRadius");
  loc_outline_color_ = shader_.GetLocation("uOutlineColor");
  initialized_ = true;
}

void TextOutliner::EnsureCapacity(int min_w, int min_h) {
  if (rt_.Valid() && rt_.GetWidth() >= min_w && rt_.GetHeight() >= min_h) return;
  // Grow-only: round up to the next 64-px multiple so a few size bumps don't
  // thrash the GL allocator.
  const int target_w = std::max(min_w, rt_.Valid() ? rt_.GetWidth() : 0);
  const int target_h = std::max(min_h, rt_.Valid() ? rt_.GetHeight() : 0);
  const int rounded_w = ((target_w + 63) / 64) * 64;
  const int rounded_h = ((target_h + 63) / 64) * 64;
  rt_.Load(rounded_w, rounded_h);
  ::SetTextureFilter(rt_.GetTexture(), TEXTURE_FILTER_BILINEAR);
}

void TextOutliner::DrawText(const char* text, int x, int y, int font_size,
                            ::Color text_color, ::Color outline_color,
                            float outline_px) {
  EnsureLoaded();
  if (!initialized_) {
    // Shader failed to load — degrade to plain text so the HUD still works.
    ck::DrawText(text, x, y, font_size, text_color);
    return;
  }

  const int text_w = ck::MeasureText(text, font_size);
  const int text_h = font_size;
  const int pad = static_cast<int>(std::ceil(outline_px)) + 2;
  const int needed_w = text_w + 2 * pad;
  const int needed_h = text_h + 2 * pad;
  EnsureCapacity(needed_w, needed_h);

  {
    ck::TextureMode tm{rt_.Get()};
    ck::ClearBackground(::Color{0, 0, 0, 0});
    ck::DrawText(text, pad, pad, font_size, text_color);
  }

  const float texel_size[2] = {1.0f / static_cast<float>(rt_.GetWidth()),
                               1.0f / static_cast<float>(rt_.GetHeight())};
  const float outline_color_n[4] = {outline_color.r / 255.0f,
                                    outline_color.g / 255.0f,
                                    outline_color.b / 255.0f,
                                    outline_color.a / 255.0f};
  shader_.SetValue(loc_texel_size_, texel_size, SHADER_UNIFORM_VEC2);
  shader_.SetValue(loc_outline_radius_, &outline_px, SHADER_UNIFORM_FLOAT);
  shader_.SetValue(loc_outline_color_, outline_color_n, SHADER_UNIFORM_VEC4);

  // RT textures are stored bottom-up (GL convention), and BeginTextureMode
  // flips the projection so user code wrote text at the TOP of the
  // framebuffer — which lives at HIGH GL-y in the texture. The RT is grown
  // to 64-px multiples and is almost always larger than the drawn region, so
  // we must offset src.y by (rt_height - needed_h) to land on the rows where
  // the text actually was. Negative source height keeps the V flip.
  // The canonical raylib pattern `(0, 0, w, -h)` only works when h == rt_h.
  const ::Texture2D tex = rt_.GetTexture();
  const ::Rectangle src{0.0f,
                        static_cast<float>(rt_.GetHeight() - needed_h),
                        static_cast<float>(needed_w),
                        -static_cast<float>(needed_h)};
  const ::Vector2 dst{static_cast<float>(x - pad),
                      static_cast<float>(y - pad)};
  {
    ck::ShaderMode sm{shader_.Get()};
    ::DrawTextureRec(tex, src, dst, ck::WHITE);
  }
}

}  // namespace ck
