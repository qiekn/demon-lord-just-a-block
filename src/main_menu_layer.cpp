#include "main_menu_layer.hpp"

#include <raylib.h>

#include "raygui-hpp/raygui.hpp"

#include "application.hpp"
#include "log.hpp"

namespace ck {

struct MainMenuLayer::State {
  Application* app = nullptr;
  bool visible = true;
};

MainMenuLayer::MainMenuLayer(Application* app)
    : Layer("MainMenuLayer"), state_(new State{.app = app}) {}

MainMenuLayer::~MainMenuLayer() { delete state_; }

void MainMenuLayer::OnAttach() {
  // Slightly larger control text than raygui's 10px default so Chinese
  // glyphs are legible against the NotoSansSC atlas (loaded at baseSize 64
  // by main.cpp). The font itself is registered via ck::gui::SetFont() in
  // main.cpp so every Gui control picks it up.
  ck::gui::SetStyle(DEFAULT, TEXT_SIZE, 28);
  ck::gui::SetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
}

void MainMenuLayer::OnDetach() {}

void MainMenuLayer::OnRender() {
  if (!state_->visible) return;

  const float w = static_cast<float>(::GetScreenWidth());
  const float h = static_cast<float>(::GetScreenHeight());

  // Dim whatever the gameplay layers drew underneath.
  ::DrawRectangle(0, 0, static_cast<int>(w), static_cast<int>(h), ::Color{0, 0, 0, 200});

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
    state_->visible = false;
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
    if (state_->app) state_->app->Close();
  }
}

bool MainMenuLayer::IsVisible() const { return state_ && state_->visible; }
void MainMenuLayer::Show() {
  if (state_) state_->visible = true;
}
void MainMenuLayer::Hide() {
  if (state_) state_->visible = false;
}

}  // namespace ck
