#include "cursor_layer.hpp"

#include <imgui.h>
#include <raylib.h>

#include "assets.hpp"
#include "log.hpp"

namespace ck {

struct CursorLayer::State {
  Texture2D tex{};
  bool loaded = false;
};

void CursorLayer::OnAttach() {
  state_ = new State{};
  state_->tex = ::LoadTexture(CK_ASSET("sprites/cursor.png"));
  state_->loaded = state_->tex.id != 0;
  if (state_->loaded) {
    ::SetTextureFilter(state_->tex, TEXTURE_FILTER_BILINEAR);
    ::HideCursor();
  } else {
    log::Warn("CursorLayer: failed to load assets/sprites/cursor.png");
  }
}

void CursorLayer::OnDetach() {
  if (state_ && state_->loaded) {
    ::ShowCursor();
    ::UnloadTexture(state_->tex);
  }
  delete state_;
  state_ = nullptr;
}

void CursorLayer::OnImGuiRender() {
  if (!state_ || !state_->loaded) return;
  const Vector2 m = ::GetMousePosition();
  const float w = static_cast<float>(state_->tex.width);
  const float h = static_cast<float>(state_->tex.height);
  ImGui::GetForegroundDrawList()->AddImage(
      static_cast<ImTextureID>(state_->tex.id),
      ImVec2(m.x, m.y), ImVec2(m.x + w, m.y + h));
}

}  // namespace ck
