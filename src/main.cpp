import std;
import raylib;
import raygui;

#include "application.hpp"
#include "assets.hpp"
#include "cursor_layer.hpp"
#include "game_layer.hpp"
#include "imgui_layer.hpp"
#include "log.hpp"
#include "main_menu_layer.hpp"
#include "tile_editor_layer.hpp"

using namespace ck;
using namespace ck::raii;

namespace {

std::string ReadFile(const std::string& path) {
  std::ifstream file{path};
  return std::string{std::istreambuf_iterator<char>{file}, {}};
}

}  // namespace

int main() {
  ck::Log::Init();

  ck::Application app{{.name = "Block"}};

  // Font is created after Application's Window exists (GL context required)
  // and destructed before Application's destructor closes that Window. The
  // declaration order below + standard reverse-destruction order matches.
  auto codepoints = LoadCodepoints(ReadFile(CK_ASSET("zh-sc-3500.txt")));
  Font noto(CK_ASSET("fonts/NotoSansSC-Regular.ttf"), 64, codepoints);
  SetTextureFilter(noto.Get().texture, TEXTURE_FILTER_BILINEAR);
  SetDefaultFont(noto);
  // raygui owns its own font slot — register the same atlas so ck::gui::*
  // buttons render Chinese instead of falling back to raygui's ASCII default.
  ck::gui::SetFont(noto.Get());

  app.PushOverlay(new ck::ImGuiLayer);
  app.PushLayer(new ck::TileEditorLayer);
  app.PushLayer(new ck::GameLayer);
  app.PushOverlay(new ck::MainMenuLayer(&app));
  app.PushOverlay(new ck::CursorLayer);

  app.Run();

  ClearDefaultFont();
  return 0;
}
