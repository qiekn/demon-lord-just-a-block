#pragma once

#include "layer.hpp"

// Std-free header. State (Grid + Player + ImGui flag) lives behind PImpl so
// main.cpp's `import std;` chain stays clean.

namespace ck {

class GameLayer : public Layer {
 public:
  GameLayer();
  ~GameLayer() override;

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
