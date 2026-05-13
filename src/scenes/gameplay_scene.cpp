#include "gameplay_scene.hpp"

#include <algorithm>
#include <array>
#include <fstream>
#include <vector>

#include <imgui.h>

import raylib;

#include "assets.hpp"
#include "grid.hpp"
#include "imgui_layer.hpp"
#include "log.hpp"
#include "main_menu_scene.hpp"
#include "player.hpp"
#include "scene_manager.hpp"

namespace ck {

namespace {

// Match GameLayer's Grid(11, 7, 96.0f) so painted tiles align with the player.
constexpr int kDefaultCols = 11;
constexpr int kDefaultRows = 7;
constexpr int kDefaultTilePx = 96;
constexpr const char* kTileFile = "tilemap.txt";

// Floor brush palette. Each biome contributes an A/B pair; the Checkerboard
// generator uses the biome of whichever brush is currently selected. Grass
// pair sits at indices 0/1 so old tilemap.txt saves (which only know IDs 0
// and 1) keep rendering as a grass checkerboard.
struct TileDef {
  const char* label;
  const char* path;
};

constexpr std::array<TileDef, 18> kTiles{{
    {"grass A",   CK_ASSET("sprites/grid/grass_a.png")},
    {"grass B",   CK_ASSET("sprites/grid/grass_b.png")},
    {"throne A",  CK_ASSET("sprites/grid/throne_a.png")},
    {"throne B",  CK_ASSET("sprites/grid/throne_b.png")},
    {"brick A",   CK_ASSET("sprites/grid/brick_a.png")},
    {"brick B",   CK_ASSET("sprites/grid/brick_b.png")},
    {"dirt A",    CK_ASSET("sprites/grid/dirt_a.png")},
    {"dirt B",    CK_ASSET("sprites/grid/dirt_b.png")},
    {"sand A",    CK_ASSET("sprites/grid/sand_a.png")},
    {"sand B",    CK_ASSET("sprites/grid/sand_b.png")},
    {"void A",    CK_ASSET("sprites/grid/void_a.png")},
    {"void B",    CK_ASSET("sprites/grid/void_b.png")},
    {"ice A",     CK_ASSET("sprites/grid/ice_a.png")},
    {"ice B",     CK_ASSET("sprites/grid/ice_b.png")},
    {"water A",   CK_ASSET("sprites/grid/water_a.png")},
    {"water B",   CK_ASSET("sprites/grid/water_b.png")},
    {"dungeon A", CK_ASSET("sprites/grid/dungeon_a.png")},
    {"dungeon B", CK_ASSET("sprites/grid/dungeon_b.png")},
}};

}  // namespace

// Player owns its textures (RAII Texture). Tile textures are managed here
// via ck::raii::Texture; both share the same scene lifetime, so OnExit fully
// restores the GL state to "no GameplayScene resources loaded".
struct GameplayScene::State {
  Grid grid;
  Player player;
  bool show_demo = false;

  // Tile editor data.
  std::array<raii::Texture, kTiles.size()> textures;
  int cols = kDefaultCols;
  int rows = kDefaultRows;
  int tile_px = kDefaultTilePx;
  bool center = true;
  int origin_x = 0;
  int origin_y = 0;
  std::vector<int> tiles;
  int brush_id = 0;
  bool show_grid = false;
  bool paint_mode = true;

  State() : grid(11, 7, 96.0f), player({grid.Cols() / 2, grid.Rows() / 2}) {}

  // Checkerboard uses the A/B pair of whichever brush is currently selected.
  // Biomes are laid out in (A, B) pairs starting at index 0, so the A index
  // is `brush & ~1` and B is `A | 1`.
  void Checkerboard() {
    const int a = brush_id & ~1;
    const int b = a | 1;
    tiles.assign(static_cast<size_t>(cols) * rows, 0);
    for (int r = 0; r < rows; ++r) {
      for (int c = 0; c < cols; ++c) {
        tiles[r * cols + c] = ((r + c) & 1) ? b : a;
      }
    }
  }

