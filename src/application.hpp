#pragma once

// Std-free public interface — clients that `import std;` can include this
// header without dragging conflicting std decls. Internal state (LayerStack,
// std containers) is hidden via PImpl in application.cpp.

namespace block {

class Layer;

struct ApplicationSpec {
  const char* name = "Block";
  int width = 1280;
  int height = 720;
  int target_fps = 160;
  bool high_dpi = true;
  bool resizable = true;
};

class Application {
 public:
  explicit Application(ApplicationSpec spec = {});
  ~Application();

  Application(const Application&) = delete;
  Application& operator=(const Application&) = delete;

  // Takes ownership of `layer`. Returned pointer is an observer.
  Layer* PushLayer(Layer* layer);
  Layer* PushOverlay(Layer* overlay);

  void Run();
  void Close();

 private:
  struct State;
  State* state_;
};

}  // namespace block
