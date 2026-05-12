#include "imgui_layer.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <raylib.h>

#include "assets.hpp"

namespace block {

namespace {

// Hazel-style dark theme, ported from ck-engine's ImGuiLayer. Color tuples
// describe widget category triples (resting / hovered / active) so each
// state stays a small step from the base. WindowBg sets the canvas tone;
// Tab and Title overrides keep the docking chrome consistent.
void ApplyDarkTheme() {
  auto& colors = ImGui::GetStyle().Colors;
  colors[ImGuiCol_WindowBg] = ImVec4{0.1f, 0.105f, 0.11f, 1.0f};

  colors[ImGuiCol_Header] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
  colors[ImGuiCol_HeaderHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
  colors[ImGuiCol_HeaderActive] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

  colors[ImGuiCol_Button] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
  colors[ImGuiCol_ButtonHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
  colors[ImGuiCol_ButtonActive] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

  colors[ImGuiCol_FrameBg] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
  colors[ImGuiCol_FrameBgHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
  colors[ImGuiCol_FrameBgActive] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

  colors[ImGuiCol_Tab] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
  colors[ImGuiCol_TabHovered] = ImVec4{0.38f, 0.3805f, 0.381f, 1.0f};
  colors[ImGuiCol_TabActive] = ImVec4{0.28f, 0.2805f, 0.281f, 1.0f};
  colors[ImGuiCol_TabUnfocused] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
  colors[ImGuiCol_TabUnfocusedActive] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};

  colors[ImGuiCol_TitleBg] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
  colors[ImGuiCol_TitleBgActive] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
  colors[ImGuiCol_TitleBgCollapsed] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
}

}  // namespace

void ImGuiLayer::OnAttach() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.IniFilename = "imgui.ini";

  // NotoSansSC covers CJK; rebake at dpi-scaled size so HiDPI displays
  // don't show blurry text. GetWindowScaleDPI() returns the active scale
  // from GLFW; raylib's FLAG_WINDOW_HIGHDPI must already be set.
  const float dpi = ::GetWindowScaleDPI().x;
  ImFontConfig cfg;
  cfg.OversampleH = 2;
  cfg.OversampleV = 2;
  io.Fonts->AddFontFromFileTTF(BLOCK_ASSET("fonts/NotoSansSC-Regular.ttf"), 18.0f * dpi, &cfg,
                               io.Fonts->GetGlyphRangesChineseSimplifiedCommon());

  ImGui::StyleColorsDark();
  ImGuiStyle& style = ImGui::GetStyle();
  style.WindowRounding = 8.0f;
  style.FrameRounding = 8.0f;
  style.ScaleAllSizes(dpi);
  ApplyDarkTheme();

  // raylib is already the current GLFW context after InitWindow.
  GLFWwindow* window = glfwGetCurrentContext();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 330");
}

void ImGuiLayer::OnDetach() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void ImGuiLayer::Begin() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void ImGuiLayer::End() {
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiLayer::DrawDemo(bool* p_open) {
  if (p_open && *p_open) ImGui::ShowDemoWindow(p_open);
}

}  // namespace block
