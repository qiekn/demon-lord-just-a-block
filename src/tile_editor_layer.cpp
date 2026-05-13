#include "tile_editor_layer.hpp"

#include <algorithm>
#include <array>
#include <fstream>
#include <vector>

#include <imgui.h>
#include <raylib.h>

#include "assets.hpp"
#include "log.hpp"

namespace ck {

namespace {

// Match GameLayer's Grid(11, 7, 96.0f) so painted tiles align with the player.
constexpr int kDefaultCols = 11;
constexpr int kDefaultRows = 7;
constexpr int kDefaultTilePx = 96;
constexpr const char* kTileFile = "tilemap.txt";

// Floor brush palette. First two form the default checkerboard pattern; the
// rest are alternative grid sprites loaded from assets/sprites/grid/.
struct TileDef {
  const char* label;
  const char* path;
};

constexpr std::array<TileDef, 2> kTiles{{
    {"grass A", CK_ASSET("sprites/grid/grid #2327.png")},
    {"grass B", CK_ASSET("sprites/grid/grid2 #2366.png")},
}};

}  // namespace

struct TileEditorLayer::State {
  std::array<Texture2D, kTiles.size()> textures{};
  std::array<bool, kTiles.size()> loaded{};

  int cols = kDefaultCols;
  int rows = kDefaultRows;
  int tile_px = kDefaultTilePx;
  bool center = true;
  int origin_x = 0;
  int origin_y = 0;

  // Row-major: tiles[r * cols + c]. Values index kTiles[].
  std::vector<int> tiles;

  int brush_id = 0;
  bool show_grid = false;
  bool paint_mode = true;

  void Checkerboard() {
    tiles.assign(static_cast<size_t>(cols) * rows, 0);
    for (int r = 0; r < rows; ++r) {
      for (int c = 0; c < cols; ++c) {
        tiles[r * cols + c] = (r + c) & 1;
      }
    }
  }

  void Resize(int new_cols, int new_rows) {
    new_cols = std::max(1, new_cols);
    new_rows = std::max(1, new_rows);
    std::vector<int> next(static_cast<size_t>(new_cols) * new_rows, 0);
    const int copy_cols = std::min(cols, new_cols);
    const int copy_rows = std::min(rows, new_rows);
    for (int r = 0; r < copy_rows; ++r) {
      for (int c = 0; c < copy_cols; ++c) {
        next[r * new_cols + c] = tiles[r * cols + c];
      }
    }
    // Fill the new region with checkerboard so resizing extends the pattern.
    for (int r = 0; r < new_rows; ++r) {
      for (int c = 0; c < new_cols; ++c) {
        if (r >= copy_rows || c >= copy_cols) next[r * new_cols + c] = (r + c) & 1;
      }
    }
    tiles = std::move(next);
    cols = new_cols;
    rows = new_rows;
  }

