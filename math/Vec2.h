#ifndef VEC2
#define VEC2

#include <math.h>
#include <stdbool.h>

typedef struct Vec2 {
  float x, y;
} Vec2;

static inline bool v_eq(Vec2 a, Vec2 b) { return a.x == b.x && a.y == b.y; }

static inline Vec2 v_add(Vec2 a, Vec2 b) { return (Vec2){a.x + b.x, a.y + b.y}; }
static inline Vec2 v_sub(Vec2 a, Vec2 b) { return (Vec2){a.x - b.x, a.y - b.y}; }
static inline Vec2 v_mul(Vec2 a, Vec2 b) { return (Vec2){a.x * b.x, a.y * b.y}; }
static inline Vec2 v_dif(Vec2 a, Vec2 b) { return (Vec2){a.x / b.x, a.y / b.y}; }

static inline Vec2 v_mulf(Vec2 a, float v) { return (Vec2){a.x * v, a.y * v}; }
static inline Vec2 v_diff(Vec2 a, float v) { return (Vec2){a.x / v, a.y / v}; }

static inline Vec2 v_lerp(Vec2 a, Vec2 b, float t) { return (Vec2){a.x + (b.x - a.x) * t, a.y + (b.y - b.x) * t}; }

static inline float v_length(Vec2 a) { return sqrt(a.x * a.x + a.y * a.y); }
static inline Vec2 v_normalized(Vec2 a) {
  const float l = v_length(a);
  return (l == 0.0f) ? (Vec2){0.0f, 0.0f} : v_diff(a, l);
}

static inline Vec2 v_lerp_about(Vec2 a, Vec2 b, float t) {
  const Vec2 v = v_sub(b, a);
  const float l = v_length(v);
  return (l <= t) ? b : v_add(a, v_mulf(v_diff(v, l), t));
}

#endif