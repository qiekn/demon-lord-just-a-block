#include "cursor_layer.hpp"

#include <imgui.h>
#include <raylib.h>

#include "assets.hpp"
#include "log.hpp"

namespace ck {

struct CursorLayer::State {
  Texture2D tex{};
  bool loaded = false;
  bool enabled = true;
  // Track OS cursor visibility so we don't spam GLFW each frame.
  bool os_cursor_visible = true;
};

void CursorLayer::OnAttach() {
  state_ = new State{};
  state_->tex = ::LoadTexture(CK_ASSET("sprites/cursor.png"));
  state_->loaded = state_->tex.id != 0;
  if (state_->loaded) {
    ::SetTextureFilter(state_->tex, TEXTURE_FILTER_BILINEAR);
  } else {
    log::Warn("CursorLayer: failed to load assets/sprites/cursor.png");
  }
}

void CursorLayer::OnDetach() {
  if (!state_) return;
  if (!state_->os_cursor_visible) ::ShowCursor();
  if (state_->loaded) ::UnloadTexture(state_->tex);
  delete state_;
  state_ = nullptr;
}

void CursorLayer::OnImGuiRender() {
  if (!state_) return;

  if (ImGui::Begin("Cursor")) {
    ImGui::Checkbox("Use custom cursor", &state_->enabled);
    ImGui::TextDisabled("Hides the OS cursor everywhere (including over\nImGui panels) and draws the sprite instead.");
  }
  ImGui::End();

  const bool show_sprite = state_->enabled && state_->loaded;
  const bool want_os_cursor = !show_sprite;

  if (want_os_cursor != state_->os_cursor_visible) {
    if (want_os_cursor) ::ShowCursor();
    else ::HideCursor();
    state_->os_cursor_visible = want_os_cursor;
  }

  if (show_sprite) {
    // ImGui foreground drawlist uses DisplaySize (framebuffer-pixel) space.
    // raylib's GetMousePosition is scaled to logical coords by Application,
    // so use ImGui's mouse pos here to stay in the same coord system.
    const ImVec2 m = ImGui::GetIO().MousePos;
    const float w = static_cast<float>(state_->tex.width);
    const float h = static_cast<float>(state_->tex.height);
    ImGui::GetForegroundDrawList()->AddImage(
        static_cast<ImTextureID>(state_->tex.id),
        ImVec2(m.x, m.y), ImVec2(m.x + w, m.y + h));
  }
}

}  // namespace ck
