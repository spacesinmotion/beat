#ifndef RECT
#define RECT

#include "Vec2.h"
#include <stdbool.h>

typedef struct Rect {
  Vec2 pos, size;
} Rect;

static inline bool Rect_contains(Rect r, Vec2 p) {
  return r.pos.x <= p.x && r.pos.y <= p.y && r.pos.x + r.size.x > p.x && r.pos.y + r.size.y > p.y;
}

#endif
