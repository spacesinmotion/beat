
#include "game/GameScene.h"
#include "Scene.h"
#include "extern/cjsonh/cjsonh.h"
#include "game/Game.h"
#include "game/GameGo.h"
#include "game/Meteorite.h"
#include "game/Player.h"
#include "game/SceneObject.h"
#include "game/SpaceShip.h"
#include "game/Star.h"
#include "game/assets.h"
#include "game/effects/Bling.h"
#include "gc/gc.h"
#include "math.h"
#include "math/Color.h"
#include "math/Rect.h"
#include "math/Vec2.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

void gs_toggle_pause(GameScene *gs, Game *g, int id) {
  (void)id;
  (void)g;

  gs->game_paused = false;
}
void gs_set_game_speed(GameScene *gs, int speed) {
  gs->game_paused = false;
  gs->game_speed = (float)speed;
}

void gs_select_bottom_menu(GameScene *gs, Game *g, int button) {
  (void)g;
  gs->menu_selected = button;
  if (button == MI_Castle) {
  } else {
    gs->r.w = gs->r.h = 0;
  }
}

bool gs_pick(GameScene *gs, OnClickCB onclick, int id, Vec2 p, float s) {
  assert(gs->pick_rect_count < (int)(sizeof(gs->pick_rects) / sizeof(PickRect)));
  Recti r = (Recti){p.x - 8 * s, p.y - 8 * s, 16 * s, 16 * s};
  gs->pick_rects[gs->pick_rect_count] = (PickRect){.rect = r, .click = onclick, .id = id};
  ++gs->pick_rect_count;
  return ri_contains(r, gs->mouse_overlay_position.x, gs->mouse_overlay_position.y);
}

#define LEVEL_RANGE_FACTOR 50.0f

void gs_update(GameScene *gs, Game *g, float dt) {

  g_set_background_color(g, rgb(226, 219, 197));

  for (int i = 0; i < gs->scene_objects.len; ++i)
    so_update(&gs->scene_objects.data[i], gs, g, dt);
  so_vec_filter_dead(&gs->scene_objects);

  qsort(gs->scene_objects.data, gs->scene_objects.len, sizeof(SceneObject), so_render_order_compare);

  gs->points_flush *= 0.99f;
  if (gs->points != gs->points_cache) {
    gs->points_cache = gs->points;
    g_create_text(g, &gs->points_text, Oswald_Regular_12, str("%d", gs->points));
  }

  if (gs->initialized <= 0.0f)
    return;
  if (gs->initialized < 1.0f) {
    gs->initialized += 0.5f * dt;
    return;
  }
  gs->initialized = 1.0f;

  if (rand() % 1000 < (1.5 * gs->level))
    Meteorite_init(gs);

  gs->energy = f_max(0.0f, gs->energy - dt);
  if (gs->energy <= 0.0)
    gs->speed = f_max(0.0f, gs->speed - dt);
  else
    gs->speed = f_min(1.0f, gs->speed + 1.25f * dt);

  const int or = (int)gs->range;
  gs->range += gs->speed * dt;
  if (or < (int)gs->range)
    gs_add_points(gs, 1);

  if (gs->range >= gs->level * LEVEL_RANGE_FACTOR) {
    gs->range = 0;
    gs->level++;
  }
}

void gs_draw(GameScene *gs, Game *g) {
  c_printf(g, "\n\n\n\n\n\n\n\n\n\n");
  c_printf(g, "\n\n\n\n\n\n\n\n\n\n");

  for (int i = 0; i < gs->scene_objects.len; ++i)
    so_draw(&gs->scene_objects.data[i], gs, g);
}

void gs_draw_menu_overlay(GameScene *gs, Game *g) {
  const Color cn = gray(25);
  const Color ch = gray(75);
  for (int i = 0; i < Nb_MI; ++i) {
    const Vec2 p = (Vec2){8 + 4 + i * 16, 8 + 4};
    const bool hover = gs_pick(gs, gs_select_bottom_menu, i, p, 1.0f);
    g_color(g, hover ? gray(100) : (gs->menu_selected == i ? cn : ch));
    g_object(g, g_animation_buffer(g), Img_menubar, i % 16, p);
    g_color(g, hover ? red() : (gs->menu_selected == i ? green() : blue()));
    g_object(g, g_animation_buffer(g), Img_marker, hover ? g_frame(g) % 4 : i % 4, p);
  }
}

