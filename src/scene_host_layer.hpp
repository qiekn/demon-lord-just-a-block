#pragma once

// Std-free header. Owns a SceneManager via PImpl so callers that
// `import std;` can include this without dragging <memory>+<vector> into
// the double-decl trap.

#include "layer.hpp"

namespace ck {

class Application;

class SceneHostLayer : public Layer {
 public:
  explicit SceneHostLayer(Application* app);
  ~SceneHostLayer() override;

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
