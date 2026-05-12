import std;
import raylib;

using namespace ck;
using namespace ck::raii;

namespace {
std::string ReadFile(const std::string& path) {
  std::ifstream file{path};
  return std::string{std::istreambuf_iterator<char>{file}, {}};
}
}  // namespace

int main() {
  // HiDPI must be set before Window construction.
  SetConfigFlags(FLAG_WINDOW_HIGHDPI | FLAG_WINDOW_RESIZABLE);

  Window window(1280, 720, "Block");
  SetTargetFPS(60);

  // Bake glyphs at 64px so text stays sharp at HiDPI render scale.
  auto codepoints = LoadCodepoints(ReadFile("assets/zh-sc-3500.txt"));
  Font noto("assets/fonts/NotoSansSC-Regular.ttf", 64, codepoints);
  SetTextureFilter(noto.Get().texture, TEXTURE_FILTER_BILINEAR);
  SetDefaultFont(noto);

  const std::string title = "Block · 方块";

  while (!window.ShouldClose()) {
    if (IsKeyPressed(KEY_ESCAPE)) break;

    Drawing draw;
    ClearBackground(RAYWHITE);

    int tw = MeasureText(title, 48);
    DrawText(title, (window.GetScreenWidth() - tw) / 2,
             (window.GetScreenHeight() - 48) / 2, 48, BLACK);
  }

  ClearDefaultFont();
  return 0;
}
