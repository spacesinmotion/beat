
#include "game/GameScene.h"
#include "extern/cjsonh/cjsonh.h"
#include "game/Game.h"
#include "game/SceneObject.h"
#include "game/assets.h"
#include "gc/gc.h"
#include "math.h"
#include "math/Color.h"
#include "math/Rect.h"
#include "math/Vec2.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifndef M_PI
#define M_PI 3.1457
#endif

const char *str(const char *format, ...) {
  static char b[256] = {0};
  va_list args;
  va_start(args, format);
  vsnprintf(b, sizeof(b), format, args);
  va_end(args);
  return b;
}

void so_vec_push(SceneObjectVec *vec, SceneObject so) {
  if (vec->len + 1 > vec->cap) {
    vec->cap += 16;
    vec->data = (SceneObject *)gc_realloc(&gc, vec->data, vec->cap * sizeof(SceneObject));
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

void gs_toggle_pause(GameScene *gs, int id) { (void)id, gs->game_paused = !gs->game_paused; }
void gs_set_game_speed(GameScene *gs, int speed) {
  gs->game_paused = false;
  gs->game_speed = (float)speed;
}

void gs_select_bottom_menu(GameScene *gs, int button) { gs->menu_selected = button; }

bool gs_pick(GameScene *gs, OnClickCB onclick, int id, Vec2 p, float s) {
  assert(gs->pick_rect_count < (int)(sizeof(gs->pick_rects) / sizeof(PickRect)));
  Recti r = (Recti){p.x - 8 * s, p.y - 8 * s, 16 * s, 16 * s};
  gs->pick_rects[gs->pick_rect_count] = (PickRect){.rect = r, .click = onclick, .id = id};
  ++gs->pick_rect_count;
  return ri_contains(r, gs->mouse_overlay_position.x, gs->mouse_overlay_position.y);
}

void gs_update(GameScene *gs, Game *g, float dt) {
  (void)g;

  dt = gs->game_paused ? 0.0f : dt * gs->game_speed;
  g_set_background_color(g, rgb(66, 64, 78));

  for (int i = 0; i < gs->scene_objects.len; ++i)
    so_update(&gs->scene_objects.data[i], gs, g, dt);

  so_vec_filter_dead(&gs->scene_objects);

  qsort(gs->scene_objects.data, gs->scene_objects.len, sizeof(SceneObject), so_render_order_compare);

  // if (gs->clicks != gs->click_counter_text_cache) {
  //   g_create_text(g, &gs->click_counter_text, Oswald_Regular_12, str("%.2d", gs->clicks));
  //   gs->click_counter_text_cache = gs->clicks;
  // }
  // if (gs->resource_pool.water != gs->water_counter_text_cache) {
  //   g_create_text(g, &gs->water_counter_text, Oswald_Regular_8, str("%d", gs->resource_pool.water));
  //   gs->water_counter_text_cache = gs->resource_pool.water;
  // }
  // if (gs->resource_pool.food != gs->food_counter_text_cache) {
  //   g_create_text(g, &gs->food_counter_text, Oswald_Regular_8, str("%d", gs->resource_pool.food));
  //   gs->food_counter_text_cache = gs->resource_pool.food;
  // }
  // if (gs->resource_pool.construction_material != gs->construction_material_counter_text_cache) {
  //   g_create_text(g, &gs->construction_material_counter_text, Oswald_Regular_8,
  //                 str("%d", gs->resource_pool.construction_material));
  //   gs->construction_material_counter_text_cache = gs->resource_pool.construction_material;
  // }
  // const int free_storage = gs_free_storage(gs);
  // if (gs->free_storage_text_cache != free_storage) {
  //   g_create_text(g, &gs->free_storage_text, Oswald_Regular_8,
  //                 str("%d/%d", gs->storage_size - free_storage, gs->storage_size));
  //   gs->free_storage_text_cache = free_storage;
  // }
  // if (gs->day != gs->day_counter_text_cache) {
  //   g_create_text(g, &gs->day_counter_text, Oswald_Regular_12, str("day %d", gs->day));
  //   gs->day_counter_text_cache = gs->day;
  // }
  // if (gs->wearisome_count != gs->bot_counter_text_cache) {
  //   g_create_text(g, &gs->bot_counter_text, Oswald_Regular_12, str("%d", gs->wearisome_count));
  //   gs->bot_counter_text_cache = gs->wearisome_count;
  // }
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

bool gs_valid_gird(GameScene *gs, Sizei gp) {
  return gp.w >= 0 && gp.w < (int)(sizeof(gs->grid) / sizeof(gs->grid[0])) && gp.h >= 0 &&
         gp.h < (int)(sizeof(gs->grid[0]) / sizeof(gs->grid[0][0])) && gs->grid[gp.w][gp.h] != So_None;
}

bool gs_empty_gird(GameScene *gs, Sizei gp) { return gs_valid_gird(gs, gp) && gs->grid[gp.w][gp.h] == So_Empty; }

bool is_neighbor(Sizei gp, Sizei p) {
  const Sizei n[6] = {{gp.w + 1, gp.h},
                      {gp.w - 1, gp.h},
                      {gp.w, gp.h + 1},
                      {gp.w, gp.h - 1},
                      {gp.w + 1, gp.h + (gp.w & 1 ? 1 : -1)},
                      {gp.w - 1, gp.h + (gp.w & 1 ? 1 : -1)}};
  for (int i = 0; i < 6; ++i)
    if (p.w == n[i].w && p.h == n[i].h)
      return true;
  return false;
}

void gs_draw(GameScene *gs, Game *g) {
  Sizei gp = gs_scene_to_grid(g_mouse_in_scene(g));

  for (int i = 0; i < (int)(sizeof(gs->grid) / sizeof(gs->grid[0])); ++i)
    for (int j = 0; j < (int)(sizeof(gs->grid[0]) / sizeof(gs->grid[0][0])); ++j) {
      if (gs->grid[i][j] == So_None)
        continue;

      g_color(g, is_neighbor(gp, (Sizei){i, j}) ? gray(200) : (gp.w == i && gp.h == j ? gray(150) : white()));
      g_objectRS(g, g_animation_buffer(g), Img_menubar, 0, gs_grid_to_scene((Sizei){i, j}), 0.0, 1.0f);

      g_color(g, gray(50));
      if (gs->grid[i][j] != So_Empty)
        g_objectRS(g, g_animation_buffer(g), Img_menubar, gs->grid[i][j], gs_grid_to_scene((Sizei){i, j}), 0.0, 1.0f);
    }

  g_color(g, red());

  if (gs->menu_selected > 0) {
    if (gs_valid_gird(gs, gp)) {
      Vec2 p = gs_grid_to_scene(gp);
      g_objectRS(g, g_animation_buffer(g), Img_menubar, gs->menu_selected, p, 0.0f, 1.0f);
    }
  }
}

void gs_draw_menu_overlay(GameScene *gs, Game *g) {
  const Color cn = false ? gray(25) : gray(225);
  const Color ch = false ? gray(75) : gray(175);
  for (int i = 1; i < So_None; ++i) {
    const Vec2 p = (Vec2){8 + 4 + i * 16, 8 + 4};
    const bool hover = gs_pick(gs, gs_select_bottom_menu, i, p, 1.0f);
    g_color(g, hover ? gray(100) : (gs->menu_selected == i ? cn : ch));
    g_object(g, g_animation_buffer(g), Img_menubar, i % 16, p);
    g_color(g, hover ? red() : (gs->menu_selected == i ? green() : blue()));
    g_object(g, g_animation_buffer(g), Img_marker, hover ? g_frame(g) % 4 : i % 4, p);
  }
}

void gs_draw_overlay(GameScene *gs, Game *g) {
  gs->pick_rect_count = 0;
  gs_draw_menu_overlay(gs, g);
}

void gs_mouse_move(GameScene *gs, Game *g, Vec2 mp, Vec2 op) {
  (void)g, (void)mp;
  gs->mouse_overlay_position = op;

  gs->pick_under_mouse = -1;
  for (int i = 0; i < gs->pick_rect_count; ++i) {
    if (ri_contains(gs->pick_rects[i].rect, op.x, op.y)) {
      gs->pick_under_mouse = i;
      break;
    }
  }
}

void gs_mouse_down(GameScene *gs, Game *g, Vec2 mp, Vec2 op, int button) {
  (void)g, (void)op, (void)mp;

  if (button == 0) {
    if (gs->pick_under_mouse >= 0)
      gs->pick_rects[gs->pick_under_mouse].click(gs, gs->pick_rects[gs->pick_under_mouse].id);

    else if (gs->menu_selected > 0) {
      Sizei gp = gs_scene_to_grid(g_mouse_in_scene(g));
      printf("GRID: %d,%d %d\n", gp.w, gp.h, gs->menu_selected);
      if (gs_empty_gird(gs, gp))
        gs->grid[gp.w][gp.h] = (ObjectType)gs->menu_selected;
    }

  } else if (button == 1) {
    gs->menu_selected = -1;
  }
}

typedef enum GameKeys {
  PAUSE_KEY = 32,
  SPEED_1_KEY = 49,
  SPEED_2_KEY = 50,
  SPEED_4_KEY = 51,
  SPEED_8_KEY = 52,
} GameKeys;

void gs_key_up(GameScene *gs, Game *g, int key) {
  (void)g;

  if (key == PAUSE_KEY)
    gs_toggle_pause(gs, 0);
  else if (key == SPEED_1_KEY)
    gs_set_game_speed(gs, 1);
  else if (key == SPEED_2_KEY)
    gs_set_game_speed(gs, 2);
  else if (key == SPEED_4_KEY)
    gs_set_game_speed(gs, 4);
  else if (key == SPEED_8_KEY)
    gs_set_game_speed(gs, 8);
  else
    printf("KEY UP (%d)\n", key);
}

void gs_add_object(GameScene *gs, SceneObject so) { so_vec_push(&gs->scene_objects, so); }

void gs_to_json(CJHObject *o, void *ud);
void gs_from_json(CJHObjectR *o, const char *key, void *ud);

SceneTable GameScene_table = {
    .update = (SceneUpdateCB)gs_update,
    .draw = (SceneDrawCB)gs_draw,
    .draw_overlay = (SceneDrawCB)gs_draw_overlay,
    .mouse_move = (SceneMouseMoveCB)gs_mouse_move,
    .mouse_down = (SceneMouseCB)gs_mouse_down,
    .key_up = (SceneKeyCB)gs_key_up,
    .save = gs_to_json,
    .load = gs_from_json,
};
void GameScene_init(Game *g) {
  GameScene *gs = g_malloc(sizeof(GameScene));
  *gs = (GameScene){
      .scene_objects = (SceneObjectVec){NULL, 0, 0},
      .game_speed = 2.0f,
      .game_paused = false,
      .menu_selected = -1,
      .pick_rects = {},
      .pick_rect_count = 0,
      .pick_under_mouse = -1,
  };

  for (int i = 0; i < (int)(sizeof(gs->grid) / sizeof(gs->grid[0])); ++i)
    for (int j = 0; j < (int)(sizeof(gs->grid[0]) / sizeof(gs->grid[0][0])); ++j)
      gs->grid[i][j] = So_Empty;
  gs->grid[0][0] = gs->grid[2][0] = gs->grid[4][0] = gs->grid[6][0] = So_None;
  gs->grid[3][3] = So_Water;
  gs->grid[0][1] = gs->grid[6][6] = So_House;
  gs->grid[0][6] = gs->grid[6][1] = So_Trees;

  g_set_scene(g, (Scene){gs, &GameScene_table});
}

void gs_stuff_to_json(CJHObject *o, void *ud) {
  const Stuff *s = (Stuff *)ud;
  cjh_o_add_number(o, "water", s->water);
  cjh_o_add_number(o, "food", s->food);
  cjh_o_add_number(o, "construction_material", s->construction_material);
}

void gs_sceneobjects_to_json(CJHArray *a, void *ud) {
  SceneObjectVec *s = (SceneObjectVec *)ud;
  for (int i = 0; i < s->len; ++i)
    if (so_can_be_stored(&s->data[i]))
      cjh_a_add_object(a, (CJHWriteObjectCB)so_to_json, &s->data[i]);
}

void gs_to_json(CJHObject *o, void *ud) {
  GameScene *gs = (GameScene *)ud;
  cjh_o_add_array(o, "scene_objects", gs_sceneobjects_to_json, &gs->scene_objects);
  cjh_o_add_number(o, "game_speed", gs->game_speed);
  cjh_o_add_bool(o, "game_paused", gs->game_paused);
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

void gs_from_json(CJHObjectR *o, const char *key, void *ud) {
  GameScene *gs = (GameScene *)ud;

  if (streq(key, "scene_objects")) {
    printf("%s:\n", key);
    indent += 2;
    cjh_o_read_array(o, gs_sceneobjects_from_json, gs);
    indent -= 2;
  }

  else if (streq(key, "game_speed"))
    printf("%s: %g\n", key, cjh_o_read_number(o));

  else if (streq(key, "game_paused"))
    printf("%s: %s\n", key, (cjh_o_read_bool(o) ? "true" : "false"));

  else if (streq(key, "daytime_step"))
    printf("%s: %g\n", key, cjh_o_read_number(o));

  else if (streq(key, "daytime"))
    printf("%s: %g\n", key, cjh_o_read_number(o));

  else if (streq(key, "day"))
    printf("%s: %g\n", key, cjh_o_read_number(o));

  else if (streq(key, "clicks"))
    printf("%s: %g\n", key, cjh_o_read_number(o));
  else if (streq(key, "clicks_produced"))
    printf("%s: %g\n", key, cjh_o_read_number(o));
  else if (streq(key, "clicks_lost"))
    printf("%s: %g\n", key, cjh_o_read_number(o));

  else if (streq(key, "clicks_in_houses"))
    printf("%s: %g\n", key, cjh_o_read_number(o));

  else if (streq(key, "wearisome_count"))
    printf("%s: %g\n", key, cjh_o_read_number(o));

  else if (streq(key, "storage_size"))
    printf("%s: %g\n", key, cjh_o_read_number(o));
  else if (streq(key, "storage_claimed"))
    printf("%s: %g\n", key, cjh_o_read_number(o));

  else if (streq(key, "reasearch_needed"))
    printf("%s: %g\n", key, cjh_o_read_number(o));

  else {
    printf("%s: SKIP\n", key);
    cjh_o_skip(o);
  }
}
