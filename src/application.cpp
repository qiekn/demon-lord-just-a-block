#include "application.hpp"

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <system_error>
#include <vector>

import raylib;

#include "assets.hpp"
#include "layer.hpp"
#include "log.hpp"
#include "window_state.hpp"

namespace ck {

namespace {

constexpr const char* kWindowStateFile = "window.state";

namespace fs = std::filesystem;

// Pin CWD to the directory that holds `assets/`. Without this, the binary
// works only when launched from the repo root: launching from `build/` or via
// IDE would leave CWD elsewhere and every `assets/...` path would fail.
// Candidates checked in order: current CWD, binary dir, binary dir / "..".
void AnchorWorkingDirectory() {
  std::error_code ec;
  const fs::path cwd = fs::current_path(ec);
  const fs::path exe_dir = fs::path{GetApplicationDirectory()};

  const fs::path candidates[] = {cwd, exe_dir, exe_dir / ".."};
  for (const auto& base : candidates) {
    if (fs::is_directory(base / "assets", ec)) {
      const fs::path resolved = fs::weakly_canonical(base, ec);
      if (resolved != cwd) {
        fs::current_path(resolved, ec);
        log::Info("Anchored working directory to assets root");
      }
      return;
    }
  }
  log::Warn("No assets/ directory found near CWD or binary; file loads may fail");
}

class LayerStack {
 public:
  LayerStack() = default;
  ~LayerStack() { Clear(); }

  LayerStack(const LayerStack&) = delete;
  LayerStack& operator=(const LayerStack&) = delete;

  Layer* PushLayer(std::unique_ptr<Layer> layer) {
    layer->OnAttach();
    Layer* raw = layer.get();
    layers_.insert(layers_.begin() + static_cast<std::ptrdiff_t>(overlay_start_),
                   std::move(layer));
    ++overlay_start_;
    return raw;
  }

  Layer* PushOverlay(std::unique_ptr<Layer> overlay) {
    overlay->OnAttach();
    Layer* raw = overlay.get();
    layers_.push_back(std::move(overlay));
    return raw;
  }

  void Clear() {
    for (auto it = layers_.rbegin(); it != layers_.rend(); ++it) {
      (*it)->OnDetach();
    }
    layers_.clear();
    overlay_start_ = 0;
  }

  auto begin() { return layers_.begin(); }
  auto end() { return layers_.end(); }

 private:
  std::vector<std::unique_ptr<Layer>> layers_;
  std::size_t overlay_start_ = 0;
};

}  // namespace

struct Application::State {
  ApplicationSpec spec;
  // AudioDevice declared before LayerStack so layer-owned ck::raii::Sound
  // resources are torn down BEFORE the audio device closes.
  raii::AudioDevice audio;
  LayerStack layers;
  bool running = true;
};

Application::Application(ApplicationSpec spec) : state_(new State{.spec = spec}) {
  AnchorWorkingDirectory();

  unsigned int flags = 0;
  if (spec.high_dpi) flags |= FLAG_WINDOW_HIGHDPI;
  if (spec.resizable) flags |= FLAG_WINDOW_RESIZABLE;
  SetConfigFlags(flags);

  // Restore previous window geometry; LoadWindowState returns sensible
  // defaults if the state file is missing or malformed.
  const WindowState ws = LoadWindowState(kWindowStateFile);
  rl::InitWindow(ws.w, ws.h, spec.name);
  rl::SetWindowPosition(ws.x, ws.y);
  SetTargetFPS(spec.target_fps);

  // Window icon. Loaded after InitWindow because SetWindowIcon forwards to
  // the GLFW window handle. ck::raii::Image unloads the pixel data when it
  // goes out of scope — raylib copies internally during SetWindowIcon.
  raii::Image icon(CK_ASSET("sprites/icon.png"));
  if (icon) rl::SetWindowIcon(icon.Get());
  else log::Warn("Failed to load window icon");

  // Disable raylib's built-in "ESC quits the app" — WindowShouldClose()
  // otherwise returns true on ESC, short-circuiting any scene that wants
  // to handle ESC itself (e.g. GameplayScene → MainMenuScene).
  SetExitKey(KEY_NULL);

  // With HIGHDPI raylib draws in logical coords (screen.width) and scales to
  // the framebuffer via screenScale, but GetMousePosition() returns raw GLFW
  // pixels. Invert that here so mouse coords match raylib's drawing space.
  if (spec.high_dpi) {
    const Vector2 dpi = GetWindowScaleDPI();
    if (dpi.x > 0.0f && dpi.y > 0.0f) SetMouseScale(1.0f / dpi.x, 1.0f / dpi.y);
  }

  log::Info("Application started");
}

Application::~Application() {
  // Capture geometry before CloseWindow tears down GLFW.
  const Vector2 pos = rl::GetWindowPosition();
  SaveWindowState(kWindowStateFile, {.x = static_cast<int>(pos.x),
                                     .y = static_cast<int>(pos.y),
                                     .w = rl::GetScreenWidth(),
                                     .h = rl::GetScreenHeight()});
  state_->layers.Clear();
  rl::CloseWindow();
  log::Info("Application shut down");
  delete state_;
}

Layer* Application::PushLayer(Layer* layer) {
  return state_->layers.PushLayer(std::unique_ptr<Layer>(layer));
}

Layer* Application::PushOverlay(Layer* overlay) {
  return state_->layers.PushOverlay(std::unique_ptr<Layer>(overlay));
}

void Application::Run() {
  while (!rl::WindowShouldClose() && state_->running) {
    const float dt = GetFrameTime();
    for (auto& layer : state_->layers) layer->OnUpdate(dt);

    BeginDrawing();
    ClearBackground(RAYWHITE);
    for (auto& layer : state_->layers) layer->OnRender();

    // ImGuiLayer overrides OnImGuiBegin/End to wrap the imgui frame; the
    // other layers' overrides default to no-ops.
    for (auto& layer : state_->layers) layer->OnImGuiBegin();
    for (auto& layer : state_->layers) layer->OnImGuiRender();
    for (auto& layer : state_->layers) layer->OnImGuiEnd();

    EndDrawing();
  }
}

void Application::Close() { state_->running = false; }

}  // namespace ck
