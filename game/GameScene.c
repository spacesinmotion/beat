
#include "game/GameScene.h"
#include "engine/DrawTransformation.h"
#include "engine/Game.h"
#include "engine/Scene.h"
#include "engine/math/Color.h"
#include "engine/math/Rect.h"
#include "engine/math/Vec2.h"
#include "extern/cjsonh/cjsonh.h"
#include "game/Board.h"
#include "game/ObjectType.h"
#include "game/PointOverview.h"
#include "game/SceneObject.h"
#include "game/assets.h"
#include "game/effects/Bling.h"
#include "stdbool.h"
#include <stdlib.h>
#include <time.h>

#ifndef M_PI
#define M_PI 3.1457
#endif

void so_vec_push(SceneObjectVec *vec, SceneObject so) {
  if (vec->len + 1 > vec->cap) {
    vec->cap += 16;
    vec->data = (SceneObject *)g_realloc(vec->data, vec->cap * sizeof(SceneObject));
  }
  vec->data[vec->len] = so;
  vec->len++;
}

void so_vec_filter_dead(SceneObjectVec *vec) {
  for (int i = vec->len - 1; i >= 0; --i) {
    if (!vec->data[i].table->dead(vec->data[i].context))
      continue;
    vec->data[i] = vec->data[vec->len - 1];
    vec->data[vec->len - 1] = (SceneObject){NULL, NULL};
    --vec->len;
  }
}

int so_render_order_compare(const void *va, const void *vb) {
  const float a = so_render_order((SceneObject *)va);
  const float b = so_render_order((SceneObject *)vb);
  return a < b ? -1 : (a > b ? 1 : 0);
}

bool gs_pick(GameScene *gs, OnClickCB onclick, int id, Vec2 p, float s) {
  assert(gs->pick_rect_count < (int)(sizeof(gs->pick_rects) / sizeof(PickRect)));
  Rect r = (Rect){{p.x - 8 * s, p.y - 8 * s}, {16 * s, 16 * s}};
  gs->pick_rects[gs->pick_rect_count] = (PickRect){.rect = r, .click = onclick, .id = id};
  ++gs->pick_rect_count;
  return r_contains(r, gs->mouse_pos);
}

void gs_update(GameScene *gs, Game *g, float dt) {
  (void)g;

  gs->wobble_time = f_max(0.0f, gs->wobble_time - dt);

  g_set_background_color(g, rgb(66, 64, 78));

  for (int i = 0; i < gs->scene_objects.len; ++i)
    so_update(&gs->scene_objects.data[i], gs, g, dt);

  so_vec_filter_dead(&gs->scene_objects);

  qsort(gs->scene_objects.data, gs->scene_objects.len, sizeof(SceneObject), so_render_order_compare);

  po_update(gs->points, g);

  gs->dice[0].p = v_lerp_about(gs->dice[0].p, (Vec2){-30, 55}, 80.0f * dt);
  gs->dice[0].s = 0.1 * 2.0f + 0.9 * gs->dice[0].s;
  gs->dice[1].p = v_lerp_about(gs->dice[1].p, (Vec2){-30, 30}, 80.0f * dt);
  gs->dice[1].s = 0.1 * 1.5f + 0.9 * gs->dice[1].s;
}

Vec2 gs_grid_to_scene(Sizei s) {
  const float ox = 3.0f / 2.0f * 8.0f;
  const float oy = sqrt(3.0) * 8.0f;
  return (Vec2){s.w * ox, (s.w & 1 ? oy / 2.0f : 0.0f) + s.h * oy};
}

Sizei gs_scene_to_grid(Vec2 p) {
  const float ox = 3.0f / 2.0f * 8.0f;
  const float oy = sqrt(3.0) * 8.0f;
  const int i = (int)roundf(p.x / ox);
  return (Sizei){i, (int)roundf((p.y - (i & 1 ? oy / 2.0f : 0.0f)) / oy)};
}

