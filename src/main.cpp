import std;
import raylib;

#include "assets.hpp"
#include "imgui_layer.hpp"
#include "log.hpp"
#include "window_state.hpp"

using namespace ck;
using namespace ck::raii;

namespace {

constexpr const char* kWindowStateFile = "window.state";

std::string ReadFile(const std::string& path) {
  std::ifstream file{path};
  return std::string{std::istreambuf_iterator<char>{file}, {}};
}

}  // namespace

int main() {
  block::Log::Init();
  block::log::Info("Block starting");

  SetConfigFlags(FLAG_WINDOW_HIGHDPI | FLAG_WINDOW_RESIZABLE);

  const auto state = block::LoadWindowState(kWindowStateFile);
  Window window(state.w, state.h, "Block");
  rl::SetWindowPosition(state.x, state.y);
  SetTargetFPS(160);
  BLOCK_LOG_INFO("Window {}x{} @ ({}, {})", state.w, state.h, state.x, state.y);

  auto codepoints = LoadCodepoints(ReadFile(BLOCK_ASSET("zh-sc-3500.txt")));
  Font noto(BLOCK_ASSET("fonts/NotoSansSC-Regular.ttf"), 64, codepoints);
  SetTextureFilter(noto.Get().texture, TEXTURE_FILTER_BILINEAR);
  SetDefaultFont(noto);

  block::ImGuiLayer imgui;
  imgui.OnAttach();

  const std::string title = "Block · 方块";
  bool show_demo = true;

  while (!window.ShouldClose()) {
    if (IsKeyPressed(KEY_ESCAPE)) break;

    Drawing draw;
    ClearBackground(RAYWHITE);

    int tw = MeasureText(title, 48);
    DrawText(title, (window.GetScreenWidth() - tw) / 2, (window.GetScreenHeight() - 48) / 2, 48,
             BLACK);

    imgui.Begin();
    imgui.DrawDemo(&show_demo);
    imgui.End();
  }

  // Capture geometry before Window destructs and tears down the GL context.
  const ::Vector2 pos = rl::GetWindowPosition();
  block::SaveWindowState(kWindowStateFile,
                         {.x = static_cast<int>(pos.x),
                          .y = static_cast<int>(pos.y),
                          .w = window.GetScreenWidth(),
                          .h = window.GetScreenHeight()});
  block::log::Info("Block shutting down");

  imgui.OnDetach();
  ClearDefaultFont();
  return 0;
}
