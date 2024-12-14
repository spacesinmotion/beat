#ifndef CIRCLE
#define CIRCLE

#include "Vec2.h"
#include <stdbool.h>

typedef struct Circle {
  Vec2 center;
  float radius;
} Circle;

static inline bool Circle_contains(Circle c, Vec2 p) { return v_distance(c.center, p) <= c.radius; }
static inline bool Circle_distance(Circle c1, Circle c2) {
  return f_max(0.0f, v_distance(c1.center, c2.center) - c1.radius - c2.radius);
}

#endif
