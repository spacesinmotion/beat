
#include "game/GameScene.h"
#include "game/ClickFactory.h"
#include "game/ConstructionMaterialFactory.h"
#include "game/ConstructionSite.h"
#include "game/Entertainment.h"
#include "game/Farm.h"
#include "game/Game.h"
#include "game/House.h"
#include "game/Level.h"
#include "game/Marketplace.h"
#include "game/SceneObject.h"
#include "game/StreetMap.h"
#include "game/TileContent.h"
#include "game/Wearisome.h"
#include "game/Well.h"
#include "game/assets.h"
#include "gc/gc.h"
// #include "math.h"
#include "math.h"
#include "math/Color.h"
#include "math/Rect.h"
#include "math/Vec2.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

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
void gs_set_game_speed(GameScene *gs, int speed) { gs->game_speed = (float)speed; }

void gs_select_bottom_menu(GameScene *gs, int button) {
  gs->menu_selected = button;
  if (button == MI_Street) {
    gs->preview = Street_color();
    gs->r.w = gs->r.h = 1;
  } else if (button == MI_Marketplace) {
    gs->preview = mp_color();
    ri_set_size(&gs->r, mp_size());
  } else if (button == MI_House) {
    gs->preview = h_color();
    ri_set_size(&gs->r, h_size());
  } else if (button == MI_Water) {
    gs->preview = wl_color();
    ri_set_size(&gs->r, wl_size());
  } else if (button == MI_Food) {
    gs->preview = fa_color();
    ri_set_size(&gs->r, fa_size());
  } else if (button == MI_Click) {
    gs->preview = cf_color();
    ri_set_size(&gs->r, cf_size());
  } else if (button == MI_Entertainment) {
    gs->preview = em_color();
    ri_set_size(&gs->r, em_size());
  } else if (button == MI_ConstructionMaterial) {
    gs->preview = cmf_color();
    ri_set_size(&gs->r, cmf_size());
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

  dt = gs->game_paused ? 0.0f : dt * gs->game_speed;

  gs->daytime_step = dt / 60.0f;
  gs->daytime += gs->daytime_step;
  if (gs->daytime > 1.0f) {
    gs->daytime -= 1.0f;
    gs->day++;
  }

  float t = gs->daytime;
  if (t < 0.05f)
    t = 0.5 + t / 0.1f;
  else if (t < 0.7f)
    t = 1.0f;
  else if (t < 0.8f)
    t = 1.0 - (t - 0.7) / 0.1f;
  else if (t < 0.95f)
    t = 0.0;
  else
    t = (t - 0.95f) / 0.1f;
  t = (1.0f - cos(t * M_PI)) / 2.0f;
  g_set_background_color(g, c_mix(rgb(66, 64, 78), rgb(226, 219, 197), t));

  gs->clicks_in_houses = 0;
  for (int i = 0; i < gs->scene_objects.len; ++i)
    so_update(&gs->scene_objects.data[i], gs, g, dt);

  so_vec_filter_dead(&gs->scene_objects);

  qsort(gs->scene_objects.data, gs->scene_objects.len, sizeof(SceneObject), so_render_order_compare);

  if (gs->clicks != gs->click_counter_text_cache) {
    g_create_text(g, &gs->click_counter_text, Oswald_Regular_12, str("%.2d", gs->clicks));
    gs->click_counter_text_cache = gs->clicks;
  }
  if (gs->resource_pool.water != gs->water_counter_text_cache) {
    g_create_text(g, &gs->water_counter_text, Oswald_Regular_8, str("%d", gs->resource_pool.water));
    gs->water_counter_text_cache = gs->resource_pool.water;
  }
  if (gs->resource_pool.food != gs->food_counter_text_cache) {
    g_create_text(g, &gs->food_counter_text, Oswald_Regular_8, str("%d", gs->resource_pool.food));
    gs->food_counter_text_cache = gs->resource_pool.food;
  }
  if (gs->resource_pool.construction_material != gs->construction_material_counter_text_cache) {
    g_create_text(g, &gs->construction_material_counter_text, Oswald_Regular_8,
                  str("%d", gs->resource_pool.construction_material));
    gs->construction_material_counter_text_cache = gs->resource_pool.construction_material;
  }
  const int free_storage = gs_free_storage(gs);
  if (gs->free_storage_text_cache != free_storage) {
    g_create_text(g, &gs->free_storage_text, Oswald_Regular_8,
                  str("%d/%d", gs->storage_size - free_storage, gs->storage_size));
    gs->free_storage_text_cache = free_storage;
  }
}

bool gs_construction_available(GameScene *gs) {
  if (!l_freeR(gs->level, gs->r))
    return false;
  return gs->clicks > 0;
}

void gs_draw(GameScene *gs, Game *g) {
  c_printf(g, "\n\n\n\n\n\n\n\n\n\n");
  c_printf(g, "\n\n\n\n\n\n\n\n\n\n");
  c_printf(g, "----------------------\n");
  c_printf(g, " %10s: %g\n", "game speed", gs->game_paused ? 0.0f : gs->game_speed);
  c_printf(g, "----------------------\n");
  c_printf(g, " %10s: %d\n", "day", gs->day);
  c_printf(g, " %10s: %f\n", "daytime", gs->daytime);
  c_printf(g, "----------------------\n\n");
  c_printf(g, " %10s: %d\n", "all $", gs->clicks_in_houses + gs->clicks);
  c_printf(g, " %10s: %d\n", "spread $", gs->clicks_in_houses);
  c_printf(g, " %10s: %d\n", "produced $", gs->clicks_produced);
  c_printf(g, " %10s: %d\n", "lost $", gs->clicks_lost);
  c_printf(g, "----------------------\n\n");

  StreetMap_draw(gs->street_map, g);

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

void gs_draw_clock_overlay(GameScene *gs, Game *g) {
  Sizei vp = g_viewport(g);
  Vec2 clock_pos = (Vec2){vp.w - 24.0f, vp.h - 24.0f};
  g_color(g, rgb(137, 197, 184));
  g_objectS(g, g_animation_buffer(g), Img_wearisome, 12, v_add(clock_pos, (Vec2){-58, 44}), 5.5f);
  g_color(g, rgb(182, 205, 70));
  g_objectS(g, g_animation_buffer(g), Img_wearisome, 12, v_add(clock_pos, (Vec2){4, 4}), 4.5f);
  g_color(g, gs->daytime > 0.75 ? rgb(102, 121, 129) : white());
  g_objectRS(g, g_animation_buffer(g), Img_overlay_images, 0, clock_pos, -gs->daytime * M_PI * 2.0f, 3.0f);
  g_objectS(g, g_animation_buffer(g), Img_overlay_images, 1, clock_pos, 3.0f);

  int i = 0;
  Vec2 p = v_add(clock_pos, (Vec2){-86 + i * 16, 16});
  bool hover = gs_pick(gs, gs_toggle_pause, 0, p, 1.0f);
  g_color(g, hover ? gray(200) : gs->game_paused ? red() : white());
  g_object(g, g_animation_buffer(g), Img_overlay_images, 4 + i, p);
  ++i;
  p = v_add(clock_pos, (Vec2){-86 + i * 16, 16});
  hover = gs_pick(gs, gs_set_game_speed, 2, p, 1.0f);
  g_color(g, hover ? gray(200) : gs->game_speed == 2.0 ? red() : white());
  g_object(g, g_animation_buffer(g), Img_overlay_images, 4 + i, p);
  ++i;
  p = v_add(clock_pos, (Vec2){-86 + i * 16, 16});
  hover = gs_pick(gs, gs_set_game_speed, 4, p, 1.0f);
  g_color(g, hover ? gray(200) : gs->game_speed == 4.0 ? red() : white());
  g_object(g, g_animation_buffer(g), Img_overlay_images, 4 + i, p);
  ++i;
  p = v_add(clock_pos, (Vec2){-86 + i * 16, 16});
  hover = gs_pick(gs, gs_set_game_speed, 8, p, 1.0f);
  g_color(g, hover ? gray(200) : gs->game_speed == 8.0 ? red() : white());
  g_object(g, g_animation_buffer(g), Img_overlay_images, 4 + i, p);
}

void gs_draw_storage_overlay(GameScene *gs, Game *g) {
  g_color(g, rgb(137, 197, 184));
  g_objectS(g, g_animation_buffer(g), Img_wearisome, 12, (Vec2){5, g_viewport(g).h - 12}, 3.5f);
  g_color(g, rgb(182, 205, 70));
  g_objectS(g, g_animation_buffer(g), Img_wearisome, 12, (Vec2){15, g_viewport(g).h - 1}, 3.0f);
  g_color(g, gray(75));
  g_object(g, g_animation_buffer(g), Img_menubar, MI_Click, (Vec2){9, g_viewport(g).h - 9});
  g_text(g, gs->click_counter_text, Oswald_Regular_12, (Vec2){15, g_viewport(g).h - 13});

  g_color(g, rgb(85, 154, 139));
  g_objectS(g, g_animation_buffer(g), Img_wearisome, 12, (Vec2){5, g_viewport(g).h - 53}, 2.75f);

  const float o = 12;
  float h = g_viewport(g).h - 30;
  g_color(g, gray(45));
  g_objectS(g, g_animation_buffer(g), Img_menubar, MI_WareHouse, (Vec2){5, h - 0 * o + 2}, 0.75f);
  g_text(g, gs->free_storage_text, Oswald_Regular_8, (Vec2){10, h - 0 * o});
  g_objectS(g, g_animation_buffer(g), Img_menubar, MI_Water, (Vec2){5, h - 1 * o + 2}, 0.75f);
  g_text(g, gs->water_counter_text, Oswald_Regular_8, (Vec2){10, h - 1 * o});
  g_objectS(g, g_animation_buffer(g), Img_menubar, MI_Food, (Vec2){5, h - 2 * o + 2}, 0.75f);
  g_text(g, gs->food_counter_text, Oswald_Regular_8, (Vec2){10, h - 2 * o});
  g_objectS(g, g_animation_buffer(g), Img_menubar, MI_ConstructionMaterial, (Vec2){5, h - 3 * o + 2}, 0.75f);
  g_text(g, gs->construction_material_counter_text, Oswald_Regular_8, (Vec2){10, h - 3 * o});
}

void gs_draw_overlay(GameScene *gs, Game *g) {
  gs->pick_rect_count = 0;
  gs_draw_menu_overlay(gs, g);
  gs_draw_clock_overlay(gs, g);
  gs_draw_storage_overlay(gs, g);

  if (tc_can_click(l_content(gs->level, gs->r.x, gs->r.y))) {
    g_color(g, gray(45));
    g_object(g, g_animation_buffer(g), Img_wearisome, 12, v_add(gs->mouse_overlay_position, (Vec2){13, -9}));
    g_color(g, gray(215));
    g_text(g, gs->click_counter_text, Oswald_Regular_12, v_add(gs->mouse_overlay_position, (Vec2){10, -12}));
  }
}

void gs_mouse_move(GameScene *gs, Game *g, Vec2 mp, Vec2 op) {
  (void)g;
  gs->r.x = (int)((mp.x + 8) / 16.0f);
  gs->r.y = (int)((mp.y + 8) / 16.0f);

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
  (void)op;

  if (button == 0) {

    if (gs->pick_under_mouse >= 0)
      gs->pick_rects[gs->pick_under_mouse].click(gs, gs->pick_rects[gs->pick_under_mouse].id);

    else if (gs->menu_selected >= 0) {
      if (gs_construction_available(gs) && gs->r.w * gs->r.h > 0)
        ConstructionSite_init(g, gs, gs->r, gs->menu_selected);

    } else {
      tc_click(l_content(gs->level, gs->r.x, gs->r.y), gs);
      Bling_init(g, gs, mp, gray(45));
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

  if (key == PAUSE_KEY) {
    gs->game_paused = !gs->game_paused;
  } else if (key == SPEED_1_KEY) {
    gs->game_paused = false;
    gs->game_speed = 1.0f;
  } else if (key == SPEED_2_KEY) {
    gs->game_speed = 2.0f;
    gs->game_paused = false;
  } else if (key == SPEED_4_KEY) {
    gs->game_speed = 4.0f;
    gs->game_paused = false;
  } else if (key == SPEED_8_KEY) {
    gs->game_speed = 8.0f;
    gs->game_paused = false;
  }
}

void gs_add_object(GameScene *gs, SceneObject so) { so_vec_push(&gs->scene_objects, so); }

void gs_construction_done(GameScene *gs, Game *g, Recti r, int key) {
  if (key == 0) {
    l_set_movable(gs->level, r.x, r.y, true);
    StreetMap_update(gs->street_map);
  } else if (key == MI_Marketplace) {
    Marketplace_init(g, gs, (Point){r.x, r.y});
  } else if (key == MI_House) {
    Wearisome_House_init(g, gs, (Point){r.x, r.y});
  } else if (key == MI_Water) {
    Well_init(g, gs, (Point){r.x, r.y});
  } else if (key == MI_Food) {
    Farm_init(g, gs, (Point){r.x, r.y});
  } else if (key == MI_Click) {
    ClickFactory_init(g, gs, (Point){r.x, r.y});
  } else if (key == MI_Entertainment) {
    Entertainment_init(g, gs, (Point){r.x, r.y});
  } else if (key == MI_ConstructionMaterial) {
    ConstructionMaterialFactory_init(g, gs, (Point){r.x, r.y});
  }
}
SceneTable GameScene_table = {
    .update = (SceneUpdateCB)gs_update,
    .draw = (SceneDrawCB)gs_draw,
    .draw_overlay = (SceneDrawCB)gs_draw_overlay,
    .mouse_move = (SceneMouseMoveCB)gs_mouse_move,
    .mouse_down = (SceneMouseCB)gs_mouse_down,
    .key_up = (SceneKeyCB)gs_key_up,
};
void GameScene_init(Game *g) {
  GameScene *gs = g_malloc(g, sizeof(GameScene));
  *gs = (GameScene){
      .scene_objects = (SceneObjectVec){NULL, 0, 0},
      .game_speed = 2.0f,
      .game_paused = false,
      .menu_selected = -1,
      .day = 1,
      .daytime = 0.0f,
      .clicks = 4,
      .resource_pool = {.water = 25, .food = 25, .construction_material = 30},
      .resource_pool_claimed = {.water = 0, .food = 0, .construction_material = 0},
      .storage_size = 120,
      .storage_claimed = 0,
      .level = g_malloc(g, sizeof(Level)),
      .r = (Recti){-1, -1, 0, 0},
      .click_counter_text_cache = -1,
      .water_counter_text_cache = -1,
      .food_counter_text_cache = -1,
      .construction_material_counter_text_cache = -1,
      .free_storage_text_cache = -1,
      .pick_rects = {},
      .pick_rect_count = 0,
      .pick_under_mouse = -1,
  };

  l_init(gs->level);

  Marketplace_init(g, gs, (Point){17, 10});
  for (int i = 8; i < 30; ++i)
    l_set_movable(gs->level, i, 9, true);
  for (int i = 13; i < 27; ++i)
    l_set_movable(gs->level, i, 20, true);
  for (int i = 12; i < 31; ++i)
    l_set_movable(gs->level, i, 13, true);
  for (int i = 2; i < 23; ++i)
    l_set_movable(gs->level, 16, i, true);
  for (int i = 1; i < 26; ++i)
    l_set_movable(gs->level, 21, i, true);
  for (int i = 0; i < 3; ++i) {
    // h_earn_click(Wearisome_House_init(g, gs, (Point){14, 10 + 4 + 2 * i}), rand() % 3 + 1);
    h_earn_click(Wearisome_House_init(g, gs, (Point){17, 10 + 4 + 2 * i}), rand() % 3 + 1);
    h_earn_click(Wearisome_House_init(g, gs, (Point){19, 10 + 4 + 2 * i}), rand() % 3 + 1);
    // h_earn_click(Wearisome_House_init(g, gs, (Point){22, 10 + 4 + 2 * i}), rand() % 3 + 1);
  }

  gs->street_map = StreetMap_init(g, gs);

  g_set_scene(g, (Scene){gs, &GameScene_table});
}