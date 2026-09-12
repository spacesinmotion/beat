#ifndef CLICKFACTORY_H
#define CLICKFACTORY_H

#include "SokEngWrap/Game.h"
#include "SokEngWrap/SceneObject.h"
#include "SokEngWrap/math/Rect.h"
#include "SokEngWrap/math/random.h"
#include "game/BuildingDisplay.h"
#include "game/Level.h"
#include "game/TileContent.h"
#include "game/Wearisome.h"
#include "game/WorkProvider.h"
#include "game/assets/textures.h"
#include "game/effects/Bling.h"

typedef struct ClickFactory {
  WorkProvider work_provider;
  BuildingDisplay display;

  int id;
  int missing_blings;

  Resource currently_selling;
  float switch_selling_flash;
} ClickFactory;

static inline Color cf_color() { return rgb(156, 154, 107); }
static inline Sizei cf_size() { return (Sizei){3, 2}; }

bool cf_dead(ClickFactory *cf) {
  (void)cf;
  return false;
}

float cf_render_order(ClickFactory *cf) { return l_to_y(cf->display.location.y); }

void cf_update(ClickFactory *cf, GameScene *gs, Game *g, float dt) {
  (void)dt;
  bd_update(&cf->display, g);

  if (cf->missing_blings > 0 && r_float() > 0.9f) {
    Bling_init(gs, bd_random_point_inside(&cf->display), red());
    cf->missing_blings--;
  }

  if (gs->a_new_day_just_started)
    wp_clear_done_work(&cf->work_provider);

  cf->switch_selling_flash = f_max(0.0f, cf->switch_selling_flash - g_animation_delta(g));
}

int icon_for_resource(Resource r) {
  if (r == R_Food)
    return MI_Food;
  if (r == R_Water)
    return MI_Water;
  if (r == R_ConstructionMaterial)
    return MI_ConstructionMaterial;

  assert(false);
  return MI_Street;
}

void cf_draw(ClickFactory *cf, GameScene *gs, Game *g) {
  if (ri_contains(cf->display.location, gs->r.x, gs->r.y)) {
    c_printf(g, "----------------------\n");
    c_printf(g, "  ClickFactory (%d,%d,%d,%d)\n", cf->display.location.x, cf->display.location.y, 4, 3);
    c_printf(g, "----------------------\n");
  }

  bd_draw(&cf->display, g, cf_color(), MI_Click);

  Vec2 p = l_to_vecP(ri_bottom_right(cf->display.location));
  bd_draw_icon(g, v_add(p, l_to_vec(1, 0)), icon_for_resource(cf->currently_selling), cf->switch_selling_flash);
  wp_draw_click_fields(&cf->work_provider, g, v_add(p, l_to_vec(1, 1)), false);
}

static SceneObjectTable ClickFactory_table = {
    .type = "ClickFactory",
    .dead = (SceneObjectDeadCB)cf_dead,
    .render_order = (SceneObjectRenderOrderCB)cf_render_order,
    .update = (SceneObjectUpdateCB)cf_update,
    .draw = (SceneObjectDrawCB)cf_draw,
};

Recti cf_location(const ClickFactory *cf) { return cf->display.location; }

bool cf_done_work(void *context, Wearisome *w, GameScene *gs) {
  (void)gs;

  ClickFactory *cf = (ClickFactory *)context;
  wp_done(&cf->work_provider, w);

  cf->missing_blings += 5;
  gs_produce_click(gs);
  // wp_reduce_clicks(&cf->work_provider, 1);
  bd_flash(&cf->display);
  CoinAnimation_init(gs, bd_gain_something_location(&cf->display));

  return false;
}
bool cf_start_work(void *context, Wearisome *w, GameScene *gs) {
  (void)gs;

  ClickFactory *cf = (ClickFactory *)context;
  wp_start(&cf->work_provider, w);
  w_deliver_clear(w);
  return w_queue_wait_for(w, 9.0f, QI(cf, cf_done_work));
}

