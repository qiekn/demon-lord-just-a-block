#pragma once

// Tile grid. Owns the floor texture as ck::raii::Texture (RAII unload), and
// renders a cols × rows board centered on the current screen each frame.
//
// Reaches raylib only via "texture.hpp" → raylib_c.hpp, which strips the
// RED/BLUE/WHITE color macros — safe to include from any TU that also goes
// through the ck:: color constants. Don't include <raylib.h> directly.

#include "texture.hpp"

namespace ck {

struct GridCoord {
  int x;
  int y;
};

class Grid {
 public:
  Grid(int cols, int rows, float cell_size);

  Grid(const Grid&) = delete;
  Grid& operator=(const Grid&) = delete;

  void Render() const;

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
  ::ck::raii::Texture floor_;
};

}  // namespace ck
