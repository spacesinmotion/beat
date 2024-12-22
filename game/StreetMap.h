#ifndef STREETMAP
#define STREETMAP

#include "game/Game.h"
#include "game/GameScene.h"
#include "game/Level.h"
#include "game/SceneObject.h"
#include "game/assets.h"
#include "gc/gc.h"
#include "math/Vec2.h"

#include <stdlib.h>

typedef struct StreetMap {
  const sg_image *texture;
  Level *level;

  Buffer *street_tile_map;
} StreetMap;

bool StreetMap_dead(StreetMap *sm) {
  (void)sm;
  return false;
}

void StreetMap_update(StreetMap *sm) {
  if (sm->street_tile_map)
    Buffer_free(sm->street_tile_map);
  sm->street_tile_map = create_tile_rect_buffer(LEVEL_WIDTH, LEVEL_HEIGHT, (IsSetCB)Level_movable, sm->level);
}

int street_tex_for(StreetMap *sm, int i, int j) {
  int k = 0;
  if (Level_movable(sm->level, i + 1, j))
    k += 1;
  if (Level_movable(sm->level, i, j + 1))
    k += 2;
  if (Level_movable(sm->level, i - 1, j))
    k += 4;
  if (Level_movable(sm->level, i, j - 1))
    k += 8;
  return k;
}

void StreetMap_draw(StreetMap *sm, Game *g) {
  g_noise(g, 0.0f);
  g_color(g, white());
  g_buffer(g, sm->street_tile_map, g_image(g, Img_tilemap), (Vec2){0, 0});

  for (int i = 0; i < LEVEL_WIDTH; ++i) {
    for (int j = 0; j < LEVEL_WIDTH; ++j) {
      TileType tc = Level_tile(sm->level, i, j);
      if (tc == T_PathStartEnd)
        g_color(g, red());
      else if (tc == T_Path)
        g_color(g, rgb(194, 130, 130));
      else
        continue;

      g_object(g, g_animation_buffer(g), sm->texture, v_mulf((Vec2){i * 16, j * 16}, 1.0f), 0.0f,
               street_tex_for(sm, i, j));
    }
  }
}

// SceneObjectTable StreetMap_table = (SceneObjectTable){
//     .dead = (SceneObjectDeadCB)StreetMap_dead,
//     .draw = (SceneObjectDrawCB)StreetMap_draw,
// };
StreetMap *StreetMap_init(Game *g, GameScene *gs) {
  StreetMap *sm = gc_malloc(&gc, sizeof(StreetMap));
  *sm = (StreetMap){
      .texture = g_image(g, Img_street),
      .level = gs->level,
      .street_tile_map = NULL,
  };

  int last_x = 2, last_y = 2;
  for (int n = 0; n < 5; ++n) {
    int x = 2 + rand() % (LEVEL_WIDTH - 4);
    int y = 2 + rand() % (LEVEL_HEIGHT - 4);

    int width = 3 + rand() % 6;
    int height = 3 + rand() % 6;
    int next_x = x + width / 2;
    int next_y = y + height / 2;

    if (n > 0) {
      if (rand() % 2 == 0) {
        for (int i = i_min(last_y, next_y); i <= i_max(last_y, next_y); ++i)
          Level_set_movable(sm->level, i_min(last_x, next_x), i, true);
        for (int i = i_min(last_x, next_x); i <= i_max(last_x, next_x); ++i)
          Level_set_movable(sm->level, i, last_x < next_x ? next_y : last_y, true);
      } else {
        for (int i = i_min(last_x, next_x); i <= i_max(last_x, next_x); ++i)
          Level_set_movable(sm->level, i, i_min(last_y, next_y), true);
        for (int i = i_min(last_y, next_y); i <= i_max(last_y, next_y); ++i)
          Level_set_movable(sm->level, last_y < next_y ? next_x : last_x, i, true);
      }
    }
    last_x = next_x;
    last_y = next_y;
  }

  StreetMap_update(sm);

  return sm;
}

#endif