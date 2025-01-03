#ifndef WELL_H
#define WELL_H

#include "game/GameScene.h"
#include "game/Level.h"
#include "game/TileContent.h"
#include "game/WorkProvider.h"

typedef struct Well {
  WorkProvider work_provider;

  G_Object buffer;
  Recti location;
} Well;

Color wl_color() { return rgb(0, 80, 133); }

bool wl_dead(Well *wl) {
  (void)wl;
  return false;
}

float wl_render_order(Well *wl) { return l_to_y(wl->location.y); }

void wl_update(Well *wl, GameScene *gs, float dt) {
  (void)wl;
  (void)gs;
  (void)dt;
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
  g_object(g, g_animation_buffer(g), Img_menubar, 3, p);

  wp_draw_click_fields(&wl->work_provider, g, v_add(p, l_to_vec(1, 1)), false);
}

static SceneObjectTable Well_table = {
    .dead = (SceneObjectDeadCB)wl_dead,
    .render_order = (SceneObjectRenderOrderCB)wl_render_order,
    .update = (SceneObjectUpdateCB)wl_update,
    .draw = (SceneObjectDrawCB)wl_draw,
};

void wl_done(WorkProvider *wp, GameScene *gs, Resource r) {
  assert(r == R_Work);

  wp->clicks_done++;
  if (wp->clicks_done == 2) {
    wp->clicks = wp->clicks_claimed = wp->clicks_work = wp->clicks_done = 0;
    gs->resource_pool.water += 2;
  }
}

static TileContentTable Well_TileContent_Table = {
    .provides = (ProvidesCB)wp_provides,
    .claim = (ClaimCB)wp_claim,
    .start = (StartCB)wp_start,
    .done = (DoneCB)wl_done,
    .click = (ClickCB)wp_click,
};

Well *Well_init(Game *g, GameScene *gs, Point p) {
  Well *wl = g_malloc(g, sizeof(Well));
  *wl = (Well){
      .buffer = g_tilerect_buffer(g, 2, 3),
      .location = (Recti){p.x, p.y, 2, 3},
  };
  assert((void *)wl == (void *)&wl->work_provider);
  wp_init(&wl->work_provider, 1, 2, 9.0f);

  l_set_tileR(gs->level, wl->location, T_Well);
  l_set_tile_contentR(gs->level, wl->location, to_TileContent(wl, &Well_TileContent_Table));

  gs_add_object(gs, (SceneObject){.context = wl, &Well_table});
  return wl;
}

#endif