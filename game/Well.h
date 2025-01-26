#ifndef WELL_H
#define WELL_H

#include "game/BuildingDisplay.h"
#include "game/GameScene.h"
#include "game/Level.h"
#include "game/TileContent.h"
#include "game/WorkProvider.h"
#include "game/assets.h"

typedef struct Well {
  WorkProvider work_provider;

  int last_day_delivered;

  BuildingDisplay display;
} Well;

static inline Color wl_color() { return rgb(0, 80, 133); }
static inline Sizei wl_size() { return (Sizei){2, 3}; }

bool wl_dead(Well *wl) {
  (void)wl;
  return false;
}

float wl_render_order(Well *wl) { return l_to_y(wl->display.location.y); }

void wl_update(Well *wl, GameScene *gs, Game *g, float dt) {
  bd_update(&wl->display, g);

  if (gs->day > wl->last_day_delivered) {
    wl->last_day_delivered = gs->day;
    if (wp_update_resource(&wl->work_provider, &gs->resource_pool.water, gs->resource_pool_max.water))
      bd_flash(&wl->display);
  }
}

void wl_draw(Well *wl, GameScene *gs, Game *g) {
  if (ri_contains(wl->display.location, gs->r.x, gs->r.y)) {
    c_printf(g, "----------------------\n");
    c_printf(g, "  Well (%d,%d,%d,%d)\n", wl->display.location.x, wl->display.location.y, 4, 3);
    c_printf(g, "----------------------\n");
  }

  Vec2 p = l_to_vecP(ri_bottom_right(wl->display.location));
  bd_draw(&wl->display, g, wl_color(), MI_Water);
  // if (wl->temporary_deliver_timer > 0.0f)
  //   g_object(g, g_animation_buffer(g), Img_menubar, MI_Logistics, v_add(p, l_to_vec(0, 1)));

  wp_draw_click_fields(&wl->work_provider, g, v_add(p, l_to_vec(1, 1)), false);
}

static SceneObjectTable Well_table = {
    .dead = (SceneObjectDeadCB)wl_dead,
    .render_order = (SceneObjectRenderOrderCB)wl_render_order,
    .update = (SceneObjectUpdateCB)wl_update,
    .draw = (SceneObjectDrawCB)wl_draw,
};

Well *Well_init(Game *g, GameScene *gs, Point p) {
  const Sizei s = wl_size();
  Well *wl = g_malloc(g, sizeof(Well));
  *wl = (Well){
      .display = bd_create(g, (Recti){p.x, p.y, s.w, s.h}),
      .last_day_delivered = gs->day,
  };
  assert((void *)wl == (void *)&wl->work_provider);
  wp_init(&wl->work_provider, s.w - 1, s.h - 1, 5.0f);

  l_set_tileR(gs->level, wl->display.location, T_Well);
  l_set_tile_contentR(gs->level, wl->display.location, to_TileContent(wl, &WorkProvider_TileContent_Default_Table));

  gs_add_object(gs, (SceneObject){.context = wl, &Well_table});
  return wl;
}

#endif