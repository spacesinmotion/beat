#ifndef CONSTRUCTIONMATERIALFACTORY_H
#define CONSTRUCTIONMATERIALFACTORY_H

#include "game/BuildingDisplay.h"
#include "game/Wearisome.h"
#include "game/WorkProvider.h"

typedef struct ConstructionMaterialFactory {
  WorkProvider work_provider;

  int last_day_delivered;

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
    if (wp_finish_production_cycle(&cmf->work_provider, 8))
      bd_flash(&cmf->display);
  }
  if (wp_has_something_stored(&cmf->work_provider) && gs->daytime > 0.25 && gs->daytime < 0.26) {
    const int free_storage = gs_free_storage(gs);
    for (int i = 0; i < free_storage && cmf->work_provider.storage > 0; ++i) {
      cmf->work_provider.storage--;
      gs->resource_pool.construction_material++;
    }
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
  if (wp_has_something_stored(&cmf->work_provider))
    g_object(g, g_animation_buffer(g), Img_menubar, MI_Logistics, v_add(p, l_to_vec(0, 1)));

  wp_draw_click_fields(&cmf->work_provider, g, v_add(p, l_to_vec(1, 1)), false);
}

static SceneObjectTable ConstructionMaterialFactory_table = {
    .dead = (SceneObjectDeadCB)cmf_dead,
    .render_order = (SceneObjectRenderOrderCB)cmf_render_order,
    .update = (SceneObjectUpdateCB)cmf_update,
    .draw = (SceneObjectDrawCB)cmf_draw,
};

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
void cmf_claim(ConstructionMaterialFactory *wl, GameScene *gs, Wearisome *w, Resource r) {
  assert(r == R_Work);
  if (w_queue_move_to(w, gs, wl->display.location, (QueueItem){wl, cmf_start_work}))
    wp_claim(&wl->work_provider);
}
static TileContentTable ConstructionMaterialFactory_TileContent_Table = {
    .provides = (ProvidesCB)wp_provides,
    .claim = (ClaimCB)cmf_claim,
    .click = (ClickCB)wp_click,
};

ConstructionMaterialFactory *ConstructionMaterialFactory_init(Game *g, GameScene *gs, Point p) {
  const Sizei s = cmf_size();
  ConstructionMaterialFactory *cmf = g_malloc(g, sizeof(ConstructionMaterialFactory));
  *cmf = (ConstructionMaterialFactory){
      .display = bd_create(g, (Recti){p.x, p.y, s.w, s.h}),
      .last_day_delivered = gs->day,
  };
  assert((void *)cmf == (void *)&cmf->work_provider);
  wp_init(&cmf->work_provider, s.w - 1, s.h - 1);

  l_set_tileR(gs->level, cmf->display.location, T_ConstructionMaterialFactory);
  l_set_tile_contentR(gs->level, cmf->display.location,
                      to_TileContent(cmf, &ConstructionMaterialFactory_TileContent_Table));

  gs_add_object(gs, (SceneObject){.context = cmf, &ConstructionMaterialFactory_table});
  return cmf;
}

#endif