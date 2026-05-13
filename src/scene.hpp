#pragma once

// Std-free header. SceneManager is forward-declared so this header can be
// included from a TU that also uses `import std;` (e.g. main.cpp's friends
// down the include chain). The full scene_manager.hpp pulls <memory>+<vector>
// for its templates and must only be touched from .cpp files.

namespace ck {

class SceneManager;

// Scenes are the unit the SceneManager stacks and swaps. SceneHostLayer owns
// the manager; only the active scene's hooks fire each frame (unless an
// overlay scene sits on top — then both render, lower-first).
//
// OnEnter / OnExit fire on the manager's own timeline (deferred to the next
// Update() boundary), so a scene can allocate GL resources in OnEnter and
// free them in OnExit without worrying about mid-frame state.
class Scene {
 public:
  virtual ~Scene() = default;

  Scene(const Scene&) = delete;
  Scene& operator=(const Scene&) = delete;

  virtual void OnEnter() {}
  virtual void OnExit() {}
  virtual void OnUpdate(float /*dt*/) {}
  virtual void OnRender() {}
  virtual void OnImGuiRender() {}

  // Overlays let SceneManager keep rendering the scene underneath (useful for
  // pause-menu push). Default false: most scenes fully cover the framebuffer.
  virtual bool IsOverlay() const { return false; }

  // Set when SceneManager takes ownership; scenes use it to request
  // transitions (Manager()->Switch<NextScene>()).
  void SetManager(SceneManager* m) { manager_ = m; }
  SceneManager* Manager() const { return manager_; }

  const char* Name() const { return name_; }

 protected:
  explicit Scene(const char* name = "Scene") : name_(name) {}

  const char* name_;
  SceneManager* manager_ = nullptr;
};

}  // namespace ck
