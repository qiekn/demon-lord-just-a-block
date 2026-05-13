#pragma once

// Grid-bound player. Owns sprite + background block (both ck::raii::Texture),
// drives input, tweens position with a hopping sprite over a draggier
// background block so the two visibly desync mid-step.

#include "texture.hpp"

#include "grid.hpp"

namespace ck {

class Player {
 public:
  explicit Player(GridCoord start);

  Player(const Player&) = delete;
  Player& operator=(const Player&) = delete;

  void Update(float dt, const Grid& grid);
  void Render(const Grid& grid) const;

  GridCoord Position() const { return pos_; }
  int Hp() const { return hp_; }
  int MaxHp() const { return max_hp_; }
  void Damage(int amount);
  void Heal(int amount);

 private:
  void TryMove(int dx, int dy, const Grid& grid);

  GridCoord pos_;
  GridCoord prev_;

  int hp_ = 5;
  int max_hp_ = 5;

  // anim_t_ advances 0 → 1 over block_duration_; the sprite tween reuses the
  // same clock at a faster effective rate and adds a parabolic arc, so the
  // sprite visibly leads the block.
  float anim_t_ = 1.0f;
  float sprite_duration_ = 0.16f;
  float block_duration_ = 0.28f;
  float hop_height_ = 0.45f;

  ::ck::raii::Texture sprite_;
  ::ck::raii::Texture block_;
};

}  // namespace ck
