#include "game_layer.hpp"

#include <imgui.h>
#include <raylib.h>

namespace block {

void GameLayer::OnRender() {
  const char* title = "Block · 方块";
  const int font_size = 48;
  const int tw = MeasureText(title, font_size);
  DrawText(title, (GetScreenWidth() - tw) / 2, (GetScreenHeight() - font_size) / 2, font_size,
           BLACK);
}

void GameLayer::OnImGuiRender() {
  if (show_demo_) ImGui::ShowDemoWindow(&show_demo_);
}

}  // namespace block