void gs_draw_button(Game *g, Vec2 p, int icon, Color c, float s) {
  g_color(g, c);
  s = s + 0.01f * sin(8.0f * g_time(g));
  g_draw_icon(g, Img_menubar, 7, dt_psf(p, s));
  if (icon == So_Empty)
    return;
  g_color(g, gray(50));
  g_draw_icon(g, Img_menubar, icon, dt_psf(p, s));
}

void gs_update_allow_to_pick(GameScene *gs);

void gs_roll_dice(GameScene *gs, int id) {
  (void)id;
  gs->placed_dice = 0;
  if (id == 42) {
    gs->dice[0] = (DiceRoll){So_Empty, (Vec2){-30, 45}, 2.0f, false};
    gs->dice[1] = (DiceRoll){So_Empty, (Vec2){-30, 45}, 2.0f, false};
  } else {
    gs->dice[0] = (DiceRoll){(ObjectType)(rand() % 6), (Vec2){-30, 45}, 2.0f, false};
    gs->dice[1] = (DiceRoll){(ObjectType)(rand() % 6), (Vec2){-30, 45}, 2.0f, false};
  }

  if (gs->dice[0].o == So_Empty && gs->dice[1].o == So_Empty) {
    gs->can_select_dice = true;
  } else if (gs->dice[0].o == So_Empty) {
    ObjectType ot = gs->dice[1].o;
    gs->dice[1].o = gs->dice[0].o;
    gs->dice[0].o = ot;
  }
  gs->selected_dice = 0;
  gs->last_placed_dice_location = (Sizei){-1, -1};
  gs_update_allow_to_pick(gs);
}

void gs_switch_to_second_dice(GameScene *gs, int id) {
  (void)id;
  if (gs->placed_dice != 0 || gs->dice[1].o == So_Empty)
    return;
  DiceRoll d = gs->dice[0];
  gs->dice[0] = gs->dice[1];
  gs->dice[1] = d;
}

void gs_mark_empty_neighbors_pickable(GameScene *gs, Sizei gp) {
  const Sizei n[6] = {{gp.w + 1, gp.h},
                      {gp.w - 1, gp.h},
                      {gp.w, gp.h + 1},
                      {gp.w, gp.h - 1},
                      {gp.w + 1, gp.h + (gp.w & 1 ? 1 : -1)},
                      {gp.w - 1, gp.h + (gp.w & 1 ? 1 : -1)}};
  for (int i = 0; i < 6; ++i)
    if (bd_grid_valid(gs->board, n[i]))
      bd_set_allowed_to_pick(gs->board, n[i], bd_grid_empty(gs->board, n[i]));
}

bool gs_has_pickable_neighbors(GameScene *gs, Sizei gp) {
  const Sizei n[6] = {{gp.w + 1, gp.h},
                      {gp.w - 1, gp.h},
                      {gp.w, gp.h + 1},
                      {gp.w, gp.h - 1},
                      {gp.w + 1, gp.h + (gp.w & 1 ? 1 : -1)},
                      {gp.w - 1, gp.h + (gp.w & 1 ? 1 : -1)}};
  for (int i = 0; i < 6; ++i)
    if (bd_allowed_to_pick(gs->board, n[i]))
      return true;
  return false;
}

void gs_update_allow_to_pick(GameScene *gs) {
  for (int i = 0; i < 7; ++i)
    for (int j = 0; j < 7; ++j)
      bd_set_allowed_to_pick(gs->board, (Sizei){i, j}, false);

  if ((gs->dice[0].o == So_Empty && gs->dice[1].o == So_Empty) ||
      (gs->dice[0].o == So_None && gs->dice[1].o == So_None))
    return;

  if (gs->placed_dice == 1) {
    gs_mark_empty_neighbors_pickable(gs, gs->last_placed_dice_location);
    return;
  }

  for (int i = 0; i < 7; ++i)
    for (int j = 0; j < 7; ++j) {
      const Sizei p = {i, j};
      if (bd_grid_valid(gs->board, p) && !bd_grid_empty(gs->board, p))
        gs_mark_empty_neighbors_pickable(gs, p);
    }

  if (gs->dice[1].o != So_Empty && gs->dice[1].o != So_None) {
    for (int i = 0; i < 7; ++i)
      for (int j = 0; j < 7; ++j) {
        Sizei p = {i, j};
        if (bd_allowed_to_pick(gs->board, p) && !gs_has_pickable_neighbors(gs, p))
          bd_set_allowed_to_pick(gs->board, p, false);
      }
  }

  gs->no_move_left = true;
  for (int i = 0; i < 7; ++i)
    for (int j = 0; j < 7; ++j)
      if (bd_allowed_to_pick(gs->board, (Sizei){i, j}))
        gs->no_move_left = false;
}

