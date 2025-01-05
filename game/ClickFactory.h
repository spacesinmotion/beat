#ifndef CLICKFACTORY_H
#define CLICKFACTORY_H

#include "game/GameScene.h"
#include "game/Level.h"
#include "game/TileContent.h"
#include "game/WorkProvider.h"
#include "game/assets.h"
#include <assert.h>

typedef struct ClickFactory {
  WorkProvider work_provider;

  float temporary_deliver_timer;

  G_Object buffer;
  Recti location;
} ClickFactory;

Color cf_color() { return rgb(255, 215, 0); }

bool cf_dead(ClickFactory *cf) {
  (void)cf;
  return false;
}

float cf_render_order(ClickFactory *cf) { return l_to_y(cf->location.y); }

void cf_update(ClickFactory *cf, GameScene *gs, Game *g, float dt) {
  (void)g;

  if (cf->temporary_deliver_timer > 0.0f) {
    if ((cf->temporary_deliver_timer -= dt) <= 0.0f) {
      gs_produce_click(gs);
      wp_reset(&cf->work_provider);
    }
  } else if (wp_is_done(&cf->work_provider)) {
    cf->temporary_deliver_timer = 5.0;
  }
}

void cf_draw(ClickFactory *cf, GameScene *gs, Game *g) {
  if (ri_contains(cf->location, gs->r.x, gs->r.y)) {
    c_printf(g, "----------------------\n");
    c_printf(g, "  ClickFactory (%d,%d,%d,%d)\n", cf->location.x, cf->location.y, 4, 3);
    c_printf(g, "----------------------\n");
  }

  Vec2 p = l_to_vecP(ri_bottom_right(cf->location));
  g_color(g, cf_color());
  g_buffer(g, cf->buffer, Img_house_map, p);
  g_color(g, white());
  g_object(g, g_animation_buffer(g), Img_menubar, MI_Click, p);
  if (cf->temporary_deliver_timer > 0.0f)
    g_object(g, g_animation_buffer(g), Img_menubar, MI_Logistics, v_add(p, l_to_vec(0, 1)));

  wp_draw_click_fields(&cf->work_provider, g, v_add(p, l_to_vec(1, 1)), false);
}

static SceneObjectTable ClickFactory_table = {
    .dead = (SceneObjectDeadCB)cf_dead,
    .render_order = (SceneObjectRenderOrderCB)cf_render_order,
    .update = (SceneObjectUpdateCB)cf_update,
    .draw = (SceneObjectDrawCB)cf_draw,
};

void cf_done(WorkProvider *cf, GameScene *gs, Resource r) {
  assert(r == R_Work);

  cf->clicks_done++;
  if (cf->clicks_done == 4) {
    cf->clicks = cf->clicks_claimed = cf->clicks_work = cf->clicks_done = 0;
    gs_produce_click(gs);
  }
}

ClickFactory *ClickFactory_init(Game *g, GameScene *gs, Point p) {
  ClickFactory *cf = g_malloc(g, sizeof(ClickFactory));
  *cf = (ClickFactory){
      .buffer = g_tilerect_buffer(g, 3, 3),
      .location = (Recti){p.x, p.y, 3, 3},
  };
  assert((void *)cf == (void *)&cf->work_provider);
  wp_init(&cf->work_provider, 2, 2, 12.0f);

  l_set_tileR(gs->level, cf->location, T_ClickFactory);
  l_set_tile_contentR(gs->level, cf->location, to_TileContent(cf, &WorkProvider_TileContent_Default_Table));

  gs_add_object(gs, (SceneObject){.context = cf, &ClickFactory_table});
  return cf;
}

#endif