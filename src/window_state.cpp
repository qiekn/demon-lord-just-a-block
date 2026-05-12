#include "window_state.hpp"

#include <cstdio>

namespace ck {

WindowState LoadWindowState(const char* path) {
  WindowState s;
  if (std::FILE* f = std::fopen(path, "r")) {
    int x = 0, y = 0, w = 0, h = 0;
    if (std::fscanf(f, "%d %d %d %d", &x, &y, &w, &h) == 4 && w > 0 && h > 0) {
      s = {x, y, w, h};
    }
    std::fclose(f);
  }
  return s;
}

void SaveWindowState(const char* path, const WindowState& s) {
  if (std::FILE* f = std::fopen(path, "w")) {
    std::fprintf(f, "%d %d %d %d\n", s.x, s.y, s.w, s.h);
    std::fclose(f);
  }
}

}  // namespace ck
