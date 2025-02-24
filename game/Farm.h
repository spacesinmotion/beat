#ifndef FARM_H
#define FARM_H

#include "game/BuildingDisplay.h"
#include "game/TileContent.h"
#include "game/Wearisome.h"
#include "game/WorkProvider.h"

typedef struct Farm {
  WorkProvider work_provider;

  int last_day_delivered;
  int manager_click_counter;

  BuildingDisplay display;
} Farm;

Color fa_color() { return rgb(11, 133, 0); }
static inline Sizei fa_size() { return (Sizei){3, 4}; }

bool fa_dead(Farm *fa) {
  (void)fa;
  return false;
}

float fa_render_order(Farm *fa) { return l_to_y(fa->display.location.y); }

void fa_update(Farm *fa, GameScene *gs, Game *g, float dt) {
  (void)dt;
  bd_update(&fa->display, g);

  if (gs->day > fa->last_day_delivered) {
    fa->last_day_delivered = gs->day;
    fa->manager_click_counter = 0;
    if (wp_finish_production_cycle(&fa->work_provider, 8))
      bd_flash(&fa->display);
  }
}

void fa_draw(Farm *fa, GameScene *gs, Game *g) {
  if (ri_contains(fa->display.location, gs->r.x, gs->r.y)) {
    c_printf(g, "----------------------\n");
    c_printf(g, "  Farm (%d,%d,%d,%d)\n", fa->display.location.x, fa->display.location.y, 4, 3);
    c_printf(g, "----------------------\n");
  }

  bd_draw(&fa->display, g, fa_color(), MI_Food);
  Vec2 p = l_to_vecP(ri_bottom_right(fa->display.location));
  g_object(g, g_animation_buffer(g), Img_menubar, MI_Logistics, v_add(p, l_to_vec(0, 1)));

  wp_draw_click_fields(&fa->work_provider, g, v_add(p, l_to_vec(1, 1)), false);
}

static SceneObjectTable Farm_table = {
    .dead = (SceneObjectDeadCB)fa_dead,
    .render_order = (SceneObjectRenderOrderCB)fa_render_order,
    .update = (SceneObjectUpdateCB)fa_update,
    .draw = (SceneObjectDrawCB)fa_draw,
};

Recti fa_location(const Farm *fa) { return fa->display.location; }

bool fa_provides(Farm *fa, GameScene *gs, Resource r) {
  if (r == R_Work)
    return wp_provides(&fa->work_provider, gs, r);
  if (r >= R_ManagerWork1 && r <= R_ManagerWork4)
    return (int)r > fa->manager_click_counter && wp_has_work(&fa->work_provider);
  return r == R_Deliver && wp_has_something_to_deliver(&fa->work_provider);
}

bool fa_done_work(void *context, Wearisome *w, GameScene *gs) {
  (void)gs;
  (void)w;
  Farm *fa = (Farm *)context;
  wp_done(&fa->work_provider, w);
  return false;
}
bool fa_start_work(void *context, Wearisome *w, GameScene *gs) {
  (void)gs;

  Farm *fa = (Farm *)context;
  wp_start(&fa->work_provider, w);
  return w_queue_wait_for(w, 6.0f, (QueueItem){fa, fa_done_work});
}

bool fa_collect_storage(void *context, Wearisome *w, GameScene *gs) {
  (void)gs;

  Farm *fa = (Farm *)context;
  w_deliver(w, MI_Food, fa_color());
  if (qi_on_done(&w->queue_follow_up, w, gs))
    return wp_storage_taken(&fa->work_provider, w);
  return false;
}

void fa_claim(Farm *fa, GameScene *gs, Wearisome *w, Resource r) {
  if (r == R_Work) {
    if (w_queue_move_to(w, gs, fa->display.location, (QueueItem){fa, fa_start_work}))
      wp_claim(&fa->work_provider);
  } else if (r == R_Deliver) {
    if (w_queue_move_to(w, gs, fa->display.location, (QueueItem){fa, fa_collect_storage}))
      wp_claim_storage(&fa->work_provider, gs, w);
  } else if (r >= R_ManagerWork1 && r <= R_ManagerWork4)
    fa->manager_click_counter = r;
}

static TileContentTable Farm_TileContent_Table = {
    .location = (LocationCb)fa_location,
    .provides = (ProvidesCB)fa_provides,
    .claim = (ClaimCB)fa_claim,
    .click = (ClickCB)wp_click,
};

Farm *Farm_init(Game *g, GameScene *gs, Point p) {
  const Sizei s = fa_size();
  Farm *fa = g_malloc(g, sizeof(Farm));
  *fa = (Farm){
      .display = bd_create(g, (Recti){p.x, p.y, s.w, s.h}),
      .last_day_delivered = gs->day,
      .manager_click_counter = 0,
  };
  assert((void *)fa == (void *)&fa->work_provider);
  wp_init(&fa->work_provider, s.w - 1, s.h - 1);

  l_set_tile_contentR(gs->level, fa->display.location, to_TileContent(fa, &Farm_TileContent_Table));

  gs_add_object(gs, (SceneObject){.context = fa, &Farm_table});
  return fa;
}

#endif