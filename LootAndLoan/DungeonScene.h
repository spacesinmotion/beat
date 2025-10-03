#ifndef DUNGEONSCENE_H
#define DUNGEONSCENE_H

#include "engine/Game.h"
#include "engine/Scene.h"
#include "engine/math/Color.h"
#include "engine/math/Rect.h"
#include "engine/math/Vec2.h"
#include "game/assets.h"

typedef struct DungeonScene {
  DrawEntity *emo;
} DungeonScene;

Sizei ds_to_grid(Vec2 p) { return (Sizei){(int)((p.x + 8) / 16), (int)((p.y + 8) / 16)}; }
Vec2 ds_from_grid(Sizei s) { return (Vec2){s.w * 16.0f, s.h * 16.0f}; }

void ds_update(DungeonScene *ds, Game *g, float dt) {
  (void)ds, (void)dt;

  g_set_background_color(g, rgb(64 + fabs(30 * sin(g_time(g))), 64, 78));
}

void ds_draw(DungeonScene *ds, Game *g) {
  (void)ds;

  Sizei gp = ds_to_grid(g_mouse_in_scene(g));
  for (int i = 0; i < 8; ++i)
    for (int j = 0; j < 8; ++j) {
      g_color(g, alphaf((gp.w == i && gp.h == j) ? gray(200) : gray(100), 0.15f));
      g_draw_rect(g, dt_psf(v_sub(ds_from_grid((Sizei){i, j}), (Vec2){7.9f, 7.9f}), 15.8f));
    }

  Vec2 p = ds_from_grid((Sizei){2, 1});
  g_color(g, white());
  Vec2 s = {1.0 + 0.025 * sin(4 * g_time(g)), 1.0 - 0.015 * sin(4 * g_time(g))};
  g_draw_entity(g, ds->emo, Img_emo, dt_prs(p, 0.02 * sin(g_time(g)), s));

  p = ds_from_grid(ds_to_grid(g_mouse_in_scene(g)));
  g_color(g, red());
  g_draw_icon(g, Img_menubar, 9, dt_p(p));
}

void ds_init(DungeonScene *ds, Game *g) {
  ds->emo = g_sub_image(g, (Sizei){512, 512}, (Point){23, 25}, (Point[4]){{14, 4}, {33, 4}, {33, 31}, {14, 31}});
}
void ds_free(DungeonScene *ds, Game *g) { de_free(g, ds->emo); }

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