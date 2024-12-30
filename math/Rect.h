#ifndef RECT
#define RECT

#include "Vec2.h"
#include <stdbool.h>

typedef struct Rect {
  Vec2 pos, size;
} Rect;

static inline bool r_contains(Rect r, Vec2 p) {
  return r.pos.x <= p.x && r.pos.y <= p.y && r.pos.x + r.size.x > p.x && r.pos.y + r.size.y > p.y;
}

typedef struct Point {
  int x, y;
} Point;

typedef struct Size {
  int w, h;
} Size;

typedef struct Recti {
  int x, y, w, h;
} Recti;
static inline Point ri_bottom_right(Recti r) { return (Point){r.x, r.y}; }

static inline bool ri_contains(Recti r, int x, int y) { return r.x <= x && r.y <= y && r.x + r.w > x && r.y + r.h > y; }

#endif
