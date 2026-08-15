#ifndef HOUSE_H
#define HOUSE_H

#include "extern/cjsonh/cjsonh.h"
#include "game/BuildingDisplay.h"
#include "game/GameScene.h"
#include "game/Level.h"
#include "game/SceneObject.h"
#include "game/TileContent.h"
#include "game/assets.h"
#include "game/jobs/QueueItem.h"

typedef struct Resources {
  float food, water;
  int clicks;
} Resources;

typedef struct House {
  BuildingDisplay display;

  int id;

  Resources resources;
  Resources resources_maximum;

  int rent_to_be_payed;

  G_Object clicks_text;
  int clicks_cache;

  bool highlight;
  bool wearisome_at_home;
  bool wearisome_dead;
} House;

static inline Color h_color() { return rgb(87, 163, 106); }
static inline Sizei h_size() { return (Sizei){2, 2}; }

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

void w_deliver_clear(Wearisome *w);
bool h_get_water_done(void *context, Wearisome *w, GameScene *gs) {
  (void)gs;
  w_deliver_clear(w);
  ((House *)context)->resources.water += 1.0;
  return false;
}

Color wl_color();
void w_deliver(Wearisome *w, MenuIcon mi, Color c);
bool w_queue_move_to(Wearisome *w, GameScene *gs, Recti location, QueueItem qi);
bool h_pay_water(void *context, Wearisome *w, GameScene *gs) {
  House *h = (House *)context;
  gs->resource_pool.water--;
  gs->resource_pool_claimed.water--;
  gs->clicks++;
  h->resources.clicks--;
  w_deliver(w, MI_Water, wl_color());
  return w_queue_move_to(w, gs, h->display.location, QI(h, h_get_water_done));
}

bool h_get_food_done(void *context, Wearisome *w, GameScene *gs) {
  (void)gs;
  w_deliver_clear(w);
  ((House *)context)->resources.food += 1.0;
  return false;
}
Color fa_color();
bool h_pay_food(void *context, Wearisome *w, GameScene *gs) {
  House *h = (House *)context;
  gs->resource_pool.food--;
  gs->resource_pool_claimed.food--;
  gs->clicks++;
  h->resources.clicks--;
  w_deliver(w, MI_Food, fa_color());
  return w_queue_move_to(w, gs, h->display.location, QI(h, h_get_food_done));
}

void h_update(House *h, GameScene *gs, Game *g, float dt) {
  (void)dt;

  bd_update(&h->display, g);

  gs->clicks_in_houses += h->resources.clicks;

  if (h->clicks_cache != h->resources.clicks) {
    g_create_text(g, &h->clicks_text, Oswald_Regular_12, str("%4.d", h->resources.clicks));
    h->clicks_cache = h->resources.clicks;
  }

  if (gs->a_new_day_just_started)
    h->rent_to_be_payed++;
}

