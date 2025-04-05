
#include "game/GameScene.h"
#include "extern/cjsonh/cjsonh.h"
#include "game/ConstructionSite.h"
#include "game/Level.h"
#include "game/SceneObject.h"
#include "game/TileContent.h"
#include "game/assets.h"
#include "game/buildings/Altar.h"
#include "game/buildings/Archers.h"
#include "game/buildings/Castle.h"
#include "game/buildings/Farm.h"
#include "game/buildings/Forge.h"
#include "game/buildings/MenAtArms.h"
#include "game/buildings/Mine.h"
#include "game/buildings/Tower.h"
#include "game/buildings/Woodcutter.h"
#include "game/effects/Bling.h"
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

void gs_toggle_pause(GameScene *gs, int id) { (void)id, gs->game_paused = false; }
void gs_set_game_speed(GameScene *gs, int speed) {
  gs->game_paused = false;
  gs->game_speed = (float)speed;
}

void gs_select_bottom_menu(GameScene *gs, int button) {
  gs->menu_selected = button;
  if (button == MI_Castle) {
    gs->preview = cs_color();
    ri_set_size(&gs->r, cs_size());
  } else if (button == MI_Farm) {
    gs->preview = fa_color();
    ri_set_size(&gs->r, fa_size());
  } else if (button == MI_WoodCutter) {
    gs->preview = wc_color();
    ri_set_size(&gs->r, wc_size());
  } else if (button == MI_Mine) {
    gs->preview = mi_color();
    ri_set_size(&gs->r, mi_size());
  } else if (button == MI_MenAtArms) {
    gs->preview = maa_color();
    ri_set_size(&gs->r, maa_size());
  } else if (button == MI_Archers) {
    gs->preview = ar_color();
    ri_set_size(&gs->r, ar_size());
  } else if (button == MI_Tower) {
    gs->preview = to_color();
    ri_set_size(&gs->r, to_size());
  } else if (button == MI_Altar) {
    gs->preview = al_color();
    ri_set_size(&gs->r, al_size());
  } else if (button == MI_Forge) {
    gs->preview = fo_color();
    ri_set_size(&gs->r, fo_size());
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

void gs_update(GameScene *gs, Game *g, float dt) {
  (void)g;

  gs->daytime_step = gs->game_paused ? 0.0f : (dt / 5.0f);
  gs->daytime += gs->daytime_step;

  int next_tick = (int)(gs->daytime * 16.0);
  if (next_tick > gs->tick_of_day) {
    gs->tick_of_day = next_tick;

    Vec2 mp = l_to_vec(next_tick - 1, -1);
    Bling_init(gs, mp, red());

    for (int i = 0; i < gs->scene_objects.len; ++i)
      so_tick(&gs->scene_objects.data[i], gs, g, gs->tick_of_day);
  }

  if (gs->daytime > 1.0f) {
    gs->game_paused = true;
    gs->tick_of_day = 0;
    gs->daytime = 0.0f;
    gs->day++;
  }

  // float t = gs->daytime;
  // if (t < 0.05f)
  //   t = 0.5 + t / 0.1f;
  // else if (t < 0.7f)
  //   t = 1.0f;
  // else if (t < 0.8f)
  //   t = 1.0 - (t - 0.7) / 0.1f;
  // else if (t < 0.95f)
  //   t = 0.0;
  // else
  //   t = (t - 0.95f) / 0.1f;
  // t = (1.0f - cos(t * M_PI)) / 2.0f;
  // g_set_background_color(g, c_mix(rgb(66, 64, 78), rgb(226, 219, 197), t));
  g_set_background_color(g, rgb(226, 219, 197));

  for (int i = 0; i < gs->scene_objects.len; ++i)
    so_update(&gs->scene_objects.data[i], gs, g, dt);

  so_vec_filter_dead(&gs->scene_objects);

  qsort(gs->scene_objects.data, gs->scene_objects.len, sizeof(SceneObject), so_render_order_compare);

  if (gs->resources.money != gs->money_counter_text_cache) {
    g_create_text(g, &gs->money_counter_text, Oswald_Regular_8, str("%.2d", gs->resources.money));
    gs->money_counter_text_cache = gs->resources.money;
  }
  if (gs->resources.food != gs->food_counter_text_cache) {
    g_create_text(g, &gs->food_counter_text, Oswald_Regular_8, str("%d", gs->resources.food));
    gs->food_counter_text_cache = gs->resources.food;
  }
  if (gs->resources.wood != gs->wood_counter_text_cache) {
    g_create_text(g, &gs->wood_counter_text, Oswald_Regular_8, str("%d", gs->resources.wood));
    gs->wood_counter_text_cache = gs->resources.food;
  }
  if (gs->resources.iron != gs->iron_counter_text_cache) {
    g_create_text(g, &gs->iron_counter_text, Oswald_Regular_8, str("%d", gs->resources.iron));
    gs->iron_counter_text_cache = gs->resources.iron;
  }
  if (gs->day != gs->day_counter_text_cache) {
    g_create_text(g, &gs->day_counter_text, Oswald_Regular_12, str("day %d", gs->day));
    gs->day_counter_text_cache = gs->day;
  }
  if (0 != gs->bot_counter_text_cache) {
    g_create_text(g, &gs->bot_counter_text, Oswald_Regular_12, str("%d", 0));
    gs->bot_counter_text_cache = 0;
  }
}

bool gs_construction_available(GameScene *gs) {
  if (!l_freeR(gs->level, gs->r))
    return false;
  return gs->resources.money > 0;
}

void gs_draw(GameScene *gs, Game *g) {
  c_printf(g, "\n\n\n\n\n\n\n\n\n\n");
  c_printf(g, "\n\n\n\n\n\n\n\n\n\n");
  // c_printf(g, "----------------------\n");
  // c_printf(g, " %10s: %g\n", "game speed", gs->game_paused ? 0.0f : gs->game_speed);
  // c_printf(g, "----------------------\n");
  // c_printf(g, " %10s: %d\n", "day", gs->day);
  // c_printf(g, " %10s: %f\n", "daytime", gs->daytime);
  // c_printf(g, " %10s: %d\n", "bots", gs->wearisome_count);
  // c_printf(g, "----------------------\n\n");
  // c_printf(g, " %10s: %d\n", "all $", gs->clicks_in_houses + gs->clicks);
  // c_printf(g, " %10s: %d\n", "spread $", gs->clicks_in_houses);
  // c_printf(g, " %10s: %d\n", "produced $", gs->clicks_produced);
  // c_printf(g, " %10s: %d\n", "lost $", gs->clicks_lost);
  // c_printf(g, "----------------------\n\n");

  for (int i = 0; i < LEVEL_WIDTH; ++i)
    for (int j = 0; j < LEVEL_HEIGHT; ++j) {
      bool under_mouse = gs->r.x == i && gs->r.y == j;
      g_color(g, under_mouse ? red() : (l_movable(gs->level, i, j) ? blue() : gray(100)));
      g_object(g, g_animation_buffer(g), Img_marker, under_mouse ? g_frame(g) % 4 : 0, l_to_vec(i, j));
    }
  // StreetMap_draw(gs->street_map, g);

  for (int i = 0; i < gs->scene_objects.len; ++i)
    so_draw(&gs->scene_objects.data[i], gs, g);

  if (gs->pick_under_mouse < 0 && l_validR(gs->level, gs->r)) {
    if (gs->r.h > 0 && gs->r.w > 0) {
      g_color(g, gs->preview);
      g_buffer(g, g_tilerect_buffer(g, gs->r.w, gs->r.h), Img_house_map, l_to_vec(gs->r.x, gs->r.y));
    }
    g_color(g, gs_construction_available(gs) ? green() : red());
    for (int i = gs->r.x; i < gs->r.x + gs->r.w; ++i)
      for (int j = gs->r.y; j < gs->r.y + gs->r.h; ++j)
        g_object(g, g_animation_buffer(g), Img_marker, g_frame(g) % 4, l_to_vec(i, j));
  }
}

void gs_draw_menu_overlay(GameScene *gs, Game *g) {
  const Color cn = gs->daytime < 0.75f ? gray(25) : gray(225);
  const Color ch = gs->daytime < 0.75f ? gray(75) : gray(175);
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
void gs_draw_clock_overlay(GameScene *gs, Game *g) {
  Sizei vp = g_viewport(g);
  Vec2 clock_pos = (Vec2){vp.w - 24.0f, vp.h - 24.0f};
  g_color(g, rgba(137, 197, 184, 150));
  g_objectS(g, g_animation_buffer(g), Img_wearisome, 12, v_add(clock_pos, (Vec2){-58, 44}), 5.5f);
  g_color(g, rgb(182, 205, 70));
  g_objectS(g, g_animation_buffer(g), Img_wearisome, 12, v_add(clock_pos, (Vec2){4, 4}), 4.5f);
  g_color(g, gs->daytime > 0.75 ? rgb(102, 121, 129) : white());
  g_objectRS(g, g_animation_buffer(g), Img_overlay_images, 0, clock_pos, -gs->daytime * M_PI * 2.0f, 3.0f);
  g_objectS(g, g_animation_buffer(g), Img_overlay_images, 1, clock_pos, 3.0f);

  Vec2 p = v_add(clock_pos, (Vec2){center_num(gs->day), -3});
  bool hover = gs_pick(gs, gs_toggle_pause, 0, p, 1.0f);
  g_color(g, hover ? gray(200) : gray(45));
  g_text(g, gs->day_counter_text, Oswald_Regular_12, p);

  g_color(g, rgba(255, 255, 255, 150));
  p = v_add(clock_pos, (Vec2){-110, 21});
  g_objectS(g, g_animation_buffer(g), Img_wearisome, 12, p, 1.5f);
  g_color(g, white());
  g_objectS(g, g_animation_buffer(g), Img_wearisome, 0, v_add(p, (Vec2){-4, -4}), 0.5f);
  g_color(g, gray(45));
  g_text(g, gs->bot_counter_text, Oswald_Regular_12, v_add(p, (Vec2){center_num(0) + 10, -7}));

  // int i = 0;
  // p = v_add(clock_pos, (Vec2){-86 + i * 16, 16});
  // bool hover = gs_pick(gs, gs_toggle_pause, 0, p, 1.0f);
  // g_color(g, hover ? gray(200) : gs->game_paused ? red() : white());
  // g_object(g, g_animation_buffer(g), Img_overlay_images, 4 + i, p);
  // ++i;
  // p = v_add(clock_pos, (Vec2){-86 + i * 16, 16});
  // hover = gs_pick(gs, gs_set_game_speed, 2, p, 1.0f);
  // g_color(g, hover ? gray(200) : gs->game_speed == 2.0 ? red() : white());
  // g_object(g, g_animation_buffer(g), Img_overlay_images, 4 + i, p);
  // ++i;
  // p = v_add(clock_pos, (Vec2){-86 + i * 16, 16});
  // hover = gs_pick(gs, gs_set_game_speed, 4, p, 1.0f);
  // g_color(g, hover ? gray(200) : gs->game_speed == 4.0 ? red() : white());
  // g_object(g, g_animation_buffer(g), Img_overlay_images, 4 + i, p);
  // ++i;
  // p = v_add(clock_pos, (Vec2){-86 + i * 16, 16});
  // hover = gs_pick(gs, gs_set_game_speed, 8, p, 1.0f);
  // g_color(g, hover ? gray(200) : gs->game_speed == 8.0 ? red() : white());
  // g_object(g, g_animation_buffer(g), Img_overlay_images, 4 + i, p);
}

void gs_draw_storage_overlay(GameScene *gs, Game *g) {
  g_color(g, rgb(137, 197, 184));
  g_objectS(g, g_animation_buffer(g), Img_wearisome, 12, (Vec2){5, g_viewport(g).h - 12}, 3.5f);
  g_color(g, rgb(182, 205, 70));
  g_objectS(g, g_animation_buffer(g), Img_wearisome, 12, (Vec2){15, g_viewport(g).h - 1}, 3.0f);
  g_color(g, gray(75));
  // g_object(g, g_animation_buffer(g), Img_menubar, MI_Click, (Vec2){9, g_viewport(g).h - 9});
  // g_text(g, gs->click_counter_text, Oswald_Regular_12, (Vec2){15, g_viewport(g).h - 13});

  g_color(g, rgb(85, 154, 139));
  g_objectS(g, g_animation_buffer(g), Img_wearisome, 12, (Vec2){5, g_viewport(g).h - 53}, 2.75f);

  const float o = 12;
  float h = g_viewport(g).h - 30;
  g_color(g, gray(45));
  g_objectS(g, g_animation_buffer(g), Img_menubar, MI_Castle, (Vec2){5, h - 0 * o + 2}, 0.75f);
  g_text(g, gs->money_counter_text, Oswald_Regular_8, (Vec2){12, h - 0 * o});
  g_objectS(g, g_animation_buffer(g), Img_menubar, MI_Farm, (Vec2){5, h - 1 * o + 2}, 0.75f);
  g_text(g, gs->food_counter_text, Oswald_Regular_8, (Vec2){12, h - 1 * o});
  g_objectS(g, g_animation_buffer(g), Img_menubar, MI_WoodCutter, (Vec2){5, h - 2 * o + 2}, 0.75f);
  g_text(g, gs->wood_counter_text, Oswald_Regular_8, (Vec2){12, h - 2 * o});
  g_objectS(g, g_animation_buffer(g), Img_menubar, MI_Mine, (Vec2){5, h - 3 * o + 2}, 0.75f);
  g_text(g, gs->iron_counter_text, Oswald_Regular_8, (Vec2){12, h - 3 * o});
}

void gs_draw_overlay(GameScene *gs, Game *g) {
  gs->pick_rect_count = 0;
  gs_draw_menu_overlay(gs, g);
  gs_draw_clock_overlay(gs, g);
  gs_draw_storage_overlay(gs, g);

  if (tc_can_click(l_content(gs->level, gs->r.x, gs->r.y))) {
    g_color(g, gray(45));
    g_object(g, g_animation_buffer(g), Img_wearisome, 12, v_add(gs->mouse_overlay_position, (Vec2){13, -9}));
    g_color(g, gray(200));
    g_text(g, gs->money_counter_text, Oswald_Regular_12, v_add(gs->mouse_overlay_position, (Vec2){10, -12}));
  }
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
    const Point p = (Point){gs->r.x, gs->r.y};
    if (gs->pick_under_mouse >= 0)
      gs->pick_rects[gs->pick_under_mouse].click(gs, gs->pick_rects[gs->pick_under_mouse].id);

    else if (gs->special_click_handler)
      gs->special_click_handler(gs->special_click_handler_data, p, gs);

    else if (gs->menu_selected >= 0) {
      if (gs_construction_available(gs) && gs->r.w * gs->r.h > 0)
        ConstructionSite_init(gs, gs->r, gs->menu_selected);

    } else {
      tc_click(l_contentP(gs->level, p), p, gs);
      Bling_init(gs, mp, gray(45));
    }

  } else if (button == 1) {
    gs->special_click_handler = NULL;
    gs->special_click_handler_data = NULL;
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

void gs_construction_done(GameScene *gs, Game *g, Recti r, int key) {
  if (key == MI_Castle) {
    Castle_init(g, gs, (Point){r.x, r.y});
  } else if (key == MI_Farm) {
    Farm_init(g, gs, (Point){r.x, r.y});
  } else if (key == MI_WoodCutter) {
    WoodCutter_init(g, gs, (Point){r.x, r.y});
  } else if (key == MI_Mine) {
    Mine_init(g, gs, (Point){r.x, r.y});
  } else if (key == MI_MenAtArms) {
    MenAtArms_init(g, gs, (Point){r.x, r.y});
  } else if (key == MI_Archers) {
    Archers_init(g, gs, (Point){r.x, r.y});
  } else if (key == MI_Tower) {
    Tower_init(g, gs, (Point){r.x, r.y});
  } else if (key == MI_Altar) {
    Altar_init(g, gs, (Point){r.x, r.y});
  } else if (key == MI_Forge) {
    Forge_init(g, gs, (Point){r.x, r.y});
  }
}

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
      .game_paused = true,
      .menu_selected = -1,
      .day = 1,
      .tick_of_day = 0,
      .daytime = 0.0f,
      .resources = {.money = 100, .food = 0, .wood = 0, .iron = 0},
      .level = g_malloc(sizeof(Level)),
      .r = (Recti){-1, -1, 0, 0},
      .money_counter_text_cache = -1,
      .food_counter_text_cache = -1,
      .wood_counter_text_cache = -1,
      .iron_counter_text_cache = -1,
      .day_counter_text_cache = -1,
      .bot_counter_text_cache = -1,
      .pick_rects = {},
      .pick_rect_count = 0,
      .pick_under_mouse = -1,
  };

  l_init(gs->level);

  g_set_scene(g, (Scene){gs, &GameScene_table});
}

void gs_stuff_to_json(CJHObject *o, void *ud) {
  const Stuff *s = (Stuff *)ud;
  cjh_o_add_number(o, "money", s->money);
  cjh_o_add_number(o, "food", s->food);
  cjh_o_add_number(o, "wood", s->wood);
  cjh_o_add_number(o, "irong", s->iron);
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
  cjh_o_add_number(o, "daytime_step", gs->daytime_step);
  cjh_o_add_number(o, "daytime", gs->daytime);
  cjh_o_add_number(o, "day", gs->day);
  cjh_o_add_object(o, "resources", gs_stuff_to_json, &gs->resources);
  cjh_o_add_object(o, "level", (CJHWriteObjectCB)l_to_json, gs->level);
  // StreetMap *street_map;
}

void gs_stuff_from_json(CJHObjectR *o, const char *key, void *ud) {
  Stuff *s = (Stuff *)ud;
  if (streq(key, "money"))
    s->money = cjh_o_read_number(o);
  else if (streq(key, "food"))
    s->food = cjh_o_read_number(o);
  else if (streq(key, "wood"))
    s->wood = cjh_o_read_number(o);
  else if (streq(key, "iron"))
    s->iron = cjh_o_read_number(o);
  else {
    printf("%.*s%s: SKIP\n", indent, space, key);
    cjh_o_skip(o);
  }
}

void gs_SceneObject_from_json(CJHObjectR *o, const char *key, void *ud) {
  GameScene *gs = (GameScene *)ud;
  (void)gs;

  if (streq(key, Castle_table.type)) {
    printf("%.*s%s:\n", indent, space, key);
    indent += 2;
    Castle cs;
    cjh_o_read_object(o, (CJHReadObjectCB)cs_from_json, &cs);
    indent -= 2;
  } else if (streq(key, Farm_table.type)) {
    printf("%.*s%s:\n", indent, space, key);
    indent += 2;
    Farm fa;
    cjh_o_read_object(o, (CJHReadObjectCB)fa_from_json, &fa);
    indent -= 2;
  } else if (streq(key, WoodCutter_table.type)) {
    printf("%.*s%s:\n", indent, space, key);
    indent += 2;
    WoodCutter wc;
    cjh_o_read_object(o, (CJHReadObjectCB)wc_from_json, &wc);
    indent -= 2;
  } else if (streq(key, Mine_table.type)) {
    printf("%.*s%s:\n", indent, space, key);
    indent += 2;
    Mine mi;
    cjh_o_read_object(o, (CJHReadObjectCB)mi_from_json, &mi);
    indent -= 2;
  } else if (streq(key, MenAtArms_table.type)) {
    printf("%.*s%s:\n", indent, space, key);
    indent += 2;
    MenAtArms maa;
    cjh_o_read_object(o, (CJHReadObjectCB)maa_from_json, &maa);
    indent -= 2;
  } else if (streq(key, Archers_table.type)) {
    printf("%.*s%s:\n", indent, space, key);
    indent += 2;
    Archers ar;
    cjh_o_read_object(o, (CJHReadObjectCB)ar_from_json, &ar);
    indent -= 2;
  } else if (streq(key, Altar_table.type)) {
    printf("%.*s%s:\n", indent, space, key);
    indent += 2;
    Altar al;
    cjh_o_read_object(o, (CJHReadObjectCB)al_from_json, &al);
    indent -= 2;
  } else if (streq(key, Forge_table.type)) {
    printf("%.*s%s:\n", indent, space, key);
    indent += 2;
    Forge fo;
    cjh_o_read_object(o, (CJHReadObjectCB)fo_from_json, &fo);
    indent -= 2;
  } else if (streq(key, ConstructionSite_table.type)) {
    printf("%.*s%s:\n", indent, space, key);
    indent += 2;
    ConstructionSite cs;
    cjh_o_read_object(o, (CJHReadObjectCB)cs_from_json, &cs);
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

  else if (streq(key, "resources")) {
    printf("%s:\n", key);
    indent += 2;
    cjh_o_read_object(o, gs_stuff_from_json, &gs->resources);
    indent -= 2;
  } else if (streq(key, "level")) {
    printf("%s:\n", key);
    indent += 2;
    cjh_o_read_object(o, (CJHReadObjectCB)l_from_json, gs->level);
    indent -= 2;
  }

  else {
    printf("%s: SKIP\n", key);
    cjh_o_skip(o);
  }
}
