#ifndef RESOURCEPROVIDERSEARCH_H
#define RESOURCEPROVIDERSEARCH_H

#include "game/GameScene.h"
#include "game/Level.h"
#include "game/TileContent.h"
#include "game/search/PathPoint.h"
#include "math/Rect.h"

typedef struct ResourceProviderSearch {
  GameScene *gs;
  Recti start_rect;
  Resource resource;
  Point found;
  PathPoint *path;
} ResourceProviderSearch;

bool rps_moveable(ResourceProviderSearch *data, int x, int y) {
  if (l_movable(data->gs->level, x, y) || ri_contains(data->start_rect, x, y))
    return true;
  return tc_provides(l_content(data->gs->level, x, y), data->gs, data->resource);
}

bool rps_reached_goal(ResourceProviderSearch *data, int x, int y) {
  TileContent *c = l_content(data->gs->level, x, y);
  if (!tc_provides(c, data->gs, data->resource))
    return false;
  data->found = (Point){x, y};
  return true;
}

void rps_build_path(ResourceProviderSearch *data, int i, int j) {
  data->path = PathPoint_init(l_to_vec(i, j), data->path);
}

TileContent *find_resource_building(GameScene *gs, Recti start, Resource r) {
  ResourceProviderSearch search_data = {gs, start, r, {-1, -1}, NULL};
  l_bright_first(gs->level, start.x, start.y,
                 (SearchHandle){
                     &search_data,
                     (CanMoveCB)rps_moveable,
                     (GoalReachedCB)rps_reached_goal,
                     (PathCB)rps_build_path,
                 });
  return l_contentP(gs->level, search_data.found);
}

Recti find_resource_building_rect(GameScene *gs, Recti start, Resource r) {
  ResourceProviderSearch search_data = {gs, start, r, {-1, -1}, NULL};
  l_bright_first(gs->level, start.x, start.y,
                 (SearchHandle){
                     &search_data,
                     (CanMoveCB)rps_moveable,
                     (GoalReachedCB)rps_reached_goal,
                     (PathCB)rps_build_path,
                 });

  TileContent *c = l_contentP(gs->level, search_data.found);
  if (!c)
    return (Recti){0, 0, 0, 0};
  Recti d = {search_data.found.x, search_data.found.y, 1, 1};
  for (int i = 1; i < 10; ++i) {
    if (c == l_content(gs->level, search_data.found.x - i, search_data.found.y)) {
      d.x--;
      d.w++;
    }
    if (c == l_content(gs->level, search_data.found.x + i, search_data.found.y)) {
      d.w++;
    }
    if (c == l_content(gs->level, search_data.found.x, search_data.found.y - i)) {
      d.y--;
      d.h++;
    }
    if (c == l_content(gs->level, search_data.found.x, search_data.found.y + i)) {
      d.h++;
    }
  }
  return d;
}
#endif