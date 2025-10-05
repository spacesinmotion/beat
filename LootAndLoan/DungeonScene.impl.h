#ifndef DUNGEONSCENE_IMPL_H
#define DUNGEONSCENE_IMPL_H

#include "LootAndLoan/DungeonScene.h"
#include "LootAndLoan/mobs/Kahm.h"
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
  bool player_turn;

  Kirc *kirc;
  Kahm *mobs[3];

} DungeonScene;

void ds_add_object(DungeonScene *ds, Game *g, SceneObject so) { so_vec_push(&ds->scene_objects, g, so); }

int ds_turn(const DungeonScene *ds) { return ds->turn; }

void ds_payer_turn_finished(DungeonScene *ds, Game *g) {
  ds->player_turn = false;
  for (int i = 0; i < 3; ++i)
    kh_start_turn(ds->mobs[i], g, ds_to_grid(ds->kirc->destination));
  // kc_add_possible_actions(ds->kirc, ds, g);
}

void ds_move_player(DungeonScene *ds, Game *g, Point p) {
  ds->turn++;
  ds->kirc->destination = ds_from_grid(p);
}

void ds_set_selectable(DungeonScene *ds, Selectable sl) { ds->mouse_hander = sl; }

void ds_update(DungeonScene *ds, Game *g, float dt) {
  (void)dt;

  ds->mouse_hander = (Selectable){0};

  so_vec_update_all(&ds->scene_objects, g);
  so_vec_filter_dead(&ds->scene_objects, g);
  so_vec_sort_by_render_order(&ds->scene_objects);

  if (!ds->player_turn && ds->mobs[0]->turn_finished && ds->mobs[1]->turn_finished && ds->mobs[2]->turn_finished) {
    ds->player_turn = true;
    kc_add_possible_actions(ds->kirc, ds, g);
  }
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
  *ds = (DungeonScene){
      .scene_objects = so_vec_empty(),
      .turn = 0,
      .player_turn = true,
  };

  ds->kirc = Kirc_create(ds, g, ds_from_grid((Point){2, 1})),
  ds->mobs[0] = Kahm_create(ds, g, ds_from_grid((Point){7, 2}));
  ds->mobs[1] = Kahm_create(ds, g, ds_from_grid((Point){6, 7}));
  ds->mobs[2] = Kahm_create(ds, g, ds_from_grid((Point){1, 8}));

  g_set_scene(g, (Scene){ds, &DungeonScenetable});
  g_set_background_color(g, rgb(226, 226, 214));
}

DungeonScene *ds_get(Game *g) {
  assert(g_scene(g).table == &DungeonScenetable);
  return (DungeonScene *)g_scene(g).context;
}

#endif