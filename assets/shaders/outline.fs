#version 330

// Sample-the-neighbors UI outline. Caller renders the foreground (e.g. white
// text) onto a transparent RenderTexture, then draws the RT through this
// shader. For every transparent fragment we sweep a ring of samples at
// `uOutlineRadius` texels and emit `uOutlineColor` if any sample hits opaque
// foreground. Opaque fragments pass through unchanged.

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec2 uTexelSize;       // 1 / (rt_width, rt_height)
uniform float uOutlineRadius;  // ring radius in texels
uniform vec4 uOutlineColor;    // rgba, 0..1

out vec4 finalColor;

const int SAMPLES = 24;
const float TAU = 6.2831853;

void main() {
  vec4 center = texture(texture0, fragTexCoord);
  if (center.a > 0.0) {
    finalColor = center * fragColor;
    return;
  }
  float max_alpha = 0.0;
  for (int i = 0; i < SAMPLES; i++) {
    float a = TAU * float(i) / float(SAMPLES);
    vec2 off = vec2(cos(a), sin(a)) * uTexelSize * uOutlineRadius;
    max_alpha = max(max_alpha, texture(texture0, fragTexCoord + off).a);
  }
  if (max_alpha <= 0.0) discard;
  finalColor = vec4(uOutlineColor.rgb, max_alpha * uOutlineColor.a);
}
