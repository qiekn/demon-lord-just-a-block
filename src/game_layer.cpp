#include "game_layer.hpp"

#include <imgui.h>

#include "grid.hpp"
#include "player.hpp"

namespace ck {

// Grid and Player are both RAII (textures unload in their destructors).
// State packages them by value so a single `delete state_` tears everything
// down — no per-resource new/delete in OnAttach/OnDetach.
struct GameLayer::State {
  Grid grid;
  Player player;
  bool show_demo = false;

  State() : grid(11, 7, 96.0f), player({grid.Cols() / 2, grid.Rows() / 2}) {}
};

GameLayer::GameLayer() : Layer("GameLayer") {}
GameLayer::~GameLayer() { delete state_; }

void GameLayer::OnAttach() { state_ = new State{}; }

void GameLayer::OnDetach() {
  delete state_;
  state_ = nullptr;
}

void GameLayer::OnUpdate(float dt) {
  if (state_) state_->player.Update(dt, state_->grid);
}

void GameLayer::OnRender() {
  if (!state_) return;
  state_->grid.Render();
  state_->player.Render(state_->grid);
}

void GameLayer::OnImGuiRender() {
  if (!state_) return;
  if (ImGui::Begin("Player tuning")) {
    auto& t = state_->player.tuning;
    ImGui::SliderFloat("Key repeat", &t.repeat_interval, 0.05f, 0.8f, "%.2f s");
    ImGui::SliderFloat("Sprite tween", &t.sprite_duration, 0.05f, 0.6f, "%.2f s");
    ImGui::SliderFloat("Block tween", &t.block_duration, 0.05f, 0.6f, "%.2f s");
    ImGui::SliderFloat("Hop height", &t.hop_height, 0.0f, 1.0f, "%.2f cells");
    ImGui::Checkbox("ImGui demo", &state_->show_demo);
  }
  ImGui::End();
  if (state_->show_demo) ImGui::ShowDemoWindow(&state_->show_demo);
}

}  // namespace ck
