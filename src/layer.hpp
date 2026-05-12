#pragma once

// Layer base for the LayerStack-driven main loop. Subclasses override the
// hooks they care about; the Application calls every hook on every layer.
//
// Std-free deliberately. Consumers that mix `import std;` with this header
// (e.g. main.cpp) would otherwise hit the clang+libc++ double-decl trap.

namespace ck {

class Layer {
 public:
  explicit Layer(const char* name = "Layer") : name_(name) {}
  virtual ~Layer() = default;

  Layer(const Layer&) = delete;
  Layer& operator=(const Layer&) = delete;

  virtual void OnAttach() {}
  virtual void OnDetach() {}
  virtual void OnUpdate(float /*dt*/) {}
  virtual void OnRender() {}
  virtual void OnImGuiRender() {}

  // ImGuiLayer overrides these to bracket each frame's ImGui drawing.
  // The Application calls Begin on every layer before any OnImGuiRender,
  // and End after, so non-imgui layers stay no-op.
  virtual void OnImGuiBegin() {}
  virtual void OnImGuiEnd() {}

  const char* Name() const { return name_; }

 protected:
  const char* name_;
};

}  // namespace ck