int center_num(int num) { return (num / 10 > 0) ? -10 : -8; }

void gs_draw_overlay(GameScene *gs, Game *g) {
  (void)g;
  gs->pick_rect_count = 0;
  // gs_draw_menu_overlay(gs, g);

  if (gs->initialized >= 1.0f) {
    g_color(g, c_mix(rgb(168, 159, 128), rgb(88, 136, 22), gs->points_flush * gs->points_flush));
    g_text(g, gs->points_text, Oswald_Regular_12, (Vec2){10, g_viewport(g).h - 15});
  }
  if (gs->initialized <= 0.0f)
    return;

  const Sizei vp = g_viewport(g);
  g_color(g, alphaf(gray(150), gs->initialized));
  g_objectS(g, g_rect_buffer(g), Img_starship, 15, (Vec2){19, 9}, (Vec2){vp.w - 38, 6});
  g_color(g, alphaf(yellow(), gs->initialized));
  const float r = gs->range / (LEVEL_RANGE_FACTOR * gs->level);
  g_objectS(g, g_rect_buffer(g), Img_starship, 15, (Vec2){20, 10}, (Vec2){r * (vp.w - 40), 4});

  g_color(g, alphaf(gray(150), gs->initialized));
  g_objectS(g, g_rect_buffer(g), Img_starship, 15, (Vec2){19, 19}, (Vec2){102, 6});
  g_color(g, alphaf(blue(), gs->initialized));
  g_objectS(g, g_rect_buffer(g), Img_starship, 15, (Vec2){20, 20}, (Vec2){gs->speed * 100.0f, 4});

  g_color(g, alphaf(gray(150), gs->initialized));
  g_objectS(g, g_rect_buffer(g), Img_starship, 15, (Vec2){19, 29}, (Vec2){102, 6});
  g_color(g, alphaf(green(), gs->initialized));
  const float e = gs->energy / 32.0f;
  g_objectS(g, g_rect_buffer(g), Img_starship, 15, (Vec2){20, 30}, (Vec2){e * 100.0f, 4});
}

void gs_mouse_move(GameScene *gs, Game *g, Vec2 mp, Vec2 op) {
  (void)g;
  gs->mouse_overlay_position = op;

  gs->r.x = (int)((mp.x + 8) / 16.0f);
  gs->r.y = (int)((mp.y + 8) / 16.0f);

  gs->pick_under_mouse = -1;
  for (int i = 0; i < gs->pick_rect_count; ++i) {
    if (ri_contains(gs->pick_rects[i].rect, op.x, op.y)) {
      gs->pick_under_mouse = i;
      break;
    }
  }
}

