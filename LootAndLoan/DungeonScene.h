#ifndef DUNGEONSCENE_H
#define DUNGEONSCENE_H

#include "engine/Game.h"
#include "engine/Scene.h"

typedef struct DungeonScen {

} DungeonScen;

void ds_update(DungeonScen *ds, Game *g, float dt) {
  g_set_background_color(g, rgb(64 + fabs(30 * sin(g_time(g))), 64, 78));
}

void ds_draw(DungeonScen *ds, Game *g) {}

void DungeonScen_init(DungeonScen *ds, Game *g) {}

SceneTable DungeonScen_table = {
    .init = (SceneInitCB)DungeonScen_init,
    .update = (SceneUpdateCB)ds_update,
    .draw = (SceneDrawCB)ds_draw,
};
Scene DungeonScen_create() {
  DungeonScen *ds = g_malloc(sizeof(DungeonScen));
  *ds = (DungeonScen){};

  return (Scene){ds, &DungeonScen_table};
}

#endif