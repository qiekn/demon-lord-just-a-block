#pragma once

// Grid-bound player. Owns sprite + background block (both ck::raii::Texture),
// drives input, tweens position with a hopping sprite over a draggier
// background block so the two visibly desync mid-step.

#include "texture.hpp"

#include "grid.hpp"

namespace ck {

class TextOutliner;

class Player {
 public:
  // Tunable knobs exposed publicly so ImGui (or a settings panel) can write
  // through them without ceremony. Defaults tuned against refs/player_move_anim/
  // (30 fps captures, ~6 frames per move ≈ 200 ms total).
  struct Tuning {
    float repeat_interval = 0.18f;  // seconds between auto-repeats while held
    float sprite_duration = 0.13f;
    float block_duration = 0.20f;
    float hop_height = 0.45f;             // multiples of cell size (vertical moves)
    float hop_height_horizontal = 0.75f;  // multiples of cell size (L/R moves)
  };

  explicit Player(GridCoord start);

  Player(const Player&) = delete;
  Player& operator=(const Player&) = delete;

  void Update(float dt, const Grid& grid);
  void Render(const Grid& grid, TextOutliner& outliner) const;

  GridCoord Position() const { return pos_; }
  int Hp() const { return hp_; }
  int MaxHp() const { return max_hp_; }
  void Damage(int amount);
  void Heal(int amount);

  Tuning tuning;

 private:
  bool TryMove(int dx, int dy, const Grid& grid);

  GridCoord pos_;
  GridCoord prev_;

  int hp_ = 5;
  int max_hp_ = 5;

  // Tween clock: 0 → 1 over `tuning.block_duration` seconds.
  float anim_t_ = 1.0f;

  // Repeat-key clock. Primed to `tuning.repeat_interval` when no direction
  // key is held, so the first frame a key is detected fires immediately.
  float repeat_timer_ = 0.0f;

  ::ck::raii::Texture sprite_;
  ::ck::raii::Texture block_;
};

}  // namespace ck
