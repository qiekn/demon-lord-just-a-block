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

  // raygui gets its own atlas: fusion.ttf is a pixel font, so we bake it at
  // the actual rendered size (kFontBody) for 1:1 sampling in DrawTextEx, and
  // use POINT filter to preserve sharp pixel edges. No mipmaps — they would
  // smooth out exactly the pixel quality we want to keep. 18 logical px also
  // happens to divide cleanly into 1x / 1.5x / 2x HIGHDPI framebuffer sizes,
  // so the projection stage doesn't introduce subpixel sampling either.
  Font fusion(CK_ASSET("fonts/fusion.ttf"), ck::ui::kFontBody, codepoints);
  SetTextureFilter(fusion.Get().texture, TEXTURE_FILTER_POINT);
  ck::gui::SetFont(fusion.Get());

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
