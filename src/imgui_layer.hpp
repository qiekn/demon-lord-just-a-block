#pragma once

#include "layer.hpp"

namespace block {

// Overlay that owns the ImGui context + glfw/opengl3 backends. Brackets each
// frame via OnImGuiBegin/End so other layers can drop ImGui windows in their
// OnImGuiRender override.
class ImGuiLayer : public Layer {
 public:
  ImGuiLayer() : Layer("ImGuiLayer") {}
  ~ImGuiLayer() override = default;

  void OnAttach() override;
  void OnDetach() override;
  void OnImGuiBegin() override;
  void OnImGuiEnd() override;
};

}  // namespace block
