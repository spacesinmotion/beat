#ifndef VEC2
#define VEC2

#include "extern/cjsonh/cjsonh.h"
#include <X11/Xcursor/Xcursor.h>
#include <math.h>
#include <stdbool.h>

static inline int i_max(int a, int b) { return a < b ? b : a; }
static inline int i_min(int a, int b) { return a < b ? a : b; }

static inline float f_max(float a, float b) { return a < b ? b : a; }
static inline float f_min(float a, float b) { return a < b ? a : b; }

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

static inline Vec2 v_lerp(Vec2 a, Vec2 b, float t) { return (Vec2){a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t}; }

static inline float v_length(Vec2 a) { return sqrt(a.x * a.x + a.y * a.y); }
static inline float v_distance(Vec2 a, Vec2 b) { return v_length(v_sub(b, a)); }
static inline Vec2 v_normalized(Vec2 a) {
  const float l = v_length(a);
  return (l == 0.0f) ? (Vec2){0.0f, 0.0f} : v_diff(a, l);
}

static inline Vec2 v_lerp_about(Vec2 a, Vec2 b, float t) {
  const Vec2 v = v_sub(b, a);
  const float l = v_length(v);
  return (l <= t) ? b : v_add(a, v_mulf(v_diff(v, l), t));
}

static inline void v_to_json(CJHArray *a, const Vec2 *v) {
  cjh_a_add_number(a, v->x);
  cjh_a_add_number(a, v->y);
}

static inline void v_from_json(CJHArrayR *a, int index, Vec2 *v) {
  if (index == 0) {
    v->x = cjh_a_read_number(a);
    printf("%.*sx: %g\n", indent, space, v->x);
  } else if (index == 1) {
    v->y = cjh_a_read_number(a);
    printf("%.*sy: %g\n", indent, space, v->y);
  } else
    assert(false);
}

#endif