
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

void gs_select_bottom_menu(GameScene *gs, int button) { gs->menu_selected = button; }

bool gs_pick(GameScene *gs, OnClickCB onclick, int id, Vec2 p, float s) {
  assert(gs->pick_rect_count < (int)(sizeof(gs->pick_rects) / sizeof(PickRect)));
  Recti r = (Recti){p.x - 8 * s, p.y - 8 * s, 16 * s, 16 * s};
  gs->pick_rects[gs->pick_rect_count] = (PickRect){.rect = r, .click = onclick, .id = id};
  ++gs->pick_rect_count;
  return ri_contains(r, gs->mouse_overlay_position.x, gs->mouse_overlay_position.y);
}

void gs_init_group_counter(GroupCounter *gc, Game *g) {
  g_create_text(g, &gc->text_3to5, Oswald_Regular_8, "3-5");
  g_create_text(g, &gc->text_6, Oswald_Regular_8, "6+");
  gc->g1_cache = gc->g2_cache = gc->points_cache = -1;
}

void gs_update_group_counter(GameScene *gs, Game *g, GroupCounter *gc) {
  (void)gs;
  (void)g;

  if (gc->g1 != gc->g1_cache) {
    g_create_text(g, &gc->text_g1, Oswald_Regular_8, str("10x%d", gc->g1));
    gc->g1_cache = gc->g1;
  }
  if (gc->g2 != gc->g2_cache) {
    g_create_text(g, &gc->text_g2, Oswald_Regular_8, str("25x%d", gc->g2));
    gc->g2_cache = gc->g2;
  }
  if (gc->points != gc->points_cache) {
    g_create_text(g, &gc->text_points, Oswald_Regular_12, str("%d", gc->points));
    gc->points_cache = gc->points;
  }
}

void gs_draw_group_counter(GroupCounter *gc, Game *g, Vec2 p, int icon) {
  g_color(g, gray(170));
  g_objectRS(g, g_animation_buffer(g), Img_menubar, icon, p, 0.0, 0.7f);
  g_text(g, gc->text_3to5, Oswald_Regular_8, v_add(p, (Vec2){5, -3}));
  g_objectRS(g, g_animation_buffer(g), Img_menubar, icon, v_add(p, (Vec2){20, 0}), 0.0, 0.7f);
  g_text(g, gc->text_6, Oswald_Regular_8, v_add(p, (Vec2){25, -3}));

  g_text(g, gc->text_g1, Oswald_Regular_8, v_add(p, (Vec2){-4, -10}));
  g_text(g, gc->text_g2, Oswald_Regular_8, v_add(p, (Vec2){16, -10}));

  g_color(g, gray(220));
  g_text(g, gc->text_points, Oswald_Regular_12, v_add(p, (Vec2){35, -6}));
}

void gs_update(GameScene *gs, Game *g, float dt) {
  (void)g;

  g_set_background_color(g, rgb(66, 64, 78));

  for (int i = 0; i < gs->scene_objects.len; ++i)
    so_update(&gs->scene_objects.data[i], gs, g, dt);

  so_vec_filter_dead(&gs->scene_objects);

  qsort(gs->scene_objects.data, gs->scene_objects.len, sizeof(SceneObject), so_render_order_compare);

  gs_update_group_counter(gs, g, &gs->house);
  gs_update_group_counter(gs, g, &gs->trees);
  gs_update_group_counter(gs, g, &gs->animals);
  gs_update_group_counter(gs, g, &gs->flowers);

  if (gs->points != gs->points_cache) {
    g_create_text(g, &gs->text_points, Oswald_Regular_12, str("%d", gs->points));
    gs->points_cache = gs->points;
  }
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

      g_color(g, (gp.w == i && gp.h == j && gs->grid[i][j] == So_Empty) ? rgb(0xff, 0xda, 0x89)
                                                                        : (3 == i && 3 == j ? gray(150) : white()));
      g_objectRS(g, g_animation_buffer(g), Img_menubar, 0, gs_grid_to_scene((Sizei){i, j}), 0.0, 1.0f);

      g_color(g, gray(50));
      if (gs->grid[i][j] != So_Empty)
        g_objectRS(g, g_animation_buffer(g), Img_menubar, gs->grid[i][j], gs_grid_to_scene((Sizei){i, j}), 0.0, 1.0f);
    }

  g_color(g, red());

  if (gs->menu_selected > 0) {
    if (gs_valid_gird(gs, gp) && gs->grid[gp.w][gp.h] == So_Empty) {
      Vec2 p = gs_grid_to_scene(gp);
      g_objectRS(g, g_animation_buffer(g), Img_menubar, gs->menu_selected, p, 0.0f, 1.0f);
    }
  }

  int row = 0;
  gs_draw_group_counter(&gs->house, g, (Vec2){100, 90 - (16 * row++)}, So_House);
  gs_draw_group_counter(&gs->trees, g, (Vec2){100, 90 - (16 * row++)}, So_Trees);
  gs_draw_group_counter(&gs->animals, g, (Vec2){100, 90 - (16 * row++)}, So_Animals);
  gs_draw_group_counter(&gs->flowers, g, (Vec2){100, 90 - (16 * row++)}, So_Flowers);
  g_text(g, gs->text_points, Oswald_Regular_12, (Vec2){135, 80 - (16 * row++)});
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

