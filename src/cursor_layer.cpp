#include "cursor_layer.hpp"

#include <imgui.h>

import raylib;

#include "assets.hpp"
#include "imgui_layer.hpp"
#include "log.hpp"

namespace ck {

struct CursorLayer::State {
  raii::Texture tex;
  bool enabled = true;
  float scale = 0.55f;
  // Track OS cursor visibility so we don't spam GLFW each frame.
  bool os_cursor_visible = true;
};

void CursorLayer::OnAttach() {
  state_ = new State{.tex = raii::Texture{CK_ASSET("sprites/cursor.png")}};
  if (state_->tex) {
    state_->tex.SetFilter(TEXTURE_FILTER_BILINEAR);
  } else {
    log::Warn("CursorLayer: failed to load assets/sprites/cursor.png");
  }
}

void CursorLayer::OnDetach() {
  if (!state_) return;
  if (!state_->os_cursor_visible) ShowCursor();
  delete state_;
  state_ = nullptr;
}

void CursorLayer::OnImGuiRender() {
  if (!state_) return;

  // The settings window is a normal panel — hide alongside the rest when
  // backtick toggle is off. The actual cursor draw (below) is unconditional
  // so the custom sprite keeps moving when imgui panels are hidden.
  if (ImGuiLayer::PanelsVisible()) {
    if (ImGui::Begin("Cursor")) {
      ImGui::Checkbox("Use custom cursor", &state_->enabled);
      ImGui::SliderFloat("Size", &state_->scale, 0.25f, 3.0f, "%.2fx");
      ImGui::TextDisabled("Hides the OS cursor everywhere (including over\nImGui panels) and draws the sprite instead.");
    }
    ImGui::End();
  }

  // When the custom cursor is on we need to stop imgui's glfw backend from
  // resetting GLFW_CURSOR back to NORMAL each frame (it does that whenever its
  // current cursor shape isn't ImGuiMouseCursor_None) — otherwise HideCursor()
  // gets undone every frame and the OS arrow keeps showing.
  ImGuiIO& io = ImGui::GetIO();
  if (state_->enabled) {
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
  } else {
    io.ConfigFlags &= ~ImGuiConfigFlags_NoMouseCursorChange;
  }

  const bool show_sprite = state_->enabled && state_->tex;
  const bool want_os_cursor = !show_sprite;

  if (want_os_cursor != state_->os_cursor_visible) {
    if (want_os_cursor) ShowCursor();
    else HideCursor();
    state_->os_cursor_visible = want_os_cursor;
  }

  if (show_sprite) {
    // ImGui foreground drawlist uses DisplaySize (framebuffer-pixel) space.
    // raylib's GetMousePosition is scaled to logical coords by Application,
    // so use ImGui's mouse pos here to stay in the same coord system.
    const ImVec2 m = io.MousePos;
    const float w = static_cast<float>(state_->tex.GetWidth()) * state_->scale;
    const float h = static_cast<float>(state_->tex.GetHeight()) * state_->scale;
    ImGui::GetForegroundDrawList()->AddImage(
        static_cast<ImTextureID>(state_->tex.Get().id),
        ImVec2(m.x, m.y), ImVec2(m.x + w, m.y + h));
  }
}

}  // namespace ck
