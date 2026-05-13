#pragma once

#include "layer.hpp"

namespace ck {

// Overlay that owns the ImGui context + glfw/opengl3 backends. Brackets each
// frame via OnImGuiBegin/End so other layers can drop ImGui windows in their
// OnImGuiRender override.
//
// Backtick (`) toggles panel visibility (`PanelsVisible()`). When false the
// ImGui frame still starts/ends each tick — only panel-drawing is skipped —
// so the cursor layer's foreground draws stay live.
class ImGuiLayer : public Layer {
 public:
  ImGuiLayer() : Layer("ImGuiLayer") {}
  ~ImGuiLayer() override = default;

  void OnAttach() override;
  void OnDetach() override;
  void OnImGuiBegin() override;
  void OnImGuiRender() override;
  void OnImGuiEnd() override;

  static bool PanelsVisible();
};

}  // namespace ck
