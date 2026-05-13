#pragma once

#include "scene.hpp"

namespace ck {

// Boot scene. Centered vertical stack of raygui buttons; "开始游戏" switches
// to GameplayScene, "退出游戏" asks the manager to quit. Other items are
// no-op TODOs (matches the prior MainMenuLayer behaviour).
class MainMenuScene : public Scene {
 public:
  MainMenuScene() : Scene("MainMenu") {}

  void OnRender() override;

 private:
  int focused_index_ = 0;
};

}  // namespace ck
