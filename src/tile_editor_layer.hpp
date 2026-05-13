#pragma once

// Background tile editor. Paints Card100..Card135 onto a 2D grid and renders
// it as the game's tile layer. PImpl + std-free header so callers can
// `import std;` next to this include.

#include "layer.hpp"

namespace ck {

class TileEditorLayer : public Layer {
 public:
  TileEditorLayer() : Layer("TileEditorLayer") {}

  void OnAttach() override;
  void OnDetach() override;
  void OnUpdate(float dt) override;
  void OnRender() override;
  void OnImGuiRender() override;

 private:
  struct State;
  State* state_ = nullptr;
};

}  // namespace ck