bool cf_work_collect_construction_material(void *context, Wearisome *w, GameScene *gs) {
  ClickFactory *cf = (ClickFactory *)context;
  TileContent *tc = l_contentP(gs->level, w_current_point(w));
  assert(tc && tc->table->take);
  tc_take(tc, gs, R_ConstructionMaterial, false);
  w_deliver(w, MI_ConstructionMaterial, cf_color());
  return w_queue_move_to(w, gs, cf->display.location, QI(cf, cf_start_work));
}
bool cf_work_collect_water(void *context, Wearisome *w, GameScene *gs) {
  ClickFactory *cf = (ClickFactory *)context;
  TileContent *tc = l_contentP(gs->level, w_current_point(w));
  assert(tc && tc->table->take);
  tc_take(tc, gs, R_Water, false);
  w_deliver(w, MI_Water, cf_color());
  return w_queue_move_to(w, gs, cf->display.location, QI(cf, cf_start_work));
}
bool cf_work_collect_food(void *context, Wearisome *w, GameScene *gs) {
  ClickFactory *cf = (ClickFactory *)context;
  TileContent *tc = l_contentP(gs->level, w_current_point(w));
  assert(tc && tc->table->take);
  tc_take(tc, gs, R_Food, false);
  w_deliver(w, MI_Food, cf_color());
  return w_queue_move_to(w, gs, cf->display.location, QI(cf, cf_start_work));
}

void cf_claim(ClickFactory *cf, GameScene *gs, Wearisome *w, Resource r) {
  assert(r == R_Work);

  TileContent *provider = find_resource_building(gs, cf->display.location, cf->currently_selling);
  if (provider) {
    QueueItem qi = {0};
    tc_claim(provider, gs, w, cf->currently_selling);
    if (cf->currently_selling == R_ConstructionMaterial) {
      qi = QI(cf, cf_work_collect_construction_material);
    } else if (cf->currently_selling == R_Water) {
      qi = QI(cf, cf_work_collect_water);
    } else if (cf->currently_selling == R_Food) {
      qi = QI(cf, cf_work_collect_food);
    }
    if (qi.context && w_queue_move_to(w, gs, tc_location(provider), qi))
      wp_claim(&cf->work_provider);
  }
}

void cf_click(ClickFactory *cf, Point p, GameScene *gs) {
  Point lp = {p.x - cf->display.location.x, p.y - cf->display.location.y};
  if (lp.x == 1 && lp.y == 0) {
    cf->currently_selling = cf->currently_selling == R_ConstructionMaterial
                                ? R_Water
                                : (cf->currently_selling == R_Water ? R_Food : R_ConstructionMaterial);
    cf->switch_selling_flash = 1.0f;
  } else
    wp_click(&cf->work_provider, p, gs);
}

static TileContentTable ClickFactory_TileContent_Table = {
    .location = (LocationCb)cf_location,
    .provides = (ProvidesCB)wp_provides,
    .claim = (ClaimCB)cf_claim,
    .click = (ClickCBx)cf_click,
};

ClickFactory *ClickFactory_init(Game *g, GameScene *gs, Point p) {
  Sizei s = cf_size();
  ClickFactory *cf = g_malloc(sizeof(ClickFactory));
  *cf = (ClickFactory){
      .display = bd_create(g, (Recti){p.x, p.y, s.w, s.h}),
      .id = unique_id(cf),
      .missing_blings = 15,
      .currently_selling = R_ConstructionMaterial,
      .switch_selling_flash = 1.0,
  };
  assert((void *)cf == (void *)&cf->work_provider);
  wp_init(&cf->work_provider, s.w - 1, s.h - 1);

  l_set_tile_contentR(gs->level, cf->display.location, to_TileContent(cf, &ClickFactory_TileContent_Table));

  gs_add_object(gs, (SceneObject){.context = cf, &ClickFactory_table});
  return cf;
}

#endif