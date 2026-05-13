import std;
import raylib;
import raygui;

#include "application.hpp"
#include "assets.hpp"
#include "cursor_layer.hpp"
#include "font_spec.hpp"
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
  Font noto(CK_ASSET("fonts/NotoSansSC-Regular.ttf"), ck::ui::kFontAtlasBake, codepoints);

  // Mipmaps + trilinear keep glyphs crisp when raygui / ck::DrawText render
  // at sizes smaller than the atlas (e.g. body 18 sampled from a 64-px bake).
  // GenTextureMipmaps mutates the GL texture by id, so even though we pass a
  // local copy of ::Texture2D, the GL state is the one that persists — and
  // SetTextureFilter on the same copy then sees mipmaps>1 and configures the
  // GL filter accordingly.
  auto font_tex = noto.Get().texture;
  GenTextureMipmaps(&font_tex);
  SetTextureFilter(font_tex, TEXTURE_FILTER_TRILINEAR);

  SetDefaultFont(noto);
  // raygui keeps its built-in raylib default font (pixel ASCII) — its
  // controls render English labels in that retro style. ck::DrawText goes
  // through SetDefaultFont above for any Chinese the rest of the game needs.
  ck::gui::SetStyle(DEFAULT, TEXT_SIZE, ck::ui::kFontBody);
  ck::gui::SetStyle(DEFAULT, TEXT_SPACING, 1);
  ck::gui::SetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);

  app.PushOverlay(new ck::ImGuiLayer);
  app.PushLayer(new ck::TileEditorLayer);
  app.PushLayer(new ck::GameLayer);
  app.PushOverlay(new ck::MainMenuLayer(&app));
  app.PushOverlay(new ck::CursorLayer);

  app.Run();

  ClearDefaultFont();
  return 0;
}
