#ifndef CONSTRUCTIONMATERIALFACTORY_H
#define CONSTRUCTIONMATERIALFACTORY_H

#include "game/BuildingDisplay.h"
#include "game/TileContent.h"
#include "game/Wearisome.h"
#include "game/WorkProvider.h"

typedef struct ConstructionMaterialFactory {
  WorkProvider work_provider;

  int last_day_delivered;
  int manager_click_counter;

  BuildingDisplay display;
} ConstructionMaterialFactory;

static inline Color cmf_color() { return rgb(85, 84, 80); }
static inline Sizei cmf_size() { return (Sizei){3, 3}; }

bool cmf_dead(ConstructionMaterialFactory *cmf) {
  (void)cmf;
  return false;
}

float cmf_render_order(ConstructionMaterialFactory *cmf) { return l_to_y(cmf->display.location.y); }

void cmf_update(ConstructionMaterialFactory *cmf, GameScene *gs, Game *g, float dt) {
  (void)dt;
  bd_update(&cmf->display, g);

  if (gs->day > cmf->last_day_delivered) {
    cmf->last_day_delivered = gs->day;
    cmf->manager_click_counter = 0;
    // if (wp_finish_production_cycle(&cmf->work_provider, 8))
    //   bd_flash(&cmf->display);
  }
}

void cmf_draw(ConstructionMaterialFactory *cmf, GameScene *gs, Game *g) {
  if (ri_contains(cmf->display.location, gs->r.x, gs->r.y)) {
    c_printf(g, "----------------------\n");
    c_printf(g, "  ConstructionMaterialFactory (%d,%d,%d,%d)\n", cmf->display.location.x, cmf->display.location.y, 4,
             3);
    c_printf(g, "----------------------\n");
  }

  bd_draw(&cmf->display, g, cmf_color(), MI_ConstructionMaterial);
  Vec2 p = l_to_vecP(ri_bottom_right(cmf->display.location));
  g_object(g, g_animation_buffer(g), Img_menubar, MI_Logistics, v_add(p, l_to_vec(0, 1)));

  wp_draw_click_fields(&cmf->work_provider, g, v_add(p, l_to_vec(1, 1)), false);
}

static SceneObjectTable ConstructionMaterialFactory_table = {
    .dead = (SceneObjectDeadCB)cmf_dead,
    .render_order = (SceneObjectRenderOrderCB)cmf_render_order,
    .update = (SceneObjectUpdateCB)cmf_update,
    .draw = (SceneObjectDrawCB)cmf_draw,
};

Recti cmf_location(const ConstructionMaterialFactory *cmf) { return cmf->display.location; }

bool cmf_provides(ConstructionMaterialFactory *cmf, GameScene *gs, Resource r) {
  if (r == R_Work)
    return wp_provides(&cmf->work_provider, gs, r);
  if (r >= R_ManagerWork1 && r <= R_ManagerWork4)
    return (int)r > cmf->manager_click_counter && wp_has_work(&cmf->work_provider);
  return r == R_Deliver && wp_has_something_to_deliver(&cmf->work_provider);
}

bool cmf_done_work(void *context, Wearisome *w, GameScene *gs) {
  (void)gs;

  ConstructionMaterialFactory *cmf = (ConstructionMaterialFactory *)context;
  wp_done(&cmf->work_provider, w);
  return false;
}
bool cmf_start_work(void *context, Wearisome *w, GameScene *gs) {
  (void)gs;

  ConstructionMaterialFactory *cmf = (ConstructionMaterialFactory *)context;
  wp_start(&cmf->work_provider, w);
  return w_queue_wait_for(w, 10.0f, (QueueItem){cmf, cmf_done_work});
}

bool cmf_collect_storage(void *context, Wearisome *w, GameScene *gs) {
  (void)gs;

  ConstructionMaterialFactory *cmf = (ConstructionMaterialFactory *)context;
  w_deliver(w, MI_ConstructionMaterial, cmf_color());
  if (qi_on_done(&w->queue_follow_up, w, gs))
    return wp_deliver_taken(&cmf->work_provider);
  return false;
}

void cmf_claim(ConstructionMaterialFactory *cmf, GameScene *gs, Wearisome *w, Resource r) {
  if (r == R_Work) {
    if (w_queue_move_to(w, gs, cmf->display.location, (QueueItem){cmf, cmf_start_work}))
      wp_claim(&cmf->work_provider);
  } else if (r == R_Deliver) {
    if (w_queue_move_to(w, gs, cmf->display.location, (QueueItem){cmf, cmf_collect_storage}))
      wp_claim_deliver(&cmf->work_provider, gs);
  } else if (r >= R_ManagerWork1 && r <= R_ManagerWork4)
    cmf->manager_click_counter = r;
}
static TileContentTable ConstructionMaterialFactory_TileContent_Table = {
    .location = (LocationCb)cmf_location,
    .provides = (ProvidesCB)cmf_provides,
    .claim = (ClaimCB)cmf_claim,
    .click = (ClickCB)wp_click,
};

ConstructionMaterialFactory *ConstructionMaterialFactory_init(Game *g, GameScene *gs, Point p) {
  const Sizei s = cmf_size();
  ConstructionMaterialFactory *cmf = g_malloc(g, sizeof(ConstructionMaterialFactory));
  *cmf = (ConstructionMaterialFactory){
      .display = bd_create(g, (Recti){p.x, p.y, s.w, s.h}),
      .last_day_delivered = gs->day,
      .manager_click_counter = 0,
  };
  assert((void *)cmf == (void *)&cmf->work_provider);
  wp_init(&cmf->work_provider, s.w - 1, s.h - 1);

  l_set_tile_contentR(gs->level, cmf->display.location,
                      to_TileContent(cmf, &ConstructionMaterialFactory_TileContent_Table));

  gs_add_object(gs, (SceneObject){.context = cmf, &ConstructionMaterialFactory_table});
  return cmf;
}

#endif