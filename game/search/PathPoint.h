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

#endif