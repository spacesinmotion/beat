#ifndef DUNGEONSCENE_H
#define DUNGEONSCENE_H

#include "engine/Game.h"
#include "engine/Scene.h"
#include "engine/math/Color.h"
#include "engine/math/Rect.h"
#include "engine/math/Vec2.h"
#include "game/assets.h"

typedef struct DungeonScene {

} DungeonScene;

Sizei ds_to_grid(Vec2 p) { return (Sizei){(int)((p.x + 4) / 8), (int)((p.y + 4) / 8)}; }
Vec2 ds_from_grid(Sizei s) { return (Vec2){s.w * 8.0f, s.h * 8.0f}; }

void ds_update(DungeonScene *ds, Game *g, float dt) {
  (void)ds, (void)dt;

  g_set_background_color(g, rgb(64 + fabs(30 * sin(g_time(g))), 64, 78));
}

void ds_draw(DungeonScene *ds, Game *g) {
  (void)ds;

  Sizei gp = ds_to_grid(g_mouse_in_scene(g));
  for (int i = 0; i < 8; ++i)
    for (int j = 0; j < 8; ++j) {
      g_color(g, (gp.w == i && gp.h == j) ? gray(200) : gray(100));
      g_draw_rect(g, dt_psf(v_sub(ds_from_grid((Sizei){i, j}), (Vec2){3.9f, 3.9f}), 7.8f));
      g_color(g, (gp.w == i && gp.h == j) ? white() : gray(200));
      g_draw_icon(g, Img_menubar, 7, dt_psf(ds_from_grid((Sizei){i, j}), 0.5f));
    }

  Vec2 p = ds_from_grid(ds_to_grid(g_mouse_in_scene(g)));
  g_color(g, red());
  g_draw_icon(g, Img_menubar, 9, dt_psf(p, 0.5f));
}

void ds_init(DungeonScene *ds, Game *g) { (void)ds, (void)g; }
void ds_free(DungeonScene *ds, Game *g) { (void)ds, (void)g; }

SceneTable DungeonScenetable = {
    .init = (SceneInitCB)ds_init,
    .free = (SceneFreeCB)ds_free,
    .update = (SceneUpdateCB)ds_update,
    .draw = (SceneDrawCB)ds_draw,
};
Scene DungeonScene_create(Game *g) {
  DungeonScene *ds = g_malloc(g, sizeof(DungeonScene));
  *ds = (DungeonScene){};

  return (Scene){ds, &DungeonScenetable};
}

#endif