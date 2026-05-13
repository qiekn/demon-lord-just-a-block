#include "player.hpp"

#include <algorithm>
#include <string>

#include "colors.hpp"
#include "font_default.hpp"
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
      block_(CK_ASSET("sprites/cards/card100.png")) {
  sprite_.SetFilter(::TEXTURE_FILTER_BILINEAR);
  block_.SetFilter(::TEXTURE_FILTER_BILINEAR);
}

void Player::Damage(int amount) { hp_ = std::max(0, hp_ - amount); }
void Player::Heal(int amount) { hp_ = std::min(max_hp_, hp_ + amount); }

bool Player::TryMove(int dx, int dy, const Grid& grid) {
  if (anim_t_ < 1.0f) return false;  // ignore input while tweening
  const GridCoord next{pos_.x + dx, pos_.y + dy};
  if (!grid.InBounds(next)) return false;
  prev_ = pos_;
  pos_ = next;
  if (dx != 0) facing_right_ = dx > 0;
  anim_t_ = 0.0f;
  return true;
}

void Player::Update(float dt, const Grid& grid) {
  // ImGui can write max_hp_ below hp_; snap hp back into range each frame.
  if (hp_ > max_hp_) hp_ = max_hp_;

  // Advance the tween FIRST so input can react to a tween that just finished
  // this frame, instead of losing a frame to it.
  if (anim_t_ < 1.0f) anim_t_ = Clamp01(anim_t_ + dt / tuning.block_duration);

  // Drain a buffered press the instant the tween is idle again. Done before
  // edge detection so a fresh press this frame doesn't overwrite the older
  // queued one — both get to execute in order.
  if (has_buffered_input_ && anim_t_ >= 1.0f) {
    TryMove(buffered_dx_, buffered_dy_, grid);
    awaiting_first_repeat_ = true;
    repeat_timer_ = 0.0f;
    has_buffered_input_ = false;
  }

  // Rising-edge taps: if the tween is idle, fire now; otherwise stash for the
  // next drain. Priority matches the held-key resolution below.
  int edge_dx = 0;
  int edge_dy = 0;
  if (::IsKeyPressed(::KEY_RIGHT) || ::IsKeyPressed(::KEY_D))
    edge_dx = 1;
  else if (::IsKeyPressed(::KEY_LEFT) || ::IsKeyPressed(::KEY_A))
    edge_dx = -1;
  else if (::IsKeyPressed(::KEY_DOWN) || ::IsKeyPressed(::KEY_S))
    edge_dy = 1;
  else if (::IsKeyPressed(::KEY_UP) || ::IsKeyPressed(::KEY_W))
    edge_dy = -1;

  if (edge_dx != 0 || edge_dy != 0) {
    if (anim_t_ >= 1.0f) {
      TryMove(edge_dx, edge_dy, grid);
      awaiting_first_repeat_ = true;
      repeat_timer_ = 0.0f;
      has_buffered_input_ = false;
    } else {
      buffered_dx_ = edge_dx;
      buffered_dy_ = edge_dy;
      has_buffered_input_ = true;
    }
  }

  // Auto-repeat for keys held across frames. Priority order keeps behavior
  // deterministic when diagonal-equivalent keys overlap.
  int dx = 0;
  int dy = 0;
  if (::IsKeyDown(::KEY_RIGHT) || ::IsKeyDown(::KEY_D))
    dx = 1;
  else if (::IsKeyDown(::KEY_LEFT) || ::IsKeyDown(::KEY_A))
    dx = -1;
  else if (::IsKeyDown(::KEY_DOWN) || ::IsKeyDown(::KEY_S))
    dy = 1;
  else if (::IsKeyDown(::KEY_UP) || ::IsKeyDown(::KEY_W))
    dy = -1;

  const bool held = (dx != 0 || dy != 0);
  if (!held) {
    awaiting_first_repeat_ = true;
    repeat_timer_ = 0.0f;
  } else {
    repeat_timer_ += dt;
    const float threshold =
        awaiting_first_repeat_ ? tuning.repeat_delay : tuning.repeat_interval;
    if (repeat_timer_ >= threshold) {
      if (TryMove(dx, dy, grid)) {
        repeat_timer_ = 0.0f;
        awaiting_first_repeat_ = false;
      }
    }
  }
}

