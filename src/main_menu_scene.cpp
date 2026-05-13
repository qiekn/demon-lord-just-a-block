#include "main_menu_scene.hpp"

#include <raylib.h>

#include "raygui-hpp/raygui.hpp"

#include "font_spec.hpp"
#include "gameplay_scene.hpp"
#include "log.hpp"
#include "scene_manager.hpp"

namespace ck {

void MainMenuScene::OnRender() {
  const float w = static_cast<float>(::GetScreenWidth());
  const float h = static_cast<float>(::GetScreenHeight());

  // raygui style is global; each layer that draws controls is responsible for
  // asserting the style it expects. Save / restore the bits we change so
  // other raygui consumers (or the next frame's default) aren't disturbed.
  const int saved_text_size = ck::gui::GetStyle(DEFAULT, TEXT_SIZE);
  ck::gui::SetStyle(DEFAULT, TEXT_SIZE, ck::ui::kFontTitle);

  // Centered vertical stack of 5 buttons.
  constexpr float kBtnW = 320.0f;
  constexpr float kBtnH = 64.0f;
  constexpr float kGap = 18.0f;
  constexpr int kCount = 5;
  const float total_h = kCount * kBtnH + (kCount - 1) * kGap;
  const float x = (w - kBtnW) * 0.5f;
  float y = (h - total_h) * 0.5f;

  auto next_rect = [&]() {
    ::Rectangle r{x, y, kBtnW, kBtnH};
    y += kBtnH + kGap;
    return r;
  };

  if (ck::gui::Button(next_rect(), "开始游戏")) {
    log::Info("MainMenu: start game");
    Manager()->Switch<GameplayScene>();
  }
  if (ck::gui::Button(next_rect(), "设置")) {
    log::Info("MainMenu: settings (TODO)");
  }
  if (ck::gui::Button(next_rect(), "成就")) {
    log::Info("MainMenu: achievements (TODO)");
  }
  if (ck::gui::Button(next_rect(), "模组")) {
    log::Info("MainMenu: mods (TODO)");
  }
  if (ck::gui::Button(next_rect(), "退出游戏")) {
    log::Info("MainMenu: exit");
    Manager()->RequestQuit();
  }

  ck::gui::SetStyle(DEFAULT, TEXT_SIZE, saved_text_size);
}

}  // namespace ck