int gs_count_group_at(GameScene *gs, Sizei gp, ObjectType t) {
  if (!gs_valid_gird(gs, gp) || gs->visited[gp.w][gp.h] || t != gs->grid[gp.w][gp.h])
    return 0;

  const Sizei n[6] = {{gp.w + 1, gp.h},
                      {gp.w - 1, gp.h},
                      {gp.w, gp.h + 1},
                      {gp.w, gp.h - 1},
                      {gp.w + 1, gp.h + (gp.w & 1 ? 1 : -1)},
                      {gp.w - 1, gp.h + (gp.w & 1 ? 1 : -1)}};
  int count = 1;
  gs->visited[gp.w][gp.h] = true;
  for (int i = 0; i < 6; ++i)
    count += gs_count_group_at(gs, n[i], t);
  return count;
}

void gs_group_counter_add_group(GroupCounter *gc, int c) {
  if (c >= 6) {
    gc->g2 += 1;
    gc->points += 25;
  } else if (c >= 3) {
    gc->g1 += 1;
    gc->points += 10;
  }
}
void gs_count_points(GameScene *gs) {
  for (int i = 0; i < (int)(sizeof(gs->grid) / sizeof(gs->grid[0])); ++i)
    for (int j = 0; j < (int)(sizeof(gs->grid[0]) / sizeof(gs->grid[0][0])); ++j)
      gs->visited[i][j] = false;

  gs->house.g1 = gs->house.g2 = gs->house.points = 0;
  gs->trees.g1 = gs->trees.g2 = gs->trees.points = 0;
  gs->animals.g1 = gs->animals.g2 = gs->animals.points = 0;
  gs->flowers.g1 = gs->flowers.g2 = gs->flowers.points = 0;

  for (int i = 0; i < (int)(sizeof(gs->grid) / sizeof(gs->grid[0])); ++i) {
    for (int j = 0; j < (int)(sizeof(gs->grid[0]) / sizeof(gs->grid[0][0])); ++j) {
      if (gs->visited[i][j])
        continue;

      const ObjectType t = gs->grid[i][j];
      if (t == So_Empty || t == So_None || t == So_Water) {
        gs->visited[i][j] = true;
        continue;
      }

      const int c = gs_count_group_at(gs, (Sizei){i, j}, t);
      if (t == So_House) {
        gs_group_counter_add_group(&gs->house, c);
      } else if (t == So_Trees) {
        gs_group_counter_add_group(&gs->trees, c);
      } else if (t == So_Animals) {
        gs_group_counter_add_group(&gs->animals, c);
      } else if (t == So_Flowers) {
        gs_group_counter_add_group(&gs->flowers, c);
      }
    }
  }
  gs->points = gs->house.points + gs->trees.points + gs->animals.points + gs->flowers.points;
}

void gs_mouse_down(GameScene *gs, Game *g, Vec2 mp, Vec2 op, int button) {
  (void)g, (void)op, (void)mp;

  if (button == 0) {
    if (gs->pick_under_mouse >= 0)
      gs->pick_rects[gs->pick_under_mouse].click(gs, gs->pick_rects[gs->pick_under_mouse].id);

    else if (gs->menu_selected > 0) {
      Sizei gp = gs_scene_to_grid(g_mouse_in_scene(g));
      printf("GRID: %d,%d %d\n", gp.w, gp.h, gs->menu_selected);
      if (gs_empty_gird(gs, gp)) {
        gs->grid[gp.w][gp.h] = (ObjectType)gs->menu_selected;
        gs_count_points(gs);
      }
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
  (void)g, (void)gs;

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
      .menu_selected = -1,
      .pick_rects = {},
      .pick_rect_count = 0,
      .pick_under_mouse = -1,
      .house = (GroupCounter){0},
      .trees = (GroupCounter){0},
      .animals = (GroupCounter){0},
      .flowers = (GroupCounter){0},
      .text_points = {0},
      .points = 0,
      .points_cache = -1,
  };
  gs_init_group_counter(&gs->house, g);
  gs_init_group_counter(&gs->trees, g);
  gs_init_group_counter(&gs->animals, g);
  gs_init_group_counter(&gs->flowers, g);

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

  else {
    printf("%s: SKIP\n", key);
    cjh_o_skip(o);
  }
}