void gs_revert_first_placed_dice(GameScene *gs, int id) {
  (void)id;
  if (gs->placed_dice != 1)
    return;
  const Sizei p = gs->last_placed_dice_location;
  bd_set_grid(gs->board, p, So_Empty);
  gs->placed_dice = 0;
  gs->last_placed_dice_location = (Sizei){-1, -1};

  bd_count_points(gs->board, gs->points);
  gs_update_allow_to_pick(gs);

  DiceRoll d = gs->dice[0];
  gs->dice[0] = gs->dice[1];
  gs->dice[1] = d;
}

void gs_custom_dice_select(GameScene *gs, int id) {
  gs->placed_dice = 0;
  gs->dice[0] = (DiceRoll){(ObjectType)id, (Vec2){-30, 45}, 2.0f, false};
  gs->dice[1] = (DiceRoll){So_Empty, (Vec2){-30, 45}, 2.0f, false};
  gs->selected_dice = 0;
  gs->last_placed_dice_location = (Sizei){-1, -1};
  gs->can_select_dice = false;
  gs_update_allow_to_pick(gs);
}

typedef void (*OnClickCB)(GameScene *, int id);

void gs_reset_level(GameScene *gs, int x) {
  (void)x;
  gs->dice[0] = gs->dice[1] = (DiceRoll){So_None, {0, 0}, 1.0f, false};
  gs->selected_dice = -1;
  gs->placed_dice = 0;
  gs->no_move_left = false;
  gs->last_placed_dice_location = (Sizei){-1, -1};

  bd_reset(gs->board);
  bd_count_points(gs->board, gs->points);
  gs_update_allow_to_pick(gs);
}

