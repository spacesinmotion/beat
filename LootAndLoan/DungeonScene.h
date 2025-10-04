#ifndef DUNGEONSCENE_H
#define DUNGEONSCENE_H

#include "LootAndLoan/mobs/Kirc.h"
#include "engine/Game.h"
#include "engine/math/Color.h"
#include "engine/math/Rect.h"
#include "engine/math/Vec2.h"
#include "engine/scene/Scene.h"
#include "engine/scene/SceneObject.h"
#include "engine/scene/SceneObjectVec.h"
#include "game/assets.h"

typedef struct DungeonScene {
  SceneObjectVec scene_objects;

  Kirc *kirc;
} DungeonScene;

Point ds_to_grid(Vec2 p) { return (Point){(int)((p.x + 8) / 16), (int)((p.y + 8) / 16)}; }
Vec2 ds_from_grid(Point s) { return (Vec2){s.x * 16.0f, s.y * 16.0f}; }

void ds_add_object(DungeonScene *ds, Game *g, SceneObject so) { so_vec_push(&ds->scene_objects, g, so); }

void ds_update(DungeonScene *ds, Game *g, float dt) {
  (void)ds, (void)dt;

  g_set_background_color(g, rgb(64 + fabs(30 * sin(g_time(g))), 64, 78));

  so_vec_update_all(&ds->scene_objects, g);
  so_vec_filter_dead(&ds->scene_objects, g);
  so_vec_sort_by_render_order(&ds->scene_objects);
}

void ds_draw(DungeonScene *ds, Game *g) {
  (void)ds;

  Point gp = ds_to_grid(g_mouse_in_scene(g));
  for (int i = 0; i < 8; ++i)
    for (int j = 0; j < 8; ++j) {
      g_color(g, alphaf((gp.x == i && gp.y == j) ? gray(200) : gray(100), 0.15f));
      g_draw_rect(g, dt_psf(v_sub(ds_from_grid((Point){i, j}), (Vec2){7.9f, 7.9f}), 15.8f));
    }

  so_vec_draw_all(&ds->scene_objects, g);
  const Vec2 p = ds_from_grid(ds_to_grid(g_mouse_in_scene(g)));
  g_color(g, red());
  g_draw_icon(g, Img_menubar, 9, dt_p(p));
}

void ds_mouse_down(DungeonScene *ds, Game *g, int b) {
  if (b == 0) {
    Point p = ds_to_grid(g_mouse_in_scene(g));
    ds->kirc->position = ds_from_grid(p);
  }
}

void ds_free(DungeonScene *ds, Game *g) { so_vec_clear(&ds->scene_objects, g); }

SceneTable DungeonScenetable = {
    .free = (SceneFreeCB)ds_free,
    .update = (SceneUpdateCB)ds_update,
    .draw = (SceneDrawCB)ds_draw,
    .mouse_down = (SceneMouseCB)ds_mouse_down,
};
void DungeonScene_create(Game *g) {
  DungeonScene *ds = g_malloc(g, sizeof(DungeonScene));
  *ds = (DungeonScene){0};

  ds->kirc = Kirc_create(ds, g, ds_from_grid((Point){2, 1}));

  g_set_scene(g, (Scene){ds, &DungeonScenetable});
}

#endif