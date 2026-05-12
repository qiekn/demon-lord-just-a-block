#pragma once

// Forward-declared ImGui-free interface. Keeps <imgui.h> (and the std
// headers it transitively pulls in) out of any TU that mixes `import std;`
// with this header — clang+libc++ will otherwise redeclare std types both
// through the module and through the headers and refuse to compile.

namespace block {

class ImGuiLayer {
 public:
  ImGuiLayer() = default;
  ~ImGuiLayer() = default;

  ImGuiLayer(const ImGuiLayer&) = delete;
  ImGuiLayer& operator=(const ImGuiLayer&) = delete;

  // OnAttach: create the ImGui context, load fonts, init glfw + opengl3
  //   backends against the GLFW window raylib already created.
  // OnDetach: shutdown backends and destroy the context.
  void OnAttach();
  void OnDetach();

  // Wrap each frame with Begin() ... End(). Anything drawn between
  // (DrawDemo, custom panels) is recorded by ImGui and submitted to
  // raylib's current GL context inside End().
  void Begin();
  void End();

  // Built-in convenience window for verifying the integration. Toggle the
  // flag from your gameplay code; once we ship a real editor layer this
  // moves into its OnImGuiRender override.
  void DrawDemo(bool* p_open);
};

}  // namespace block
