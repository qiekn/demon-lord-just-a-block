#include "grid.hpp"

#include "colors.hpp"

#include "assets.hpp"

namespace ck {

Grid::Grid(int cols, int rows, float cell_size)
    : cols_(cols),
      rows_(rows),
      cell_size_(cell_size),
      floor_(CK_ASSET("sprites/tile_floor.png")) {
  floor_.SetFilter(::TEXTURE_FILTER_BILINEAR);
}

::Vector2 Grid::CellTopLeft(GridCoord c) const {
  const float total_w = cols_ * cell_size_;
  const float total_h = rows_ * cell_size_;
  const float origin_x = (::GetScreenWidth() - total_w) * 0.5f;
  const float origin_y = (::GetScreenHeight() - total_h) * 0.5f;
  return ::Vector2{origin_x + c.x * cell_size_, origin_y + c.y * cell_size_};
}

::Vector2 Grid::CellCenter(GridCoord c) const {
  const ::Vector2 tl = CellTopLeft(c);
  return ::Vector2{tl.x + cell_size_ * 0.5f, tl.y + cell_size_ * 0.5f};
}

bool Grid::InBounds(GridCoord c) const {
  return c.x >= 0 && c.x < cols_ && c.y >= 0 && c.y < rows_;
}

void Grid::Render() const {
  const ::Rectangle src{0, 0, static_cast<float>(floor_.GetWidth()),
                        static_cast<float>(floor_.GetHeight())};
  for (int y = 0; y < rows_; ++y) {
    for (int x = 0; x < cols_; ++x) {
      const ::Vector2 tl = CellTopLeft({x, y});
      const ::Rectangle dst{tl.x, tl.y, cell_size_, cell_size_};
      floor_.DrawPro(src, dst, {0, 0}, 0.0f, ck::WHITE);
    }
  }
}

}  // namespace ck