  void Resize(int new_cols, int new_rows) {
    new_cols = std::max(1, new_cols);
    new_rows = std::max(1, new_rows);
    const int a = brush_id & ~1;
    const int b = a | 1;
    std::vector<int> next(static_cast<size_t>(new_cols) * new_rows, 0);
    const int copy_cols = std::min(cols, new_cols);
    const int copy_rows = std::min(rows, new_rows);
    for (int r = 0; r < copy_rows; ++r) {
      for (int c = 0; c < copy_cols; ++c) {
        next[r * new_cols + c] = tiles[r * cols + c];
      }
    }
    for (int r = 0; r < new_rows; ++r) {
      for (int c = 0; c < new_cols; ++c) {
        if (r >= copy_rows || c >= copy_cols)
          next[r * new_cols + c] = ((r + c) & 1) ? b : a;
      }
    }
    tiles = std::move(next);
    cols = new_cols;
    rows = new_rows;
  }

  void RecomputeOriginIfCentered() {
    if (!center) return;
    origin_x = (rl::GetScreenWidth() - cols * tile_px) / 2;
    origin_y = (rl::GetScreenHeight() - rows * tile_px) / 2;
  }
};

GameplayScene::~GameplayScene() { delete state_; }

void GameplayScene::OnEnter() {
  state_ = new State{};

  for (size_t i = 0; i < kTiles.size(); ++i) {
    state_->textures[i].Load(kTiles[i].path);
    if (state_->textures[i]) {
      state_->textures[i].SetFilter(TEXTURE_FILTER_BILINEAR);
    } else {
      log::Warn("GameplayScene: failed to load a floor tile");
    }
  }

  state_->Checkerboard();
}

void GameplayScene::OnExit() {
  if (!state_) return;
  delete state_;
  state_ = nullptr;
}

void GameplayScene::OnUpdate(float dt) {
  if (!state_) return;

  if (IsKeyPressed(KEY_ESCAPE)) {
    Manager()->Switch<MainMenuScene>();
    return;
  }

  state_->RecomputeOriginIfCentered();
  state_->player.Update(dt, state_->grid);

  if (!state_->paint_mode) return;
  const ImGuiIO& io = ImGui::GetIO();
  if (io.WantCaptureMouse) return;

  const Vector2 m = GetMousePosition();
  const int gx = (static_cast<int>(m.x) - state_->origin_x) / state_->tile_px;
  const int gy = (static_cast<int>(m.y) - state_->origin_y) / state_->tile_px;
  if (gx < 0 || gx >= state_->cols || gy < 0 || gy >= state_->rows) return;

  if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
    state_->tiles[gy * state_->cols + gx] = state_->brush_id;
  } else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
    // RMB paints the OTHER half of the brush's biome (A↔B).
    state_->tiles[gy * state_->cols + gx] = state_->brush_id ^ 1;
  }
}

void GameplayScene::OnRender() {
  if (!state_) return;

  const int tp = state_->tile_px;
  const int ox = state_->origin_x;
  const int oy = state_->origin_y;

  for (int r = 0; r < state_->rows; ++r) {
    for (int c = 0; c < state_->cols; ++c) {
      const int id = state_->tiles[r * state_->cols + c];
      if (id < 0 || id >= static_cast<int>(kTiles.size()) || !state_->textures[id]) continue;
      const auto& tex = state_->textures[id];
      const Rectangle src{0, 0, static_cast<float>(tex.GetWidth()),
                          static_cast<float>(tex.GetHeight())};
      const Rectangle dst{static_cast<float>(ox + c * tp),
                          static_cast<float>(oy + r * tp),
                          static_cast<float>(tp), static_cast<float>(tp)};
      tex.DrawPro(src, dst, {0, 0}, 0.0f, WHITE);
    }
  }

  if (state_->show_grid) {
    const Color gc{200, 200, 200, 64};
    for (int c = 0; c <= state_->cols; ++c) {
      const int x = ox + c * tp;
      DrawLine(x, oy, x, oy + state_->rows * tp, gc);
    }
    for (int r = 0; r <= state_->rows; ++r) {
      const int y = oy + r * tp;
      DrawLine(ox, y, ox + state_->cols * tp, y, gc);
    }
  }

  // Hover highlight + brush preview (Baba-style).
  if (state_->paint_mode && !ImGui::GetIO().WantCaptureMouse) {
    const Vector2 m = GetMousePosition();
    const int gx = (static_cast<int>(m.x) - ox) / tp;
    const int gy = (static_cast<int>(m.y) - oy) / tp;
    if (gx >= 0 && gx < state_->cols && gy >= 0 && gy < state_->rows) {
      const int bid = state_->brush_id;
      if (bid >= 0 && bid < static_cast<int>(kTiles.size()) && state_->textures[bid]) {
        const auto& tex = state_->textures[bid];
        const Rectangle src{0, 0, static_cast<float>(tex.GetWidth()),
                            static_cast<float>(tex.GetHeight())};
        const Rectangle dst{static_cast<float>(ox + gx * tp),
                            static_cast<float>(oy + gy * tp),
                            static_cast<float>(tp), static_cast<float>(tp)};
        tex.DrawPro(src, dst, {0, 0}, 0.0f, Color{255, 255, 255, 160});
      }
      DrawRectangleLines(ox + gx * tp, oy + gy * tp, tp, tp,
                         Color{255, 220, 0, 255});
    }
  }

  state_->player.Render(state_->grid);
}

