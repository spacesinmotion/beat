
#include "game/GameScene.h"
#include "game/ClickFactory.h"
#include "game/ConstructionSite.h"
#include "game/Farm.h"
#include "game/Game.h"
#include "game/House.h"
#include "game/Level.h"
#include "game/Marketplace.h"
#include "game/SceneObject.h"
#include "game/StreetMap.h"
#include "game/TileContent.h"
#include "game/Well.h"
#include "game/assets.h"
#include "gc/gc.h"
#include "math.h"
#include "math/Rect.h"
#include "math/Vec2.h"
#include <stdlib.h>

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

void gs_update(GameScene *gs, Game *g, float dt) {
  (void)g;
  gs->daytime_step = dt / 60.0f;
  gs->daytime += gs->daytime_step;
  if (gs->daytime > 1.0f) {
    gs->daytime -= 1.0f;
    gs->day++;
  }

  for (int i = 0; i < gs->scene_objects.len; ++i)
    so_update(&gs->scene_objects.data[i], gs, dt);

  so_vec_filter_dead(&gs->scene_objects);

  qsort(gs->scene_objects.data, gs->scene_objects.len, sizeof(SceneObject), so_render_order_compare);
}

void gs_draw(GameScene *gs, Game *g) {

  c_printf(g, "----------------------\n");
  c_printf(g, " %10s: %d\n", "day", gs->day);
  c_printf(g, " %10s: %f\n", "daytime", gs->daytime);
  c_printf(g, "----------------------\n\n");
  c_printf(g, " %10s: %d\n", "clicks", gs->clicks);
  c_printf(g, " %10s: %d\n", "water", gs->resources.water);
  c_printf(g, " %10s: %d\n", "food", gs->resources.food);
  c_printf(g, "----------------------\n\n");

  StreetMap_draw(gs->street_map, g);

  for (int i = 0; i < gs->scene_objects.len; ++i)
    so_draw(&gs->scene_objects.data[i], gs, g);

  if (gs->menu_under_mouse < 0 && l_validR(gs->level, gs->r)) {
    if (gs->r.h > 0 && gs->r.w > 0) {
      g_color(g, gs->preview);
      g_buffer(g, g_tilerect_buffer(g, gs->r.w, gs->r.h), Img_house_map, l_to_vec(gs->r.x, gs->r.y));
    }
    g_color(g, l_freeR(gs->level, gs->r) ? green() : red());
    for (int i = gs->r.x; i < gs->r.x + gs->r.w; ++i)
      for (int j = gs->r.y; j < gs->r.y + gs->r.h; ++j)
        g_object(g, g_animation_buffer(g), Img_marker, g_frame(g) % 4, l_to_vec(i, j));
  }
}

void gs_draw_overlay(GameScene *gs, Game *g) {
  for (int i = 0; i < 10; ++i) {
    g_color(g, i == gs->menu_under_mouse ? gray(100) : (gs->menu_selected == i ? gray(25) : gray(75)));
    g_object(g, g_animation_buffer(g), Img_menubar, i % 16, (Vec2){8 + 4 + i * 16, 8 + 4});
  }
  for (int i = 0; i < 10; ++i) {
    g_color(g, i == gs->menu_under_mouse ? red() : (gs->menu_selected == i ? green() : blue()));
    g_object(g, g_animation_buffer(g), Img_marker, i == gs->menu_under_mouse ? g_frame(g) % 4 : i % 4,
             (Vec2){8 + 4 + i * 16, 8 + 4});
  }

  Size vp = g_viewport(g);
  Vec2 clock_pos = (Vec2){vp.w - 16.0f, vp.h - 16.0f};
  g_color(g, gs->daytime > 0.75 ? red() : white());
  g_objectRS(g, g_animation_buffer(g), Img_overlay_images, 0, clock_pos, -gs->daytime * M_PI * 2.0f, 2.0f);
  g_objectS(g, g_animation_buffer(g), Img_overlay_images, 1, clock_pos, 2.0f);
}

void gs_mouse_move(GameScene *gs, Game *g, Vec2 mp, Vec2 op) {
  (void)g;
  gs->r.x = (int)((mp.x + 8) / 16.0f);
  gs->r.y = (int)((mp.y + 8) / 16.0f);

  gs->menu_under_mouse = -1;
  for (int i = 0; i < 10; ++i)
    if (r_contains((Rect){(Vec2){4 + i * 16, 4}, (Vec2){16, 16}}, op))
      gs->menu_under_mouse = i;
}

