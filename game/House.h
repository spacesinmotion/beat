#ifndef HOUSE_H
#define HOUSE_H

#include "game/BuildingDisplay.h"
#include "game/Game.h"
#include "game/GameScene.h"
#include "game/Level.h"
#include "game/SceneObject.h"
#include "game/assets.h"
#include "math/Color.h"
#include "math/Rect.h"

typedef struct Resources {
  float food, water;
  int clicks;
} Resources;

typedef struct House {
  BuildingDisplay display;

  Resources resources;
  Resources resources_maximum;

  G_Object clicks_text;
  int clicks_cache;

  bool highlight;
  bool wearisome_at_home;
  bool wearisome_dead;
} House;

Color h_color() { return rgb(87, 163, 106); }

bool h_dead(House *h) {
  (void)h;
  return false;
}

float h_render_order(House *h) { return l_to_y(h->display.location.y); }

Point h_current_entry(House *h, Level *l) {
  int options[8][2] = {{-1, 0}, {-1, 1}, {2, 0}, {2, 1}, {0, -1}, {1, -1}, {0, 2}, {1, 2}};
  for (int i = 0; i < 8; ++i) {
    int ii = h->display.location.x + options[i][0];
    int jj = h->display.location.y + options[i][1];
    if (l_movable(l, ii, jj))
      return (Point){ii, jj};
  }
  return (Point){h->display.location.x, h->display.location.y};
}

void h_earn_click(House *h, int c) {
  h->resources.clicks += c;
  h->resources_maximum.clicks += c;
}

void h_pay_water(House *h, GameScene *gs) {
  (void)h;
  gs->resource_pool.water--;
  gs->resource_pool_claimed.water--;
  gs->clicks++;
  h->resources.clicks--;
}

void h_pay_food(House *h, GameScene *gs) {
  (void)h;
  gs->resource_pool.food--;
  gs->resource_pool_claimed.food--;
  gs->clicks++;
  h->resources.clicks--;
}

void h_get_water_done(House *h, GameScene *gs) {
  (void)gs;
  h->resources.water += 1.0;
}

void h_get_food_done(House *h, GameScene *gs) {
  (void)gs;
  h->resources.food += 1.0;
}

void h_update(House *h, GameScene *gs, Game *g, float dt) {
  (void)dt;

  bd_update(&h->display, g);

  gs->clicks_in_houses += h->resources.clicks;

  if (h->clicks_cache != h->resources.clicks) {
    g_create_text(g, &h->clicks_text, Oswald_Regular_12, str("%4.d", h->resources.clicks));
    h->clicks_cache = h->resources.clicks;
  }
}

void h_draw(House *h, GameScene *gs, Game *g) {
  h->highlight = ri_contains(h->display.location, gs->r.x, gs->r.y);
  if (h->highlight) {
    c_printf(g, "----------------------\n");
    c_printf(g, "  HOUSE (%d,%d,%d,%d)\n", h->display.location.x, h->display.location.y, 2, 2);
    c_printf(g, "----------------------\n");
    c_printf(g, " %10s: %d\n", "clicks", h->resources.clicks);
    c_printf(g, " %10s: %d\n", "clicks", h->resources_maximum.clicks);
    c_printf(g, " %10s: %f\n", "water", h->resources.water);
    c_printf(g, " %10s: %f\n", "food", h->resources.food);
    c_printf(g, "----------------------\n\n");
  }

  bd_draw(&h->display, g, h_color(), MI_Food);

  Vec2 p = l_to_vecP(ri_bottom_right(h->display.location));
  float x = h->resources.water / h->resources_maximum.water;
  g_color(g, warn(x));
  for (int i = 0; i < 6; ++i) {
    if (i / 6.0f >= x)
      break;
    g_objectS(g, g_animation_buffer(g), Img_wearisome, 12, v_add(p, (Vec2){-2, 8 + 2 * i}), 0.25);
  }

  x = h->resources.food / h->resources_maximum.food;
  g_color(g, warn(x));
  for (int i = 0; i < 6; ++i) {
    if (i / 6.0f >= x)
      break;
    g_objectS(g, g_animation_buffer(g), Img_wearisome, 12, v_add(p, (Vec2){4, 8 + 2 * i}), 0.25);
  }

  g_color(g, white());
  g_objectS(g, g_animation_buffer(g), Img_wearisome, h->wearisome_at_home ? 14 : 15, v_add(p, l_to_vec(1, 1)), 0.75f);

  g_color(g, rgb(255, 215, 0));
  g_text(g, h->clicks_text, Oswald_Regular_12, v_add(p, (Vec2){12, -5}));
}

static SceneObjectTable House_table = (SceneObjectTable){
    .dead = (SceneObjectDeadCB)h_dead,
    .render_order = (SceneObjectRenderOrderCB)h_render_order,
    .update = (SceneObjectUpdateCB)h_update,
    .draw = (SceneObjectDrawCB)h_draw,
};
House *House_init(Game *g, GameScene *gs, Point p) {
  House *h = g_malloc(g, sizeof(House));
  *h = (House){
      .display = bd_create(g, (Recti){p.x, p.y, 2, 2}),
      .resources = {.food = 0.0f, .water = 0.0f, .clicks = 0},
      .resources_maximum = {.food = 2.0f, .water = 2.0f, .clicks = 0},
      .clicks_cache = -1,
      .highlight = false,
      .wearisome_at_home = true,
      .wearisome_dead = false,
  };

  l_set_tileR(gs->level, h->display.location, T_House);
  gs_add_object(gs, (SceneObject){h, &House_table});

  return h;
}
#endif // HOUSE_H