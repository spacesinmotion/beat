#ifndef MANAGER_H
#define MANAGER_H

#include "game/BuildingDisplay.h"
#include "game/House.h"
#include "game/Level.h"
#include "game/TileContent.h"
#include "game/Wearisome.h"
#include "game/WorkProvider.h"
#include "game/assets.h"
#include "math/Rect.h"

typedef struct Manager {
  WorkProvider work_provider;

  int last_day_delivered;
  int click_used;

  BuildingDisplay display;
} Manager;

static inline Color mg_color() { return rgb(116, 31, 38); }
static inline Sizei mg_size() { return (Sizei){2, 2}; }

bool mg_dead(Manager *mg) {
  (void)mg;
  return false;
}

float mg_render_order(Manager *mg) { return l_to_y(mg->display.location.y); }

void mg_update(Manager *mg, GameScene *gs, Game *g, float dt) {
  (void)dt;
  bd_update(&mg->display, g);

  if (gs->day > mg->last_day_delivered) {
    mg->last_day_delivered = gs->day;
    if (mg->work_provider.clicks_done > 0) {
      mg->click_used = 8;
      wp_reduce_clicks(&mg->work_provider, mg->work_provider.clicks_done);
      bd_flash(&mg->display);
    }
  }
}

void mg_draw(Manager *mg, GameScene *gs, Game *g) {
  if (ri_contains(mg->display.location, gs->r.x, gs->r.y)) {
    c_printf(g, "----------------------\n");
    c_printf(g, "  Manager (%d,%d,%d,%d)\n", mg->display.location.x, mg->display.location.y, mg->display.location.w,
             mg->display.location.h);
    c_printf(g, "----------------------\n");
  }

  Vec2 p = l_to_vecP(ri_bottom_right(mg->display.location));
  bd_draw(&mg->display, g, mg_color(), MI_Manager);

  wp_draw_click_fields(&mg->work_provider, g, v_add(p, l_to_vec(1, 1)), false);
}

static SceneObjectTable Manager_table = {
    .dead = (SceneObjectDeadCB)mg_dead,
    .render_order = (SceneObjectRenderOrderCB)mg_render_order,
    .update = (SceneObjectUpdateCB)mg_update,
    .draw = (SceneObjectDrawCB)mg_draw,
};

bool mg_provides(Manager *mg, GameScene *gs, Resource r) {
  if (r == R_Work)
    return wp_provides(&mg->work_provider, gs, r);
  return r == R_Deliver && wp_has_something_to_deliver(&mg->work_provider);
}

bool mg_find_manager_work(void *context, Wearisome *w, GameScene *gs);
bool mg_click_building(void *context, Wearisome *w, GameScene *gs) {
  Manager *mg = (Manager *)context;

  TileContent *tc = l_contentP(gs->level, l_to_point(w->destination));
  if (tc)
    tc_click(tc, gs);

  return w_leave_building(w, gs, (QueueItem){mg, mg_find_manager_work});
}

bool mg_find_manager_work(void *context, Wearisome *w, GameScene *gs) {
  Manager *mg = (Manager *)context;
  if (mg->click_used == 0) {
    wp_done(&mg->work_provider, w);
    return false;
  }

  mg->click_used--;
  Recti r = find_resource_building_rect(gs, mg->display.location, R_ManagerWork);

  if (r.w > 0 && w_queue_move_to(w, gs, r, (QueueItem){mg, mg_click_building}))
    return true;
  return w_queue_wait_for(w, 0.25f, (QueueItem){mg, mg_find_manager_work});
}

bool mg_start_work(void *context, Wearisome *w, GameScene *gs) {
  Manager *mg = (Manager *)context;
  wp_start(&mg->work_provider, w);
  w->need_mode = W_Normal;
  return mg_find_manager_work(mg, w, gs);
}

void mg_claim(Manager *mg, GameScene *gs, Wearisome *w, Resource r) {
  if (r == R_Work) {
    if (w_queue_move_to(w, gs, mg->display.location, (QueueItem){mg, mg_start_work}))
      wp_claim(&mg->work_provider);
  }
}

static TileContentTable Manager_TileContent_Table = {
    .provides = (ProvidesCB)mg_provides,
    .claim = (ClaimCB)mg_claim,
    .click = (ClickCB)wp_click,
};

Manager *Manager_init(Game *g, GameScene *gs, Point p) {
  const Sizei s = mg_size();
  Manager *mg = g_malloc(g, sizeof(Manager));
  *mg = (Manager){
      .display = bd_create(g, (Recti){p.x, p.y, s.w, s.h}),
      .last_day_delivered = gs->day,
      .click_used = 8,
  };
  assert((void *)mg == (void *)&mg->work_provider);
  wp_init(&mg->work_provider, s.w - 1, s.h - 1);

  l_set_tileR(gs->level, mg->display.location, T_Manager);
  l_set_tile_contentR(gs->level, mg->display.location, to_TileContent(mg, &Manager_TileContent_Table));

  gs_add_object(gs, (SceneObject){.context = mg, &Manager_table});
  return mg;
}

#endif