void gs_mouse_down(GameScene *gs, Game *g, Vec2 mp, Vec2 op, int button) {
  (void)mp;
  (void)op;

  if (button == 1) {
    gs->menu_selected = -1;
    gs->r.w = gs->r.h = 0;

  } else if (button == 0) {
    if (gs->menu_under_mouse >= 0) {
      gs->menu_selected = gs->menu_under_mouse;
      if (gs->menu_selected == 0) {
        gs->preview = Street_color();
        gs->r.w = gs->r.h = 1;
      } else if (gs->menu_selected == 1) {
        gs->preview = mp_color();
        gs->r.w = 4;
        gs->r.h = 3;
      } else if (gs->menu_selected == 2) {
        gs->preview = h_color();
        gs->r.w = gs->r.h = 2;
      } else if (gs->menu_selected == 3) {
        gs->preview = wl_color();
        gs->r.w = 2;
        gs->r.h = 3;
      } else if (gs->menu_selected == 4) {
        gs->preview = fa_color();
        gs->r.w = 4;
        gs->r.h = 4;
      } else if (gs->menu_selected == 5) {
        gs->preview = cf_color();
        gs->r.w = 3;
        gs->r.h = 3;
      } else {
        gs->r.w = gs->r.h = 0;
      }
    } else if (gs->menu_selected >= 0) {
      ConstructionSite_init(g, gs, gs->r, gs->menu_selected);
    } else {
      tc_click(l_content(gs->level, gs->r.x, gs->r.y), gs);
    }
  }
}

void gs_add_object(GameScene *gs, SceneObject so) { so_vec_push(&gs->scene_objects, so); }

void gs_construction_done(GameScene *gs, Game *g, Recti r, int key) {
  if (key == 0) {
    l_set_movable(gs->level, r.x, r.y, true);
    StreetMap_update(gs->street_map);
  } else if (key == 1) {
    if (l_freeR(gs->level, r))
      Marketplace_init(g, gs, (Point){r.x, r.y});
  } else if (key == 2) {
    if (l_freeR(gs->level, r))
      House_init(g, gs, (Point){r.x, r.y});
  } else if (key == 3) {
    if (l_freeR(gs->level, r))
      Well_init(g, gs, (Point){r.x, r.y});
  } else if (key == 4) {
    if (l_freeR(gs->level, r))
      Farm_init(g, gs, (Point){r.x, r.y});
  } else if (key == 5) {
    if (l_freeR(gs->level, r))
      ClickFactory_init(g, gs, (Point){r.x, r.y});
  }
}
SceneTable GameScene_table = {
    .update = (SceneUpdateCB)gs_update,
    .draw = (SceneDrawCB)gs_draw,
    .draw_overlay = (SceneDrawCB)gs_draw_overlay,
    .mouse_move = (SceneMouseMoveCB)gs_mouse_move,
    .mouse_down = (SceneMouseCB)gs_mouse_down,
};
void GameScene_init(Game *g) {
  GameScene *gs = g_malloc(g, sizeof(GameScene));
  *gs = (GameScene){
      .scene_objects = (SceneObjectVec){NULL, 0, 0},
      .menu_under_mouse = -1,
      .menu_selected = -1,
      .day = 1,
      .daytime = 0.0f,
      .clicks = 0,
      .resources = {.water = 60, .food = 50},
      .resources_claimed = {.water = 0, .food = 0},
      .level = g_malloc(g, sizeof(Level)),
      .r = (Recti){-1, -1, 0, 0},
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
    h_earn_click(House_init(g, gs, (Point){17, 10 - 3 - 2 * i}), rand() % 3 + 1);
    h_earn_click(House_init(g, gs, (Point){19, 10 - 3 - 2 * i}), rand() % 3 + 1);
    h_earn_click(House_init(g, gs, (Point){17, 10 + 4 + 2 * i}), rand() % 3 + 1);
    h_earn_click(House_init(g, gs, (Point){19, 10 + 4 + 2 * i}), rand() % 3 + 1);
    h_earn_click(House_init(g, gs, (Point){14, 10 + 4 + 2 * i}), rand() % 3 + 1);
    h_earn_click(House_init(g, gs, (Point){22, 10 + 4 + 2 * i}), rand() % 3 + 1);
  }

  gs->street_map = StreetMap_init(g, gs);

  g_set_scene(g, (Scene){gs, &GameScene_table});
}