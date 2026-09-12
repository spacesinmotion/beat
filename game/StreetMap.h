#ifndef STREETMAP
#define STREETMAP

#include "SokEngWrap/SceneObject.h"
#include "SokEngWrap/math/Vec2.h"
#include "game/GameScene.h"
#include "game/Level.h"
#include "game/assets/textures.h"

typedef struct StreetMap {
  Level *level;

  G_Object street_tile_map;
} StreetMap;

Color Street_color() { return rgb(151, 151, 151); }

bool StreetMap_dead(StreetMap *sm) {
  (void)sm;
  return false;
}

void StreetMap_update(StreetMap *sm) {
  if (G_Object_valid(&sm->street_tile_map))
    G_Object_free(&sm->street_tile_map);
  sm->street_tile_map = create_tile_rect_buffer(LEVEL_WIDTH, LEVEL_HEIGHT, (IsSetCB)l_movable, sm->level);
}

int street_tex_for(StreetMap *sm, int i, int j) {
  int k = 0;
  if (l_movable(sm->level, i + 1, j))
    k += 1;
  if (l_movable(sm->level, i, j + 1))
    k += 2;
  if (l_movable(sm->level, i - 1, j))
    k += 4;
  if (l_movable(sm->level, i, j - 1))
    k += 8;
  return k;
}

void StreetMap_draw(StreetMap *sm, Game *g) {
  g_color(g, Street_color());
  g_buffer(g, sm->street_tile_map, Img_tilemap, (Vec2){0, 0});
}

StreetMap *StreetMap_init(GameScene *gs) {
  StreetMap *sm = g_malloc(sizeof(StreetMap));
  *sm = (StreetMap){
      .level = gs->level,
      .street_tile_map = {0},
  };

  StreetMap_update(sm);

  return sm;
}

#endif