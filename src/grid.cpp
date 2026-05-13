#include "grid.hpp"

namespace ck {

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

}  // namespace ck