void h_draw(House *h, GameScene *gs, Game *g) {
  h->highlight = ri_contains(h->display.location, gs->r.x, gs->r.y);
  if (h->highlight) {
    c_printf(g, "----------------------\n");
    c_printf(g, "  HOUSE (%d,%d,%d,%d)\n", h->display.location.x, h->display.location.y, 2, 2);
    c_printf(g, "----------------------\n");
    c_printf(g, " %10s: %d\n", "clicks", h->resources.clicks);
    c_printf(g, " %10s: %d\n", "max clicks", h->resources_maximum.clicks);
    c_printf(g, " %10s: %f\n", "water", h->resources.water);
    c_printf(g, " %10s: %f\n", "food", h->resources.food);
    c_printf(g, "----------------------\n\n");
    c_printf(g, " %10s: %d\n", "rent", h->rent_to_be_payed);
    c_printf(g, "----------------------\n\n");
  }

  bd_draw(&h->display, g, h->wearisome_dead ? gray(25) : h_color(), MI_House);

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

Recti find_resource_building_rect(GameScene *gs, Recti start, Resource r);
bool h_check_needs(House *h, GameScene *gs, Wearisome *w) {
  if (h->resources_maximum.clicks <= 0)
    return false;

  bool need_water = h->resources_maximum.water - h->resources.water >= 1.0f;
  bool need_food = h->resources_maximum.food - h->resources.food >= 1.0f;
  bool food_is_more_urgent = need_water && need_food && h->resources.water > h->resources.food;
  if (!food_is_more_urgent && need_water && (gs->resource_pool.water - gs->resource_pool_claimed.water > 0)) {
    Recti waterProvider = find_resource_building_rect(gs, h->display.location, R_Water);
    if (waterProvider.w > 0 && w_queue_move_to(w, gs, waterProvider, QI(h, h_pay_water))) {
      tc_claim(l_content(gs->level, waterProvider.x, waterProvider.y), gs, w, R_Water);
      h->resources_maximum.clicks--;
      return true;
    }
  } else if (need_food && (gs->resource_pool.food - gs->resource_pool_claimed.food > 0)) {
    Recti foodProvider = find_resource_building_rect(gs, h->display.location, R_Food);
    if (foodProvider.w > 0 && w_queue_move_to(w, gs, foodProvider, QI(h, h_pay_food))) {
      tc_claim(l_content(gs->level, foodProvider.x, foodProvider.y), gs, w, R_Food);
      h->resources_maximum.clicks--;
      return true;
    }
  }

  if (h->rent_to_be_payed > 0 && h->resources_maximum.clicks > 0) {
    h->rent_to_be_payed--;
    h->resources_maximum.clicks--;
    h->resources.clicks--;
    gs->clicks++;
  }

  return false;
}

void h_resources_to_json(CJHObject *o, void *ud) {
  Resources *r = (Resources *)ud;
  cjh_o_add_number_if(o, "food", r->food, 0.0);
  cjh_o_add_number_if(o, "wateer", r->water, 0.0);
  cjh_o_add_number_if(o, "clicks", r->clicks, 0.0);
}
void h_to_json(CJHObject *o, House *h) {
  cjh_o_add_object(o, "display", (CJHWriteObjectCB)bd_to_json, &h->display);
  cjh_o_add_number(o, "id", h->id);
  cjh_o_add_object(o, "resources", h_resources_to_json, &h->resources);
  cjh_o_add_object(o, "resources_maximum", h_resources_to_json, &h->resources_maximum);
}

void h_resource_from_json(CJHObjectR *o, const char *key, House *h) {
  (void)h;

  if (streq(key, "food"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));
  else if (streq(key, "wateer"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));
  else if (streq(key, "clicks"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));
  else {
    printf("%.*s%s: SKIP\n", indent, space, key);
    cjh_o_skip(o);
  }
}

void h_from_json(CJHObjectR *o, const char *key, House *h) {

  if (streq(key, "id"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));

  else if (streq(key, "resources")) {
    printf("%.*s%s:\n", indent, space, key);
    indent += 2;
    cjh_o_read_object(o, (CJHReadObjectCB)h_resource_from_json, &h->resources);
    indent -= 2;
  } else if (streq(key, "resources_maximum")) {
    printf("%.*s%s:\n", indent, space, key);
    indent += 2;
    cjh_o_read_object(o, (CJHReadObjectCB)h_resource_from_json, &h->resources_maximum);
    indent -= 2;
  } else if (streq(key, "display")) {
    printf("%.*s%s:\n", indent, space, key);
    indent += 2;
    cjh_o_read_object(o, (CJHReadObjectCB)bd_from_json, &h->display);
    indent -= 2;
  }

  else {
    printf("%.*s%s: SKIP\n", indent, space, key);
    cjh_o_skip(o);
  }
}

static SceneObjectTable House_table = {
    .type = "House",
    .dead = (SceneObjectDeadCB)h_dead,
    .render_order = (SceneObjectRenderOrderCB)h_render_order,
    .update = (SceneObjectUpdateCB)h_update,
    .draw = (SceneObjectDrawCB)h_draw,
    .save = (SceneObjectSaveCB)h_to_json,
};

void h_to_json_ref(CJHObject *o, House *h) { cjh_o_add_number(o, House_table.type, h ? h->id : 0); }

Recti h_location(const House *mp) { return mp->display.location; }

static TileContentTable House_TileContent_Table = (TileContentTable){.location = (LocationCb)h_location};

House *House_init(Game *g, GameScene *gs, Point p) {
  const Sizei s = h_size();
  House *h = g_malloc(sizeof(House));
  *h = (House){
      .display = bd_create(g, (Recti){p.x, p.y, s.w, s.h}),
      .id = unique_id(h),
      .resources = {.food = 0.0f, .water = 0.0f, .clicks = 0},
      .resources_maximum = {.food = 2.0f, .water = 2.0f, .clicks = 0},
      .rent_to_be_payed = 0,
      .clicks_cache = -1,
      .highlight = false,
      .wearisome_at_home = true,
      .wearisome_dead = false,
  };

  l_set_tile_contentR(gs->level, h->display.location, to_TileContent(h, &House_TileContent_Table));

  gs_add_object(gs, (SceneObject){h, &House_table});

  return h;
}
#endif // HOUSE_H