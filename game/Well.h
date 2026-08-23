#ifndef WELL_H
#define WELL_H

#include "game/BuildingDisplay.h"
#include "game/Game.h"
#include "game/Level.h"
#include "game/SceneObject.h"
#include "game/TileContent.h"
#include "game/Wearisome.h"
#include "game/WorkProvider.h"
#include "game/assets.h"
#include "math/Rect.h"

typedef struct Well {
  WorkProvider work_provider;
  BuildingDisplay display;

  int id;

  int manager_click_counter;
} Well;

Color wl_color() { return rgb(0, 80, 133); }
static inline Sizei wl_size() { return (Sizei){2, 3}; }

bool wl_dead(Well *wl) {
  (void)wl;
  return false;
}

float wl_render_order(Well *wl) { return l_to_y(wl->display.location.y); }

void wl_update(Well *wl, GameScene *gs, Game *g, float dt) {
  (void)dt;
  bd_update(&wl->display, g);

  gs->resource_pool_spread.water += wl->work_provider.local_storage;

  if (gs->a_new_day_just_started) {
    wl->manager_click_counter = 0;
    wp_clear_done_work(&wl->work_provider);
  }
}

void wl_draw(Well *wl, GameScene *gs, Game *g) {
  if (ri_contains(wl->display.location, gs->r.x, gs->r.y)) {
    c_printf(g, "----------------------\n");
    c_printf(g, "  Well (%d,%d,%d,%d)\n", wl->display.location.x, wl->display.location.y, 4, 3);
    c_printf(g, "----------------------\n");
    c_printf(g, " %10s: %d\n", "storage", wl->work_provider.local_storage);
    c_printf(g, " %10s: %d\n", "taken", wp_storage_taken(&wl->work_provider));
    c_printf(g, "----------------------\n");
  }

  Vec2 p = l_to_vecP(ri_bottom_right(wl->display.location));
  bd_draw(&wl->display, g, wl_color(), MI_Water);

  wp_draw_click_fields(&wl->work_provider, g, v_add(p, l_to_vec(1, 1)), false);
  wp_draw_storage(&wl->work_provider, g, v_add(p, l_to_vec(0, 1)));
}

void wl_to_json(CJHObject *o, Well *wl) {
  cjh_o_add_object(o, "work_provider", (CJHWriteObjectCB)wp_to_json, &wl->work_provider);
  cjh_o_add_object(o, "display", (CJHWriteObjectCB)bd_to_json, &wl->display);
  cjh_o_add_number(o, "manager_click_counter", wl->manager_click_counter);
}

void wl_from_json(CJHObjectR *o, const char *key, Well *wl) {

  if (streq(key, "last_day_delivered"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));
  else if (streq(key, "id"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));
  else if (streq(key, "manager_click_counter"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));

  else if (streq(key, "work_provider")) {
    printf("%.*s%s:\n", indent, space, key);
    indent += 2;
    cjh_o_read_object(o, (CJHReadObjectCB)wp_from_json, &wl->work_provider);
    indent -= 2;
  } else if (streq(key, "display")) {
    printf("%.*s%s:\n", indent, space, key);
    indent += 2;
    cjh_o_read_object(o, (CJHReadObjectCB)bd_from_json, &wl->display);
    indent -= 2;
  }

  else {
    printf("%.*s%s: SKIP\n", indent, space, key);
    cjh_o_skip(o);
  }
}

static SceneObjectTable Well_table = {
    .type = "Well",
    .dead = (SceneObjectDeadCB)wl_dead,
    .render_order = (SceneObjectRenderOrderCB)wl_render_order,
    .update = (SceneObjectUpdateCB)wl_update,
    .draw = (SceneObjectDrawCB)wl_draw,
    .save = (SceneObjectSaveCB)wl_to_json,
};

Recti wl_location(const Well *wl) { return wl->display.location; }
bool wl_provides(Well *wl, GameScene *gs, Resource r) {
  if (r == R_Water)
    return wp_has_something_to_deliver(&wl->work_provider);
  if (r == R_Work)
    return wp_provides(&wl->work_provider, gs, r);
  if (r >= R_ManagerWork1 && r <= R_ManagerWork4)
    return (int)r > wl->manager_click_counter && wp_has_work(&wl->work_provider);
  return r == R_Deliver && wp_has_something_to_deliver(&wl->work_provider);
}

bool wl_done_work(void *context, Wearisome *w, GameScene *gs) {
  (void)gs;

  Well *wl = (Well *)context;
  wp_done_and_store(&wl->work_provider, w);
  return false;
}
bool wl_start_work(void *context, Wearisome *w, GameScene *gs) {
  (void)gs;

  Well *wl = (Well *)context;
  wp_start(&wl->work_provider, w);
  return w_queue_wait_for(w, 5.0f, QI(wl, wl_done_work));
}
bool wl_collect_storage(void *context, Wearisome *w, GameScene *gs) {
  (void)gs;

  Well *wl = (Well *)context;
  w_deliver(w, MI_Water, wl_color());
  if (qi_on_done(&w->queue_follow_up, w, gs))
    return wp_deliver_taken(&wl->work_provider);
  return false;
}
void wl_claim(Well *wl, GameScene *gs, Wearisome *w, Resource r) {
  if (r == R_Water) {
    wl->work_provider.local_storage_claimed++;
  } else if (r == R_Work) {
    if (w_queue_move_to(w, gs, wl->display.location, QI(wl, wl_start_work)))
      wp_claim(&wl->work_provider);
  } else if (r == R_Deliver) {
    if (w_queue_move_to(w, gs, wl->display.location, QI(wl, wl_collect_storage)))
      wp_claim_deliver(&wl->work_provider, gs);
  } else if (r >= R_ManagerWork1 && r <= R_ManagerWork4)
    wl->manager_click_counter = r;
}

void wl_take(Well *wl, GameScene *gs, Resource r) {
  assert(r == R_Water);

  if (r == R_Water) {
    wl->work_provider.local_storage--;
    wl->work_provider.local_storage_claimed--;
    gs->clicks++;

    bd_flash(&wl->display);
    CoinAnimation_init(gs, bd_gain_something_location(&wl->display));
  }
}

static TileContentTable Well_TileContent_Table = {
    .location = (LocationCb)wl_location,
    .provides = (ProvidesCB)wl_provides,
    .claim = (ClaimCB)wl_claim,
    .take = (TakeCB)wl_take,
    .click = (ClickCBx)wp_click_with_storage,
};

Well *Well_init(Game *g, GameScene *gs, Point p) {
  const Sizei s = wl_size();
  Well *wl = g_malloc(sizeof(Well));
  *wl = (Well){
      .display = bd_create(g, (Recti){p.x, p.y, s.w, s.h}),
      .id = unique_id(wl),
      .manager_click_counter = 0,
  };
  assert((void *)wl == (void *)&wl->work_provider);
  wp_init(&wl->work_provider, s.w - 1, s.h - 1);

  l_set_tile_contentR(gs->level, wl->display.location, to_TileContent(wl, &Well_TileContent_Table));

  gs_add_object(gs, (SceneObject){.context = wl, &Well_table});
  return wl;
}

#endif