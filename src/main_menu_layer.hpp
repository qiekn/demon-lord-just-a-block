#pragma once

#include "layer.hpp"

// Std-free header. The menu drives Application::Close() for the exit
// button — Application is forward-declared so this header can be safely
// included from a TU that also uses `import std;`.

namespace ck {

class Application;

class MainMenuLayer : public Layer {
 public:
  explicit MainMenuLayer(Application* app);
  ~MainMenuLayer() override;

  void OnAttach() override;
  void OnDetach() override;
  void OnRender() override;

  bool IsVisible() const;
  void Show();
  void Hide();

 private:
  struct State;
  State* state_ = nullptr;
};

}  // namespace ck
