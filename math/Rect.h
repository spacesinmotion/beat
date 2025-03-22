#ifndef RECT
#define RECT

#include "Vec2.h"
#include "extern/cjsonh/cjsonh.h"
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

static inline void pi_to_json(CJHArray *a, Point *pi) {
  cjh_a_add_number(a, pi->x);
  cjh_a_add_number(a, pi->y);
}

static inline bool p_eq(Point a, Point b) { return a.x == b.x && a.y == b.y; }

typedef struct Sizei {
  int w, h;
} Sizei;

typedef struct Recti {
  int x, y, w, h;
} Recti;

static inline Point ri_bottom_right(Recti r) { return (Point){r.x, r.y}; }

static inline bool ri_contains(Recti r, int x, int y) { return r.x <= x && r.y <= y && r.x + r.w > x && r.y + r.h > y; }

static inline void ri_set_size(Recti *r, Sizei s) {
  r->w = s.w;
  r->h = s.h;
}

static inline void ri_to_json(CJHArray *a, Recti *ri) {
  cjh_a_add_number(a, ri->x);
  cjh_a_add_number(a, ri->y);
  cjh_a_add_number(a, ri->w);
  cjh_a_add_number(a, ri->h);
}

#endif
