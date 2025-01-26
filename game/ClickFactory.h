#ifndef CLICKFACTORY_H
#define CLICKFACTORY_H

#include "game/BuildingDisplay.h"
#include "game/GameScene.h"
#include "game/Level.h"
#include "game/TileContent.h"
#include "game/WorkProvider.h"
#include "game/assets.h"
#include "game/effects/Bling.h"
#include "math/Rect.h"
#include "math/Vec2.h"
#include "math/random.h"
#include <assert.h>

typedef struct ClickFactory {
  WorkProvider work_provider;

  int last_day_delivered;

  BuildingDisplay display;

  int missing_starts;
} ClickFactory;

static inline Color cf_color() { return rgb(255, 215, 0); }
static inline Sizei cf_size() { return (Sizei){3, 2}; }

bool cf_dead(ClickFactory *cf) {
  (void)cf;
  return false;
}

float cf_render_order(ClickFactory *cf) { return l_to_y(cf->display.location.y); }

void cf_update(ClickFactory *cf, GameScene *gs, Game *g, float dt) {
  bd_update(&cf->display, g);

  if (gs->day > cf->last_day_delivered) {
    cf->last_day_delivered = gs->day;
    if (cf->work_provider.clicks_done > 0) {
      while (cf->work_provider.clicks_done > 0) {
        gs_produce_click(gs);
        wp_reduce_clicks(&cf->work_provider);
        cf->missing_starts += 5;
      }
      bd_flash(&cf->display);
    }
  }

  if (cf->missing_starts > 0 && r_float() > 0.9f) {
    Vec2 p = l_to_vecP(ri_bottom_right(cf->display.location));
    Vec2 s = l_to_vec(cf->display.location.w - 1, cf->display.location.h - 1);
    p = v_add(p, (Vec2){r_float() * s.x, r_float() * s.y});
    Bling_init(g, gs, p, red());
    cf->missing_starts--;
  }
}

void cf_draw(ClickFactory *cf, GameScene *gs, Game *g) {
  if (ri_contains(cf->display.location, gs->r.x, gs->r.y)) {
    c_printf(g, "----------------------\n");
    c_printf(g, "  ClickFactory (%d,%d,%d,%d)\n", cf->display.location.x, cf->display.location.y, 4, 3);
    c_printf(g, "----------------------\n");
  }

  Vec2 p = l_to_vecP(ri_bottom_right(cf->display.location));

  bd_draw(&cf->display, g, cf_color(), MI_Click);
  // if (cf->temporary_deliver_timer > 0.0f)
  //   g_object(g, g_animation_buffer(g), Img_menubar, MI_Logistics, v_add(p, l_to_vec(0, 1)));

  wp_draw_click_fields(&cf->work_provider, g, v_add(p, l_to_vec(1, 1)), false);
}

static SceneObjectTable ClickFactory_table = {
    .dead = (SceneObjectDeadCB)cf_dead,
    .render_order = (SceneObjectRenderOrderCB)cf_render_order,
    .update = (SceneObjectUpdateCB)cf_update,
    .draw = (SceneObjectDrawCB)cf_draw,
};

ClickFactory *ClickFactory_init(Game *g, GameScene *gs, Point p) {
  Sizei s = cf_size();
  ClickFactory *cf = g_malloc(g, sizeof(ClickFactory));
  *cf = (ClickFactory){
      .display = bd_create(g, (Recti){p.x, p.y, s.w, s.h}),
      .last_day_delivered = gs->day,
      .missing_starts = 15,
  };
  assert((void *)cf == (void *)&cf->work_provider);
  wp_init(&cf->work_provider, s.w - 1, s.h - 1, 9.0f);

  l_set_tileR(gs->level, cf->display.location, T_ClickFactory);
  l_set_tile_contentR(gs->level, cf->display.location, to_TileContent(cf, &WorkProvider_TileContent_Default_Table));

  gs_add_object(gs, (SceneObject){.context = cf, &ClickFactory_table});
  return cf;
}

#endif