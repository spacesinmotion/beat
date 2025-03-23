#ifndef PATHPOINT_H
#define PATHPOINT_H

#include "game/Game.h"
#include "math/Vec2.h"

typedef struct PathPoint {
  Vec2 p;
  struct PathPoint *next;
} PathPoint;

PathPoint *PathPoint_init(Vec2 p, PathPoint *next) {
  PathPoint *pp = g_malloc(sizeof(PathPoint));
  *pp = (PathPoint){p, next};
  return pp;
}

static inline void pp_to_json(CJHArray *a, const PathPoint *pp) {
  const PathPoint *x = pp;
  while (x) {
    cjh_a_add_array(a, (CJHWriteArrayCB)v_to_json, (void *)&x->p);
    x = x->next;
  }
}

static inline void pp_from_json(CJHArrayR *a, int index, PathPoint *pp) {
  printf("%.*s%d:\n", indent, space, index);
  indent += 2;
  Vec2 p;
  cjh_a_read_array(a, (CJHReadArrayCB)v_from_json, &p);
  indent -= 2;
}
#endif