#pragma once

// Custom mouse cursor. Hides the OS cursor on attach, draws the mouseUI sprite
// on top of all ImGui windows via the foreground draw list.

#include "layer.hpp"

namespace ck {

class CursorLayer : public Layer {
 public:
  CursorLayer() : Layer("CursorLayer") {}

  void OnAttach() override;
  void OnDetach() override;
  void OnImGuiRender() override;

 private:
  struct State;
  State* state_ = nullptr;
};

}  // namespace ck
