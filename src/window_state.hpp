#pragma once

// Window position + size persistence. Loaded before InitWindow, saved before
// destroying the Window so the next launch restores the previous layout. Same
// shape as baba's window.state (`x y w h`, single line). File path is fixed
// at `window.state` in the CWD; the binary is run from the repo root.

namespace block {

struct WindowState {
  int x = 100;
  int y = 100;
  int w = 1280;
  int h = 720;
};

// Returns defaults if the file is missing or malformed.
WindowState LoadWindowState(const char* path);

// Best-effort write; silently no-ops on filesystem error.
void SaveWindowState(const char* path, const WindowState& state);

}  // namespace block
