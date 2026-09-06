#ifndef RECTSEARCH_H
#define RECTSEARCH_H

#include "engine/math/Rect.h"
#include "game/GameScene.h"
#include "game/Level.h"
#include "game/search/PathPoint.h"

typedef struct RectSearch {
  Level *level;
  Recti start, destination;
  PathPoint *path;
} RectSearch;
bool rs_moveable(RectSearch *data, int x, int y) {
  return l_movable(data->level, x, y) || ri_contains(data->start, x, y) || ri_contains(data->destination, x, y);
}
bool rs_reached_goal(RectSearch *data, int x, int y) { return ri_contains(data->destination, x, y); }

void rs_build_path(RectSearch *data, int i, int j) { data->path = PathPoint_init(l_to_vec(i, j), data->path); }

PathPoint *find_path_from_rect_to_rect(GameScene *gs, Point p, Recti start, Recti destination) {
  RectSearch search_data = {gs->level, start, destination, NULL};
  l_bright_first(gs->level, p.x, p.y,
                 (SearchHandle){
                     &search_data,
                     (CanMoveCB)rs_moveable,
                     (GoalReachedCB)rs_reached_goal,
                     (PathCB)rs_build_path,
                 });
  return search_data.path;
}

PathPoint *find_path_to_rect(GameScene *gs, Point p, Recti destination) {
  return find_path_from_rect_to_rect(gs, p, (Recti){p.x, p.y, 1, 1}, destination);
}

#endif