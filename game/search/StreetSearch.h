#ifndef STREETSEARCH_H
#define STREETSEARCH_H

#include "game/GameScene.h"
#include "game/Level.h"
#include "game/TileContent.h"
#include "game/search/PathPoint.h"

typedef struct FindStreetSearch {
  Level *level;
  Recti start_rect;
  PathPoint *path;
} FindStreetSearch;

bool fs_moveable(FindStreetSearch *fs, int x, int y) {
  return l_movable(fs->level, x, y) || ri_contains(fs->start_rect, x, y);
}
bool fs_reached_goal(FindStreetSearch *fs, int x, int y) { return l_movable(fs->level, x, y); }
void fs_build_path(FindStreetSearch *fs, int i, int j) { fs->path = PathPoint_init(l_to_vec(i, j), fs->path); }

PathPoint *find_street(GameScene *gs, Recti start) {
  FindStreetSearch search_data = {gs->level, start, NULL};
  l_bright_first(gs->level, start.x, start.y,
                 (SearchHandle){
                     &search_data,
                     (CanMoveCB)fs_moveable,
                     (GoalReachedCB)fs_reached_goal,
                     (PathCB)fs_build_path,
                 });
  return search_data.path;
}

#endif