void GameplayScene::OnImGuiRender() {
  if (!state_) return;
  if (!ImGuiLayer::PanelsVisible()) return;

  if (ImGui::Begin("Player tuning")) {
    auto& t = state_->player.tuning;
    ImGui::SliderFloat("Repeat delay", &t.repeat_delay, 0.05f, 0.8f, "%.2f s");
    ImGui::SliderFloat("Repeat interval", &t.repeat_interval, 0.03f, 0.5f, "%.2f s");
    ImGui::SliderFloat("Sprite tween", &t.sprite_duration, 0.05f, 0.6f, "%.2f s");
    ImGui::SliderFloat("Block tween", &t.block_duration, 0.05f, 0.6f, "%.2f s");
    ImGui::SliderFloat("Hop height (V)", &t.hop_height, 0.0f, 1.5f, "%.2f cells");
    ImGui::SliderFloat("Hop height (H)", &t.hop_height_horizontal, 0.0f, 1.5f, "%.2f cells");
    ImGui::SliderFloat("Sprite scale", &t.sprite_scale, 0.1f, 1.0f, "%.2f cells");
    ImGui::SliderInt("HP font size", &t.hp_font_size, 6, 48, "%d px");
    ImGui::Separator();
    ImGui::SliderInt("Max HP", &state_->player.MaxHpRef(), 1, 99);
    ImGui::SliderInt("HP", &state_->player.HpRef(), 0, state_->player.MaxHpRef());
    ImGui::Checkbox("ImGui demo", &state_->show_demo);
  }
  ImGui::End();
  if (state_->show_demo) ImGui::ShowDemoWindow(&state_->show_demo);

  if (!ImGui::Begin("Tile Editor")) {
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

  ImGui::SeparatorText("Brush");
  constexpr ImVec2 kBrushSize(48, 48);
  const ImVec4 kSelectedBg(1.0f, 0.85f, 0.2f, 0.85f);
  const ImVec4 kSelectedHover(1.0f, 0.9f, 0.3f, 1.0f);
  // Two columns: each row is one biome's A/B pair.
  for (int i = 0; i < static_cast<int>(kTiles.size()); ++i) {
    if (i % 2 == 1) ImGui::SameLine();
    if (!state_->textures[i]) continue;
    const bool selected = (state_->brush_id == i);
    ImGui::PushID(i);
    if (selected) {
      ImGui::PushStyleColor(ImGuiCol_Button, kSelectedBg);
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, kSelectedHover);
      ImGui::PushStyleColor(ImGuiCol_ButtonActive, kSelectedHover);
    }
    if (ImGui::ImageButton("##tile",
                           static_cast<ImTextureID>(state_->textures[i].Get().id),
                           kBrushSize)) {
      state_->brush_id = i;
    }
    if (selected) ImGui::PopStyleColor(3);
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", kTiles[i].label);
    ImGui::PopID();
  }
  ImGui::TextDisabled("Selected: %s", kTiles[state_->brush_id].label);

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
