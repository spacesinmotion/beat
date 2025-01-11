#ifndef WELL_H
#define WELL_H

#include "game/GameScene.h"
#include "game/Level.h"
#include "game/TileContent.h"
#include "game/WorkProvider.h"
#include "game/assets.h"

typedef struct Well {
  WorkProvider work_provider;

  float temporary_deliver_timer;

  G_Object buffer;
  Recti location;
} Well;

Color wl_color() { return rgb(0, 80, 133); }

bool wl_dead(Well *wl) {
  (void)wl;
  return false;
}

float wl_render_order(Well *wl) { return l_to_y(wl->location.y); }

void wl_update(Well *wl, GameScene *gs, Game *g, float dt) {
  (void)g;

  if (wl->temporary_deliver_timer > 0.0f) {
    wl->temporary_deliver_timer -= dt;
    if (wl->temporary_deliver_timer <= 0.0f) {
      if (gs->resource_pool.water + 2 <= gs->resource_pool_max.water) {
        gs->resource_pool.water += 2;
        wp_reset(&wl->work_provider);
      } else
        wl->temporary_deliver_timer = 1.0f;
    }
  } else if (wp_is_done(&wl->work_provider)) {
    wl->temporary_deliver_timer = 5.0;
  }
}

void wl_draw(Well *wl, GameScene *gs, Game *g) {
  if (ri_contains(wl->location, gs->r.x, gs->r.y)) {
    c_printf(g, "----------------------\n");
    c_printf(g, "  Well (%d,%d,%d,%d)\n", wl->location.x, wl->location.y, 4, 3);
    c_printf(g, "----------------------\n");
  }

  Vec2 p = l_to_vecP(ri_bottom_right(wl->location));
  g_color(g, wl_color());
  g_buffer(g, wl->buffer, Img_house_map, p);
  g_color(g, white());
  g_object(g, g_animation_buffer(g), Img_menubar, MI_Water, p);
  if (wl->temporary_deliver_timer > 0.0f)
    g_object(g, g_animation_buffer(g), Img_menubar, MI_Logistics, v_add(p, l_to_vec(0, 1)));

  wp_draw_click_fields(&wl->work_provider, g, v_add(p, l_to_vec(1, 1)), false);
}

static SceneObjectTable Well_table = {
    .dead = (SceneObjectDeadCB)wl_dead,
    .render_order = (SceneObjectRenderOrderCB)wl_render_order,
    .update = (SceneObjectUpdateCB)wl_update,
    .draw = (SceneObjectDrawCB)wl_draw,
};

Well *Well_init(Game *g, GameScene *gs, Point p) {
  Well *wl = g_malloc(g, sizeof(Well));
  *wl = (Well){
      .buffer = g_tilerect_buffer(g, 2, 3),
      .location = (Recti){p.x, p.y, 2, 3},
      .temporary_deliver_timer = 0.0f,
  };
  assert((void *)wl == (void *)&wl->work_provider);
  wp_init(&wl->work_provider, 1, 2, 9.0f);

  l_set_tileR(gs->level, wl->location, T_Well);
  l_set_tile_contentR(gs->level, wl->location, to_TileContent(wl, &WorkProvider_TileContent_Default_Table));

  gs_add_object(gs, (SceneObject){.context = wl, &Well_table});
  return wl;
}

#endif