void gs_draw(GameScene *gs, Game *g) {
  gs->pick_rect_count = 0;

  const Sizei gp = gs_scene_to_grid(g_mouse_in_scene(g));

  for (int i = 0; i < 7; ++i)
    for (int j = 0; j < 7; ++j) {
      if (bd_get_grid(gs->board, (Sizei){i, j}) == So_None)
        continue;

      const float x = gs->no_move_left ? 1.0f : gs->wobble_time * gs->wobble_time;
      Sizei p = {i, j};
      if (bd_allowed_to_pick(gs->board, p)) {
        g_color(g, (gp.w == i && gp.h == j) ? rgb(0xff, 0xda, 0x89) : white());
      } else
        g_color(g, gray(200 + x * 10 * sin(5.0f * g_time(g) + i - j)));

      float s = 1.0f + x * 0.04f * sin(10.0f * g_time(g) + i + j);
      g_draw_icon(g, Img_menubar, 0, dt_psf(gs_grid_to_scene((Sizei){i, j}), s));

      const bool last = (gs->last_placed_dice_location.w == i && gs->last_placed_dice_location.h == j);
      g_color(g, last ? red() : gray(50));
      if (bd_get_grid(gs->board, (Sizei){i, j}) != So_Empty) {
        const float r = x * 0.04f * sin(100.0f + 40.0f * cos(g_time(g)) + i * j);
        g_draw_icon(g, Img_menubar, bd_get_grid(gs->board, (Sizei){i, j}),
                    dt_prsf(gs_grid_to_scene((Sizei){i, j}), r, 1.0f));
      }
    }

  g_color(g, white());
  const float d = 3.0f / 2.0f * 8.0f * 6.0f;
  g_draw_rect(g, dt_ps((Vec2){-1, -11}, (Vec2){d + 2, 4 + 2}));
  g_color(g, rgb(0xff, 0xda, 0x89));
  g_draw_rect(g, dt_ps((Vec2){0, -10}, (Vec2){d * bd_fill_ratio(gs->board), 4}));

  g_color(g, red());

  if (gs->selected_dice >= 0 && bd_allowed_to_pick(gs->board, gp)) {
    Vec2 p = gs_grid_to_scene(gp);
    g_draw_icon(g, Img_menubar, gs->dice[gs->selected_dice].o, dt_p(p));
  }

  po_draw(gs->points, g, gs->no_move_left);

  if (gs->no_move_left) {
    bool h = gs_pick(gs, gs_reset_level, 0, (Vec2){-30, 45}, 2.0f);
    gs_draw_button(g, (Vec2){-30, 45}, 9, h ? rgb(0xff, 0xda, 0x89) : gray(220), 2.0f);
    return;
  }

  if (gs->can_select_dice) {
    for (int i = 1; i <= 5; ++i) {
      Vec2 p = (Vec2){-30, 45 + (i - 3) * 18};
      bool h = gs_pick(gs, gs_custom_dice_select, i, p, 1.0f);
      gs_draw_button(g, p, (ObjectType)i, h ? rgb(0xff, 0xda, 0x89) : gray(170), 1.0f);
    }
  } else if (gs->dice[0].o == So_None && gs->dice[1].o == So_None) {
    bool h = gs_pick(gs, gs_roll_dice, 0, (Vec2){-30, 45}, 2);
    gs_draw_button(g, (Vec2){-30, 45}, 8, h ? rgb(0xff, 0xda, 0x89) : gray(220), 2.0f);
  } else {
    if (gs->dice[1].o != So_None) {
      bool h = gs_pick(gs, gs->placed_dice == 0 ? gs_switch_to_second_dice : gs_revert_first_placed_dice, 1,
                       (Vec2){-30, 25}, 2);
      gs_draw_button(g, gs->dice[1].p, gs->dice[1].o, h || gs->selected_dice == 1 ? rgb(0xff, 0xda, 0x89) : gray(170),
                     gs->dice[1].s);
    }
    if (gs->dice[0].o != So_None)
      gs_draw_button(g, gs->dice[0].p, gs->dice[0].o, rgb(0xff, 0xda, 0x89), gs->dice[0].s);
  }

  for (int i = 0; i < gs->scene_objects.len; ++i)
    so_draw(&gs->scene_objects.data[i], gs, g);
}

void gs_draw_menu_overlay(GameScene *gs, Game *g) { (void)g, (void)gs; }

void gs_draw_overlay(GameScene *gs, Game *g) { (void)g, (void)gs; }

void gs_mouse_move(GameScene *gs, Game *g) {
  (void)g;
  gs->mouse_pos = g_mouse_in_scene(g);

  gs->pick_under_mouse = -1;
  for (int i = 0; i < gs->pick_rect_count; ++i) {
    if (r_contains(gs->pick_rects[i].rect, gs->mouse_pos)) {
      gs->pick_under_mouse = i;
      break;
    }
  }
}

void gs_mouse_down(GameScene *gs, Game *g, int button) {
  (void)g;

  if (button == 0) {
    if (gs->pick_under_mouse >= 0)
      gs->pick_rects[gs->pick_under_mouse].click(gs, gs->pick_rects[gs->pick_under_mouse].id);

    else if (gs->selected_dice >= 0) {
      Sizei gp = gs_scene_to_grid(g_mouse_in_scene(g));
      if (bd_allowed_to_pick(gs->board, gp)) {
        bd_set_grid(gs->board, gp, gs->dice[gs->selected_dice].o);
        Bling_init(gs, gs_grid_to_scene(gp), rgb(0xFF, 0x00, 0x00));
        gs->wobble_time = 1.0f;

        if (gs->placed_dice == 0 && gs->dice[1].o != So_Empty) {
          gs_switch_to_second_dice(gs, 1);
          gs->last_placed_dice_location = gp;
          gs->placed_dice = 1;
        } else {
          gs->last_placed_dice_location = (Sizei){-1, -1};
          gs->dice[0].o = gs->dice[1].o = So_None;
          gs->selected_dice = -1;
          gs->placed_dice = 2;
        }

        bd_count_points(gs->board, gs->points);
        gs_update_allow_to_pick(gs);
      }
    }
  } else if (button == 1) {
    if (gs->placed_dice == 1)
      gs_revert_first_placed_dice(gs, 1);
  }
}

