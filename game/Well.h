#ifndef WELL_H
#define WELL_H

#include "game/BuildingDisplay.h"
#include "game/Wearisome.h"
#include "game/WorkProvider.h"

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
  (void)dt;
  bd_update(&wl->display, g);

  if (gs->day > wl->last_day_delivered) {
    wl->last_day_delivered = gs->day;
    if (wp_finish_production_cycle(&wl->work_provider, 8))
      bd_flash(&wl->display);
  }
  if (wp_has_something_stored(&wl->work_provider) && gs->daytime > 0.25 && gs->daytime < 0.26) {
    const int free_storage = gs_free_storage(gs);
    for (int i = 0; i < free_storage && wl->work_provider.storage > 0; ++i) {
      wl->work_provider.storage--;
      gs->resource_pool.water++;
    }
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
  if (wp_has_something_stored(&wl->work_provider))
    g_object(g, g_animation_buffer(g), Img_menubar, MI_Logistics, v_add(p, l_to_vec(0, 1)));

  wp_draw_click_fields(&wl->work_provider, g, v_add(p, l_to_vec(1, 1)), false);
}

static SceneObjectTable Well_table = {
    .dead = (SceneObjectDeadCB)wl_dead,
    .render_order = (SceneObjectRenderOrderCB)wl_render_order,
    .update = (SceneObjectUpdateCB)wl_update,
    .draw = (SceneObjectDrawCB)wl_draw,
};

bool wl_done_entainment(void *context, Wearisome *w, GameScene *gs) {
  (void)gs;
  (void)w;
  Well *wl = (Well *)context;
  wp_done(&wl->work_provider);
  w_earn_clicks(w, 1);
  w->need_mode = W_Normal;
  return false;
}
bool wl_start_entainment(void *context, Wearisome *w, GameScene *gs) {
  (void)gs;

  Well *wl = (Well *)context;
  wp_start(&wl->work_provider);
  w->need_mode = W_IsWorking;
  return w_queue_wait_for(w, 5.0f, (QueueItem){wl, wl_done_entainment});
}
void wl_claim(Well *wl, GameScene *gs, Wearisome *w, Resource r) {
  assert(r == R_Work);
  if (w_queue_move_to(w, gs, wl->display.location, (QueueItem){wl, wl_start_entainment}))
    wp_claim(&wl->work_provider);
}

static TileContentTable Well_TileContent_Table = {
    .provides = (ProvidesCB)wp_provides,
    .claim = (ClaimCB)wl_claim,
    .click = (ClickCB)wp_click,
};

Well *Well_init(Game *g, GameScene *gs, Point p) {
  const Sizei s = wl_size();
  Well *wl = g_malloc(g, sizeof(Well));
  *wl = (Well){
      .display = bd_create(g, (Recti){p.x, p.y, s.w, s.h}),
      .last_day_delivered = gs->day,
  };
  assert((void *)wl == (void *)&wl->work_provider);
  wp_init(&wl->work_provider, s.w - 1, s.h - 1);

  l_set_tileR(gs->level, wl->display.location, T_Well);
  l_set_tile_contentR(gs->level, wl->display.location, to_TileContent(wl, &Well_TileContent_Table));

  gs_add_object(gs, (SceneObject){.context = wl, &Well_table});
  return wl;
}

#endif