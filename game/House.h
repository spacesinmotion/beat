#ifndef HOUSE_H
#define HOUSE_H

#include "game/Game.h"
#include "game/GameScene.h"
#include "game/Level.h"
#include "game/SceneObject.h"
#include "game/Wearisome.h"
#include "game/assets.h"
#include "math/Rect.h"
#include <time.h>

typedef struct Resources {
  float food, water;
} Resources;

typedef struct House {
  G_Object buffer;
  Recti location;

  Resources resources;
  Resources resources_maximum;

  Wearisome *wearisome;
} House;

Color h_color() { return rgb(87, 163, 106); }

bool h_dead(House *h) {
  (void)h;
  return false;
}

float h_render_order(House *h) { return l_to_y(h->location.y); }

Point h_current_entry(House *h, Level *l) {
  int options[8][2] = {{-1, 0}, {-1, 1}, {2, 0}, {2, 1}, {0, -1}, {1, -1}, {0, 2}, {1, 2}};
  for (int i = 0; i < 8; ++i) {
    int ii = h->location.x + options[i][0];
    int jj = h->location.y + options[i][1];
    if (l_movable(l, ii, jj))
      return (Point){ii, jj};
  }
  return (Point){h->location.x, h->location.y};
}

void h_pay_stuff(House *h, GameScene *gs) {
  (void)h;
  gs->clicks++;
}
void h_get_water_done(House *h, GameScene *gs) {
  (void)gs;
  h->resources.water += 1.0;
}
void h_get_food_done(House *h, GameScene *gs) {
  (void)gs;
  h->resources.food += 1.0;
}

void h_update(House *h, GameScene *gs, float dt) {
  (void)dt;

  if (h->wearisome && w_dead(h->wearisome))
    h->wearisome = NULL;

  if (h->wearisome && w_is_home(h->wearisome)) {
    w_sleep(h->wearisome, 8.0f * gs->daytime_step);

    h->resources.water -= w_drink(h->wearisome, f_min(h->resources.water, 12.0 * gs->daytime_step));
    h->resources.food -= w_eat(h->wearisome, f_min(h->resources.food, 12.0 * gs->daytime_step));
  }

  if (h->resources_maximum.water - h->resources.water >= 1.0f && w_is_free(h->wearisome)) {
    w_deliver(h->wearisome, gs,
              deliver_job((Recti){17, 10, 4, 3}, h->location, h, (CollectDoneCB)h_pay_stuff,
                          (DeliverDoneCB)h_get_water_done));
  }
  if (h->resources_maximum.food - h->resources.food >= 1.0f && w_is_free(h->wearisome)) {
    w_deliver(
        h->wearisome, gs,
        deliver_job((Recti){17, 10, 4, 3}, h->location, h, (CollectDoneCB)h_pay_stuff, (DeliverDoneCB)h_get_food_done));
  }
}

void h_draw(House *h, GameScene *gs, Game *g) {
  if (ri_contains(h->location, gs->r.x, gs->r.y)) {
    c_printf(g, "#####################\n");
    c_printf(g, "# HOUSE (%d,%d,%d,%d)\n", h->location.x, h->location.y, 2, 2);
    c_printf(g, "#####################\n");
    c_printf(g, "#%10s: %f\n", "water", h->resources.water);
    c_printf(g, "#%10s: %f\n", "food", h->resources.food);
    c_printf(g, "#####################\n\n");
  }

  Vec2 p = l_to_vecP(ri_bottom_right(h->location));
  g_color(g, h->wearisome ? h_color() : rgb(0, 0, 0));
  g_buffer(g, h->buffer, Img_house_map, p);

  float x = h->resources.water / h->resources_maximum.water;
  g_color(g, warn(x));
  for (int i = 0; i < 6; ++i) {
    if (i / 6.0f >= x)
      break;
    g_objectS(g, g_animation_buffer(g), Img_wearisome, 12, v_add(p, (Vec2){-2, -2 + 4 * i}), 0.25);
  }

  x = h->resources.food / h->resources_maximum.food;
  g_color(g, warn(x));
  for (int i = 0; i < 6; ++i) {
    if (i / 6.0f >= x)
      break;
    g_objectS(g, g_animation_buffer(g), Img_wearisome, 12, v_add(p, (Vec2){4, -2 + 4 * i}), 0.25);
  }
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
      .buffer = g_tilerect_buffer(g, 2, 2),
      .location = {p.x, p.y, 2, 2},
      .resources = {.food = 2.0f, .water = 2.0f},
      .resources_maximum = {.food = 2.0f, .water = 2.0f},
      .wearisome = NULL,
  };

  Level_set_tileR(gs->level, h->location, T_House);
  gs_add_object(gs, (SceneObject){h, &House_table});

  h->wearisome = Wearisome_init(g, gs, h->location);

  return h;
}
#endif // HOUSE_H