#pragma once

#include "layer.hpp"

namespace block {

// Placeholder gameplay layer: renders the title text and an ImGui demo window
// so we can see something on-screen while the engine matures. Real gameplay
// will replace this.
class GameLayer : public Layer {
 public:
  GameLayer() : Layer("GameLayer") {}

  void OnRender() override;
  void OnImGuiRender() override;

 private:
  bool show_demo_ = true;
};

}  // namespace block
