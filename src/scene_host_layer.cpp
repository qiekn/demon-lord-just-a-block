#include "scene_host_layer.hpp"

#include "application.hpp"
#include "main_menu_scene.hpp"
#include "scene_manager.hpp"

namespace ck {

struct SceneHostLayer::State {
  Application* app = nullptr;
  SceneManager scenes;
};

SceneHostLayer::SceneHostLayer(Application* app)
    : Layer("SceneHostLayer"), state_(new State{.app = app}) {}

SceneHostLayer::~SceneHostLayer() { delete state_; }

void SceneHostLayer::OnAttach() {
  // Boot scene. Application::Run sees nothing on the screen until the first
  // Update() flushes the pending Switch (i.e. one frame of empty stack), but
  // that's identical to enigmash and is invisible at 60+ FPS.
  state_->scenes.Switch<MainMenuScene>();
}

void SceneHostLayer::OnDetach() {
  // Drain the stack so each scene's OnExit runs while the GL context is
  // still alive. Without this, ~SceneManager would still tear scenes down,
  // but by then ~Application has already CloseWindow()'d the GL context.
  while (state_->scenes.Active() != nullptr) {
    state_->scenes.Pop();
    state_->scenes.Update(0.0f);  // applies the pending Pop
  }
}

void SceneHostLayer::OnUpdate(float dt) {
  state_->scenes.Update(dt);
  if (state_->scenes.QuitRequested() && state_->app) {
    state_->app->Close();
  }
}

void SceneHostLayer::OnRender() { state_->scenes.Render(); }

void SceneHostLayer::OnImGuiRender() { state_->scenes.ImGuiRender(); }

}  // namespace ck
