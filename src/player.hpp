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
    float repeat_delay = 0.25f;     // wait before the FIRST auto-repeat after a key starts being held
    float repeat_interval = 0.12f;  // between subsequent auto-repeats
    float sprite_duration = 0.13f;
    float block_duration = 0.20f;
    float hop_height = 0.2f;             // multiples of cell size (vertical moves)
    float hop_height_horizontal = 0.2f;  // multiples of cell size (L/R moves)
    float sprite_scale = 0.55f;          // sprite size as multiples of cell size
  };

  explicit Player(GridCoord start);

  Player(const Player&) = delete;
  Player& operator=(const Player&) = delete;

  void Update(float dt, const Grid& grid);
  void Render(const Grid& grid, TextOutliner& outliner) const;

  GridCoord Position() const { return pos_; }
  int Hp() const { return hp_; }
  int MaxHp() const { return max_hp_; }
  // Mutable handles for ImGui scrubbing. Gameplay code should still go through
  // Damage / Heal — these exist so a slider can write through directly.
  int& HpRef() { return hp_; }
  int& MaxHpRef() { return max_hp_; }
  void Damage(int amount);
  void Heal(int amount);

  Tuning tuning;

 private:
  bool TryMove(int dx, int dy, const Grid& grid);

  GridCoord pos_;
  GridCoord prev_;

  // Last horizontal direction the player moved in (true = right, false = left).
  // Updated only when dx != 0 so vertical-only moves preserve facing.
  bool facing_right_ = true;

  int hp_ = 5;
  int max_hp_ = 5;

  // Tween clock: 0 → 1 over `tuning.block_duration` seconds.
  float anim_t_ = 1.0f;

  // Two-stage key repeat. `prev_held_` tracks the rising edge so a fresh press
  // fires immediately. `awaiting_first_repeat_` selects which threshold the
  // timer is racing against — `tuning.repeat_delay` until the first
  // auto-repeat fires, then `tuning.repeat_interval` for every subsequent one.
  bool prev_held_ = false;
  bool awaiting_first_repeat_ = true;
  float repeat_timer_ = 0.0f;

  ::ck::raii::Texture sprite_;
  ::ck::raii::Texture block_;
};

}  // namespace ck
