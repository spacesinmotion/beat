#ifndef COMBINATOR_H
#define COMBINATOR_H

#include "extern/cjsonh/cjsonh.h"
#include "game/BuildingDisplay.h"
#include "game/GameScene.h"
#include "game/Level.h"
#include "game/TileContent.h"
#include "game/Wearisome.h"
#include "game/WorkProvider.h"
#include "game/assets.h"
#include "game/effects/Connection.h"
#include "math/Rect.h"
#include "stdbool.h"

typedef struct Combinator {
  WorkProvider work_provider;
  BuildingDisplay display;

  int last_day_delivered;
  int manager_click_counter;
  Connection *sources[2];
} Combinator;

Color cb_color() { return rgb(115, 96, 91); }
static inline Sizei cb_size() { return (Sizei){2, 3}; }

bool cb_dead(Combinator *cb) {
  (void)cb;
  return false;
}

float cb_render_order(Combinator *cb) { return l_to_y(cb->display.location.y); }

void cb_update(Combinator *cb, GameScene *gs, Game *g, float dt) {
  (void)dt;
  bd_update(&cb->display, g);

  if (gs->day > cb->last_day_delivered) {
    cb->last_day_delivered = gs->day;
    cb->manager_click_counter = 0;
  }
  for (int i = 0; i < 2; ++i)
    if (cb->sources[i] && cb->sources[i]->state == CS_Defining)
      cb->sources[i]->start = l_to_vec(gs->r.x, gs->r.y);
}

void cb_draw(Combinator *cb, GameScene *gs, Game *g) {
  if (ri_contains(cb->display.location, gs->r.x, gs->r.y)) {
    c_printf(g, "----------------------\n");
    c_printf(g, "  Combinator (%d,%d,%d,%d)\n", cb->display.location.x, cb->display.location.y, 4, 3);
    c_printf(g, "----------------------\n");
  }

  Vec2 p = l_to_vecP(ri_bottom_right(cb->display.location));
  bd_draw(&cb->display, g, cb_color(), MI_Combinator);

  wp_draw_click_fields(&cb->work_provider, g, v_add(p, l_to_vec(1, 1)), false);
}

void cb_sources_to_json(CJHArray *a, Combinator *cb) {
  for (int i = 0; i < 2; ++i)
    cjh_a_add_object(a, (CJHWriteObjectCB)co_to_json_ref, cb->sources[i]);
}
void cb_to_json(CJHObject *o, Combinator *cb) {
  cjh_o_add_object(o, "work_provider", (CJHWriteObjectCB)wp_to_json, &cb->work_provider);
  cjh_o_add_object(o, "display", (CJHWriteObjectCB)bd_to_json, &cb->display);
  cjh_o_add_number(o, "last_day_delivered", cb->last_day_delivered);
  cjh_o_add_number(o, "manager_click_counter", cb->manager_click_counter);
  cjh_o_add_array(o, "source", (CJHWriteArrayCB)cb_sources_to_json, cb);
}

static SceneObjectTable Combinator_table = {
    .type = "Combinator",
    .dead = (SceneObjectDeadCB)cb_dead,
    .render_order = (SceneObjectRenderOrderCB)cb_render_order,
    .update = (SceneObjectUpdateCB)cb_update,
    .draw = (SceneObjectDrawCB)cb_draw,
    .save = (SceneObjectSaveCB)cb_to_json,
};

Recti cb_location(const Combinator *cb) { return cb->display.location; }
bool cb_provides(Combinator *cb, GameScene *gs, Resource r) {
  if (r == R_Work)
    return wp_provides(&cb->work_provider, gs, r);
  if (r >= R_ManagerWork1 && r <= R_ManagerWork4)
    return (int)r > cb->manager_click_counter && wp_has_work(&cb->work_provider);
  return r == R_Deliver && wp_has_something_to_deliver(&cb->work_provider);
}

bool cb_done_work(void *context, Wearisome *w, GameScene *gs) {
  (void)gs;

  Combinator *cb = (Combinator *)context;
  wp_done(&cb->work_provider, w);
  return false;
}
bool cb_start_work(void *context, Wearisome *w, GameScene *gs) {
  (void)gs;

  Combinator *cb = (Combinator *)context;
  wp_start(&cb->work_provider, w);
  return w_queue_wait_for(w, 5.0f, QI(cb, cb_done_work));
}
bool cb_collect_storage(void *context, Wearisome *w, GameScene *gs) {
  (void)gs;

  Combinator *cb = (Combinator *)context;
  w_deliver(w, MI_Water, cb_color());
  if (qi_on_done(&w->queue_follow_up, w, gs))
    return wp_deliver_taken(&cb->work_provider);
  return false;
}
void cb_claim(Combinator *cb, GameScene *gs, Wearisome *w, Resource r) {
  if (r == R_Work) {
    if (w_queue_move_to(w, gs, cb->display.location, QI(cb, cb_start_work)))
      wp_claim(&cb->work_provider);
  } else if (r == R_Deliver) {
    if (w_queue_move_to(w, gs, cb->display.location, QI(cb, cb_collect_storage)))
      wp_claim_deliver(&cb->work_provider, gs);
  } else if (r >= R_ManagerWork1 && r <= R_ManagerWork4)
    cb->manager_click_counter = r;
}

void cb_select_source(Combinator *cb, Point p, GameScene *gs) {
  if (!cb->sources[0])
    return;

  if (!cb->sources[1]) {
    cb->sources[0]->state = CS_Running;
    cb->sources[1] = Connection_init(gs, l_to_vecP(p), l_to_vecP(ri_bottom_right(cb->display.location)));
    return;
  }

  cb->sources[1]->state = CS_Running;
  gs->special_click_handler = NULL;
  gs->special_click_handler_data = NULL;
}

void cb_click(Combinator *cb, Point p, GameScene *gs) {
  if (p_eq(p, ri_bottom_right(cb->display.location))) {
    if (cb->sources[0])
      cb->sources[0]->state = CS_Dead;
    if (cb->sources[1])
      cb->sources[1]->state = CS_Dead;
    cb->sources[0] = Connection_init(gs, l_to_vecP(p), l_to_vecP(ri_bottom_right(cb->display.location)));
    cb->sources[1] = NULL;
    gs->special_click_handler = (ClickCBx)cb_select_source;
    gs->special_click_handler_data = cb;
    bd_flash(&cb->display);
  } else
    wp_click(&cb->work_provider, p, gs);
}

static TileContentTable Combinator_TileContent_Table = {
    .location = (LocationCb)cb_location,
    .provides = (ProvidesCB)cb_provides,
    .claim = (ClaimCB)cb_claim,
    .click = (ClickCBx)cb_click,
};

Combinator *Combinator_init(Game *g, GameScene *gs, Point p) {
  const Sizei s = cb_size();
  Combinator *cb = g_malloc(sizeof(Combinator));
  *cb = (Combinator){
      .display = bd_create(g, (Recti){p.x, p.y, s.w, s.h}),
      .last_day_delivered = gs->day,
      .manager_click_counter = 0,
      .sources = {NULL, NULL},
  };
  assert((void *)cb == (void *)&cb->work_provider);
  wp_init(&cb->work_provider, s.w - 1, s.h - 1);

  l_set_tile_contentR(gs->level, cb->display.location, to_TileContent(cb, &Combinator_TileContent_Table));

  gs_add_object(gs, (SceneObject){.context = cb, &Combinator_table});
  return cb;
}

#endif