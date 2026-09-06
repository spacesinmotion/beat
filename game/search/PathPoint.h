#ifndef PATHPOINT_H
#define PATHPOINT_H

#include "engine/Game.h"
#include "engine/math/Vec2.h"

typedef struct PathPoint {
  Vec2 p;
  struct PathPoint *next;
} PathPoint;

PathPoint *PathPoint_init(Vec2 p, PathPoint *next) {
  PathPoint *pp = g_malloc(sizeof(PathPoint));
  *pp = (PathPoint){p, next};
  return pp;
}

#endif