void gs_mouse_down(GameScene *gs, Game *g, Vec2 mp, Vec2 op, int button) {
  (void)g;
  (void)op;

  if (button == 0) {
    // const Point p = (Point){gs->r.x, gs->r.y};
    if (gs->pick_under_mouse >= 0)
      gs->pick_rects[gs->pick_under_mouse].click(gs, g, gs->pick_rects[gs->pick_under_mouse].id);

    else if (gs->menu_selected >= 0) {
      ; // ToDo
    } else {
      Bling_init(gs, mp, gray(45));
    }

  } else if (button == 1) {
    gs->menu_selected = -1;
    gs->r.w = gs->r.h = 0;
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
    gs_toggle_pause(gs, g, 0);
  else if (key == SPEED_1_KEY)
    gs_set_game_speed(gs, 1);
  else if (key == SPEED_2_KEY)
    gs_set_game_speed(gs, 2);
  else if (key == SPEED_4_KEY)
    gs_set_game_speed(gs, 4);
  else if (key == SPEED_8_KEY)
    gs_set_game_speed(gs, 8);
  else if (key == 259) { // backspace
    const size_t l = sizeof(gs->entered_until_now);
    assert(l == 32);

    for (size_t i = 1; i < l; ++i)
      gs->entered_until_now[l - i] = gs->entered_until_now[l - i - 1];
    gs->entered_until_now[0] = ' ';
    // printf("back'%.*s'\n", 32, gs->entered_until_now);
  } else
    printf("KEY UP (%d)\n", key);
}

void gs_char_enter(GameScene *gs, Game *g, uint32_t c) {
  (void)g;

  const size_t l = sizeof(gs->entered_until_now);
  assert(l == 32);

  if (isalnum(c)) {
    for (size_t i = 1; i < l; ++i)
      gs->entered_until_now[i - 1] = gs->entered_until_now[i];
    gs->entered_until_now[l - 1] = (char)(c % 255);
    // printf("set '%.*s'\n", 32, gs->entered_until_now);
  }
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
    .char_enter = (SceneCharCB)gs_char_enter,
    .save = gs_to_json,
    .load = gs_from_json,
};
void GameScene_init(Game *g) {
  GameScene *gs = g_malloc(sizeof(GameScene));
  *gs = (GameScene){
      .scene_objects = (SceneObjectVec){NULL, 0, 0},
      .game_speed = 2.0f,
      .game_paused = true,
      .menu_selected = -1,
      .user = {.resources = {}},
      .r = (Recti){-1, -1, 0, 0},
      .entered_until_now = {'_'},
      .entered_until_now_back = {'x'},
      .pick_rects = {},
      .pick_rect_count = 0,
      .pick_under_mouse = -1,
      .points = 0,
      .points_cache = -1,
      .points_flush = 0.0f,
      .initialized = 0.0f,
      .health = 100,
      .range = 0.0f,
      .speed = 1.0f,
      .energy = 5.0f,
      .level = 1,
  };

  gs->spaceship = SpaceShip_init(gs, (Vec2){150, 170});
  for (int i = 0; i < 40; ++i)
    Star_init(gs);

  GameGo_init(gs, g);

  g_set_scene(g, (Scene){gs, &GameScene_table});
}

void gs_stuff_to_json(CJHObject *o, void *ud) {
  const Stuff *s = (Stuff *)ud;
  (void)s;
  (void)o;
  // cjh_o_add_number(o, "money", s->money);
  // cjh_o_add_number(o, "food", s->food);
  // cjh_o_add_number(o, "wood", s->wood);
  // cjh_o_add_number(o, "irong", s->iron);
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
  cjh_o_add_object(o, "user", gs_stuff_to_json, &gs->user.resources);
}

void gs_stuff_from_json(CJHObjectR *o, const char *key, void *ud) {
  Stuff *s = (Stuff *)ud;
  // StreetMap *street_map;
  (void)s;
  // if (streq(key, "money"))
  //   s->money = cjh_o_read_number(o);
  // else if (streq(key, "food"))
  //   s->food = cjh_o_read_number(o);
  // else if (streq(key, "wood"))
  //   s->wood = cjh_o_read_number(o);
  // else if (streq(key, "iron"))
  //   s->iron = cjh_o_read_number(o);
  // else {
  printf("%.*s%s: SKIP\n", indent, space, key);
  cjh_o_skip(o);
  // }
}

void gs_SceneObject_from_json(CJHObjectR *o, const char *key, void *ud) {
  GameScene *gs = (GameScene *)ud;
  (void)gs;

  if (streq(key, "Castle_table.type")) {
    printf("%.*s%s:\n", indent, space, key);
    indent += 2;
    // Castle cs;
    // cjh_o_read_object(o, (CJHReadObjectCB)cs_from_json, &cs);
    cjh_o_skip(o);
    indent -= 2;

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

  else if (streq(key, "user")) {
    printf("%s:\n", key);
    indent += 2;
    cjh_o_read_object(o, gs_stuff_from_json, &gs->user.resources);
    indent -= 2;

  } else {
    printf("%s: SKIP\n", key);
    cjh_o_skip(o);
  }
}