void Player::Render(const Grid& grid) const {
  const float cs = grid.CellSize();
  const ::Vector2 from = grid.CellCenter(prev_);
  const ::Vector2 to = grid.CellCenter(pos_);

  // Block tween: ease-out across the full block_duration. Drags behind sprite.
  const float block_e = EaseOutCubic(anim_t_);
  const float block_x = Lerp(from.x, to.x, block_e);
  const float block_y = Lerp(from.y, to.y, block_e);

  // Sprite tween: completes in sprite_duration (< block_duration) and then
  // idles on the destination cell while the block catches up. Hop height is
  // direction-dependent — horizontal moves get a taller arc because vertical
  // moves already provide their own y travel and a big arc on top reads odd.
  const float sprite_t =
      Clamp01(anim_t_ * (tuning.block_duration / tuning.sprite_duration));
  const float sprite_e = EaseOutCubic(sprite_t);
  const float arc = 4.0f * sprite_t * (1.0f - sprite_t);
  const float sprite_x = Lerp(from.x, to.x, sprite_e);
  const bool horizontal = from.y == to.y;
  const float hop = horizontal ? tuning.hop_height_horizontal : tuning.hop_height;
  const float sprite_y = Lerp(from.y, to.y, sprite_e) - hop * cs * arc;

  // Background block: textured fill + pixelated dark outline.
  const float bg_size = cs;
  const float bg_half = bg_size * 0.5f;
  const ::Rectangle bg_dst{block_x - bg_half, block_y - bg_half, bg_size,
                           bg_size};
  const ::Rectangle bg_src{0, 0, static_cast<float>(block_.GetWidth()),
                           static_cast<float>(block_.GetHeight())};
  block_.DrawPro(bg_src, bg_dst, {0, 0}, 0.0f, ck::WHITE);
  // ck::DrawRectangleLinesEx(bg_dst, 3.0f, ::Color{20, 40, 90, 255});

  // Sprite, centered on hop-offset position. Smaller than the cell so the
  // block frame around it stays visible — matches refs/player_move_anim/.
  // Negative source width flips the sprite horizontally for left-facing moves.
  const float sp_size = cs * tuning.sprite_scale;
  const ::Rectangle sp_dst{sprite_x - sp_size * 0.5f,
                            sprite_y - sp_size * 0.5f, sp_size, sp_size};
  const float sp_w = static_cast<float>(sprite_.GetWidth());
  const ::Rectangle sp_src{0, 0, facing_right_ ? sp_w : -sp_w,
                            static_cast<float>(sprite_.GetHeight())};
  sprite_.DrawPro(sp_src, sp_dst, {0, 0}, 0.0f, ck::WHITE);

  // HP bar: rounded red pill sitting on top of the block, black outline,
  // centered HP number. Sizes are proportional to cs so the bar scales from
  // windowed up to 4K (one cell ≈ 215 px) without going invisible or pixelating.
  const float bar_w = cs * 0.42f;
  const float bar_h = cs * 0.15f;
  const float bar_x = block_x - bar_w * 0.5f;
  const float bar_y = block_y - bg_half - bar_h * 0.5f;
  const ::Rectangle bar{bar_x, bar_y, bar_w, bar_h};
  const float roundness = 0.55f;
  const int segments = 12;
  const float stroke = std::max(1.5f, cs * 0.012f);
  ck::DrawRectangleRounded(bar, roundness, segments, ::Color{210, 55, 55, 255});
  ck::DrawRectangleRoundedLinesEx(bar, roundness, segments, stroke,
                                  ::Color{0, 0, 0, 255});

  const std::string hp_str = std::to_string(hp_);
  const int font_size = std::max(6, tuning.hp_font_size);
  const int text_w = ck::MeasureText(hp_str, font_size);
  const int text_x = static_cast<int>(bar_x + bar_w * 0.5f) - text_w / 2;
  const int text_y = static_cast<int>(bar_y + bar_h * 0.5f) - font_size / 2;
  ck::DrawText(hp_str.c_str(), text_x, text_y, font_size, ck::WHITE);
}

}  // namespace ck
