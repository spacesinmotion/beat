#ifndef DUNGEONSCENE_H
#define DUNGEONSCENE_H

#include "LootAndLoan/mobs/Kirc.h"
#include "LootAndLoan/mobs/MoveMarker.h"
#include "engine/Game.h"
#include "engine/interaction/Selectable.h"
#include "engine/math/Color.h"
#include "engine/math/Rect.h"
#include "engine/math/Vec2.h"
#include "engine/scene/SceneObjectVec.h"

typedef struct DungeonScene {
  SceneObjectVec scene_objects;

  Selectable mouse_hander;

  int turn;

  Kirc *kirc;
} DungeonScene;

Point ds_to_grid(Vec2 p) { return (Point){(int)((p.x + 8) / 16), (int)((p.y + 8) / 16)}; }
Vec2 ds_from_grid(Point s) { return (Vec2){s.x * 16.0f, s.y * 16.0f}; }
Rect ds_rect_from_grid(Point s) { return (Rect){{s.x * 16 - 8, s.y * 16 - 8}, {16, 16}}; }

void ds_add_object(DungeonScene *ds, Game *g, SceneObject so) { so_vec_push(&ds->scene_objects, g, so); }

int ds_turn(const DungeonScene *ds) { return ds->turn; }

void ds_move_player(DungeonScene *ds, Game *g, Point p) {
  ds->turn++;
  ds->kirc->position = ds_from_grid(p);
  ks_add_possible_actions(ds->kirc, ds, g);
}

void ds_set_selectable(DungeonScene *ds, Selectable sl) { ds->mouse_hander = sl; }

void ds_update(DungeonScene *ds, Game *g, float dt) {
  (void)dt;

  g_set_background_color(g, rgb(64 + fabs(30 * sin(g_time(g))), 64, 78));

  ds->mouse_hander = (Selectable){0};

  so_vec_update_all(&ds->scene_objects, g);
  so_vec_filter_dead(&ds->scene_objects, g);
  so_vec_sort_by_render_order(&ds->scene_objects);
}

void ds_draw(DungeonScene *ds, Game *g) {
  (void)ds;

  Point gp = ds_to_grid(g_mouse_in_scene(g));
  for (int i = 0; i < 8; ++i)
    for (int j = 0; j < 8; ++j) {
      g_color(g, alphaf((gp.x == i && gp.y == j) ? gray(100) : gray(100), 0.15f));
      g_draw_rect(g, dt_psf(v_sub(ds_from_grid((Point){i, j}), (Vec2){7.9f, 7.9f}), 15.8f));
    }

  so_vec_draw_all(&ds->scene_objects, g);
}

void ds_mouse_down(DungeonScene *ds, Game *g, int b) {
  if (b == 0 && sl_valid(&ds->mouse_hander))
    sl_click(&ds->mouse_hander, g);
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
  ks_add_possible_actions(ds->kirc, ds, g);

  g_set_scene(g, (Scene){ds, &DungeonScenetable});
}

#endif