#ifndef SCIENCEBUILDING_H
#define SCIENCEBUILDING_H

#include "engine/SceneObject.h"
#include "engine/math/Rect.h"
#include "game/BuildingDisplay.h"
#include "game/TileContent.h"
#include "game/Wearisome.h"
#include "game/WorkProvider.h"
#include "game/assets.h"

typedef struct ScienceBuilding {
  WorkProvider work_provider;
  BuildingDisplay display;

  int id;
} ScienceBuilding;

static inline Color scb_color() { return rgb(102, 51, 153); }
static inline Sizei scb_size() { return (Sizei){3, 2}; }

bool scb_dead(ScienceBuilding *scb) {
  (void)scb;
  return false;
}

float scb_render_order(ScienceBuilding *scb) { return l_to_y(scb->display.location.y); }

void scb_update(ScienceBuilding *scb, GameScene *gs, Game *g, float dt) {
  (void)dt;
  bd_update(&scb->display, g);

  if (gs->a_new_day_just_started) {
    if (scb->work_provider.clicks_done > 0) {
      wp_reduce_clicks(&scb->work_provider, scb->work_provider.clicks_done);
      bd_flash(&scb->display);
    }
  }
}

void scb_draw(ScienceBuilding *scb, GameScene *gs, Game *g) {
  if (ri_contains(scb->display.location, gs->r.x, gs->r.y)) {
    c_printf(g, "----------------------\n");
    c_printf(g, "  ScienceBuilding (%d,%d,%d,%d)\n", scb->display.location.x, scb->display.location.y, 4, 3);
    c_printf(g, "----------------------\n");
  }

  Vec2 p = l_to_vecP(ri_bottom_right(scb->display.location));
  bd_draw(&scb->display, g, scb_color(), MI_Science);
  wp_draw_click_fields(&scb->work_provider, g, v_add(p, l_to_vec(1, 1)), false);

  scb->work_provider.local_storage = gs->research_level;
  g_color(g, rgb(102, 64, 101));
  wp_draw_storage(&scb->work_provider, g, v_add(p, l_to_vec(0, 1)));
}

static SceneObjectTable ScienceBuilding_table = {
    .type = "ScienceBuilding",
    .dead = (SceneObjectDeadCB)scb_dead,
    .render_order = (SceneObjectRenderOrderCB)scb_render_order,
    .update = (SceneObjectUpdateCB)scb_update,
    .draw = (SceneObjectDrawCB)scb_draw,
};

Recti scb_location(const ScienceBuilding *scb) { return scb->display.location; }

bool scb_provides(ScienceBuilding *scb, GameScene *gs, Resource r) {
  return r == R_Work && gs->reasearch_needed > gs->research_level && wp_provides(&scb->work_provider, gs, r);
}

bool scb_done_work(void *context, Wearisome *w, GameScene *gs) {
  (void)gs;

  ScienceBuilding *scb = (ScienceBuilding *)context;
  wp_done(&scb->work_provider, w);

  gs->research_level++;
  if (gs->research_level >= gs->reasearch_needed)
    gs->research_level -= gs->reasearch_needed;

  return false;
}

bool scb_start_work(void *context, Wearisome *w, GameScene *gs) {
  (void)gs;

  ScienceBuilding *scb = (ScienceBuilding *)context;
  wp_start(&scb->work_provider, w);
  return w_queue_wait_for(w, 5.0f, QI(scb, scb_done_work));
}

void scb_claim(ScienceBuilding *scb, GameScene *gs, Wearisome *w, Resource r) {
  if (r == R_Work) {
    if (w_queue_move_to(w, gs, scb->display.location, QI(scb, scb_start_work)))
      wp_claim(&scb->work_provider);
  }
}

static TileContentTable ScienceBuilding_TileContent_Table = {
    .location = (LocationCb)scb_location,
    .provides = (ProvidesCB)scb_provides,
    .claim = (ClaimCB)scb_claim,
    .click = (ClickCBx)wp_click,
};

ScienceBuilding *ScienceBuilding_init(Game *g, GameScene *gs, Point p) {
  const Sizei s = scb_size();
  ScienceBuilding *scb = g_malloc(sizeof(ScienceBuilding));
  *scb = (ScienceBuilding){
      .display = bd_create(g, (Recti){p.x, p.y, s.w, s.h}),
      .id = unique_id(scb),
  };
  assert((void *)scb == (void *)&scb->work_provider);
  wp_init(&scb->work_provider, s.w - 1, s.h - 1);

  l_set_tile_contentR(gs->level, scb->display.location, to_TileContent(scb, &ScienceBuilding_TileContent_Table));

  gs_add_object(gs, (SceneObject){.context = scb, &ScienceBuilding_table});
  return scb;
}

#endif