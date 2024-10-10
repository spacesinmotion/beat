

#ifndef MAT4
#define MAT4

typedef struct Vec2 {
  float x, y;
} Vec2;

static inline Vec2 v_add(Vec2 a, Vec2 b) { return (Vec2){a.x + b.x, a.y + b.y}; }
static inline Vec2 v_sub(Vec2 a, Vec2 b) { return (Vec2){a.x - b.x, a.y - b.y}; }
static inline Vec2 v_mul(Vec2 a, Vec2 b) { return (Vec2){a.x * b.x, a.y * b.y}; }
static inline Vec2 v_dif(Vec2 a, Vec2 b) { return (Vec2){a.x / b.x, a.y / b.y}; }

static inline Vec2 v_mulf(Vec2 a, float v) { return (Vec2){a.x * v, a.y * v}; }
static inline Vec2 v_diff(Vec2 a, float v) { return (Vec2){a.x / v, a.y / v}; }

static inline Vec2 v_lerp(Vec2 a, Vec2 b, float t) { return (Vec2){a.x + (b.x - a.x) * t, a.y + (b.y - b.x) * t}; }

#endif