// typedef enum GameKeys {
//   PAUSE_KEY = 32,
//   SPEED_1_KEY = 49,
//   SPEED_2_KEY = 50,
//   SPEED_4_KEY = 51,
//   SPEED_8_KEY = 52,
// } GameKeys;

void gs_key_up(GameScene *gs, Game *g, int key) {
  (void)g, (void)gs;

  // if (key == 80)
  //   gs_roll_dice(gs, 42);

  printf("KEY UP (%d)\n", key);
}

void gs_add_object(GameScene *gs, SceneObject so) { so_vec_push(&gs->scene_objects, so); }

void gs_sceneobjects_to_json(CJHArray *a, void *ud) {
  SceneObjectVec *s = (SceneObjectVec *)ud;
  for (int i = 0; i < s->len; ++i)
    if (so_can_be_stored(&s->data[i]))
      cjh_a_add_object(a, (CJHWriteObjectCB)so_to_json, &s->data[i]);
}

void gs_to_json(CJHObject *o, GameScene *gs) {
  cjh_o_add_array(o, "scene_objects", gs_sceneobjects_to_json, &gs->scene_objects);
}

void gs_SceneObject_from_json(CJHObjectR *o, const char *key, void *ud) {
  (void)ud;

  if (false) { // streq(key, ScienceBuilding_table.type)) {
  } else {
    printf("%.*s%s: SKIP\n", indent, space, key);
    cjh_o_skip(o);
  }
}

void gs_sceneobjects_from_json(CJHArrayR *a, int index, void *ud) {
  (void)index;

  GameScene *gs = (GameScene *)ud;

  SceneObject so = {NULL, NULL};
  cjh_a_read_object(a, (CJHReadObjectCB)gs_SceneObject_from_json, &so);

  if (so.context && so.table)
    gs_add_object(gs, so);
}

void gs_from_json(CJHObjectR *o, const char *key, GameScene *gs) {
  if (streq(key, "scene_objects")) {
    printf("%s:\n", key);
    indent += 2;
    cjh_o_read_array(o, gs_sceneobjects_from_json, gs);
    indent -= 2;
  }

  else {
    printf("%s: SKIP\n", key);
    cjh_o_skip(o);
  }
}

SceneTable GameScene_table = {
    .update = (SceneUpdateCB)gs_update,
    .draw = (SceneDrawCB)gs_draw,
    .draw_overlay = (SceneDrawCB)gs_draw_overlay,
    .mouse_move = (SceneMouseMoveCB)gs_mouse_move,
    .mouse_down = (SceneMouseCB)gs_mouse_down,
    .key_up = (SceneKeyCB)gs_key_up,
    .save = (SceneSaveCB)gs_to_json,
    .load = (SceneLoadCB)gs_from_json,
};
void GameScene_init(Game *g) {
  GameScene *gs = g_malloc(sizeof(GameScene));
  *gs = (GameScene){
      .scene_objects = (SceneObjectVec){NULL, 0, 0},
      .pick_rects = {},
      .pick_rect_count = 0,
      .pick_under_mouse = -1,
      .board = NULL,
      .points = NULL,
      .dice =
          {
              {So_None, {0, 0}, 1.0f, false},
              {So_None, {0, 0}, 1.0f, false},
          },
      .selected_dice = -1,
      .placed_dice = 0,
      .last_placed_dice_location = {-1, -1},
      .can_select_dice = false,
      .wobble_time = 1.0f,
  };
  gs->board = g_malloc(sizeof(Board));
  gs->points = PointOverview_init(g);

  gs_reset_level(gs, 0);
  gs->no_move_left = true;

  g_set_scene(g, (Scene){gs, &GameScene_table});
}