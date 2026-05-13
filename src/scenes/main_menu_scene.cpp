#include "main_menu_scene.hpp"

import raylib;

#include "raygui-hpp/raygui.hpp"

#include "font_spec.hpp"
#include "gameplay_scene.hpp"
#include "log.hpp"
#include "scene_manager.hpp"

namespace ck {

void MainMenuScene::OnRender() {
  const float w = static_cast<float>(rl::GetScreenWidth());
  const float h = static_cast<float>(rl::GetScreenHeight());

  // raygui style is global; each layer that draws controls is responsible for
  // asserting the style it expects. Save / restore the bits we change so
  // other raygui consumers (or the next frame's default) aren't disturbed.
  const int saved_text_size = gui::GetStyle(DEFAULT, TEXT_SIZE);
  gui::SetStyle(DEFAULT, TEXT_SIZE, ui::kFontTitle);

  // Centered vertical stack of 5 buttons.
  constexpr float kBtnW = 320.0f;
  constexpr float kBtnH = 64.0f;
  constexpr float kGap = 18.0f;
  constexpr int kCount = 5;
  const float total_h = kCount * kBtnH + (kCount - 1) * kGap;
  const float x = (w - kBtnW) * 0.5f;
  float y = (h - total_h) * 0.5f;

  // Keyboard navigation. Wrap around on either end.
  if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) {
    focused_index_ = (focused_index_ + 1) % kCount;
  }
  if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) {
    focused_index_ = (focused_index_ - 1 + kCount) % kCount;
  }
  const bool kbd_activate = IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER);
  const Vector2 mouse = GetMousePosition();

  int idx = 0;
  auto button = [&](const char* label) -> bool {
    Rectangle r{x, y, kBtnW, kBtnH};
    y += kBtnH + kGap;

    // Mouse hover steals focus so the two input modes can't disagree visually.
    if (CheckCollisionPointRec(mouse, r)) focused_index_ = idx;

    const int saved_state = gui::GetState();
    if (idx == focused_index_) gui::SetState(STATE_FOCUSED);
    const bool clicked = gui::Button(r, label);
    gui::SetState(saved_state);

    const bool activated = clicked || (idx == focused_index_ && kbd_activate);
    ++idx;
    return activated;
  };

  if (button("开始游戏")) {
    log::Info("MainMenu: start game");
    Manager()->Switch<GameplayScene>();
  }
  if (button("设置")) {
    log::Info("MainMenu: settings (TODO)");
  }
  if (button("成就")) {
    log::Info("MainMenu: achievements (TODO)");
  }
  if (button("模组")) {
    log::Info("MainMenu: mods (TODO)");
  }
  if (button("退出游戏")) {
    log::Info("MainMenu: exit");
    Manager()->RequestQuit();
  }

  gui::SetStyle(DEFAULT, TEXT_SIZE, saved_text_size);
}

}  // namespace ck