  void RecomputeOriginIfCentered() {
    if (!center) return;
    origin_x = (::GetScreenWidth() - cols * tile_px) / 2;
    origin_y = (::GetScreenHeight() - rows * tile_px) / 2;
  }
};

void TileEditorLayer::OnAttach() {
  state_ = new State{};

  for (size_t i = 0; i < kTiles.size(); ++i) {
    state_->textures[i] = ::LoadTexture(kTiles[i].path);
    state_->loaded[i] = state_->textures[i].id != 0;
    if (state_->loaded[i]) {
      ::SetTextureFilter(state_->textures[i], TEXTURE_FILTER_BILINEAR);
    } else {
      log::Warn("TileEditorLayer: failed to load a floor tile");
    }
  }

  state_->Checkerboard();
}

void TileEditorLayer::OnDetach() {
  if (!state_) return;
  for (size_t i = 0; i < kTiles.size(); ++i) {
    if (state_->loaded[i]) ::UnloadTexture(state_->textures[i]);
  }
  delete state_;
  state_ = nullptr;
}

void TileEditorLayer::OnUpdate(float /*dt*/) {
  state_->RecomputeOriginIfCentered();

  if (!state_->paint_mode) return;
  const ImGuiIO& io = ImGui::GetIO();
  if (io.WantCaptureMouse) return;

  const Vector2 m = ::GetMousePosition();
  const int gx = (static_cast<int>(m.x) - state_->origin_x) / state_->tile_px;
  const int gy = (static_cast<int>(m.y) - state_->origin_y) / state_->tile_px;
  if (gx < 0 || gx >= state_->cols || gy < 0 || gy >= state_->rows) return;

  if (::IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
    state_->tiles[gy * state_->cols + gx] = state_->brush_id;
  } else if (::IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
    // Right-click swaps to the other tile in the pair (toggle).
    const int other = 1 - state_->brush_id;
    state_->tiles[gy * state_->cols + gx] = other;
  }
}

void TileEditorLayer::OnRender() {
  const int tp = state_->tile_px;
  const int ox = state_->origin_x;
  const int oy = state_->origin_y;

  for (int r = 0; r < state_->rows; ++r) {
    for (int c = 0; c < state_->cols; ++c) {
      const int id = state_->tiles[r * state_->cols + c];
      if (id < 0 || id >= static_cast<int>(kTiles.size()) || !state_->loaded[id]) continue;
      const Texture2D& tex = state_->textures[id];
      const ::Rectangle src{0, 0, static_cast<float>(tex.width),
                            static_cast<float>(tex.height)};
      const ::Rectangle dst{static_cast<float>(ox + c * tp),
                            static_cast<float>(oy + r * tp),
                            static_cast<float>(tp), static_cast<float>(tp)};
      ::DrawTexturePro(tex, src, dst, {0, 0}, 0.0f, WHITE);
    }
  }

  if (state_->show_grid) {
    const Color gc{200, 200, 200, 64};
    for (int c = 0; c <= state_->cols; ++c) {
      const int x = ox + c * tp;
      ::DrawLine(x, oy, x, oy + state_->rows * tp, gc);
    }
    for (int r = 0; r <= state_->rows; ++r) {
      const int y = oy + r * tp;
      ::DrawLine(ox, y, ox + state_->cols * tp, y, gc);
    }
  }

  // Hover highlight when painting.
  if (state_->paint_mode && !ImGui::GetIO().WantCaptureMouse) {
    const Vector2 m = ::GetMousePosition();
    const int gx = (static_cast<int>(m.x) - ox) / tp;
    const int gy = (static_cast<int>(m.y) - oy) / tp;
    if (gx >= 0 && gx < state_->cols && gy >= 0 && gy < state_->rows) {
      ::DrawRectangleLines(ox + gx * tp, oy + gy * tp, tp, tp,
                           Color{255, 220, 0, 255});
    }
  }
}

void TileEditorLayer::OnImGuiRender() {
  if (!ImGui::Begin("Tile Editor / 贴图编辑")) {
    ImGui::End();
    return;
  }

  int cols = state_->cols;
  int rows = state_->rows;
  ImGui::DragInt("Cols", &cols, 1.0f, 1, 256);
  ImGui::DragInt("Rows", &rows, 1.0f, 1, 256);
  if (cols != state_->cols || rows != state_->rows) state_->Resize(cols, rows);
  ImGui::DragInt("Tile px", &state_->tile_px, 1.0f, 4, 256);

  ImGui::Checkbox("Center", &state_->center);
  if (!state_->center) {
    ImGui::DragInt("Origin X", &state_->origin_x, 1.0f, 0, 4096);
    ImGui::DragInt("Origin Y", &state_->origin_y, 1.0f, 0, 4096);
  }

  ImGui::SeparatorText("Brush / 笔刷");
  for (int i = 0; i < static_cast<int>(kTiles.size()); ++i) {
    if (i != 0) ImGui::SameLine();
    ImGui::RadioButton(kTiles[i].label, &state_->brush_id, i);
  }
  for (int i = 0; i < static_cast<int>(kTiles.size()); ++i) {
    if (i != 0) ImGui::SameLine();
    if (state_->loaded[i]) {
      ImGui::Image(static_cast<ImTextureID>(state_->textures[i].id), ImVec2(48, 48));
    }
  }

  ImGui::SeparatorText("View");
  ImGui::Checkbox("Paint mode", &state_->paint_mode);
  ImGui::SameLine();
  ImGui::Checkbox("Show grid", &state_->show_grid);

  ImGui::SeparatorText("Map");
  if (ImGui::Button("Checkerboard")) state_->Checkerboard();
  ImGui::SameLine();
  if (ImGui::Button("Fill")) {
    std::fill(state_->tiles.begin(), state_->tiles.end(), state_->brush_id);
  }
  ImGui::SameLine();
  if (ImGui::Button("Save")) {
    std::ofstream f(kTileFile);
    if (f) {
      f << state_->cols << ' ' << state_->rows << ' ' << state_->tile_px << '\n';
      for (int r = 0; r < state_->rows; ++r) {
        for (int c = 0; c < state_->cols; ++c) {
          f << state_->tiles[r * state_->cols + c];
          f << (c + 1 < state_->cols ? ' ' : '\n');
        }
      }
      log::Info("Tile map saved");
    }
  }
  ImGui::SameLine();
  if (ImGui::Button("Load")) {
    std::ifstream f(kTileFile);
    int cc = 0, rr = 0, tp = 0;
    if (f && (f >> cc >> rr >> tp) && cc > 0 && rr > 0 && tp > 0) {
      state_->cols = cc;
      state_->rows = rr;
      state_->tile_px = tp;
      state_->tiles.assign(static_cast<size_t>(cc) * rr, 0);
      for (auto& v : state_->tiles) f >> v;
      log::Info("Tile map loaded");
    }
  }

  ImGui::TextUnformatted("LMB: paint   RMB: swap");
  ImGui::End();
}

}  // namespace ck
