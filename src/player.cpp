#include "player.hpp"

#include <algorithm>

#include "colors.hpp"
#include "shapes.hpp"

#include "assets.hpp"

namespace ck {

namespace {

float Clamp01(float v) {
  if (v < 0.0f) return 0.0f;
  if (v > 1.0f) return 1.0f;
  return v;
}

float EaseOutCubic(float t) {
  const float u = 1.0f - t;
  return 1.0f - u * u * u;
}

float Lerp(float a, float b, float t) { return a + (b - a) * t; }

}  // namespace

Player::Player(GridCoord start)
    : pos_(start),
      prev_(start),
      sprite_(CK_ASSET("sprites/player.png")),
      block_(CK_ASSET("sprites/player_block.png")) {
  sprite_.SetFilter(::TEXTURE_FILTER_BILINEAR);
  block_.SetFilter(::TEXTURE_FILTER_BILINEAR);
}

void Player::Damage(int amount) { hp_ = std::max(0, hp_ - amount); }
void Player::Heal(int amount) { hp_ = std::min(max_hp_, hp_ + amount); }

void Player::TryMove(int dx, int dy, const Grid& grid) {
  if (anim_t_ < 1.0f) return;  // ignore input while tweening
  const GridCoord next{pos_.x + dx, pos_.y + dy};
  if (!grid.InBounds(next)) return;
  prev_ = pos_;
  pos_ = next;
  anim_t_ = 0.0f;
}

void Player::Update(float dt, const Grid& grid) {
  if (::IsKeyPressed(::KEY_RIGHT) || ::IsKeyPressed(::KEY_D))
    TryMove(1, 0, grid);
  else if (::IsKeyPressed(::KEY_LEFT) || ::IsKeyPressed(::KEY_A))
    TryMove(-1, 0, grid);
  else if (::IsKeyPressed(::KEY_DOWN) || ::IsKeyPressed(::KEY_S))
    TryMove(0, 1, grid);
  else if (::IsKeyPressed(::KEY_UP) || ::IsKeyPressed(::KEY_W))
    TryMove(0, -1, grid);

  if (anim_t_ < 1.0f) anim_t_ = Clamp01(anim_t_ + dt / block_duration_);
}

void Player::Render(const Grid& grid) const {
  const float cs = grid.CellSize();
  const ::Vector2 from = grid.CellCenter(prev_);
  const ::Vector2 to = grid.CellCenter(pos_);

  // Block tween: ease-out over the full block_duration_. Drags behind sprite.
  const float block_e = EaseOutCubic(anim_t_);
  const float block_x = Lerp(from.x, to.x, block_e);
  const float block_y = Lerp(from.y, to.y, block_e);

  // Sprite tween: completes in sprite_duration_ (< block_duration_) and then
  // idles on the destination cell while the block catches up. Parabolic hop.
  const float sprite_t = Clamp01(anim_t_ * (block_duration_ / sprite_duration_));
  const float sprite_e = EaseOutCubic(sprite_t);
  const float arc = 4.0f * sprite_t * (1.0f - sprite_t);
  const float sprite_x = Lerp(from.x, to.x, sprite_e);
  const float sprite_y = Lerp(from.y, to.y, sprite_e) - hop_height_ * cs * arc;

  // Background block: textured fill + pixelated dark outline.
  const float bg_size = cs * 0.86f;
  const float bg_half = bg_size * 0.5f;
  const ::Rectangle bg_dst{block_x - bg_half, block_y - bg_half, bg_size,
                           bg_size};
  const ::Rectangle bg_src{0, 0, static_cast<float>(block_.GetWidth()),
                           static_cast<float>(block_.GetHeight())};
  block_.DrawPro(bg_src, bg_dst, {0, 0}, 0.0f, ck::WHITE);
  ck::DrawRectangleLinesEx(bg_dst, 3.0f, ::Color{20, 40, 90, 255});

  // Sprite, centered on hop-offset position.
  const float sp_size = cs * 0.9f;
  const ::Rectangle sp_dst{sprite_x - sp_size * 0.5f,
                            sprite_y - sp_size * 0.5f, sp_size, sp_size};
  const ::Rectangle sp_src{0, 0, static_cast<float>(sprite_.GetWidth()),
                            static_cast<float>(sprite_.GetHeight())};
  sprite_.DrawPro(sp_src, sp_dst, {0, 0}, 0.0f, ck::WHITE);

  // HP bar floating above the block.
  const float bar_w = cs * 0.86f;
  const float bar_h = 8.0f;
  const float bar_x = block_x - bar_w * 0.5f;
  const float bar_y = block_y - bg_half - bar_h - 6.0f;
  ck::DrawRectangleRec({bar_x - 2, bar_y - 2, bar_w + 4, bar_h + 4},
                       ::Color{30, 30, 30, 220});
  ck::DrawRectangleRec({bar_x, bar_y, bar_w, bar_h}, ::Color{60, 60, 60, 255});
  const float ratio =
      max_hp_ > 0 ? static_cast<float>(hp_) / static_cast<float>(max_hp_) : 0.0f;
  ck::DrawRectangleRec({bar_x, bar_y, bar_w * ratio, bar_h},
                       ::Color{220, 50, 50, 255});
}

}  // namespace ck
