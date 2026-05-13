#pragma once

#include "scene.hpp"

namespace ck {

// Owns the playable grid + player + tile editor. ESC switches back to
// MainMenuScene. Resources (player textures, tile textures) load on OnEnter
// and unload on OnExit so the scene transition has a clean lifecycle.
class GameplayScene : public Scene {
 public:
  GameplayScene() : Scene("Gameplay") {}
  ~GameplayScene() override;

  void OnEnter() override;
  void OnExit() override;
  void OnUpdate(float dt) override;
  void OnRender() override;
  void OnImGuiRender() override;

 private:
  struct State;
  State* state_ = nullptr;
};

}  // namespace ck
