#pragma once

// Tile grid: cols × rows board centered on the current screen. Coord helpers
// only — background tiles are owned by TileEditorLayer.

#include "raylib_c.hpp"

namespace ck {

struct GridCoord {
  int x;
  int y;
};

class Grid {
 public:
  Grid(int cols, int rows, float cell_size)
      : cols_(cols), rows_(rows), cell_size_(cell_size) {}

  Grid(const Grid&) = delete;
  Grid& operator=(const Grid&) = delete;

  ::Vector2 CellTopLeft(GridCoord c) const;
  ::Vector2 CellCenter(GridCoord c) const;

  int Cols() const { return cols_; }
  int Rows() const { return rows_; }
  float CellSize() const { return cell_size_; }

  bool InBounds(GridCoord c) const;

 private:
  int cols_;
  int rows_;
  float cell_size_;
};

}  // namespace ck
