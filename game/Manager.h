#ifndef MANAGER_H
#define MANAGER_H

#include "game/BuildingDisplay.h"
#include "game/House.h"
#include "game/Level.h"
#include "game/TileContent.h"
#include "game/Wearisome.h"
#include "game/WorkProvider.h"
#include "game/assets.h"
#include "math/Color.h"
#include "math/Rect.h"
#include "math/Vec2.h"

typedef struct Manager {
  WorkProvider work_provider;

  int last_day_delivered;
  int click_used;
  Resource used_manager_counter;

  BuildingDisplay display;

  G_Object click_used_text;
  int click_used_cache;
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
      wp_reduce_clicks(&mg->work_provider, mg->work_provider.clicks_done);
      bd_flash(&mg->display);
    }
  }

  if (mg->click_used != mg->click_used_cache) {
    g_create_text(g, &mg->click_used_text, Oswald_Regular_12, str("%4.d", mg->click_used));
    mg->click_used_cache = mg->click_used;
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

  g_color(g, white());
  g_text(g, mg->click_used_text, Oswald_Regular_12, v_add(p, (Vec2){12, -5}));
}

static SceneObjectTable Manager_table = {
    .dead = (SceneObjectDeadCB)mg_dead,
    .render_order = (SceneObjectRenderOrderCB)mg_render_order,
    .update = (SceneObjectUpdateCB)mg_update,
    .draw = (SceneObjectDrawCB)mg_draw,
};

Recti mg_location(const Manager *mg) { return mg->display.location; }

bool mg_provides(Manager *mg, GameScene *gs, Resource r) {
  if (r == R_Work)
    return wp_provides(&mg->work_provider, gs, r);
  return r == R_Deliver && wp_has_something_to_deliver(&mg->work_provider);
}

bool mg_find_manager_work(void *context, Wearisome *w, GameScene *gs);
bool mg_click_building(void *context, Wearisome *w, GameScene *gs) {
  Manager *mg = (Manager *)context;

  TileContent *tc = l_contentP(gs->level, l_to_point(w->destination));
  if (tc) {
    tc_claim(tc, gs, w, mg->used_manager_counter);
    tc_click(tc, (Point){-1, -1}, gs);
    gs->clicks++; // clicks already taken
    mg->click_used--;
  }

  return w_leave_building(w, gs, (QueueItem){mg, mg_find_manager_work});
}

bool mg_find_manager_work(void *context, Wearisome *w, GameScene *gs) {
  Manager *mg = (Manager *)context;
  if (mg->click_used == 0) {
    wp_done(&mg->work_provider, w);
    return false;
  }

  mg->used_manager_counter = R_ManagerWork1;
  Recti r = find_resource_building_rect(gs, mg->display.location, mg->used_manager_counter);
  if (r.w <= 0) {
    mg->used_manager_counter = R_ManagerWork2;
    r = find_resource_building_rect(gs, mg->display.location, mg->used_manager_counter);
  }
  if (r.w <= 0) {
    mg->used_manager_counter = R_ManagerWork3;
    r = find_resource_building_rect(gs, mg->display.location, mg->used_manager_counter);
  }
  if (r.w <= 0) {
    mg->used_manager_counter = R_ManagerWork4;
    r = find_resource_building_rect(gs, mg->display.location, mg->used_manager_counter);
  }

  if (r.w > 0 && w_queue_move_to(w, gs, r, (QueueItem){mg, mg_click_building}))
    return true;
  return w_queue_wait_for(w, 0.25f, (QueueItem){mg, mg_find_manager_work});
}

bool mg_start_work(void *context, Wearisome *w, GameScene *gs) {
  Manager *mg = (Manager *)context;
  wp_start(&mg->work_provider, w);
  w->need_mode = W_Normal;
  mg->click_used = i_min(8, gs->clicks);
  gs->clicks -= mg->click_used;
  return mg_find_manager_work(mg, w, gs);
}

void mg_claim(Manager *mg, GameScene *gs, Wearisome *w, Resource r) {
  if (r == R_Work) {
    if (w_queue_move_to(w, gs, mg->display.location, (QueueItem){mg, mg_start_work}))
      wp_claim(&mg->work_provider);
  }
}

static TileContentTable Manager_TileContent_Table = {
    .location = (LocationCb)mg_location,
    .provides = (ProvidesCB)mg_provides,
    .claim = (ClaimCB)mg_claim,
    .click = (ClickCBx)wp_click,
};

Manager *Manager_init(Game *g, GameScene *gs, Point p) {
  const Sizei s = mg_size();
  Manager *mg = g_malloc(sizeof(Manager));
  *mg = (Manager){
      .display = bd_create(g, (Recti){p.x, p.y, s.w, s.h}),
      .last_day_delivered = gs->day,
      .click_used = 0,
      .used_manager_counter = R_None,
      .click_used_cache = -1,
  };
  assert((void *)mg == (void *)&mg->work_provider);
  wp_init(&mg->work_provider, s.w - 1, s.h - 1);

  l_set_tile_contentR(gs->level, mg->display.location, to_TileContent(mg, &Manager_TileContent_Table));

  gs_add_object(gs, (SceneObject){.context = mg, &Manager_table});
  return mg;
}

#endif